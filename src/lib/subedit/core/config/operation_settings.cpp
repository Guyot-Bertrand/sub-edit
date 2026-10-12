#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/config/settings.hpp>

#include <algorithm>
#include <array>
#include <expected>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kTrueFalse = "true or false";
constexpr std::string_view kMilliseconds = "a whole number of milliseconds, 0 or more";
constexpr std::string_view kPatternCode =
    "a pattern code, Script[-language[-COUNTRY]], not empty: Zyyy, Latn, Latn-en, Latn-en-US";
constexpr std::string_view kLength = "a number above 0, decimals allowed: 24, 42.5";
constexpr std::string_view kLineCount = "a whole number from 1 to 1000: 2, 3";

constexpr std::string_view kAdjust = "subedit-cli adjust; Adjust Durations…";
constexpr std::string_view kCorrect = "subedit-cli correct; Correct Texts…";
constexpr std::string_view kWindowTasks =
    "Correct Texts… (window only; correct takes --tasks instead)";
constexpr std::string_view kWindowCode =
    "Correct Texts… (window only; correct takes --code instead)";
constexpr std::string_view kWindowEnable =
    "Correct Texts… (window only; correct takes --enable/--disable instead)";
constexpr std::string_view kWindowMaxLength =
    "Correct Texts… (window only; correct takes --max-length instead)";
constexpr std::string_view kWindowSkipLength =
    "Correct Texts… (window only; correct takes --skip-length instead)";
constexpr std::string_view kWindowSkipLines =
    "Correct Texts… (window only; correct takes --skip-lines instead)";
constexpr std::string_view kWindowLanguage =
    "Correct Texts… (window only; correct takes --language instead)";

constexpr SettingSurface kBoth = SettingSurface::Both;
constexpr SettingSurface kWindow = SettingSurface::WindowOnly;

