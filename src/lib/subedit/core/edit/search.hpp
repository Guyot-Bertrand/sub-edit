#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/config/search_options.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

class Project;
class Selection;

/// Why a pattern cannot be searched for.
struct PatternError {
    enum class Kind {
        /// Nothing was typed: there is nothing to look for.
        Empty,
        /// The text is not a regular expression ICU can read.
        InvalidExpression,
    };

    Kind kind = Kind::Empty;

    /// ICU's name for what went wrong, for an invalid expression.
    std::string reason{};

    friend bool operator==(const PatternError&, const PatternError&) = default;
};

/// A pattern, compiled once, and the options it was compiled with.
///
/// **One engine for both modes.** Plain text is a regular expression read
/// literally, so that ignoring the case means the same thing in both — and it
/// means ICU's case folding, which knows that « É » and « é » are one letter,
/// where a byte comparison would not.
///
/// A regular expression is read as Gaupol reads one: `.` matches a line break,
/// and `^` and `$` match at every line of a subtitle.
class SearchPattern {

public:
    /// Compiles `pattern` under `options`, or says why it cannot be.
    [[nodiscard]] static std::expected<SearchPattern, PatternError>
    compile(std::string_view pattern, SearchOptions options);

    ~SearchPattern();

    SearchPattern(const SearchPattern&) = delete;
    SearchPattern& operator=(const SearchPattern&) = delete;
    SearchPattern(SearchPattern&&) noexcept;
    SearchPattern& operator=(SearchPattern&&) noexcept;

    [[nodiscard]] SearchOptions options() const;

    /// The compiled form, which only the search reads.
    struct Compiled;

    [[nodiscard]] const Compiled& compiled() const { return *m_compiled; }

private:
    explicit SearchPattern(std::unique_ptr<Compiled> compiled);

    std::unique_ptr<Compiled> m_compiled;
};

/// Where a match lies: a subtitle, and a span of its **visible** text.
///
/// The offsets are bytes of the text without its tags — what the user reads,
/// and what the search looked in. They are not offsets into the stored text.
struct TextMatch {
    SubtitleIndex index;
    std::size_t start = 0;
    std::size_t end = 0;

    friend bool operator==(const TextMatch&, const TextMatch&) = default;
};

/// Returns the first match of `pattern` in the texts of `document` of `target`
/// that comes after `after`, or the first of all when `after` is nothing.
///
/// **One document at a time** — decision D8 of the translation phase. Looking
/// in both would make every match a pair of a subtitle and a document, and the
/// table that moves to it would have nowhere clear to go. The visible text is
/// read in the format of that document's file, so that a translation in another
/// dialect than the main text is read in its own.
///
/// **It wraps around**, as Gaupol does over one document: past the last match
/// of the target, it starts again from the top. Nothing comes back only when
/// the target holds no match at all.
///
/// **The text searched is the visible one** — the rule of the phase-10 spec.
/// A tag is not text: looking for `<i>` finds nothing, and looking for
/// `Bonjour` finds the one whose first half is in italics.
[[nodiscard]] std::optional<TextMatch> findNext(const Project& project,
                                                const Selection& target,
                                                Document document,
                                                const SearchPattern& pattern,
                                                const std::optional<TextMatch>& after);

/// Returns the match that comes before `before`, wrapping around the other
/// way: past the first, it starts again from the bottom.
[[nodiscard]] std::optional<TextMatch> findPrevious(const Project& project,
                                                    const Selection& target,
                                                    Document document,
                                                    const SearchPattern& pattern,
                                                    const std::optional<TextMatch>& before);

/// What replacing one match did.
struct ReplacedMatch {
    std::unique_ptr<Command> command{};

    /// Where the replacement now lies in the visible text: the next search
    /// starts after it, so that a replacement containing the pattern is not
    /// found again.
    TextMatch written;
};

/// Builds the command that replaces the match `match` by `replacement`, in the
/// text of `document` — the one the match was found in.
///
/// **In the stored text, and without breaking a tag** — the tag-aware parser of
/// #378 does the work, under the rules `recherche.cas` writes case by case.
/// `replacement` is read in the vocabulary of the document, so it may carry
/// tags of its own. For a regular expression, `$1` to `$9` stand for the
/// groups of the match, `$0` for all of it, `\n` for a line break, `\$` and `\\`
/// for themselves.
///
/// Returns **nothing when `match` is no longer a match** — the text changed
/// since it was found, and replacing what is there now would replace something
/// the user did not see. It also returns nothing when the replacement leaves
/// the stored text exactly as it was — a match replaced by itself — for there
/// is then nothing to undo.
[[nodiscard]] std::optional<ReplacedMatch> replaceMatch(const Project& project,
                                                        Document document,
                                                        const SearchPattern& pattern,
                                                        const TextMatch& match,
                                                        std::string_view replacement);

/// What replacing every match did.
struct ReplacedAll {
    /// Nothing when nothing matched.
    std::unique_ptr<Command> command{};

    /// How many matches were replaced in the subtitles whose text changed;
    /// zero when no text did.
    ///
    /// **A subtitle is counted whole or not at all.** When its text changes,
    /// `count` takes in every match of that subtitle, including those whose
    /// replacement, taken alone, changes nothing — a match replaced by itself,
    /// say, next to another that is really rewritten. The figure can therefore
    /// be higher than the number of replacements that took effect. Only the
    /// figure is affected: the command is built from the texts, not from this
    /// count, so no replacement is lost.
    std::size_t count = 0;

    /// How many matches were found and replaced, whether or not a replacement
    /// changed the text. Nothing matched is `matched == 0`; matches that
    /// changed nothing are `count == 0` with `matched > 0` — what tells "not
    /// found" from "nothing to change".
    std::size_t matched = 0;
};

/// Builds the command that replaces every match in the texts of `document` of
/// `target`, and writes nothing to the other one.
///
/// **One entry in the history, not one per match.** Matches are found in the
/// text as it is being rewritten, from the end of each replacement on — so a
/// replacement that contains the pattern is not found again.
///
/// A subtitle nothing matched keeps the very bytes it had.
[[nodiscard]] ReplacedAll replaceAll(const Project& project,
                                     const Selection& target,
                                     Document document,
                                     const SearchPattern& pattern,
                                     std::string_view replacement);

} // namespace subedit::core
