#include <subedit/cli/changes.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <iterator>
#include <set>
#include <utility>

namespace subedit::cli {

namespace {

/// `text` with `prefix` before each of its lines, each ended by a newline.
void appendLines(std::string& out, std::string_view prefix, std::string_view text) {
    std::size_t begin = 0;
    while (true) {
        const std::size_t end = text.find('\n', begin);
        out += prefix;
        out += text.substr(begin, end == std::string_view::npos ? end : end - begin);
        out += '\n';
        if (end == std::string_view::npos) {
            return;
        }
        begin = end + 1;
    }
}

[[nodiscard]] std::string_view nameOf(core::Document document) {
    return document == core::Document::Main ? "main" : "translation";
}

/// The line that opens a change in the text of a dry run.
[[nodiscard]] std::string_view headingOf(const TextChange& change) {
    const bool translation = change.document == core::Document::Translation;
    if (!change.after) {
        // TRANSLATORS: %1 is a file, %2 the number of a subtitle.
        return translation ? core::translate("%1: subtitle %2 (translation) (removed)")
                           : core::translate("%1: subtitle %2 (removed)");
    }
    // TRANSLATORS: %1 is a file, %2 the number of a subtitle.
    return translation ? core::translate("%1: subtitle %2 (translation)")
                       : core::translate("%1: subtitle %2");
}

} // namespace

std::vector<std::string> textsOf(const core::Project& project, core::Document document) {
    std::vector<std::string> texts;
    texts.reserve(project.count());
    for (const core::Subtitle& subtitle : project.subtitles())
        texts.push_back(subtitle.text(document));
    return texts;
}

std::vector<TextChange> changesOfCommand(const core::Project& after,
                                         const std::vector<std::string>& before,
                                         const std::vector<core::Change>& described,
                                         core::Document document) {
    std::set<std::size_t> removed;
    std::set<std::size_t> rewritten;
    for (const core::Change& change : described) {
        std::set<std::size_t>& into =
            change.kind == core::ChangeKind::Removal ? removed : rewritten;
        for (const core::SubtitleIndex index : change.subtitles.indices())
            into.insert(index.value());
    }

    std::vector<TextChange> changes;
    changes.reserve(removed.size() + rewritten.size());
    // Ascending, whichever of the two a subtitle is: it is the order of the file.
    for (std::size_t at = 0; at < before.size(); ++at) {
        const bool taken = removed.contains(at);
        if (!taken && !rewritten.contains(at))
            continue;

        TextChange change{.subtitle = at + 1, .document = document, .before = before[at]};
        if (!taken) {
            const std::size_t now =
                at -
                static_cast<std::size_t>(std::distance(removed.begin(), removed.lower_bound(at)));
            change.after = after.subtitleAt(core::SubtitleIndex::fromValue(now)).text(document);
        }
        changes.push_back(std::move(change));
    }
    return changes;
}

Json changesOf(const std::vector<TextChange>& changes) {
    Json array = Json::array();
    for (const TextChange& change : changes) {
        Json one = Json::object();
        one.set("subtitle", change.subtitle);
        one.set("document", nameOf(change.document));
        one.set("before", change.before);
        one.set("after", change.after ? Json{*change.after} : Json{});
        array.push(std::move(one));
    }
    return array;
}

std::string textOf(std::string_view file, const std::vector<TextChange>& changes) {
    std::string out;
    for (const TextChange& change : changes) {
        out += core::substitute(headingOf(change), {file, std::to_string(change.subtitle)});
        out += '\n';
        appendLines(out, "- ", change.before);
        if (change.after) {
            appendLines(out, "+ ", *change.after);
        }
    }
    return out;
}

} // namespace subedit::cli
