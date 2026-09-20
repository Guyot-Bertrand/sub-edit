#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <cstddef>
#include <memory>
#include <span>

namespace subedit::core {

/// How the lines of a translation file are matched to the subtitles of the main
/// document — decision D4 of the phase-11 spec.
enum class TranslationMethod {
    /// The middle of each line is compared to the bounds of the subtitles. The
    /// default, because one line missing does not shift the ones after it.
    Position,

    /// The *n*th line goes to the *n*th subtitle. Positions are not looked at.
    Number,
};

/// What opening a translation did — said, and not left for the user to find
/// out (ADR 0008).
struct TranslationOutcome {
    /// Lines that found a subtitle of the main document.
    std::size_t attached = 0;

    /// Lines that found none, and gave one of their own.
    std::size_t born = 0;

    /// Subtitles of the main document that no line reached.
    std::size_t untranslated = 0;

    /// Lines that came after a later one in the file.
    std::size_t outOfOrder = 0;

    /// Whether everything found its place: no subtitle born, none left alone,
    /// nothing out of order. It is what decides between the status bar and a
    /// box to close.
    [[nodiscard]] bool isClean() const { return born == 0 && untranslated == 0 && outOfOrder == 0; }

    friend bool operator==(const TranslationOutcome&, const TranslationOutcome&) = default;
};

/// The opening of a translation, planned and not yet done.
struct AttachedTranslation {
    /// One command, and one entry in the history. Never null.
    std::unique_ptr<Command> command{};

    TranslationOutcome outcome{};
};

/// Plans the attachment of `lines` — the subtitles read from a translation
/// file, positions and text — to the main document of `project`, by `method`.
///
/// **One command, one entry in the history**, that undoes everything: the texts
/// put back, the subtitles it gave birth to taken away, the file of the
/// translation detached. Gaupol does not undo an opening; opening again by the
/// other method is done after undoing the first.
///
/// **Both methods go through the lines in the order of their positions**, and
/// sort nothing — not the file, not the document — ADR 0012. How many lines
/// were out of order is part of what is said.
///
/// **A subtitle a line gives birth to keeps that line's positions, by either
/// method.** Gaupol invents three seconds after the last subtitle when it goes
/// by number, though the file states the line's own.
///
/// **What the document held as a translation is erased**, whether or not a line
/// reaches the subtitle. Gaupol does the same.
///
/// `source` is what the translation file looked like, and is kept as the file of
/// the translation (ADR 0032).
[[nodiscard]] AttachedTranslation attachTranslation(const Project& project,
                                                    std::span<const Subtitle> lines,
                                                    const SourceFile& source,
                                                    TranslationMethod method);

} // namespace subedit::core
