#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/text_diff.hpp>
#include <subedit/gui/correction_diff_view.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QPainter>
#include <QTextDocument>

#include <optional>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {
/// The width `sizeHint` lays a cell's text out at before a real column width
/// is known — generous enough that the first measurement is not too tight,
/// and revised the moment the view hands a real `option.rect`.
constexpr int kFallbackTextWidth = 200;
} // namespace

/// One row: the correction itself, whether it is accepted, and what a user
/// retouched it to, if anything.
struct CorrectionResultModel::Row {
    core::ProposedCorrection correction;
    bool accepted = true;
    std::optional<std::string> retouched;

    /// What the Proposed column shows and edits — the retouched text if
    /// there is one, otherwise the proposal itself, blank for a removal.
    [[nodiscard]] std::string proposedText() const {
        if (retouched.has_value())
            return *retouched;
        return correction.proposed.value_or(std::string{});
    }
};

CorrectionResultModel::CorrectionResultModel(std::vector<core::ProposedCorrection> corrections,
                                             QObject* parent)
    : QAbstractTableModel(parent) {
    m_rows.reserve(corrections.size());
    for (core::ProposedCorrection& correction : corrections)
        m_rows.push_back(
            Row{.correction = std::move(correction), .accepted = true, .retouched = std::nullopt});
}

CorrectionResultModel::~CorrectionResultModel() = default;

int CorrectionResultModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int CorrectionResultModel::columnCount(const QModelIndex& /*parent*/) const {
    return 3;
}

QVariant CorrectionResultModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};
    const Row& row = m_rows.at(static_cast<std::size_t>(index.row()));

    if (index.column() == Accept && role == Qt::CheckStateRole)
        return row.accepted ? Qt::Checked : Qt::Unchecked;

    if (index.column() == Original && role == Qt::DisplayRole) {
        const core::TextDiff diff = core::diffTexts(row.correction.original, row.proposedText());
        return correctionDiffHtml(diff.original);
    }
    if (index.column() == Proposed) {
        if (role == Qt::EditRole)
            return QString::fromStdString(row.proposedText());
        if (role == Qt::DisplayRole) {
            const core::TextDiff diff =
                core::diffTexts(row.correction.original, row.proposedText());
            return correctionDiffHtml(diff.proposed);
        }
    }
    return {};
}

bool CorrectionResultModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid())
        return false;
    Row& row = m_rows.at(static_cast<std::size_t>(index.row()));

    if (index.column() == Accept && role == Qt::CheckStateRole) {
        row.accepted = value.toInt() == Qt::Checked;
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }
    if (index.column() == Proposed && role == Qt::EditRole) {
        row.retouched = value.toString().toStdString();
        // The Original column's diff is computed against the current
        // Proposed text (proposedText()), so retouching this cell also
        // moves where "changed" falls in Original — both columns need a
        // repaint, not just the one edited.
        emit dataChanged(
            this->index(index.row(), Original), index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

Qt::ItemFlags CorrectionResultModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags common = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == Accept)
        return common | Qt::ItemIsUserCheckable;
    if (index.column() == Proposed)
        return common | Qt::ItemIsEditable;
    return common;
}

QVariant
CorrectionResultModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    switch (section) {
    case Accept:
        return QStringLiteral("Accept");
    case Original:
        return QStringLiteral("Original");
    case Proposed:
        return QStringLiteral("Corrected Text");
    default:
        return {};
    }
}

void CorrectionResultModel::markAll(bool accepted) {
    for (Row& row : m_rows)
        row.accepted = accepted;
    if (!m_rows.empty())
        emit dataChanged(index(0, Accept),
                         index(static_cast<int>(m_rows.size()) - 1, Accept),
                         {Qt::CheckStateRole});
}

const core::ProposedCorrection& CorrectionResultModel::correctionAt(int row) const {
    return m_rows.at(static_cast<std::size_t>(row)).correction;
}

std::vector<core::ProposedCorrection> CorrectionResultModel::acceptedCorrections() const {
    std::vector<core::ProposedCorrection> result;
    for (const Row& row : m_rows) {
        if (!row.accepted)
            continue;
        core::ProposedCorrection correction = row.correction;
        if (row.retouched.has_value())
            correction.proposed = *row.retouched;
        if (correction.proposed.has_value() && *correction.proposed == correction.original)
            continue; // retouched back to the original: nothing to apply
        result.push_back(std::move(correction));
    }
    return result;
}

void CorrectionDiffDelegate::paint(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->save();

    QTextDocument document;
    document.setHtml(opt.text);
    opt.text.clear();
    if (opt.widget != nullptr)
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    painter->translate(opt.rect.topLeft());
    document.setTextWidth(opt.rect.width());
    document.drawContents(painter);
    painter->restore();
}

QSize CorrectionDiffDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const {
    QTextDocument document;
    document.setHtml(index.data(Qt::DisplayRole).toString());
    document.setTextWidth(option.rect.width() > 0 ? option.rect.width() : kFallbackTextWidth);
    return QSize{static_cast<int>(document.idealWidth()),
                 static_cast<int>(document.size().height())};
}

} // namespace subedit::gui
