#pragma once

#include <subedit/core/config/insert_placement.hpp>

#include <QDialog>

#include <cstddef>

class QRadioButton;
class QSpinBox;
class QWidget;

namespace subedit::gui {

/// How many blank rows to insert, and on which side of the selection.
///
/// **Not an `OperationDialog`**, and the difference is not one of form: the
/// four others announce "Applies to: 4 subtitles", because they transform
/// subtitles that exist. This one touches none — it adds some — and the
/// sentence would be false in the one case that counts, the empty document.
///
/// `hasSubtitles` puts the choice of side out rather than hiding it: in an
/// empty document there is no selection, so no side, and the insertion happens
/// at index zero. A greyed box says why the choice is not on offer; an absent
/// one reads as something missing.
class InsertDialog final : public QDialog {
    Q_OBJECT

public:
    InsertDialog(bool hasSubtitles, core::InsertPlacement placement, QWidget* parent = nullptr);

    /// How many rows, never zero.
    [[nodiscard]] std::size_t count() const;

    /// The side chosen, whether or not it was accepted — the caller looks at
    /// the return code to know whether to take it into account, as for the
    /// theme.
    [[nodiscard]] core::InsertPlacement placement() const;

    /// The fields, so that a test sets them without clicking.
    [[nodiscard]] QSpinBox* countBox() const { return m_count; }

    void setPlacement(core::InsertPlacement placement);

private:
    QSpinBox* m_count;
    QRadioButton* m_above;
    QRadioButton* m_below;
};

} // namespace subedit::gui
