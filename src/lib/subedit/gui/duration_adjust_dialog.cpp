#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLocale>
#include <QString>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace subedit::gui {

namespace {

// **Gaupol's bounds**, read in `duration-adjust-dialog.ui`: a speed from one to
// ninety-nine characters a second, to the tenth; durations up to ninety-nine
// seconds, to the millisecond.
constexpr double kSlowestSpeed = 1.0;
constexpr double kFastestSpeed = 99.0;
constexpr double kSpeedStep = 0.5;
constexpr int kSpeedDecimals = 1;
constexpr double kLongestDuration = 99.0;
constexpr double kDurationStep = 0.1;
constexpr int kDurationDecimals = 3;

/// The defaults a switched-off constraint shows, Gaupol's.
constexpr double kDefaultMaximumSeconds = 6.0;

constexpr double kMillisecondsPerSecond = 1000.0;

/// A decimal field that writes a period, whatever the machine's locale.
///
/// **The window speaks English, and its numbers with it.** Left to the system
/// locale, a French desktop showed `15,0 char/s` under English labels — two
/// languages in one field — and the capture of the manual changed with the
/// machine that took it. The table already writes positions without asking the
/// locale; these follow.
[[nodiscard]] QDoubleSpinBox* decimalBox(QWidget* parent) {
    auto* box = new QDoubleSpinBox{parent};
    box->setLocale(QLocale::c());
    return box;
}

[[nodiscard]] QDoubleSpinBox* durationBox(QWidget* parent) {
    auto* box = decimalBox(parent);
    box->setRange(0.0, kLongestDuration);
    box->setSingleStep(kDurationStep);
    box->setDecimals(kDurationDecimals);
    box->setSuffix(QStringLiteral(" s"));
    return box;
}

[[nodiscard]] double secondsOf(core::Duration duration) {
    return static_cast<double>(duration.milliseconds()) / kMillisecondsPerSecond;
}

/// The duration a box holds, rounded once to the millisecond it shows.
[[nodiscard]] core::Duration durationOf(const QDoubleSpinBox& box) {
    return core::Duration::fromMilliseconds(
        static_cast<std::int64_t>(std::llround(box.value() * kMillisecondsPerSecond)));
}

/// The duration of `box` when `check` is on, and nothing otherwise.
[[nodiscard]] std::optional<core::Duration> whenOn(const QCheckBox& check,
                                                   const QDoubleSpinBox& box) {
    if (!check.isChecked())
        return std::nullopt;
    return durationOf(box);
}

} // namespace

DurationAdjustDialog::DurationAdjustDialog(std::size_t targetCount,
                                           const core::DurationConstraints& initial,
                                           QWidget* parent)
    : OperationDialog(targetCount, parent),
      m_speed(decimalBox(this)),
      m_lengthen(new QCheckBox{QStringLiteral("Lengthen durations to match it"), this}),
      m_shorten(new QCheckBox{QStringLiteral("Shorten durations to match it"), this}),
      m_useMinimum(new QCheckBox{QStringLiteral("Minimum duration"), this}),
      m_minimum(durationBox(this)),
      m_useMaximum(new QCheckBox{QStringLiteral("Maximum duration"), this}),
      m_maximum(durationBox(this)),
      m_useGap(new QCheckBox{QStringLiteral("Gap between subtitles"), this}),
      m_gap(durationBox(this)) {
    setWindowTitle(QStringLiteral("Adjust durations"));

    m_speed->setRange(kSlowestSpeed, kFastestSpeed);
    m_speed->setSingleStep(kSpeedStep);
    m_speed->setDecimals(kSpeedDecimals);
    m_speed->setSuffix(QStringLiteral(" char/s"));

    const core::ReadingSpeed speed = initial.speed.value_or(
        *core::ReadingSpeed::create(core::kDefaultReadingSpeed, true, false));
    m_speed->setValue(speed.charactersPerSecond());
    m_lengthen->setChecked(initial.speed.has_value() && speed.lengthen());
    m_shorten->setChecked(initial.speed.has_value() && speed.shorten());

    // A constraint switched off still shows a value: the one it had, or the
    // default, so that switching it on is one click and not two gestures.
    m_useMinimum->setChecked(initial.minimum.has_value());
    m_minimum->setValue(secondsOf(initial.minimum.value_or(
        core::Duration::fromMilliseconds(core::kDefaultMinimumMilliseconds))));
    m_useMaximum->setChecked(initial.maximum.has_value());
    m_maximum->setValue(initial.maximum.has_value() ? secondsOf(*initial.maximum)
                                                    : kDefaultMaximumSeconds);
    m_useGap->setChecked(initial.gap.has_value());
    m_gap->setValue(secondsOf(initial.gap.value_or(core::Duration::zero())));

    // In the order they are applied, which is the order the manual gives.
    fields()->addRow(QStringLiteral("Reading speed"), m_speed);
    fields()->addRow(QString{}, m_lengthen);
    fields()->addRow(QString{}, m_shorten);
    fields()->addRow(m_useMinimum, m_minimum);
    fields()->addRow(m_useMaximum, m_maximum);
    fields()->addRow(m_useGap, m_gap);

    for (QCheckBox* check : {m_lengthen, m_shorten, m_useMinimum, m_useMaximum, m_useGap})
        connect(check, &QCheckBox::toggled, this, [this] { refresh(); });

    refresh();
    finish();
}

core::DurationConstraints DurationAdjustDialog::constraints() const {
    std::optional<core::ReadingSpeed> speed;
    if (m_lengthen->isChecked() || m_shorten->isChecked()) {
        // The box is bounded to [1, 99]: `create` cannot refuse what it holds.
        speed = core::ReadingSpeed::create(
            m_speed->value(), m_lengthen->isChecked(), m_shorten->isChecked());
    }

    return core::DurationConstraints{.speed = speed,
                                     .minimum = whenOn(*m_useMinimum, *m_minimum),
                                     .maximum = whenOn(*m_useMaximum, *m_maximum),
                                     .gap = whenOn(*m_useGap, *m_gap)};
}

bool DurationAdjustDialog::isComplete() const {
    return constraints().isAny();
}

void DurationAdjustDialog::refresh() {
    m_speed->setEnabled(m_lengthen->isChecked() || m_shorten->isChecked());
    m_minimum->setEnabled(m_useMinimum->isChecked());
    m_maximum->setEnabled(m_useMaximum->isChecked());
    m_gap->setEnabled(m_useGap->isChecked());
    revalidate();
}

} // namespace subedit::gui
