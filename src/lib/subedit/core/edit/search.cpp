#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/config/search_options.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/markup_parser.hpp>

#include <unicode/parseerr.h>
#include <unicode/regex.h>
#include <unicode/stringpiece.h>
#include <unicode/uregex.h>
#include <unicode/utext.h>
#include <unicode/utypes.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

struct SearchPattern::Compiled {
    std::unique_ptr<icu::RegexPattern> regex;
    SearchOptions options;
};

namespace {

/// A span of visible text, in bytes.
struct Span {
    std::size_t start = 0;
    std::size_t end = 0;

    friend bool operator==(const Span&, const Span&) = default;
};

/// Tells whether ICU reported a failure.
///
/// ICU answers in `UBool`, a signed char; the comparison gives a `bool` without
/// a conversion nobody wrote.
[[nodiscard]] bool failed(UErrorCode status) {
    return status > U_ZERO_ERROR;
}

/// A UTF-8 text as ICU reads it, closed when it goes.
class Utf8Text {

public:
    explicit Utf8Text(std::string_view text) {
        UErrorCode status = U_ZERO_ERROR;
        // On a failure ICU hands back nothing, which `get` then reports: the
        // one caller checks it before reading anything.
        m_text =
            utext_openUTF8(nullptr, text.data(), static_cast<std::int64_t>(text.size()), &status);
    }

    ~Utf8Text() { utext_close(m_text); }

    Utf8Text(const Utf8Text&) = delete;
    Utf8Text& operator=(const Utf8Text&) = delete;
    Utf8Text(Utf8Text&&) = delete;
    Utf8Text& operator=(Utf8Text&&) = delete;

