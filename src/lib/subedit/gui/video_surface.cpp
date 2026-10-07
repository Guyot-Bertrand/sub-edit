#include <subedit/gui/frame_source.hpp>
#include <subedit/gui/video_surface.hpp>

#include <QColor>
#include <QFont>
#include <QImage>
#include <QMetaObject>
#include <QPaintEvent>
#include <QPainter>
#include <QRect>
#include <QResizeEvent>
#include <QSize>
#include <QString>
#include <QWidget>
#include <Qt>

#include <cmath>
#include <cstddef>
#include <span>
#include <utility>

namespace subedit::gui {

namespace {

/// The timecode: its distance from the corner, its padding and the opacity of its backing.
constexpr int kTimecodeMargin = 8;
constexpr int kTimecodePadding = 6;
constexpr int kTimecodeBackingAlpha = 150;

/// How many bytes a pixel takes, and which of them libmpv leaves unused (`bgr0`: the fourth).
constexpr std::size_t kBytesAPixel = 4;
constexpr std::size_t kUnusedByte = 3;
constexpr unsigned char kOpaque = 0xff;

/// Sets the byte libmpv leaves unused to 0xff, which is what `Format_RGB32` is promised to hold.
///
/// **Qt does not mask it, it assumes it** — issue #632. `libmpv` writes `bgr0`, and the zero it
/// leaves in the fourth byte of every pixel is read as an alpha by the paths of Qt that copy a
/// `Format_RGB32` image as it stands. On X11 the byte is dropped on the way to the screen and
/// nothing shows; on Wayland the window's buffer keeps it, and the compositor draws what the
/// buffer says: a picture full of holes through which the desktop shows, and a subtitle whose
/// edges are as translucent as its antialiasing.
void makeOpaque(std::span<unsigned char> pixels) {
    for (std::size_t at = kUnusedByte; at < pixels.size(); at += kBytesAPixel)
        pixels[at] = kOpaque;
}

} // namespace

VideoSurface::VideoSurface(QWidget* parent) : QWidget{parent} {
    // Painted entirely by `paintEvent`: no background to clear first, which would show
    // between two frames as a flicker.
    setAttribute(Qt::WA_OpaquePaintEvent);
}

VideoSurface::~VideoSurface() {
    // The source calls back from its own thread; once this returns it no longer does.
    attach(nullptr);
}

void VideoSurface::attach(FrameSource* source) {
    if (m_source != nullptr)
        m_source->onFrameReady({});

    m_source = source;
    if (m_source == nullptr) {
        update();
        return;
    }

    // Queued, which is what turns a call from another thread into one of this thread —
    // and a no-op if the widget is gone by the time it runs.
    //
    // NOLINT: the analyzer follows `invokeMethod` into Qt's header, sees the functor slot
    // object allocated, and does not see the event loop that owns and frees it once the
    // queued call has run.
    m_source->onFrameReady([this] {
        QMetaObject::invokeMethod( // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
            this,
            [this] { refresh(); },
            Qt::QueuedConnection);
    });
    refresh();
}

void VideoSurface::refresh() {
    // The buffer is in physical pixels, so that a scaled screen gets a sharp picture and
    // not a small one stretched.
    const qreal scale = devicePixelRatioF();
    const QSize wanted{static_cast<int>(std::lround(width() * scale)),
                       static_cast<int>(std::lround(height() * scale))};
    if (wanted.isEmpty())
        return;

    if (m_image.size() != wanted) {
        m_image = QImage{wanted, QImage::Format_RGB32};
        m_image.setDevicePixelRatio(scale);
        m_image.fill(Qt::black);
    }

    if (m_source != nullptr) {
        // `Format_RGB32` is blue, green, red and one unused byte in memory — what
        // libmpv's `bgr0` is.
        const std::span<unsigned char> pixels{m_image.bits(),
                                              static_cast<std::size_t>(m_image.sizeInBytes())};
        if (m_source->render(pixels,
                             m_image.width(),
                             m_image.height(),
                             static_cast<std::size_t>(m_image.bytesPerLine())))
            makeOpaque(pixels);
    }
    update();
}

void VideoSurface::setTimecode(const QString& text) {
    if (text == m_timecode)
        return;

    m_timecode = text;
    update();
}

void VideoSurface::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter{this};
    painter.fillRect(rect(), Qt::black);
    if (!m_image.isNull())
        painter.drawImage(0, 0, m_image);

    if (m_timecode.isEmpty())
        return;

    // The window's font, a little bold, on a backing that is not opaque: it reads over a light
    // frame as well as over a dark one, and it lets what is under it show.
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);

    const QRect text = painter.fontMetrics().boundingRect(m_timecode);
    const QRect backing =
        text.adjusted(
                -kTimecodePadding, -kTimecodePadding / 2, kTimecodePadding, kTimecodePadding / 2)
            .translated(kTimecodeMargin - text.left() + kTimecodePadding,
                        kTimecodeMargin - text.top() + (kTimecodePadding / 2));
    painter.fillRect(backing, QColor{0, 0, 0, kTimecodeBackingAlpha});
    painter.setPen(Qt::white);
    painter.drawText(backing, Qt::AlignCenter, m_timecode);
}

void VideoSurface::resizeEvent(QResizeEvent* /*event*/) {
    refresh();
}

} // namespace subedit::gui
