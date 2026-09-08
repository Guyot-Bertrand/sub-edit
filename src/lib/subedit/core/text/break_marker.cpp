#include <subedit/core/text/break_marker.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace subedit::core {

std::string textFromMarker(std::string_view line, std::string_view marker) {
    std::string text;
    std::size_t start = 0;
    while (true) {
        const std::size_t found = line.find(marker, start);
        if (found == std::string_view::npos) {
            text += line.substr(start);
            return text;
        }
        text += line.substr(start, found - start);
        text += '\n';
        start = found + marker.size();
    }
}

std::string textToMarker(std::string_view text, std::string_view marker) {
    std::string line;
    std::size_t start = 0;
    while (true) {
        const std::size_t feed = text.find('\n', start);
        if (feed == std::string_view::npos) {
            line += text.substr(start);
            return line;
        }
        line += text.substr(start, feed - start);
        line += marker;
        start = feed + 1;
    }
}

} // namespace subedit::core