// In the order `renderOperationSettings` writes them; a unit test holds the
// set equal to what it writes.
constexpr std::array kCatalogue{
    SettingKey{.key = "duration-adjust.speed",
               .description = "Reading speed the durations are brought to.",
               .accepts = "a number of characters per second, above 0, decimals allowed",
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.lengthen",
               .description = "Let the reading speed move an end later.",
               .accepts = kTrueFalse,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.shorten",
               .description = "Let the reading speed move an end earlier.",
               .accepts = kTrueFalse,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.minimum-enabled",
               .description = "Whether a shortest duration is enforced.",
               .accepts = kTrueFalse,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.minimum-ms",
               .description = "Shortest duration of a subtitle.",
               .accepts = kMilliseconds,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.maximum-enabled",
               .description = "Whether a longest duration is enforced.",
               .accepts = kTrueFalse,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.maximum-ms",
               .description = "Longest duration of a subtitle.",
               .accepts = kMilliseconds,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.gap-enabled",
               .description = "Whether a least gap before the next subtitle is enforced.",
               .accepts = kTrueFalse,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "duration-adjust.gap-ms",
               .description = "Least time left before the next subtitle.",
               .accepts = kMilliseconds,
               .usedBy = kAdjust,
               .surface = kBoth},
    SettingKey{.key = "correction.mentions.enabled",
               .description = "Whether the window ticks the hearing-impaired task.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.mentions.code",
               .description = "Pattern code the window reads for the hearing-impaired task.",
               .accepts = kPatternCode,
               .usedBy = kWindowCode,
               .surface = kWindow},
    SettingKey{.key = "correction.common-errors.enabled",
               .description = "Whether the window ticks the common-errors task.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.common-errors.code",
               .description = "Pattern code the window reads for the common-errors task.",
               .accepts = kPatternCode,
               .usedBy = kWindowCode,
               .surface = kWindow},
    SettingKey{.key = "correction.capitalization.enabled",
               .description = "Whether the window ticks the capitalization task.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.capitalization.code",
               .description = "Pattern code the window reads for the capitalization task.",
               .accepts = kPatternCode,
               .usedBy = kWindowCode,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.enabled",
               .description = "Whether the window ticks the line-break task.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.code",
               .description = "Pattern code the window reads for the line-break task.",
               .accepts = kPatternCode,
               .usedBy = kWindowCode,
               .surface = kWindow},
    SettingKey{.key = "correction.human",
               .description = "Apply the patterns of human-made errors.",
               .accepts = kTrueFalse,
               .usedBy = kCorrect,
               .surface = kBoth},
    SettingKey{.key = "correction.ocr",
               .description = "Apply the patterns of recognition (OCR) errors.",
               .accepts = kTrueFalse,
               .usedBy = kCorrect,
               .surface = kBoth},
    SettingKey{.key = "correction.sound-in-brackets",
               .description =
                   "Whether the window's hearing-impaired task acts on text in brackets.",
               .accepts = kTrueFalse,
               .usedBy = kWindowEnable,
               .surface = kWindow},
    SettingKey{.key = "correction.sound-in-parentheses",
               .description =
                   "Whether the window's hearing-impaired task acts on text in parentheses.",
               .accepts = kTrueFalse,
               .usedBy = kWindowEnable,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.max-length",
               .description =
                   "Longest line the window's line-break task aims at, in the unit in-ems names.",
               .accepts = kLength,
               .usedBy = kWindowMaxLength,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.max-lines",
               .description = "Most lines a subtitle is broken into.",
               .accepts = kLineCount,
               .usedBy = kCorrect,
               .surface = kBoth},
    SettingKey{.key = "correction.line-break.in-ems",
               .description = "Whether the window measures lines in ems rather than characters.",
               .accepts = kTrueFalse,
               .usedBy = kWindowMaxLength,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.skip-on-length",
               .description =
                   "Whether the window's line-break task leaves alone a subtitle whose lines are "
                   "short enough.",
               .accepts = kTrueFalse,
               .usedBy = kWindowSkipLength,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.skip-max-length",
               .description =
                   "Longest line a subtitle may have and still be left alone by the window's "
                   "line-break task.",
               .accepts = kLength,
               .usedBy = kWindowSkipLength,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.skip-on-lines",
               .description =
                   "Whether the window's line-break task leaves alone a subtitle with few enough "
                   "lines.",
               .accepts = kTrueFalse,
               .usedBy = kWindowSkipLines,
               .surface = kWindow},
    SettingKey{.key = "correction.line-break.skip-max-lines",
               .description =
                   "Most lines a subtitle may have and still be left alone by the window's "
                   "line-break task.",
               .accepts = kLineCount,
               .usedBy = kWindowSkipLines,
               .surface = kWindow},
    SettingKey{.key = "correction.join-split.enabled",
               .description = "Whether the window ticks the join/split-words task.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.join-split.join",
               .description = "Whether the window's join/split-words task joins words.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.join-split.split",
               .description = "Whether the window's join/split-words task splits words.",
               .accepts = kTrueFalse,
               .usedBy = kWindowTasks,
               .surface = kWindow},
    SettingKey{.key = "correction.join-split.language",
               .description = "Dictionary the window's join/split-words task checks words by.",
               .accepts =
                   "a locale code, two lower-case letters, then optionally _ and two upper-case, "
                   "then optionally @ and a script: fr, en_US, sr@Latn",
               .usedBy = kWindowLanguage,
               .surface = kWindow},
    SettingKey{.key = "correction.remove-blank",
               .description =
                   "Remove the subtitles the correction empties, instead of leaving them empty.",
               .accepts = kTrueFalse,
               .usedBy = kCorrect,
               .surface = kBoth},
};

// A key of the file, in the correction family, that is not in the catalogue:
// the window manages the list, and `correct` has its own flags for it.
constexpr std::string_view kActivationsKey = "correction.activations";

constexpr std::string_view kOptionSeparator = " = ";
constexpr char kComment = '#';

/// One option of a rendered text: its value, and whether it was commented
/// out, which is what the file does with a value at its default.
struct RenderedOption {
    std::string value;
    bool commented = false;
};

/// The options of a rendered text, by key. A line with no `" = "` — a banner,
/// a header, a blank — names no option and is skipped.
[[nodiscard]] std::map<std::string, RenderedOption, std::less<>>
renderedOptionsOf(std::string_view text) {
    std::map<std::string, RenderedOption, std::less<>> options;
    std::size_t start = 0;
    while (start < text.size()) {
        const std::size_t end = std::min(text.find('\n', start), text.size());
        std::string_view line = text.substr(start, end - start);
        start = end + 1;

        const bool commented = !line.empty() && line.front() == kComment;
        if (commented)
            line.remove_prefix(1);
        const std::size_t separator = line.find(kOptionSeparator);
        if (separator == std::string_view::npos)
            continue;
        options.insert_or_assign(
            std::string{line.substr(0, separator)},
            RenderedOption{.value = std::string{line.substr(separator + kOptionSeparator.size())},
                           .commented = commented});
    }
    return options;
}

