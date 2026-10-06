#include <subedit/gui/frame_source.hpp>
#include <subedit/gui/video_surface.hpp>

#include <QColor>
#include <QImage>
#include <QMetaObject>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QSize>
#include <QWidget>
#include <Qt>

#include <cmath>
#include <cstddef>
#include <span>
#include <utility>

namespace subedit::gui {

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
        (void)m_source->render(pixels,
                               m_image.width(),
                               m_image.height(),
                               static_cast<std::size_t>(m_image.bytesPerLine()));
    }
    update();
}

void VideoSurface::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter{this};
    painter.fillRect(rect(), Qt::black);
    if (!m_image.isNull())
        painter.drawImage(0, 0, m_image);
}

void VideoSurface::resizeEvent(QResizeEvent* /*event*/) {
    refresh();
}

} // namespace subedit::gui
