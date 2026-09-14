#pragma once

#include <subedit/gui/operation_dialog.hpp>

#include <cstddef>

class QCheckBox;
class QDoubleSpinBox;

// Declared and not included. With the core header included here, moc stopped on
// a parse error inside `<concept>` while reading this file — and a declaration
// is all a header that only names the type in a signature needs anyway.
namespace subedit::core {
struct DurationConstraints;
} // namespace subedit::core

namespace subedit::gui {

/// Asks for the four constraints of an adjustment of durations.
///
/// **Each one can be switched off**, and a constraint switched off is absent —
/// not a value of zero. Its field goes grey, and keeps the value it had so
/// that switching it back on does not ask for it again.
///
/// The reading speed has two switches rather than one, which are Gaupol's: it
/// may lengthen what is too short, shorten what is too long, or both. With
/// neither, it has nothing to do, and its field goes grey too.
///
/// **Nothing to do is not a request**: with every constraint off, `OK` is out.
class DurationAdjustDialog final : public OperationDialog {
    Q_OBJECT

public:
    /// Opens on `initial`, which is what the last adjustment of this window
    /// asked for, or Gaupol's defaults.
    DurationAdjustDialog(std::size_t targetCount,
                         const core::DurationConstraints& initial,
                         QWidget* parent = nullptr);

    /// Returns the constraints as the fields say them.
    [[nodiscard]] core::DurationConstraints constraints() const;

    [[nodiscard]] bool isComplete() const override;

    /// The fields, for a test to fill them.
    [[nodiscard]] QDoubleSpinBox* speedBox() const { return m_speed; }

    [[nodiscard]] QCheckBox* lengthenCheck() const { return m_lengthen; }

    [[nodiscard]] QCheckBox* shortenCheck() const { return m_shorten; }

    [[nodiscard]] QCheckBox* minimumCheck() const { return m_useMinimum; }

    [[nodiscard]] QDoubleSpinBox* minimumBox() const { return m_minimum; }

    [[nodiscard]] QCheckBox* maximumCheck() const { return m_useMaximum; }

    [[nodiscard]] QDoubleSpinBox* maximumBox() const { return m_maximum; }

    [[nodiscard]] QCheckBox* gapCheck() const { return m_useGap; }

    [[nodiscard]] QDoubleSpinBox* gapBox() const { return m_gap; }

private:
    /// Greys out what is switched off, and asks the base whether `OK` holds.
    void refresh();

    QDoubleSpinBox* m_speed;
    QCheckBox* m_lengthen;
    QCheckBox* m_shorten;
    QCheckBox* m_useMinimum;
    QDoubleSpinBox* m_minimum;
    QCheckBox* m_useMaximum;
    QDoubleSpinBox* m_maximum;
    QCheckBox* m_useGap;
    QDoubleSpinBox* m_gap;
};

} // namespace subedit::gui