/// Refuses a key that is not an operation key, saying why.
[[nodiscard]] std::expected<void, std::string> checkOperationKey(std::string_view key) {
    if (findOperationKey(key) != nullptr)
        return {};
    if (key == kActivationsKey)
        return std::unexpected{std::string{key} +
                               " is managed by the window; use --enable/--disable on correct"};
    if (renderedOptionsOf(renderSettings(Settings{})).contains(key))
        return std::unexpected{std::string{key} + " is a display preference; set it in the window"};
    return std::unexpected{"unknown setting " + std::string{key} +
                           "; subedit-cli config list names them"};
}

/// Writes `value` for `key` with the file's own reader, or says it cannot.
[[nodiscard]] std::expected<void, std::string>
applyChecked(Settings& settings, std::string_view key, std::string_view value) {
    if (applySettingText(settings, key, value).has_value())
        return std::unexpected{"cannot read '" + std::string{value} + "' for " + std::string{key} +
                               "; subedit-cli config list says what it accepts"};
    return {};
}

} // namespace

OperationSettingsRead readOperationSettings(const FileSystem& files,
                                            const std::filesystem::path& path) {
    SettingsRead read = readSettings(files, path);
    OperationSettingsRead operation{.settings = operationPartOf(read.settings),
                                    .unreadable = std::move(read.unreadable),
                                    .found = read.found};
    for (SettingsDiagnostic& diagnostic : read.diagnostics) {
        if (findOperationKey(diagnostic.key) != nullptr)
            operation.diagnostics.push_back(std::move(diagnostic));
    }
    return operation;
}

OperationSettings operationPartOf(const Settings& settings) {
    return {.durationAdjustment = settings.durationAdjustment, .correction = settings.correction};
}

std::span<const SettingKey> operationKeys() {
    return kCatalogue;
}

const SettingKey* findOperationKey(std::string_view key) {
    const auto* const found =
        std::ranges::find(kCatalogue, key, [](const SettingKey& one) { return one.key; });
    return found == kCatalogue.end() ? nullptr : found;
}

std::vector<SettingEntry> describeOperationSettings(const Settings& settings) {
    const auto current = renderedOptionsOf(renderOperationSettings(settings));
    const auto defaults = renderedOptionsOf(renderOperationSettings(Settings{}));

    std::vector<SettingEntry> entries;
    entries.reserve(kCatalogue.size());
    for (const SettingKey& one : kCatalogue) {
        const RenderedOption& now = current.at(std::string{one.key});
        entries.push_back({.key = &one,
                           .value = now.value,
                           .defaultValue = defaults.at(std::string{one.key}).value,
                           .fromFile = !now.commented});
    }
    return entries;
}

std::expected<void, std::string>
setOperationOption(Settings& settings, std::string_view key, std::string_view value) {
    if (std::expected<void, std::string> checked = checkOperationKey(key); !checked)
        return checked;
    return applyChecked(settings, key, value);
}

std::expected<void, std::string> unsetOperationOption(Settings& settings, std::string_view key) {
    if (std::expected<void, std::string> checked = checkOperationKey(key); !checked)
        return checked;
    // **Rebuilt from the defaults rather than written back as text**: a default
    // the reader refuses — the empty dictionary language — is one the file
    // only ever writes commented out. So the operation part starts over from a
    // fresh `Settings`, and every other key that differs from its default is
    // read back from what the file would write for it.
    Settings rebuilt = settings;
    rebuilt.durationAdjustment = {};
    rebuilt.correction =
        CorrectionSettings{.patternActivations = std::move(rebuilt.correction.patternActivations)};
    for (const SettingEntry& entry : describeOperationSettings(settings)) {
        if (!entry.fromFile || entry.key->key == key)
            continue;
        if (std::expected<void, std::string> kept =
                applyChecked(rebuilt, entry.key->key, entry.value);
            !kept)
            return kept;
    }
    settings = std::move(rebuilt);
    return {};
}

} // namespace subedit::core
