#include <subedit/core/config/video_settings.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/play_bar.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QString>
#include <QToolButton>
#include <Qt>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>

namespace subedit::gui {

namespace {

/// What a position reads as: the same `HH:MM:SS,mmm` the table writes, so that the bar and the
/// row it points at say the same thing in the same way.
[[nodiscard]] QString textOf(core::Timestamp position) {
    return QString::fromStdString(position.format(core::DecimalMark::Comma));
}

/// The widest the slider's range can be: it counts in milliseconds, and a film cannot last longer
/// than `int` holds — twenty-four days, which is not a limit anyone meets.
constexpr std::int64_t kLargestRange = std::numeric_limits<int>::max();

/// How many steps a click on the groove, or a page key, moves the volume.
constexpr int kVolumePage = 10;

/// How wide the volume slider is, in pixels: a control, not a part of the timeline.
constexpr int kVolumeWidth = 90;

} // namespace

PlayBar::PlayBar(QWidget* parent)
    : QWidget{parent},
      m_play(new QToolButton{this}),
      m_positionText(new QLabel{this}),
      m_position(new QSlider{Qt::Horizontal, this}),
      m_lengthText(new QLabel{this}),
      m_volume(new QSlider{Qt::Horizontal, this}) {
    m_play->setText(QStringLiteral("Play"));
    m_play->setToolTip(QStringLiteral("Play / Pause"));
    m_play->setAutoRaise(true);

    m_positionText->setText(textOf(core::Timestamp::origin()));
    m_lengthText->setText(textOf(core::Timestamp::origin()));
    // Tabular figures would be better; what the label can do is not to jump about, and a fixed
    // minimum width is what stops the slider from shifting when a digit changes.
    m_positionText->setMinimumWidth(m_positionText->sizeHint().width());
    m_lengthText->setMinimumWidth(m_lengthText->sizeHint().width());

    m_position->setRange(0, 0);
    m_position->setToolTip(QStringLiteral("Position"));

    m_volume->setRange(0, core::kLargestVolume);
    m_volume->setPageStep(kVolumePage);
    m_volume->setFixedWidth(kVolumeWidth);
    m_volume->setToolTip(QStringLiteral("Volume"));

    auto* row = new QHBoxLayout{this};
    row->setContentsMargins(4, 2, 4, 2);
    row->addWidget(m_play);
    row->addWidget(m_positionText);
    row->addWidget(m_position, 1);
    row->addWidget(m_lengthText);
    row->addWidget(m_volume);

    connect(m_play, &QToolButton::clicked, this, &PlayBar::playToggled);

    // **A change is a gesture unless the bar made it.** `valueChanged` fires for a drag, a click
    // on the groove and a key, which are the three ways a person asks, and for `setValue`, which
    // is how the follower moves the handle: the flag is what tells them apart.
    connect(m_position, &QSlider::valueChanged, this, [this](int value) {
        if (m_updating)
            return;

        // The position says where the handle is while it is held: the follower leaves the bar
        // alone then, and a label that waited for the player would trail the hand.
        const core::Timestamp position = core::Timestamp::fromMilliseconds(value);
        m_positionText->setText(textOf(position));
        emit seekRequested(position);
    });
    connect(m_position, &QSlider::sliderReleased, this, &PlayBar::seekFinished);

    connect(m_volume, &QSlider::valueChanged, this, [this](int value) {
        if (!m_updating)
            emit volumeRequested(value);
    });
}

void PlayBar::showPosition(std::optional<core::Timestamp> position,
                           std::optional<core::Duration> length) {
    if (m_position->isSliderDown())
        return;

    const std::int64_t range =
        length.has_value() ? std::clamp<std::int64_t>(length->milliseconds(), 0, kLargestRange) : 0;
    const std::int64_t where =
        position.has_value() ? std::clamp<std::int64_t>(position->milliseconds(), 0, range) : 0;

    m_updating = true;
    m_position->setRange(0, static_cast<int>(range));
    m_position->setValue(static_cast<int>(where));
    m_updating = false;

    m_positionText->setText(textOf(position.value_or(core::Timestamp::origin())));
    m_lengthText->setText(
        textOf(core::Timestamp::fromMilliseconds(length.has_value() ? length->milliseconds() : 0)));
}

void PlayBar::showVolume(int volume) {
    m_updating = true;
    m_volume->setValue(volume);
    m_updating = false;
}

void PlayBar::showPlaying(bool playing) {
    m_play->setText(playing ? QStringLiteral("Pause") : QStringLiteral("Play"));
}

} // namespace subedit::gui
