#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

class Project;
class Selection;

/// What a copy carries: **texts, and nothing else** — decision D6 of the
/// phase-10 spec.
///
/// Gaupol's clipboard is a list of strings, one per subtitle, and so is this.
/// No position, no subtitle as a whole: pasting positions would overwrite a
/// timing nobody asked to touch.
///
/// **One entry per row from the first selected to the last**, and a hole where
/// a row was not selected. A discontinuous copy keeps its shape: pasted, the row
/// in the gap is left as it was rather than overwritten.
///
/// **The format the texts were written in, when it is known.** A copy made in
/// this program knows it, and a paste into a document of another format
/// translates the tags — ADR 0031, at the clipboard's door. A text that came
/// from outside has no format, and is pasted as it is: there is nothing to
/// translate from nowhere.
struct ClipboardTexts {
    std::vector<std::optional<std::string>> texts{};
    std::optional<SubtitleFormat> format{};

    [[nodiscard]] bool isEmpty() const { return texts.empty(); }

    friend bool operator==(const ClipboardTexts&, const ClipboardTexts&) = default;
};

/// Returns the texts of `document` over `selection`, in the format of `project`.
///
/// Empty for an empty selection.
[[nodiscard]] ClipboardTexts
copyTexts(const Project& project, const Selection& selection, Document document);

/// Returns what the system clipboard receives: the texts glued by a blank line,
/// a hole written as an empty text.
///
/// Gaupol's `get_string`, and the reason it is plain text: a copy made here can
/// be pasted anywhere, and a text copied anywhere can be pasted here.
[[nodiscard]] std::string plainTextOf(const ClipboardTexts& clipboard);

/// Returns the texts a plain string carries, split on blank lines, with no
/// format.
///
/// Gaupol's `set_string`: an empty piece is a hole, which is also what a hole
/// became on the way out — so a copy read back is the copy it was.
[[nodiscard]] ClipboardTexts textsFromPlain(std::string_view plain);

/// Builds the command that empties the texts of `document` over `selection`.
///
/// Copying is the caller's, and comes first: this is the half of « cut » that
/// changes the document. Returns **nothing when every text is already empty**,
/// which is not an operation to undo.
[[nodiscard]] std::unique_ptr<Command>
cutTexts(const Project& project, const Selection& selection, Document document);

/// What pasting will do, read before it is applied.
struct PastedTexts {
    /// Nothing when the paste would change nothing.
    std::unique_ptr<Command> command{};

    /// How many blank rows are laid down past the end to receive the texts.
    std::size_t inserted = 0;

    /// How many tags did not survive the translation into the document's
    /// format. Zero for texts of no format or of the document's own.
    std::size_t droppedTags = 0;
};

/// Builds the command that writes `clipboard` into `document` from `at` down.
///
/// **No position moves** — `paste_texts`. What runs past the last row lands in
/// blank rows laid down at the end, as an insertion lays them, and the count is
/// handed back so that it can be said.
///
/// A hole leaves its row as it was. Texts from another format are translated
/// by `convertMarkup`, and what fell is counted.
///
/// **One entry in the history** however many rows it writes and lays down.
[[nodiscard]] PastedTexts pasteTexts(const Project& project,
                                     const ClipboardTexts& clipboard,
                                     SubtitleIndex at,
                                     Document document);

} // namespace subedit::core
