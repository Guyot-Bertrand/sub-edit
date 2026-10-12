// The operation part of the settings and its catalogue — spec 17, D3, D7, D8.
//
// The catalogue is a table written by hand, so the first tests pin it to the
// file's own writer: a key the file writes and the catalogue omits, or the
// reverse, is a failure here rather than a silent gap in `config list`.

#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace subedit::core;

namespace {
const std::filesystem::path kPath = "/cfg/subedit/settings.conf";

// Every `key` the operation block of a default render names.
std::set<std::string> renderedOperationKeys() {
    std::set<std::string> keys;
    std::istringstream in{renderOperationSettings(Settings{})};
    for (std::string line; std::getline(in, line);) {
        if (!line.empty() && line.front() == '#')
            line.erase(0, 1);
        if (const std::size_t eq = line.find(" = "); eq != std::string::npos)
            keys.insert(line.substr(0, eq));
    }
    return keys;
}
} // namespace

TEST_CASE("The catalogue names every operation key the file writes, bar the activations list",
          "[operation-settings]") {
    std::set<std::string> catalogued;
    for (const SettingKey& one : operationKeys())
        catalogued.insert(std::string{one.key});

    std::set<std::string> rendered = renderedOperationKeys();
    rendered.erase("correction.activations");
    CHECK(catalogued == rendered);
}

TEST_CASE("Every catalogue entry says what it is for and what it accepts", "[operation-settings]") {
    for (const SettingKey& one : operationKeys()) {
        INFO(one.key);
        CHECK_FALSE(one.description.empty());
        CHECK_FALSE(one.accepts.empty());
    }
}

TEST_CASE(
    "Every entry says who uses it, and the command line is named exactly when it honours the key",
    "[operation-settings]") {
    for (const SettingKey& one : operationKeys()) {
        INFO(one.key);
        CHECK_FALSE(one.usedBy.empty());
        const bool namesCommand = one.usedBy.find("subedit-cli") != std::string_view::npos;
        CHECK(namesCommand == (one.surface == SettingSurface::Both));
    }
}

TEST_CASE("The command line honours exactly thirteen keys", "[operation-settings]") {
    int both = 0;
    for (const SettingKey& one : operationKeys())
        if (one.surface == SettingSurface::Both)
            ++both;
    CHECK(both == 13);
}

TEST_CASE("findOperationKey finds a catalogued key and nothing else", "[operation-settings]") {
    const SettingKey* speed = findOperationKey("duration-adjust.speed");
    REQUIRE(speed != nullptr);
    CHECK(speed->key == "duration-adjust.speed");
    CHECK(findOperationKey("correction.activations") == nullptr);
    CHECK(findOperationKey("general.theme") == nullptr);
}

TEST_CASE("Reading operation settings ignores a display key without a word",
          "[operation-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "general.theme = nonsense\nduration-adjust.speed = 12\n")
                .has_value());
    const OperationSettingsRead read = readOperationSettings(files, kPath);
    CHECK(read.diagnostics.empty());
    CHECK(read.settings.durationAdjustment.charactersPerSecond == 12.0);
    CHECK(read.found);
}

TEST_CASE("An unreadable operation value keeps the default and is reported",
          "[operation-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = abc\n").has_value());
    const OperationSettingsRead read = readOperationSettings(files, kPath);
    REQUIRE(read.diagnostics.size() == 1);
    CHECK(read.diagnostics[0].key == "duration-adjust.speed");
    CHECK(read.settings == OperationSettings{});
}

TEST_CASE("A missing file is every default and not an error", "[operation-settings]") {
    const InMemoryFileSystem files;
    const OperationSettingsRead read = readOperationSettings(files, kPath);
    CHECK_FALSE(read.found);
    CHECK_FALSE(read.unreadable.has_value());
    CHECK(read.settings == OperationSettings{});
}

TEST_CASE("operationPartOf takes the duration adjustment and the correction",
          "[operation-settings]") {
    Settings settings;
    settings.durationAdjustment.gapMilliseconds = 7;
    settings.correction.ocr = false;
    settings.maximised = true;
    const OperationSettings part = operationPartOf(settings);
    CHECK(part.durationAdjustment == settings.durationAdjustment);
    CHECK(part.correction == settings.correction);
}

