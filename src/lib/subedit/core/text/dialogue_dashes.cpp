#include <subedit/core/text/dialogue_dashes.hpp>
#include <subedit/core/text/markup_parser.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// What a subtitle writes for a dialogue dash.
///
/// Three, and Gaupol accepts the same three: the hyphen a keyboard gives, and
/// the two dashes a typographer gives — `–` and `—`, in their UTF-8 bytes.
constexpr std::string_view kHyphen = "-";
constexpr std::string_view kEnDash = "–";
constexpr std::string_view kEmDash = "—";

/// How many bytes of dash open `line`, or zero.
[[nodiscard]] std::size_t dashAt(std::string_view line) {
    for (const std::string_view dash : {kHyphen, kEnDash, kEmDash}) {
        if (line.starts_with(dash))
            return dash.size();
    }
    return 0;
}

/// The line without its dash and the spaces that followed.
[[nodiscard]] std::string_view undashed(std::string_view line) {
    const std::size_t dash = dashAt(line);
    if (dash == 0)
        return line;
    std::string_view rest = line.substr(dash);
    while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t'))
        rest.remove_prefix(1);
    return rest;
}

/// Walks the lines of `text`, handing each to `visit` with whether more follow.
template<typename Visit>
void overLines(std::string_view text, Visit visit) {
    std::size_t start = 0;
    while (true) {
        const std::size_t ending = text.find('\n', start);
        const bool more = ending != std::string_view::npos;
        visit(more ? text.substr(start, ending - start) : text.substr(start), more);
        if (!more)
            return;
        start = ending + 1;
    }
}

/// Whether any line of a tag-free text lacks its dash.
[[nodiscard]] bool wantsDashes(std::string_view text) {
    bool missing = false;
    overLines(text, [&missing](std::string_view line, bool) {
        // An empty line says nothing: it is not a replica without a dash, it is
        // no replica at all, and counting it would make a dialogue ask to be
        // dashed again for ever.
        if (!line.empty() && dashAt(line) == 0)
            missing = true;
    });
    return missing;
}

/// Where each line of `text` begins and ends, ends exclusive.
[[nodiscard]] std::vector<std::pair<std::size_t, std::size_t>> lineBounds(std::string_view text) {
    std::vector<std::pair<std::size_t, std::size_t>> bounds;
    std::size_t start = 0;
    while (true) {
        const std::size_t ending = text.find('\n', start);
        if (ending == std::string_view::npos) {
            bounds.emplace_back(start, text.size());
            return bounds;
        }
        bounds.emplace_back(start, ending);
        start = ending + 1;
    }
}

/// Puts `wanted` at the head of every line, in place of the dash it had.
///
/// **Line by line, and from the last to the first.** A wholesale rewrite would
/// leave the tags at the offsets they had, and two characters put at the head
/// of a subtitle would push its closing tag two letters into a word. Each line
/// is its own edit, and going backwards keeps the offsets of the ones before it
/// valid.
[[nodiscard]] std::string
headed(std::string_view text, SubtitleFormat format, std::string_view wanted) {
    MarkupParser parser{text, format};

    const std::vector<std::pair<std::size_t, std::size_t>> bounds = lineBounds(parser.visible());
    bool touched = false;
    for (std::size_t which = bounds.size(); which-- > 0;) {
        const auto [start, ending] = bounds[which];
        const std::string_view line = parser.visible().substr(start, ending - start);
        // An empty line is no replica at all, and gains nothing.
        if (line.empty())
            continue;

        const std::size_t worn = line.size() - undashed(line).size();
        if (worn == wanted.size() && line.starts_with(wanted))
            continue;

        parser.transform(start, worn, wanted);
        touched = true;
    }
    return touched ? parser.text() : std::string{text};
}

} // namespace

bool wantsDialogueDashes(std::string_view text, SubtitleFormat format) {
    return wantsDashes(MarkupParser{text, format}.visible());
}

std::string withDialogueDashes(std::string_view text, SubtitleFormat format) {
    return headed(text, format, "- ");
}

std::string withoutDialogueDashes(std::string_view text, SubtitleFormat format) {
    return headed(text, format, "");
}

} // namespace subedit::core
