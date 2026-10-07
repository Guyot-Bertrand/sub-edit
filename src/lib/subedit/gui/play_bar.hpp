#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <QWidget>

#include <optional>

class QLabel;
class QSlider;
class QToolButton;

namespace subedit::gui {

/// The bar under the picture — issue #615: where playback stands, how long the film lasts, play
/// and pause, and the volume.
///
/// **It shows and it asks, and it knows nothing of a player.** `VideoPane` tells it where
/// playback stands and what the volume is; it tells `VideoPane`, by signal, what the person
/// did. That is what keeps the throttling of a drag — the one subtle thing a seek bar has to do —
/// in one place that can be driven without a film.
///
/// **A gesture is told apart from an update.** The slider moves by itself while a film plays, and
/// that must not read as somebody asking for a position: only a movement the person made
/// — a drag, a click on the groove, a key — reaches `seekRequested`.
class PlayBar final : public QWidget {
    Q_OBJECT

public:
    explicit PlayBar(QWidget* parent = nullptr);

    /// Shows where playback stands and how long the film lasts. Nothing for either leaves the
    /// bar as it would be with no film: an empty position, a slider at the start.
    ///
    /// **Ignored while the slider is held**, so that a tick of the follower does not take the
    /// handle out of the hand that holds it.
    void showPosition(std::optional<core::Timestamp> position,
                      std::optional<core::Duration> length);

    /// Shows the volume, 0 to 100, without announcing it as a gesture.
    void showVolume(int volume);

    /// Shows whether the film is playing: the button offers the other of the two.
    void showPlaying(bool playing);

    [[nodiscard]] QSlider* positionSlider() const { return m_position; }

    [[nodiscard]] QSlider* volumeSlider() const { return m_volume; }

    [[nodiscard]] QToolButton* playButton() const { return m_play; }

    [[nodiscard]] QLabel* positionLabel() const { return m_positionText; }

    [[nodiscard]] QLabel* lengthLabel() const { return m_lengthText; }

signals:
    /// The person asked for this position: a drag, a click on the groove, a key. Many of these
    /// follow one another while a handle moves; `VideoPane` is what keeps up with them.
    void seekRequested(subedit::core::Timestamp position);

    /// The handle was let go: the last position asked is the one to reach.
    void seekFinished();

    /// The person pressed the play button.
    void playToggled();

    /// The person moved the volume.
    void volumeRequested(int volume);

private:
    QToolButton* m_play;
    QLabel* m_positionText;
    QSlider* m_position;
    QLabel* m_lengthText;
    QSlider* m_volume;

    /// Set while the bar moves a control itself, so that the change is not heard as a gesture.
    bool m_updating = false;
};

} // namespace subedit::gui
