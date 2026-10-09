#include <subedit/core/i18n/messages.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

namespace {

std::shared_ptr<const Catalogue>& installed() {
    static std::shared_ptr<const Catalogue> catalogue;
    return catalogue;
}

} // namespace

void installCatalogue(std::shared_ptr<const Catalogue> catalogue) {
    installed() = std::move(catalogue);
}

std::string_view translate(std::string_view text) {
    return translateIn({}, text);
}

std::string_view translateIn(std::string_view context, std::string_view text) {
    if (const auto& catalogue = installed()) {
        if (const auto found = catalogue->find(context, text)) {
            return *found;
        }
    }
    return text;
}

std::string_view
translatePlural(std::string_view singular, std::string_view plural, std::uint64_t n) {
    if (const auto& catalogue = installed()) {
        if (const auto found = catalogue->findPlural(singular, n)) {
            return *found;
        }
    }
    return n == 1 ? singular : plural;
}

std::string substitute(std::string_view text, std::initializer_list<std::string_view> arguments) {
    const std::vector<std::string_view> values(arguments);
    std::string result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (c == '%' && i + 1 < text.size() && text[i + 1] >= '1' && text[i + 1] <= '9') {
            const auto index = static_cast<std::size_t>(text[i + 1] - '1');
            if (index < values.size()) {
                result += values[index];
                ++i;
                continue;
            }
        }
        result += c;
    }
    return result;
}

} // namespace subedit::core