    [[nodiscard]] UText* get() const { return m_text; }

private:
    UText* m_text = nullptr;
};

/// The offset of the character after the one at `at`, or one past the end.
///
/// What a search moves by after an empty match: without it, a pattern that
/// matches nothing — `^`, `x*` — would find the same place forever.
[[nodiscard]] std::size_t nextCharacter(std::string_view text, std::size_t at) {
    constexpr unsigned kContinuationMask = 0xC0U;
    constexpr unsigned kContinuation = 0x80U;
    ++at;
    while (at < text.size() &&
           (static_cast<unsigned char>(text[at]) & kContinuationMask) == kContinuation)
        ++at;
    return at;
}

/// The replacement a regular expression's match makes of `replacement`.
[[nodiscard]] std::string expanded(const icu::RegexMatcher& matcher, std::string_view replacement) {
    std::string out;
    for (std::size_t at = 0; at < replacement.size(); ++at) {
        const char here = replacement[at];
        const bool hasNext = at + 1 < replacement.size();
        const char next = hasNext ? replacement[at + 1] : '\0';

        if (here == '\\' && hasNext) {
            out += next == 'n' ? '\n' : next;
            ++at;
        } else if (here == '$' && hasNext && next >= '0' && next <= '9') {
            // A group the expression does not have stands for nothing.
            const std::int32_t group = next - '0';
            if (group <= matcher.groupCount()) {
                UErrorCode status = U_ZERO_ERROR;
                matcher.group(group, status).toUTF8String(out);
            }
            ++at;
        } else {
            out += here;
        }
    }
    return out;
}

/// Resets `matcher` on `text` and finds the first match at or after `from`.
///
/// **One answer for « nothing found » and « nothing to search with »**: a
/// matcher or a text ICU could not make is a search that finds nothing, and
/// writing the two apart would leave a branch no test can reach.
[[nodiscard]] bool
foundFrom(icu::RegexMatcher* matcher, UText* text, std::size_t from, UErrorCode& status) {
    const bool ready = matcher != nullptr && text != nullptr && !failed(status);
    if (ready)
        matcher->reset(text);
    return ready && matcher->find(static_cast<std::int64_t>(from), status) != 0 && !failed(status);
}

/// The first match at or after `from` in `visible`, and what `replacement`
/// becomes there.
struct Found {
    Span span;
    std::string replacement;
};

[[nodiscard]] std::optional<Found> firstFrom(std::string_view visible,
                                             const SearchPattern::Compiled& compiled,
                                             std::size_t from,
                                             std::string_view replacement) {
    if (from > visible.size())
        return std::nullopt;

    const Utf8Text text{visible};
    UErrorCode status = U_ZERO_ERROR;
    const std::unique_ptr<icu::RegexMatcher> matcher{compiled.regex->matcher(status)};
    if (!foundFrom(matcher.get(), text.get(), from, status))
        return std::nullopt;

    const auto start = static_cast<std::size_t>(matcher->start64(status));
    const auto end = static_cast<std::size_t>(matcher->end64(status));
    std::string written =
        compiled.options.regex ? expanded(*matcher, replacement) : std::string{replacement};
    return Found{.span = {.start = start, .end = end}, .replacement = std::move(written)};
}

/// Every match in `visible`, in order.
[[nodiscard]] std::vector<Span> spansIn(std::string_view visible,
                                        const SearchPattern::Compiled& compiled) {
    std::vector<Span> spans;
    std::size_t from = 0;
    while (const std::optional<Found> found = firstFrom(visible, compiled, from, {})) {
        spans.push_back(found->span);
        from = found->span.end > found->span.start ? found->span.end
                                                   : nextCharacter(visible, found->span.end);
    }
    return spans;
}

/// Every match in the main text of the subtitle at `index`.
[[nodiscard]] std::vector<Span>
spansAt(const Project& project, SubtitleIndex index, const SearchPattern::Compiled& compiled) {
    const MarkupParser parser{project.subtitleAt(index).mainText, project.sourceFile().format};
    return spansIn(parser.visible(), compiled);
}

[[nodiscard]] std::vector<SubtitleIndex> indicesOf(const Selection& target) {
    std::vector<SubtitleIndex> indices;
    indices.reserve(target.count());
    for (const SubtitleIndex index : target.indices())
        indices.push_back(index);
    return indices;
}

/// Where a walk over `order` starts for a search from `from`: the position of
/// its subtitle, or of the first one after it when the target changed and no
/// longer holds it.
[[nodiscard]] std::size_t startOf(const std::vector<SubtitleIndex>& order,
                                  const std::optional<TextMatch>& from,
                                  bool forward) {
    if (!from.has_value())
        return 0;

    for (std::size_t rank = 0; rank < order.size(); ++rank) {
        const std::size_t value = order[rank].value();
        if (forward ? value >= from->index.value() : value <= from->index.value())
            return rank;
    }
    return 0;
}

/// Walks `order` from `from`, once around, and returns the first match `pick`
/// chooses in each subtitle.
///
/// `pick` receives the spans of a subtitle and whether that subtitle is the one
/// the search started from — and, if so, whether the walk is back to it after
/// going all the way round.
template<typename Pick>
[[nodiscard]] std::optional<TextMatch> walk(const Project& project,
                                            const std::vector<SubtitleIndex>& order,
                                            const SearchPattern& pattern,
                                            const std::optional<TextMatch>& from,
                                            bool forward,
                                            Pick pick) {
    if (order.empty())
        return std::nullopt;

    const std::size_t first = startOf(order, from, forward);
    for (std::size_t step = 0; step <= order.size(); ++step) {
        const std::size_t rank = (first + step) % order.size();
        const SubtitleIndex index = order[rank];
        const bool origin = from.has_value() && index == from->index;
        const std::vector<Span> spans = spansAt(project, index, pattern.compiled());
        if (const std::optional<Span> chosen = pick(spans, origin, step); chosen.has_value())
            return TextMatch{.index = index, .start = chosen->start, .end = chosen->end};
    }
    return std::nullopt;
}

/// Rewrites every match of `compiled` in `text`, or only the one at `only`.
struct Rewritten {
    std::string text;
    std::size_t count = 0;
    Span written{};
};

[[nodiscard]] Rewritten rewrite(std::string_view text,
                                SubtitleFormat format,
                                const SearchPattern::Compiled& compiled,
                                std::string_view replacement,
                                const std::optional<Span>& only) {
    Rewritten rewritten{.text = std::string{text}};
    MarkupParser parser{text, format};

    std::size_t at = only.has_value() ? only->start : 0;
    while (const std::optional<Found> found =
               firstFrom(parser.visible(), compiled, at, replacement)) {
        if (only.has_value() && found->span != *only)
            break;

        parser.replace(found->span.start, found->span.end - found->span.start, found->replacement);
        ++rewritten.count;

        const std::size_t length = MarkupParser{found->replacement, format}.visible().size();
        rewritten.written = {.start = found->span.start, .end = found->span.start + length};
        if (only.has_value())
            break;

        // Past the replacement, and one character further after an empty
        // match, which would otherwise be found again at the same place.
        at = found->span.end > found->span.start
                 ? rewritten.written.end
                 : nextCharacter(parser.visible(), rewritten.written.end);
    }

    // A subtitle nothing matched keeps its bytes: the reassembly is not the
    // identity, and a search that finds nothing must not tidy a file.
    if (rewritten.count > 0)
        rewritten.text = parser.text();
    return rewritten;
}

} // namespace

SearchPattern::SearchPattern(std::unique_ptr<Compiled> compiled)
    : m_compiled(std::move(compiled)) {}

SearchPattern::~SearchPattern() = default;

SearchPattern::SearchPattern(SearchPattern&&) noexcept = default;

SearchPattern& SearchPattern::operator=(SearchPattern&&) noexcept = default;

SearchOptions SearchPattern::options() const {
    return m_compiled->options;
}

