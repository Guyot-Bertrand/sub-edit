#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <QCheckBox>
#include <QString>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace subedit::gui {

PatternList::PatternList(const core::PatternCatalogue& catalogue,
                         core::PatternKind kind,
                         QWidget* parent)
    : QWidget(parent), m_catalogue(&catalogue), m_kind(kind), m_layout(new QVBoxLayout{this}) {
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void PatternList::setCode(std::string_view code, const core::CorrectionSettings& settings) {
    // `setParent(nullptr)` first, then `deleteLater()`: a box built for the
    // previous code must stop being this widget's child — invisible to
    // `findChild`/`findChildren` and out of the layout — before this call
    // returns, since `activations()` or a fresh `setCode` may run before the
    // event loop ever spins again — a test harness never does. But the
    // actual destruction stays deferred: `toggled` is connected to
    // `changed()`, emitted synchronously from inside the box's own call
    // stack, so a caller that ever wired `changed()` back into `setCode` on
    // this same instance would reenter here mid-emission — plain `delete`
    // would then free the very box still unwinding its signal, a
    // use-after-free. `deleteLater()` keeps that reentrant call safe: the
    // object outlives the emission, and is queued for deletion once Qt is
    // done with it.
    for (const Entry& entry : m_entries) {
        entry.box->setParent(nullptr);
        entry.box->deleteLater();
    }
    m_entries.clear();

    for (const core::CorrectionPattern* record : m_catalogue->cascade(m_kind, code)) {
        const auto found = std::ranges::find_if(
            m_entries, [record](const Entry& entry) { return entry.name == record->name; });
        if (found != m_entries.end()) {
            found->records.push_back(record);
            found->openedChecked = found->openedChecked || core::patternEnabled(*record, settings);
            found->box->setChecked(found->openedChecked);
            continue;
        }
        auto* box = new QCheckBox{QString::fromStdString(record->name), this};
        box->setObjectName(QString::fromStdString(record->name));
        const bool opened = core::patternEnabled(*record, settings);
        box->setChecked(opened);
        connect(box, &QCheckBox::toggled, this, &PatternList::changed);
        m_layout->addWidget(box);
        m_entries.push_back(
            Entry{.name = record->name, .box = box, .records = {record}, .openedChecked = opened});
    }
}

bool PatternList::touched(const Entry& entry) {
    return entry.box->isChecked() != entry.openedChecked;
}

std::vector<core::PatternActivation> PatternList::activations() const {
    std::vector<core::PatternActivation> activations;
    for (const Entry& entry : m_entries) {
        // Untouched: whatever the settings held for its records stays as is,
        // since `shownRecords()` does not erase it either.
        if (!touched(entry))
            continue;
        for (const core::CorrectionPattern* record : entry.records) {
            if (record->enabled == entry.box->isChecked())
                continue; // agrees with the shipped default: no override to write
            core::PatternActivation activation{.kind = record->kind(),
                                               .code = record->code,
                                               .name = record->name,
                                               .enabled = entry.box->isChecked()};
            // Two records of the same code sharing a name share the same key:
            // one override already covers both.
            if (std::ranges::find(activations, activation) == activations.end())
                activations.push_back(std::move(activation));
        }
    }
    return activations;
}

std::vector<const core::CorrectionPattern*> PatternList::shownRecords() const {
    std::vector<const core::CorrectionPattern*> shown;
    for (const Entry& entry : m_entries) {
        // A box merely shown is not the user's word on its records: a shared
        // box checked under `Latn-en` must not erase a `Latn` override.
        if (touched(entry))
            shown.insert(shown.end(), entry.records.begin(), entry.records.end());
    }
    return shown;
}

void PatternList::foldInto(std::vector<core::PatternActivation>& activations) const {
    core::foldActivations(activations, shownRecords(), this->activations());
}

} // namespace subedit::gui
