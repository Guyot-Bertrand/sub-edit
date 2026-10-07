#include <subedit/core/config/video_settings.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/play_bar.hpp>

#include <QAction>
#include <QColor>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPoint>
#include <QPolygon>
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

/// The symbols of a transport, drawn.
enum class Symbol { Play, Pause, SkipBack, SkipForward };

/// The side of the pixmaps the icons are drawn on, in pixels: scaled down by Qt to whatever a
/// button shows, which a drawing in plain shapes does without harm.
constexpr int kIconSide = 64;

/// A symbol of the transport in the color the palette gives to text on a button.
///
/// **The fallback of the theme's icon, and the only one**: the style's own (`SP_MediaPlay` and the
/// others) are fixed pixmaps of a dark grey that read as disabled on the dark palette. These are
/// drawn from the palette, so they read on both — which is also what the manual's captures need,
/// taken with no icon theme at all.
[[nodiscard]] QPixmap symbolPixmap(const QPalette& palette, Symbol symbol) {
    QPixmap pixmap{kIconSide, kIconSide};
    pixmap.fill(Qt::transparent);

    QPainter painter{&pixmap};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette.color(QPalette::ButtonText));

    constexpr int kInset = 12;
    constexpr int kFar = kIconSide - kInset;
    constexpr int kMiddle = kIconSide / 2;
    constexpr int kBar = 12;
    constexpr int kSkipTriangle = 28;

    switch (symbol) {
    case Symbol::Play:
        painter.drawPolygon(QPolygon{
            {QPoint{kInset + 4, kInset}, QPoint{kFar, kMiddle}, QPoint{kInset + 4, kFar}}});
        break;
    case Symbol::Pause:
        painter.drawRect(QRect{kInset + 2, kInset, kBar, kFar - kInset});
        painter.drawRect(QRect{kFar - 2 - kBar, kInset, kBar, kFar - kInset});
        break;
    case Symbol::SkipBack:
        painter.drawRect(QRect{kInset, kInset, kBar / 2, kFar - kInset});
        painter.drawPolygon(QPolygon{
            {QPoint{kFar, kInset}, QPoint{kFar - kSkipTriangle - 6, kMiddle}, QPoint{kFar, kFar}}});
        break;
    case Symbol::SkipForward:
        painter.drawRect(QRect{kFar - (kBar / 2), kInset, kBar / 2, kFar - kInset});
        painter.drawPolygon(QPolygon{{QPoint{kInset, kInset},
                                      QPoint{kInset + kSkipTriangle + 6, kMiddle},
                                      QPoint{kInset, kFar}}});
        break;
    }
    return pixmap;
}

/// The icon of the theme the desktop gives, and the drawn symbol where the theme has none.
[[nodiscard]] QIcon iconOf(const QPalette& palette, const char* themed, Symbol symbol) {
    return QIcon::fromTheme(QString::fromLatin1(themed), QIcon{symbolPixmap(palette, symbol)});
}

/// The icon of the follow button: the lines of a table, one of them marked and held in the middle.
///
/// **Drawn and not taken from the theme**, for two reasons. No theme has an icon that says *the
/// table follows the film* — `go-jump` and the like say another thing — and the one that comes
/// closest would differ from one desktop to the next, where the button has to read the same. And
/// the manual's captures are taken with no theme at all.
///
/// Two states: **on**, the marked line in the accent color with an arrow pointing at it, and
/// **off**, the same lines all alike and dimmed — the table has been let go.
[[nodiscard]] QPixmap followPixmap(const QPalette& palette, bool following) {
    QPixmap pixmap{kIconSide, kIconSide};
    pixmap.fill(Qt::transparent);

    QPainter painter{&pixmap};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    constexpr int kBars = 5;
    constexpr int kMarked = 2;
    constexpr int kBarHeight = 7;
    constexpr int kGap = 5;
    constexpr int kTop = 4;
    constexpr int kLeft = 22;
    constexpr int kRadius = 3;
    constexpr int kDimmed = 150;
    constexpr int kSwell = 2;

    QColor plain = palette.color(QPalette::ButtonText);
    plain.setAlpha(following ? kDimmed : kDimmed / 2);
    const QColor accent = palette.color(QPalette::Highlight);

    for (int bar = 0; bar < kBars; ++bar) {
        const bool marked = following && bar == kMarked;
        const int y = kTop + (bar * (kBarHeight + kGap));
        // The marked line is a little taller than the others, so that it holds at 16 pixels.
        const int swell = marked ? kSwell : 0;
        painter.setBrush(marked ? accent : plain);
        painter.drawRoundedRect(
            QRect{kLeft, y - swell, kIconSide - kLeft - 3, kBarHeight + (2 * swell)},
            kRadius,
            kRadius);

        if (marked) {
            // The arrow, in the accent color, pointing at the line that is held.
            const int middle = y + (kBarHeight / 2);
            painter.drawPolygon(QPolygon{
                {QPoint{2, middle - 11}, QPoint{kLeft - 4, middle}, QPoint{2, middle + 11}}});
        }
    }
    return pixmap;
}

