#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <QCheckBox>
#include <QString>
#include <QVBoxLayout>

#include <algorithm>

namespace subedit::gui {

PatternList::PatternList(const core::PatternCatalogue& catalogue,
                         core::PatternKind kind,
                         QWidget* parent)
    : QWidget(parent), m_catalogue(&catalogue), m_kind(kind), m_layout(new QVBoxLayout{this}) {
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void PatternList::setCode(std::string_view code, const core::CorrectionSettings& settings) {
    // Deleted right away, not with `deleteLater`: a box built for the
    // previous code must be gone before this call returns, since
    // `activations()` or a fresh `setCode` may run before the event loop
    // ever spins again — a test harness never does.
    for (const Entry& entry : m_entries)
        delete entry.box;
    m_entries.clear();

    for (const core::CorrectionPattern* record : m_catalogue->cascade(m_kind, code)) {
        const auto found = std::ranges::find_if(
            m_entries, [record](const Entry& entry) { return entry.name == record->name; });
        if (found != m_entries.end()) {
            found->records.push_back(record);
            found->box->setChecked(found->box->isChecked() ||
                                   core::patternEnabled(*record, settings));
            continue;
        }
        auto* box = new QCheckBox{QString::fromStdString(record->name), this};
        box->setObjectName(QString::fromStdString(record->name));
        box->setChecked(core::patternEnabled(*record, settings));
        connect(box, &QCheckBox::toggled, this, &PatternList::changed);
        m_layout->addWidget(box);
        m_entries.push_back(Entry{.name = record->name, .box = box, .records = {record}});
    }
}

std::vector<core::PatternActivation> PatternList::activations() const {
    std::vector<core::PatternActivation> activations;
    for (const Entry& entry : m_entries) {
        for (const core::CorrectionPattern* record : entry.records) {
            if (record->enabled == entry.box->isChecked())
                continue; // agrees with the shipped default: no override to write
            activations.push_back(core::PatternActivation{.kind = record->kind(),
                                                          .code = record->code,
                                                          .name = record->name,
                                                          .enabled = entry.box->isChecked()});
        }
    }
    return activations;
}

} // namespace subedit::gui
