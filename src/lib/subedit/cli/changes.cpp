#include <subedit/cli/changes.hpp>

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

} // namespace

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
        out += file;
        out += ": subtitle " + std::to_string(change.subtitle);
        if (change.document == core::Document::Translation) {
            out += " (translation)";
        }
        if (!change.after) {
            out += " (removed)";
        }
        out += '\n';
        appendLines(out, "- ", change.before);
        if (change.after) {
            appendLines(out, "+ ", *change.after);
        }
    }
    return out;
}

} // namespace subedit::cli
