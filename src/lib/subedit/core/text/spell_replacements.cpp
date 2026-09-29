#include <subedit/core/text/spell_replacements.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

[[nodiscard]] std::string_view stripped(std::string_view text) {
    constexpr std::string_view kBlanks = " \t\n\r\f\v";
    const std::size_t first = text.find_first_not_of(kBlanks);
    if (first == std::string_view::npos)
        return {};
    return text.substr(first, text.find_last_not_of(kBlanks) - first + 1);
}

/// `text` with every `\r\n` and lone `\r` a `\n`, then split at each `\n`.
[[nodiscard]] std::vector<std::string> linesOf(std::string_view text) {
    std::vector<std::string> lines{std::string{}};
    for (std::size_t at = 0; at < text.size(); ++at) {
        if (text[at] == '\r') {
            if (at + 1 < text.size() && text[at + 1] == '\n')
                ++at;
            lines.emplace_back();
        } else if (text[at] == '\n') {
            lines.emplace_back();
        } else {
            lines.back().push_back(text[at]);
        }
    }
    return lines;
}

} // namespace

std::filesystem::path spellReplacementFile(const std::filesystem::path& configDirectory,
                                           std::string_view language) {
    return configDirectory / "spell-check" / (std::string{language} + ".repl");
}

std::vector<SpellReplacement> parseSpellReplacements(std::string_view text) {
    std::vector<SpellReplacement> replacements;
    std::set<std::string, std::less<>> seen;
    for (const std::string& line : linesOf(text)) {
        if (!seen.insert(line).second)
            continue;
        const std::string_view kept = stripped(line);
        if (kept.empty())
            continue;
        const std::size_t bar = kept.find('|');
        if (bar == std::string_view::npos)
            continue;
        std::string word{kept.substr(0, bar)};
        std::string replacement{kept.substr(bar + 1)};
        replacements.push_back({.word = std::move(word), .replacement = std::move(replacement)});
    }
    return replacements;
}

std::string renderSpellReplacements(std::span<const SpellReplacement> replacements) {
    // Unique keeping the last of each: walk backwards keeping the first seen,
    // then turn the result around.
    std::vector<const SpellReplacement*> kept;
    std::set<std::pair<std::string_view, std::string_view>> seen;
    for (const SpellReplacement& one : std::views::reverse(replacements)) {
        if (seen.emplace(one.word, one.replacement).second)
            kept.push_back(&one);
    }
    std::ranges::reverse(kept);
    if (kept.size() > kMaxSpellReplacements)
        kept.erase(kept.begin(), kept.end() - static_cast<std::ptrdiff_t>(kMaxSpellReplacements));

    std::string text;
    for (const SpellReplacement* one : kept) {
        text += one->word;
        text += '|';
        text += one->replacement;
        text += '\n';
    }
    return text;
}

} // namespace subedit::core
