#pragma once

// Walking the misspelt words of several projects, subtitle after subtitle —
// what Gaupol's spell-check dialog does over its pages, at the core: decision
// D6 of the spec of phase 12, issue #509.
//
// **It touches no project.** The words are read through each target's
// `Project`, the texts the user corrects live in the navigator, and what comes
// out is `ProposedCorrection`s: the caller composes them with
// `applyCorrections`, one command per project, the history's own road.

#include <subedit/core/model/document.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/spell_check_navigator.hpp>
#include <subedit/core/text/spell_checker.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// One unknown word, and where the walk stands: the text it is in, as
/// the navigator holds it now, and the word's byte range within it.
struct SpellStop {
    const Project* project = nullptr;
    SubtitleIndex index;
    Document document = Document::Main;
    std::string text;
    std::size_t pos = 0;
    std::size_t endPos = 0;
    std::string word;
};

class SpellCheckWalk {

public:
    /// `targets` are walked in order, each one's selection ascending.
    SpellCheckWalk(SpellChecker checker, std::vector<CorrectionTarget> targets);

    /// The next unknown word, moving on to the next subtitle — and the next
    /// target — when a text has no more. Absent at the end.
    [[nodiscard]] std::optional<SpellStop> advance();

    /// The gestures, on the word `advance` last stopped at.
    void ignore() { m_navigator.ignore(); }

    void ignoreAll() { m_navigator.ignoreAll(); }

    void add() { m_navigator.add(); }

    void replace(std::string_view replacement) { m_navigator.replace(replacement); }

    void replaceAll(std::string_view replacement) { m_navigator.replaceAll(replacement); }

    void joinWithPrevious() { m_navigator.joinWithPrevious(); }

    void joinWithNext() { m_navigator.joinWithNext(); }

    [[nodiscard]] bool spaceBefore() const { return m_navigator.spaceBefore(); }

    [[nodiscard]] bool spaceAfter() const { return m_navigator.spaceAfter(); }

    /// Gaupol's "save and resume": `text`, typed by the user in place of the
    /// current one, is what the walk goes on with — from its start.
    void resumeWithText(std::string text);

    [[nodiscard]] std::vector<std::string> suggest() const { return m_navigator.suggest(); }

    /// What has been corrected so far: the texts already left **and** the
    /// current one as the gestures have left it — nothing of what has not
    /// been reached. Only texts that really changed.
    [[nodiscard]] std::vector<ProposedCorrection> corrections() const;

    /// For the caller to write the replacement list from.
    [[nodiscard]] const SpellChecker& checker() const { return m_navigator.checker(); }

private:
    struct Current {
        const Project* project = nullptr;
        SubtitleIndex index;
        Document document = Document::Main;
        std::string original;
    };

    /// Loads the next subtitle of the walk into the navigator; false at the end.
    [[nodiscard]] bool loadNext();
    /// Records the current text as left, if it changed.
    void leave();
    [[nodiscard]] std::optional<ProposedCorrection> correctionOfCurrent() const;

    SpellCheckNavigator m_navigator;
    std::vector<CorrectionTarget> m_targets;
    std::vector<std::vector<SubtitleIndex>> m_indices;
    std::size_t m_target = 0;
    std::size_t m_next = 0; // the next subtitle of the current target
    std::optional<Current> m_current;
    std::vector<ProposedCorrection> m_left;
};

} // namespace subedit::core