TEST_CASE("describe walks the catalogue with value, default and source", "[operation-settings]") {
    Settings settings;
    settings.durationAdjustment.charactersPerSecond = 12;
    const std::vector<SettingEntry> entries = describeOperationSettings(settings);
    CHECK(entries.size() == operationKeys().size());
    bool sawSpeed = false;
    bool sawGap = false;
    for (const SettingEntry& entry : entries) {
        if (entry.key->key == "duration-adjust.speed") {
            sawSpeed = true;
            CHECK(entry.value == "12");
            CHECK(entry.defaultValue == "15");
            CHECK(entry.fromFile);
        }
        if (entry.key->key == "duration-adjust.gap-ms") {
            sawGap = true;
            CHECK_FALSE(entry.fromFile);
            CHECK(entry.value == entry.defaultValue);
        }
    }
    CHECK(sawSpeed);
    CHECK(sawGap);
}

TEST_CASE("set stores a valid value and unset puts the default back", "[operation-settings]") {
    Settings settings;
    REQUIRE(setOperationOption(settings, "duration-adjust.speed", "12").has_value());
    CHECK(settings.durationAdjustment.charactersPerSecond == 12.0);
    REQUIRE(unsetOperationOption(settings, "duration-adjust.speed").has_value());
    CHECK(settings.durationAdjustment == DurationAdjustmentSettings{});
}

TEST_CASE("unset of a key already at its default succeeds and changes nothing",
          "[operation-settings]") {
    Settings settings;
    REQUIRE(unsetOperationOption(settings, "correction.join-split.language").has_value());
    CHECK(settings == Settings{});
}

TEST_CASE("unset puts back a default the reader would refuse, and keeps the other keys",
          "[operation-settings]") {
    Settings settings;
    settings.correction.spellLanguage = "fr_FR";
    settings.correction.ocr = false;
    settings.correction.patternActivations.push_back(
        {.kind = PatternKind::CommonError, .code = "Zyyy", .name = "One", .enabled = false});
    settings.maximised = true;
    REQUIRE(unsetOperationOption(settings, "correction.join-split.language").has_value());
    CHECK(settings.correction.spellLanguage.empty());
    CHECK_FALSE(settings.correction.ocr);
    CHECK(settings.correction.patternActivations.size() == 1);
    CHECK(settings.maximised);
}

TEST_CASE("unset refuses when another key holds a value the reader would refuse",
          "[operation-settings]") {
    // Built directly: a file cannot give it, the reader refuses "Fr".
    Settings settings;
    settings.correction.spellLanguage = "Fr";
    settings.durationAdjustment.charactersPerSecond = 12.0;
    const Settings before = settings;

    const auto refused = unsetOperationOption(settings, "duration-adjust.speed");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().find("correction.join-split.language") != std::string::npos);
    CHECK(settings == before);
}

TEST_CASE("set refuses an unreadable value and leaves the settings alone", "[operation-settings]") {
    Settings settings;
    const Settings before = settings;
    const auto refused = setOperationOption(settings, "duration-adjust.speed", "abc");
    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().find("duration-adjust.speed") != std::string::npos);
    CHECK(settings == before);
}

TEST_CASE("set names a display preference as one, and an unknown key as unknown",
          "[operation-settings]") {
    Settings settings;
    const auto display = setOperationOption(settings, "general.theme", "dark");
    REQUIRE_FALSE(display.has_value());
    CHECK(display.error().find("display preference") != std::string::npos);
    const auto unknown = setOperationOption(settings, "no.such.key", "1");
    REQUIRE_FALSE(unknown.has_value());
    CHECK(unknown.error().find("unknown") != std::string::npos);
    CHECK(settings == Settings{});
}

TEST_CASE("The activations list is refused as the window's, not as unknown",
          "[operation-settings]") {
    Settings settings;
    for (const auto& refused : {setOperationOption(settings, "correction.activations", "x"),
                                unsetOperationOption(settings, "correction.activations")}) {
        REQUIRE_FALSE(refused.has_value());
        CHECK(refused.error().find("managed by the window") != std::string::npos);
        CHECK(refused.error().find("unknown") == std::string::npos);
    }
    CHECK(settings == Settings{});
}

