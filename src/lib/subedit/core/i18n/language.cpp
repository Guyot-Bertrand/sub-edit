#include <subedit/core/i18n/language.hpp>

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace subedit::core {

namespace {

bool isEnglish(std::string_view language) {
    return language == "en" || language == "C" || language == "POSIX";
}

/// `fr_FR.UTF-8@euro` → `fr_FR`, `fr`. The encoding and the modifier are not part of a
/// catalogue's name here.
std::vector<std::string> expand(std::string_view locale) {
    auto name = locale.substr(0, locale.find_first_of(".@"));
    std::vector<std::string> names;
    if (name.empty()) {
        return names;
    }
    names.emplace_back(name);
    if (const auto underscore = name.find('_'); underscore != std::string_view::npos) {
        names.emplace_back(name.substr(0, underscore));
    }
    return names;
}

std::string_view languageOf(std::string_view name) {
    return name.substr(0, name.find('_'));
}

} // namespace

std::vector<std::string> languageCandidates(const EnvironmentLookup& environment) {
    // The locale in force: the first of these that is set and not empty.
    std::string locale;
    for (const auto* variable : {"LC_ALL", "LC_MESSAGES", "LANG"}) {
        if (const auto value = environment(variable); value && !value->empty()) {
            locale = *value;
            break;
        }
    }
    if (locale.empty() || locale == "C" || locale == "POSIX") {
        return {};
    }

    std::string requested = locale;
    if (const auto list = environment("LANGUAGE"); list && !list->empty()) {
        requested = *list;
    }

    std::vector<std::string> candidates;
    std::size_t start = 0;
    while (start <= requested.size()) {
        auto end = requested.find(':', start);
        if (end == std::string::npos) {
            end = requested.size();
        }
        const auto entry = std::string_view(requested).substr(start, end - start);
        start = end + 1;

        for (auto& name : expand(entry)) {
            if (isEnglish(languageOf(name))) {
                // English is the source: nothing after it applies.
                return candidates;
            }
            if (std::ranges::find(candidates, name) == candidates.end()) {
                candidates.push_back(std::move(name));
            }
        }
    }
    return candidates;
}

std::filesystem::path cataloguePath(const std::filesystem::path& localeDirectory,
                                    std::string_view language) {
    return localeDirectory / std::string{language} / "LC_MESSAGES" / "subedit.mo";
}

std::shared_ptr<const Catalogue> installedCatalogue(const std::vector<std::string>& candidates,
                                                    const std::filesystem::path& localeDirectory) {
    for (const std::string& language : candidates) {
        auto catalogue = Catalogue::fromFile(cataloguePath(localeDirectory, language));
        if (catalogue) {
            return std::make_shared<const Catalogue>(*std::move(catalogue));
        }
    }
    return nullptr;
}

EnvironmentLookup processEnvironment() {
    return [](std::string_view name) -> std::optional<std::string> {
        // NOLINTNEXTLINE(concurrency-mt-unsafe): read at start-up, before any thread.
        const char* value = std::getenv(std::string(name).c_str());
        if (value == nullptr) {
            return std::nullopt;
        }
        return std::string(value);
    };
}

} // namespace subedit::core
