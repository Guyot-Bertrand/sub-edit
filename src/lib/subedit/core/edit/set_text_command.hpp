#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <string>
#include <vector>

namespace subedit::core {

class Project;

/// Replaces the text of one document of one subtitle.
///
/// What it retains to undo itself is the **old text, and nothing else** — not
/// the subtitle, not the project. That is what ADR 0010 calls capturing the
/// strict minimum, and it is why an undo stack of a thousand entries does not
/// hold a thousand copies of the file.
///
/// A text that is set to what it already was is still an operation: whoever
/// built the command asked for it, and the history has to stay a faithful
/// account of what was done. Deciding that an edit was pointless belongs to
/// the caller.
class SetTextCommand final : public Command {

public:
    /// Captures the text `index` currently holds in `document`.
    SetTextCommand(const Project& project,
                   SubtitleIndex index,
                   Document document,
                   std::string text);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::SetText; }

    /// Reports a change of the document it touched, and of that one only.
    ///
    /// The two texts of a subtitle are independent — only the positions are
    /// shared — so a change of the main text must not mark the translation as
    /// differing from its file.
    [[nodiscard]] std::vector<Change> describe() const override;

private:
    SubtitleIndex m_index;
    Document m_document;
    std::string m_newText;
    std::string m_oldText;
};

} // namespace subedit::core
