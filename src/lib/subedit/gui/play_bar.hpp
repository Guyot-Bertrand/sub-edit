#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <QIcon>
#include <QWidget>

#include <optional>

class QAction;
class QEvent;
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

    /// Shows whether the table follows playback, without announcing it as a gesture — issue #619.
    void showFollowing(bool following);

    /// The button that says whether the table follows playback, and puts it right: checked while it
    /// does.
    [[nodiscard]] QToolButton* followButton() const { return m_follow; }

    [[nodiscard]] QSlider* positionSlider() const { return m_position; }

    [[nodiscard]] QSlider* volumeSlider() const { return m_volume; }

    [[nodiscard]] QToolButton* playButton() const { return m_play; }

    /// The two buttons either side of play and pause, which step the film by the frame step —
    /// issue #618.
    [[nodiscard]] QToolButton* stepBackButton() const { return m_stepBack; }

    [[nodiscard]] QToolButton* stepForwardButton() const { return m_stepForward; }

    /// Gives the two step buttons the actions of the menu to run. **The buttons trigger the same
    /// actions, and say what they say**: the same step, out for the same reasons — a button on its
    /// own would be a second way of stepping that the menu does not know about — and the same
    /// tooltip, whose shortcut is the action's. A button held **repeats**, as a held key does, and
    /// `VideoPane` is what keeps the repeats from piling up.
    void setStepActions(QAction* back, QAction* forward);

    [[nodiscard]] QLabel* positionLabel() const { return m_positionText; }

    [[nodiscard]] QLabel* lengthLabel() const { return m_lengthText; }

protected:
    /// The icon of the follow button is drawn in the colors of the palette, and drawn again when
    /// the palette changes — the window goes from the light palette to the dark one at the press of
    /// a menu entry.
    void changeEvent(QEvent* event) override;

signals:
    /// The person asked for this position: a drag, a click on the groove, a key. Many of these
    /// follow one another while a handle moves; `VideoPane` is what keeps up with them.
    void seekRequested(subedit::core::Timestamp position);

    /// The handle was let go: the last position asked is the one to reach.
    void seekFinished();

    /// The person pressed the play button.
    void playToggled();

    /// The person pressed the button that says whether the table follows: checked, it is asked to
    /// follow from now on, and unchecked, to stop.
    void followToggled(bool following);

    /// The person moved the volume.
    void volumeRequested(int volume);

private:
    QToolButton* m_stepBack;
    QToolButton* m_play;
    QToolButton* m_stepForward;
    QLabel* m_positionText;
    QSlider* m_position;
    QLabel* m_lengthText;
    QSlider* m_volume;
    QToolButton* m_follow;

    /// Draws the icons of the bar for the current palette: those of the theme where it has them,
    /// the drawn symbols where it has not, and the one of the follow button always.
    void drawIcons();

    /// Set while the bar moves a control itself, so that the change is not heard as a gesture.
    bool m_updating = false;

    /// What the play button offers, and whether the film is playing: kept, so that a change of
    /// palette draws them again without asking the player.
    QIcon m_playIcon;
    QIcon m_pauseIcon;
    bool m_playing = false;
};

} // namespace subedit::gui