TEST_CASE("The catalogue lists the keys in the order the file writes them",
          "[operation-settings]") {
    std::vector<std::string> rendered;
    std::istringstream in{renderOperationSettings(Settings{})};
    for (std::string line; std::getline(in, line);) {
        if (!line.empty() && line.front() == '#')
            line.erase(0, 1);
        if (const std::size_t eq = line.find(" = "); eq != std::string::npos)
            rendered.push_back(line.substr(0, eq));
    }
    std::erase(rendered, "correction.activations");

    std::vector<std::string> catalogued;
    for (const SettingKey& one : operationKeys())
        catalogued.emplace_back(one.key);
    CHECK(catalogued == rendered);
}

TEST_CASE("Every key can be set away from its default and unset back to it",
          "[operation-settings]") {
    // A valid value other than the default, for every catalogue key. A key
    // missing here fails the test below rather than going unchecked.
    const std::map<std::string_view, std::string_view> nonDefault{
        {"duration-adjust.speed", "12.5"},
        {"duration-adjust.lengthen", "false"},
        {"duration-adjust.shorten", "true"},
        {"duration-adjust.minimum-enabled", "false"},
        {"duration-adjust.minimum-ms", "1234"},
        {"duration-adjust.maximum-enabled", "true"},
        {"duration-adjust.maximum-ms", "4321"},
        {"duration-adjust.gap-enabled", "false"},
        {"duration-adjust.gap-ms", "50"},
        {"correction.mentions.enabled", "true"},
        {"correction.mentions.code", "Latn-en"},
        {"correction.common-errors.enabled", "false"},
        {"correction.common-errors.code", "Latn-en"},
        {"correction.capitalization.enabled", "false"},
        {"correction.capitalization.code", "Latn-en"},
        {"correction.line-break.enabled", "true"},
        {"correction.line-break.code", "Latn-en"},
        {"correction.human", "false"},
        {"correction.ocr", "false"},
        {"correction.sound-in-brackets", "true"},
        {"correction.sound-in-parentheses", "true"},
        {"correction.line-break.max-length", "30.5"},
        {"correction.line-break.max-lines", "2"},
        {"correction.line-break.in-ems", "false"},
        {"correction.line-break.skip-on-length", "false"},
        {"correction.line-break.skip-max-length", "20"},
        {"correction.line-break.skip-on-lines", "false"},
        {"correction.line-break.skip-max-lines", "1"},
        {"correction.join-split.enabled", "true"},
        {"correction.join-split.join", "false"},
        {"correction.join-split.split", "true"},
        {"correction.join-split.language", "fr_FR"},
        {"correction.remove-blank", "false"},
    };

    // A display preference and an activation, which neither command may touch.
    Settings base;
    base.maximised = true;
    base.correction.patternActivations.push_back(
        {.kind = PatternKind::CommonError, .code = "Zyyy", .name = "One", .enabled = false});

    for (const SettingKey& one : operationKeys()) {
        INFO(one.key);
        const auto value = nonDefault.find(one.key);
        REQUIRE(value != nonDefault.end());

        Settings settings = base;
        REQUIRE(setOperationOption(settings, one.key, value->second).has_value());
        CHECK(settings != base);
        for (const SettingEntry& entry : describeOperationSettings(settings)) {
            if (entry.key->key == one.key) {
                CHECK(entry.fromFile);
                CHECK(entry.value == value->second);
            } else {
                CHECK_FALSE(entry.fromFile);
            }
        }

        REQUIRE(unsetOperationOption(settings, one.key).has_value());
        CHECK(settings == base);
    }
    CHECK(nonDefault.size() == operationKeys().size());
}

TEST_CASE("set keeps the display preferences already in the settings", "[operation-settings]") {
    Settings settings;
    settings.maximised = true;
    REQUIRE(setOperationOption(settings, "duration-adjust.speed", "12").has_value());
    CHECK(settings.maximised);
}
