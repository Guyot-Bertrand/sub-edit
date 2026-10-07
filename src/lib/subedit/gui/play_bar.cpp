#include <subedit/core/config/video_settings.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/play_bar.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QRect>
#include <QSlider>
#include <QString>
#include <QStyle>
#include <QStyleOptionSlider>
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

/// A slider that goes where it is clicked.
///
/// **Qt's does not, and it is not a defect of Qt**: a click on the groove of a `QSlider` moves the
/// handle by a page, toward the click — the convention of scroll bars, which is what a slider
/// inherits. On a bar of ten minutes counted in milliseconds a page is ten of them, so a click
/// moved the film by nothing a person could see (issue #615, seen on a real window). A seek bar is
/// a ruler and not a scroll bar: the click is a position.
///
/// The handle is put under the click and the press then goes on as a drag of it, so that a click
/// that does not let go keeps moving the film.
class JumpSlider final : public QSlider {

public:
    using QSlider::QSlider;

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && !isOnHandle(event->position().toPoint()))
            setValue(valueAt(event->position().toPoint()));

        QSlider::mousePressEvent(event);
    }

private:
    [[nodiscard]] QStyleOptionSlider options() const {
        QStyleOptionSlider option;
        initStyleOption(&option);
        return option;
    }

    [[nodiscard]] bool isOnHandle(const QPoint& at) const {
        const QStyleOptionSlider option = options();
        return style()
            ->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this)
            .contains(at);
    }

    /// The value the handle takes when its centre is at `at`.
    [[nodiscard]] int valueAt(const QPoint& at) const {
        const QStyleOptionSlider option = options();
        const QRect groove =
            style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderGroove, this);
        const QRect handle =
            style()->subControlRect(QStyle::CC_Slider, &option, QStyle::SC_SliderHandle, this);

        // Horizontal, both of them: the bar has no slider that stands up.
        const int span = groove.width() - handle.width();
        const int position = at.x() - groove.x() - (handle.width() / 2);
        return QStyle::sliderValueFromPosition(
            minimum(), maximum(), position, span, option.upsideDown);
    }
};

/// How many steps a click on the groove, or a page key, moves the volume.
constexpr int kVolumePage = 10;

/// How wide the volume slider is, in pixels: a control, not a part of the timeline.
constexpr int kVolumeWidth = 90;

} // namespace

PlayBar::PlayBar(QWidget* parent)
    : QWidget{parent},
      m_play(new QToolButton{this}),
      m_positionText(new QLabel{this}),
      m_position(new JumpSlider{Qt::Horizontal, this}),
      m_lengthText(new QLabel{this}),
      m_volume(new JumpSlider{Qt::Horizontal, this}) {
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
