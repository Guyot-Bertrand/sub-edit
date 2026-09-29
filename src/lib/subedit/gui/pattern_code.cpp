// Decomposing pattern codes into script, language, country — issue #505, task 5.

#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_code.hpp>

#include <algorithm>

namespace subedit::gui {

PatternCodeParts splitCode(std::string_view code) {
    PatternCodeParts parts;
    const std::size_t first = code.find('-');
    if (first == std::string_view::npos) {
        parts.script = std::string{code};
        return parts;
    }
    parts.script = std::string{code.substr(0, first)};
    const std::size_t second = code.find('-', first + 1);
    if (second == std::string_view::npos) {
        parts.language = std::string{code.substr(first + 1)};
        return parts;
    }
    parts.language = std::string{code.substr(first + 1, second - first - 1)};
    parts.country = std::string{code.substr(second + 1)};
    return parts;
}

std::string joinCode(const PatternCodeParts& parts) {
    if (parts.script.empty())
        return "Zyyy";
    std::string code = parts.script;
    if (parts.language.empty())
        return code;
    code += "-" + parts.language;
    if (parts.country.empty())
        return code;
    code += "-" + parts.country;
    return code;
}

namespace {

/// Every distinct value `read` gives for the records of `kind`, in first-seen
/// order — the catalogue's own reading order, which is the shipped files'
/// then the user's.
template<typename Read>
[[nodiscard]] std::vector<std::string>
distinctOf(const core::PatternCatalogue& catalogue, core::PatternKind kind, Read read) {
    std::vector<std::string> found;
    for (const core::CorrectionPattern& pattern : catalogue.patterns()) {
        if (pattern.kind() != kind)
            continue;
        const std::string value = read(splitCode(pattern.code));
        if (!value.empty() && !std::ranges::contains(found, value))
            found.push_back(value);
    }
    return found;
}

} // namespace

std::vector<std::string> scriptsOf(const core::PatternCatalogue& catalogue,
                                   core::PatternKind kind) {
    std::vector<std::string> scripts;
    for (const core::CorrectionPattern& pattern : catalogue.patterns()) {
        if (pattern.kind() != kind)
            continue;
        const std::string script = splitCode(pattern.code).script;
        if (!std::ranges::contains(scripts, script))
            scripts.push_back(script);
    }
    // Zyyy first, if present.
    auto zyyy = std::ranges::find(scripts, "Zyyy");
    if (zyyy != scripts.end() && zyyy != scripts.begin()) {
        std::ranges::rotate(scripts.begin(), zyyy, zyyy + 1);
    }
    return scripts;
}

std::vector<std::string> languagesOf(const core::PatternCatalogue& catalogue,
                                     core::PatternKind kind,
                                     std::string_view script) {
    return distinctOf(catalogue, kind, [script](const PatternCodeParts& parts) {
        return parts.script == script ? parts.language : std::string{};
    });
}

std::vector<std::string> countriesOf(const core::PatternCatalogue& catalogue,
                                     core::PatternKind kind,
                                     std::string_view script,
                                     std::string_view language) {
    return distinctOf(catalogue, kind, [script, language](const PatternCodeParts& parts) {
        return parts.script == script && parts.language == language ? parts.country : std::string{};
    });
}

} // namespace subedit::gui
