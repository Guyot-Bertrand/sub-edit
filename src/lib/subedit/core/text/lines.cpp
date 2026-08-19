#include <subedit/core/text/lines.hpp>

#include <cstddef>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kBlanks = " \t";

} // namespace

std::string_view trimmedBlanks(std::string_view text) {
    const std::size_t first = text.find_first_not_of(kBlanks);
    if (first == std::string_view::npos)
        return {};
    return text.substr(first, text.find_last_not_of(kBlanks) - first + 1);
}

std::vector<std::string_view> splitLines(std::string_view content) {
    std::vector<std::string_view> lines;

    std::size_t start = 0;
    while (start <= content.size()) {
        const std::size_t ending = content.find_first_of("\r\n", start);
        if (ending == std::string_view::npos) {
            // The last line, when the file does not end with an ending. An
            // empty remainder here means the file did end with one, and adding
            // a line for it would invent a blank line that is not in the file.
            if (start < content.size())
                lines.push_back(content.substr(start));
            break;
        }

        lines.push_back(content.substr(start, ending - start));

        // CRLF is one ending, not two.
        const bool isCarriageReturnLineFeed =
            content[ending] == '\r' && ending + 1 < content.size() && content[ending + 1] == '\n';
        start = ending + (isCarriageReturnLineFeed ? 2 : 1);
    }

    return lines;
}

} // namespace subedit::core