/// How many steps a click on the groove, or a page key, moves the volume.
constexpr int kVolumePage = 10;

/// How wide the volume slider is, in pixels: a control, not a part of the timeline.
constexpr int kVolumeWidth = 90;

} // namespace

PlayBar::PlayBar(QWidget* parent)
    : QWidget{parent},
      m_stepBack(new QToolButton{this}),
      m_play(new QToolButton{this}),
      m_stepForward(new QToolButton{this}),
      m_positionText(new QLabel{this}),
      m_position(new JumpSlider{Qt::Horizontal, this}),
      m_lengthText(new QLabel{this}),
      m_volume(new JumpSlider{Qt::Horizontal, this}),
      m_follow(new QToolButton{this}) {
    // **Icons and no text**: a transport reads as the symbols everybody knows, and they say what
    // the button will do — the triangle when stopped, the bars when playing. The tooltip says it in
    // words, for whoever hovers and for a screen reader.
    for (QToolButton* button : {m_stepBack, m_play, m_stepForward})
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    drawIcons();
    showPlaying(false);
    m_play->setAutoRaise(true);
    // A held button repeats, as a held key does: the same action each time, one step at a time.
    for (QToolButton* step : {m_stepBack, m_stepForward}) {
        step->setAutoRaise(true);
        step->setAutoRepeat(true);
    }

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
    row->addWidget(m_stepBack);
    row->addWidget(m_play);
    row->addWidget(m_stepForward);
    row->addWidget(m_positionText);
    row->addWidget(m_position, 1);
    row->addWidget(m_lengthText);
    row->addWidget(m_volume);
    row->addWidget(m_follow);

    // **Text and not an icon**: it is not a transport control but the state of the table, and a
    // word says that better than a symbol would. Checked while the table follows playback.
    m_follow->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_follow->setToolTip(
        QStringLiteral("Follow playback: keep the table on the subtitle that is showing"));
    m_follow->setAccessibleName(QStringLiteral("Follow"));
    m_follow->setCheckable(true);
    m_follow->setChecked(true);
    m_follow->setAutoRaise(true);
    connect(m_follow, &QToolButton::toggled, this, [this](bool following) {
        if (!m_updating)
            emit followToggled(following);
    });

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

void PlayBar::drawIcons() {
    m_stepBack->setIcon(iconOf(palette(), "media-skip-backward", Symbol::SkipBack));
    m_stepForward->setIcon(iconOf(palette(), "media-skip-forward", Symbol::SkipForward));
    m_playIcon = iconOf(palette(), "media-playback-start", Symbol::Play);
    m_pauseIcon = iconOf(palette(), "media-playback-pause", Symbol::Pause);
    m_play->setIcon(m_playing ? m_pauseIcon : m_playIcon);

    QIcon following;
    following.addPixmap(followPixmap(palette(), true), QIcon::Normal, QIcon::On);
    following.addPixmap(followPixmap(palette(), false), QIcon::Normal, QIcon::Off);
    m_follow->setIcon(following);
}

void PlayBar::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange)
        drawIcons();
}

void PlayBar::setStepActions(QAction* back, QAction* forward) {
    // **Not as the default action of the button**: that would take the icon from the action, which
    // has none — the menu shows the step in words — and give the button a blank face at the first
    // change of the action. The button runs the action and follows what the action says.
    const auto follow = [](QToolButton* button, QAction* action) {
        connect(button, &QToolButton::clicked, action, &QAction::trigger);
        const auto mirror = [button, action] {
            button->setEnabled(action->isEnabled());
            button->setToolTip(action->toolTip());
        };
        connect(action, &QAction::changed, button, mirror);
        mirror();
    };
    follow(m_stepBack, back);
    follow(m_stepForward, forward);
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

void PlayBar::showFollowing(bool following) {
    m_updating = true;
    m_follow->setChecked(following);
    m_updating = false;
}

void PlayBar::showPlaying(bool playing) {
    m_playing = playing;
    m_play->setIcon(playing ? m_pauseIcon : m_playIcon);
    m_play->setToolTip(playing ? QStringLiteral("Pause") : QStringLiteral("Play"));
}

} // namespace subedit::gui
