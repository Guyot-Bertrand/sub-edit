#pragma once

#include <QImage>
#include <QString>
#include <QWidget>

class QPaintEvent;
class QResizeEvent;

namespace subedit::gui {

class FrameSource;

/// The widget the picture of a film is painted on — ADR 0041, issue #613.
///
/// **The picture is in the window**, which is the point: it is a plain Qt widget with
/// no native window of its own, so Qt can photograph it, a test can read it, and what
/// the window draws over it (the timecode, later) is drawn by Qt. It is the same on X11,
/// on Wayland and with no screen.
///
/// It owns a buffer **the size of the widget in physical pixels**, which libmpv fills
/// through `FrameSource::render` and keeps the film's aspect ratio in: the bars are
/// black, and the widget paints the buffer as it stands. A resize makes a buffer of the
/// new size and draws it again at once — a paused film has no frame coming to do it.
///
/// **One thread.** The source tells, from a thread of its own, that a picture is ready;
/// that is turned into an event of the window's thread, which is where the drawing and
/// the painting happen.
class VideoSurface final : public QWidget {

public:
    explicit VideoSurface(QWidget* parent = nullptr);
    ~VideoSurface() override;

    VideoSurface(const VideoSurface&) = delete;
    VideoSurface& operator=(const VideoSurface&) = delete;
    VideoSurface(VideoSurface&&) = delete;
    VideoSurface& operator=(VideoSurface&&) = delete;

    /// Shows what `source` draws, or nothing when it is null. The source must outlive
    /// this or be detached first — `VideoPane` detaches it before it lets the player go.
    void attach(FrameSource* source);

    /// Draws the source's current picture into the buffer and paints it. Called when the
    /// source announces one, on a resize, and by whoever has just moved playback and does
    /// not want to wait for the announcement.
    void refresh();

    /// Shows `text` over the picture, at the top left, or nothing when it is empty — issue #615.
    ///
    /// **Drawn by Qt, on top of what libmpv drew**, and that is what ADR 0041 made possible: the
    /// picture is in this widget, so the timecode needs nothing from the player but the
    /// position it already gives. It is in the window's own font and carries its own backing,
    /// so it reads over any frame.
    void setTimecode(const QString& text);

    /// The timecode as last given, empty when there is none.
    [[nodiscard]] const QString& timecode() const { return m_timecode; }

    /// The buffer as last drawn — what a test reads to know what the widget shows.
    [[nodiscard]] const QImage& image() const { return m_image; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    FrameSource* m_source = nullptr;
    QImage m_image;
    QString m_timecode;
};

} // namespace subedit::gui
