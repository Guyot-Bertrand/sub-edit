#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/text/spell_check_walk.hpp>

#include <utility>

namespace subedit::core {

SpellCheckWalk::SpellCheckWalk(SpellChecker checker, std::vector<CorrectionTarget> targets)
    : m_navigator(std::move(checker)), m_targets(std::move(targets)) {
    for (const CorrectionTarget& target : m_targets) {
        std::vector<SubtitleIndex>& indices = m_indices.emplace_back();
        for (const SubtitleIndex index : target.selection.indices())
            indices.push_back(index);
    }
}

bool SpellCheckWalk::loadNext() {
    while (m_target < m_targets.size()) {
        if (m_next >= m_indices[m_target].size()) {
            ++m_target;
            m_next = 0;
            continue;
        }
        const CorrectionTarget& target = m_targets[m_target];
        const SubtitleIndex index = m_indices[m_target][m_next++];
        std::string text = target.project->subtitleAt(index).text(target.document);
        m_current = Current{.project = target.project,
                            .index = index,
                            .document = target.document,
                            .original = text};
        m_navigator.reset(std::move(text));
        return true;
    }
    return false;
}

std::optional<ProposedCorrection> SpellCheckWalk::correctionOfCurrent() const {
    if (!m_current.has_value() || m_navigator.text() == m_current->original)
        return std::nullopt;
    return ProposedCorrection{.project = m_current->project,
                              .index = m_current->index,
                              .document = m_current->document,
                              .original = m_current->original,
                              .proposed = m_navigator.text()};
}

void SpellCheckWalk::leave() {
    if (std::optional<ProposedCorrection> done = correctionOfCurrent())
        m_left.push_back(std::move(*done));
    m_current.reset();
}

std::optional<SpellStop> SpellCheckWalk::advance() {
    for (;;) {
        if (!m_current.has_value() && !loadNext())
            return std::nullopt;
        if (m_current.has_value() && m_navigator.next().has_value()) {
            return SpellStop{.project = m_current->project,
                             .index = m_current->index,
                             .document = m_current->document,
                             .text = m_navigator.text(),
                             .pos = m_navigator.pos(),
                             .endPos = m_navigator.endPos(),
                             .word = m_navigator.word()};
        }
        leave();
    }
}

void SpellCheckWalk::resumeWithText(std::string text) {
    m_navigator.reset(std::move(text));
}

std::vector<ProposedCorrection> SpellCheckWalk::corrections() const {
    std::vector<ProposedCorrection> all = m_left;
    if (std::optional<ProposedCorrection> current = correctionOfCurrent())
        all.push_back(std::move(*current));
    return all;
}

} // namespace subedit::core
