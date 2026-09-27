#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/text/utf8.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

double CharacterLineMeasure::lengthOf(std::string_view text) const {
    double count = 0.0;
    for (std::size_t at = 0; at < text.size(); at = nextCodePoint(text, at))
        count += 1.0;
    return count;
}

CachedLineMeasure::CachedLineMeasure(const LineMeasure& underlying) : m_underlying(&underlying) {}

double CachedLineMeasure::lengthOf(std::string_view text) const {
    const auto found = m_cache.find(text);
    if (found != m_cache.end())
        return found->second;

    const double length = m_underlying->lengthOf(text);
    m_cache.emplace(std::string{text}, length);
    return length;
}

} // namespace subedit::core
