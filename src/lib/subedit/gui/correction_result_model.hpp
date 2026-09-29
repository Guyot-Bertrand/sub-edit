#pragma once

// The confirmation table of D8 and its diff-rendering delegate — issue #505,
// task 12.

#include <QAbstractTableModel>
#include <QSize>
#include <QStyledItemDelegate>

#include <vector>

/// **Declared rather than included, deliberately** — the same reason and the
/// same shape as `correction_progress_page.hpp`: `moc` parses this header,
/// and it chokes on the C++20 library headers `correction_run.hpp` drags in
/// through `Selection`/`SubtitleIndex` (`<iterator>` pulls `<concepts>`).
///
/// `Row` wraps a `core::ProposedCorrection` by value, so unlike the progress
/// page's `std::function` target, it does need the type complete — which is
/// exactly why `Row` itself is only forward-declared below and defined in the
/// implementation file, alongside every member whose body would otherwise
/// need `Row` or `core::ProposedCorrection` complete to compile inline.
/// `std::vector<Row>` stays a plain value member: a `std::vector` of an
/// incomplete type is well-formed as long as nothing but its destructor,
/// constructor and the like — all defined in the `.cpp`, where the real
/// header is included — actually gets used.
namespace subedit::core {
struct ProposedCorrection;
} // namespace subedit::core

class QPainter;
class QStyleOptionViewItem;

namespace subedit::gui {

/// The confirmation table of D8 — one row per proposed change, an accept box,
/// the original and the proposed text, both with their difference marked, the
/// proposed text editable in place.
class CorrectionResultModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column { Accept = 0, Original = 1, Proposed = 2 };

    explicit CorrectionResultModel(std::vector<core::ProposedCorrection> corrections,
                                   QObject* parent = nullptr);
    ~CorrectionResultModel() override;

    CorrectionResultModel(const CorrectionResultModel&) = delete;
    CorrectionResultModel& operator=(const CorrectionResultModel&) = delete;
    CorrectionResultModel(CorrectionResultModel&&) = delete;
    CorrectionResultModel& operator=(CorrectionResultModel&&) = delete;

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] QVariant
    headerData(int section, Qt::Orientation orientation, int role) const override;

    /// `Mark All` / `Unmark All`.
    void markAll(bool accepted);

    [[nodiscard]] const core::ProposedCorrection& correctionAt(int row) const;

    /// The accepted rows, `proposed` replaced by what was typed for a row
    /// that was retouched — never a row retouched back to exactly its
    /// original text, which is not a change to apply.
    [[nodiscard]] std::vector<core::ProposedCorrection> acceptedCorrections() const;

private:
    struct Row;

    std::vector<Row> m_rows;
};

/// Paints `Qt::DisplayRole` as the rich text Task 6 produces — `QStyledItemDelegate`
/// on its own treats it as plain text, HTML tags and all.
class CorrectionDiffDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const override;
};

} // namespace subedit::gui
