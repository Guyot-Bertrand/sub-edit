#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

SetTextCommand::SetTextCommand(const Project& project,
                               SubtitleIndex index,
                               Document document,
                               std::string text)
    : m_index(index),
      m_document(document),
      m_newText(std::move(text)),
      m_oldText(project.subtitleAt(index).text(document)) {}

void SetTextCommand::apply(Project& project) {
    project.subtitleAt(m_index).text(m_document) = m_newText;
}

void SetTextCommand::revert(Project& project) {
    project.subtitleAt(m_index).text(m_document) = m_oldText;
}

std::vector<Change> SetTextCommand::describe() const {
    const ChangeKind kind =
        m_document == Document::Main ? ChangeKind::MainText : ChangeKind::TranslationText;

    return {Change{.kind = kind, .subtitles = Selection::range(m_index, m_index)}};
}

} // namespace subedit::core