std::expected<SearchPattern, PatternError> SearchPattern::compile(std::string_view pattern,
                                                                  SearchOptions options) {
    if (pattern.empty())
        return std::unexpected{PatternError{.kind = PatternError::Kind::Empty}};

    // Gaupol's flags for an expression: `.` crosses a line break, and `^` and
    // `$` hold at every line.
    std::uint32_t flags = options.regex ? (UREGEX_DOTALL | UREGEX_MULTILINE)
                                        : static_cast<std::uint32_t>(UREGEX_LITERAL);
    if (options.ignoreCase)
        flags |= UREGEX_CASE_INSENSITIVE;

    UParseError where{};
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::RegexPattern> regex{
        icu::RegexPattern::compile(icu::UnicodeString::fromUTF8(icu::StringPiece{
                                       pattern.data(), static_cast<std::int32_t>(pattern.size())}),
                                   flags,
                                   where,
                                   status)};
    if (failed(status) || regex == nullptr) {
        return std::unexpected{PatternError{.kind = PatternError::Kind::InvalidExpression,
                                            .reason = std::string{u_errorName(status)}}};
    }

    return SearchPattern{
        std::make_unique<Compiled>(Compiled{.regex = std::move(regex), .options = options})};
}

std::optional<TextMatch> findNext(const Project& project,
                                  const Selection& target,
                                  const SearchPattern& pattern,
                                  const std::optional<TextMatch>& after) {
    const std::vector<SubtitleIndex> order = indicesOf(target);
    const Span last = after.has_value() ? Span{.start = after->start, .end = after->end} : Span{};

    return walk(project,
                order,
                pattern,
                after,
                true,
                [&last](const std::vector<Span>& spans, bool origin, std::size_t step) {
                    std::optional<Span> chosen;
                    for (const Span& span : spans) {
                        // In the subtitle the search came from, only what lies
                        // after the last match — until the walk comes back to
                        // it from the top.
                        if (origin && step == 0 && (span.start < last.end || span == last))
                            continue;
                        chosen = span;
                        break;
                    }
                    return chosen;
                });
}

std::optional<TextMatch> findPrevious(const Project& project,
                                      const Selection& target,
                                      const SearchPattern& pattern,
                                      const std::optional<TextMatch>& before) {
    std::vector<SubtitleIndex> order = indicesOf(target);
    std::ranges::reverse(order);
    const Span last =
        before.has_value() ? Span{.start = before->start, .end = before->end} : Span{};

    return walk(project,
                order,
                pattern,
                before,
                false,
                [&last](const std::vector<Span>& spans, bool origin, std::size_t step) {
                    std::optional<Span> chosen;
                    for (const Span& span : spans) {
                        if (origin && step == 0 && (span.end > last.start || span == last))
                            continue;
                        chosen = span;
                    }
                    return chosen;
                });
}

std::optional<ReplacedMatch> replaceMatch(const Project& project,
                                          const SearchPattern& pattern,
                                          const TextMatch& match,
                                          std::string_view replacement) {
    if (match.index.value() >= project.count())
        return std::nullopt;

    const std::string& text = project.subtitleAt(match.index).mainText;
    const Rewritten rewritten = rewrite(text,
                                        project.sourceFile().format,
                                        pattern.compiled(),
                                        replacement,
                                        Span{.start = match.start, .end = match.end});
    if (rewritten.count == 0)
        return std::nullopt;

    std::vector<std::unique_ptr<Command>> commands;
    if (rewritten.text != text) {
        commands.push_back(
            std::make_unique<SetTextCommand>(project, match.index, Document::Main, rewritten.text));
    }

    ReplacedMatch replaced{.command = nullptr,
                           .written = TextMatch{.index = match.index,
                                                .start = rewritten.written.start,
                                                .end = rewritten.written.end}};
    if (!commands.empty())
        replaced.command =
            std::make_unique<CompositeCommand>(CommandKind::Replace, std::move(commands));
    return replaced;
}

ReplacedAll replaceAll(const Project& project,
                       const Selection& target,
                       const SearchPattern& pattern,
                       std::string_view replacement) {
    ReplacedAll replaced;
    std::vector<std::unique_ptr<Command>> commands;

    for (const SubtitleIndex index : target.indices()) {
        const std::string& text = project.subtitleAt(index).mainText;
        const Rewritten rewritten = rewrite(
            text, project.sourceFile().format, pattern.compiled(), replacement, std::nullopt);
        replaced.count += rewritten.count;
        if (rewritten.text != text)
            commands.push_back(
                std::make_unique<SetTextCommand>(project, index, Document::Main, rewritten.text));
    }

    if (!commands.empty())
        replaced.command =
            std::make_unique<CompositeCommand>(CommandKind::ReplaceAll, std::move(commands));
    return replaced;
}

} // namespace subedit::core
