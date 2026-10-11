# Shared Configuration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** The command line reads the operation settings from the user's settings file (by default, `--no-config` to skip), and gains `config list|show|set|unset` to document, inspect and change them.

**Architecture:** `core::Settings` stays one flat struct (no churn in the ~100 call sites and designated-initialiser tests). A new `core::OperationSettings` is the slice the command line may see; `readOperationSettings` is the only reader it gets, so display preferences never reach it. A key catalogue in the core (key, description, accepted values, surface) is the single source for `config list`, and the guard for `config set|unset`; defaults and current values are *derived from `renderSettings`*, never written twice. The file keeps its flat `key = value` format and its keys; on writing it becomes two banner-headed blocks.

**Tech Stack:** C++23, CLI11, Catch2, `core::FileSystem` / `InMemoryFileSystem`, the existing `Reporter` / `Json`.

**Spec:** `docs/specs/17-shared-configuration.md`, `docs/adr/0045-one-settings-file-two-parts.md`. Task 1 amends both where this plan deliberately departs from them.

## Global Constraints

- **English everywhere**: code, comments, tests titles, docs, commit messages (ADR 0043). New C++ comments in English.
- **Never read `reference/`; never name a file of `src/data/`** in any versioned file. Test data lives in `src/test/data/` or is built inline.
- **Every git command is `git -C /home/beber/Projects/subedit …`**; never `cd` anywhere near `reference/`.
- **Branch `feat/699` already exists**; commit there. Conventional Commits, scope = an `area:` label (`core`, `cli`, `gui`, `doc`), subject ≤ 72 chars, checked by `./src/scripts/check-commit-message.sh "<subject>"`. Every commit ends with `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`.
- **A test that writes uses the `Scratch` harness; a test never resolves a config location**, it receives a path (`check-config-home.sh`). Only the two `main` functions call `platform::resolvedUserSettingsPath()`.
- **ADR 0022 rules hold**: unknown key ignored; unreadable value keeps the default and yields a diagnostic; absent option takes the default; option at default rewritten commented; missing file is not an error; existing files read unchanged (no migration).
- **Priority**: option on the command > file > core default.
- **Do not run `make check` per task.** Use `./src/scripts/gate.sh check --only <step>` or the targeted test binary; one full `make check-local` then `make check` at the end (Task 10).
- **clang-format only on C++ files** (never on `.md`, CMake, PNG).
- **Exit codes** are the manual's four; a refused key/value is a usage error, an unwritable file the I/O error.

## Review Focus

1. A file with `columns = nonsense` (a display key) must produce **no diagnostic** on the command line and no change in behaviour — Task 4 test.
2. `config set` on a file that holds display preferences must **keep them byte-for-meaning** (window geometry survives) — Task 4 and Task 9 tests.
3. `config set duration-adjust.speed abc` must leave the file **untouched** and exit with the usage code — Task 4 and Task 9 tests.
4. `--config` and `--no-config` together are a usage error; `--no-config` yields exactly the pre-change behaviour (e2e regression: `adjust_test.cpp` passes unchanged) — Task 5.
5. A file with `duration-adjust.speed = 0` (hand edited) or an unreadable file must not crash `adjust`; defaults + a verbosity-1 note — Task 6 test.

---

### Task 1: Amend the spec and ADR where the plan departs

**Files:**
- Modify: `docs/specs/17-shared-configuration.md`
- Modify: `docs/adr/0045-one-settings-file-two-parts.md`

Departures, each decided while reading the code:

1. `Settings` stays one flat struct; `OperationSettings` is a slice read through `readOperationSettings` (D3). A split of the struct would touch every GUI call site and every designated-initialiser test for no extra guarantee: the command line is only given the slice.
2. `OperationSettings` = `durationAdjustment` + `correction`. `spellCheck` has no command-line equivalent and stays on the display side.
3. The catalogue carries a `surface`: `Both` or `Window only`. **Honoured by the command line (Both):** the nine `duration-adjust.*` keys, and `correction.human`, `correction.ocr`, `correction.remove-blank`, `correction.line-break.max-lines`. Every other `correction.*` key is listed `Window only` (`correct` takes `--tasks`, `--code`, `--max-length`, `--language` from its own line on purpose: the length unit differs, nothing is ticked beforehand). `correction.activations` is not in the catalogue (a list the window manages; the command line has `--enable/--disable`).
4. `#697`/`#698` introduced no new field in `Settings` yet, so D5's audit finds nothing to move for them.
5. `adjust` rule: `--speed off` switches the speed off; the reading speed's directions come from the file unless a flag changes them.

- [ ] **Step 1:** Edit the spec: replace D3's first bullet with departure 1+2, add departures 3–5 as a new "D8 — What the command line honours" section (table of the 13 keys above), and add "`correction.activations`" to Out of scope.
- [ ] **Step 2:** In the ADR, point 1, replace "`Settings` is two parts in the core" with "The core exposes the operation part as `OperationSettings`; the file has two blocks".
- [ ] **Step 3: Commit**

```bash
git -C /home/beber/Projects/subedit add docs/specs/17-shared-configuration.md docs/adr/0045-one-settings-file-two-parts.md docs/superpowers/plans/2026-10-11-shared-configuration.md CLAUDE.md
git -C /home/beber/Projects/subedit commit -m "docs(doc): spec, ADR 0045 and plan for shared configuration" \
  -m "Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>"
```

---

### Task 2: One Qt-free place that resolves the settings path

**Files:**
- Create: `src/lib/subedit/core/config/settings_location.hpp`, `src/lib/subedit/core/config/settings_location.cpp`
- Modify: `src/lib/subedit/core/CMakeLists.txt` (add `config/settings_location.cpp` next to `config/settings.cpp`)
- Modify: `src/lib/subedit/platform/locations.hpp`, `locations.cpp`
- Modify: `src/lib/subedit/gui/settings_path.cpp` (drop Qt)
- Test: `src/test/unit/core/config/settings_location_test.cpp` (add to `src/test/unit/CMakeLists.txt` after line 145)

**Interfaces:**
- Produces: `std::filesystem::path core::userSettingsLocation(std::string_view xdgConfigHome, std::string_view home)`; `std::filesystem::path platform::resolvedUserSettingsPath()`.

- [ ] **Step 1: Write the failing test**

```cpp
#include <subedit/core/config/settings_location.hpp>

#include <catch2/catch_test_macros.hpp>

using subedit::core::userSettingsLocation;

TEST_CASE("The settings file lives under XDG_CONFIG_HOME when it is set", "[settings][location]") {
    CHECK(userSettingsLocation("/x/config", "/home/u") == "/x/config/subedit/settings.conf");
}

TEST_CASE("The settings file falls back to ~/.config", "[settings][location]") {
    CHECK(userSettingsLocation("", "/home/u") == "/home/u/.config/subedit/settings.conf");
}

TEST_CASE("A relative XDG_CONFIG_HOME is ignored, as Qt and the XDG spec do", "[settings][location]") {
    CHECK(userSettingsLocation("relative/dir", "/home/u") == "/home/u/.config/subedit/settings.conf");
}

TEST_CASE("With neither variable there is no location", "[settings][location]") {
    CHECK(userSettingsLocation("", "").empty());
}
```

- [ ] **Step 2:** Run: `cmake --build build/debug --target subedit_unit_tests && build/debug/src/test/unit/subedit_unit_tests "[location]"` (use the project's actual unit-test target; `make -n test` shows it). Expected: FAIL to compile (header missing).
- [ ] **Step 3: Implement**

```cpp
// settings_location.hpp
#pragma once
#include <filesystem>
#include <string_view>

namespace subedit::core {

/// Where the user's settings file is, from the two variables that decide it.
///
/// **Pure, like `userPatternsPath` (ADR 0037)**: it reads no variable. The one
/// code that does is `platform::resolvedUserSettingsPath()`. A relative
/// `xdgConfigHome` is ignored, as the XDG specification says. Empty when neither
/// variable gives a base.
[[nodiscard]] std::filesystem::path userSettingsLocation(std::string_view xdgConfigHome,
                                                         std::string_view home);

} // namespace subedit::core
```

```cpp
// settings_location.cpp
#include <subedit/core/config/settings_location.hpp>

namespace subedit::core {

std::filesystem::path userSettingsLocation(std::string_view xdgConfigHome, std::string_view home) {
    std::filesystem::path base;
    if (const std::filesystem::path xdg{xdgConfigHome}; !xdgConfigHome.empty() && xdg.is_absolute())
        base = xdg;
    else if (!home.empty())
        base = std::filesystem::path{home} / ".config";
    else
        return {};
    return base / "subedit" / "settings.conf";
}

} // namespace subedit::core
```

`platform/locations.hpp`: declare `[[nodiscard]] std::filesystem::path resolvedUserSettingsPath();` with a comment "The one place the settings location is read from the environment; a test never calls it". `locations.cpp`: `return core::userSettingsLocation(variable("XDG_CONFIG_HOME"), variable("HOME"));` and include the new header. `gui/settings_path.cpp` becomes `#include <subedit/platform/locations.hpp>` and `return platform::resolvedUserSettingsPath();` (remove the Qt includes; keep the doc comment in the header, edit its "Derived from the generic config location" paragraph to say it delegates to `platform`).

- [ ] **Step 4:** Run the same test target. Expected: PASS. Also run `build/debug/src/test/gui/…` suite if a test names `userSettingsPath` (`grep -rn userSettingsPath src/test`).
- [ ] **Step 5: Commit** `feat(core): resolve the settings path without Qt` (+ Co-Authored-By trailer).

---

### Task 3: Two blocks on writing, `found` on reading

**Files:**
- Modify: `src/lib/subedit/core/config/settings.hpp` (add `found` to `SettingsRead`; declare `renderOperationSettings`)
- Modify: `src/lib/subedit/core/config/settings.cpp` (`readSettings` ~1074, `renderSettings` ~1127)
- Test: `src/test/unit/core/config/settings_test.cpp`

**Interfaces:**
- Produces: `SettingsRead::found` (true when the file was read); `std::string renderOperationSettings(const Settings&)` — only the operation block's option lines (no header, no banner), `#`-commented when at default.

- [ ] **Step 1: Write the failing tests** (append to `settings_test.cpp`, reusing its `kPath`/`InMemoryFileSystem` idioms)

```cpp
TEST_CASE("A settings file read reports that it was found", "[settings]") {
    InMemoryFileSystem files;
    CHECK_FALSE(readSettings(files, kPath).found);
    REQUIRE(writeSettings(files, kPath, Settings{}).has_value());
    CHECK(readSettings(files, kPath).found);
}

TEST_CASE("The written file has the operation block before the display block", "[settings]") {
    const std::string text = renderSettings(Settings{});
    const std::size_t operation = text.find("duration-adjust.speed");
    const std::size_t display = text.find("window.geometry");  // use the real geometry key constant's text
    REQUIRE(operation != std::string::npos);
    REQUIRE(display != std::string::npos);
    CHECK(operation < display);
}

TEST_CASE("renderOperationSettings holds operation keys and no display key", "[settings]") {
    const std::string text = renderOperationSettings(Settings{});
    CHECK(text.find("duration-adjust.speed") != std::string::npos);
    CHECK(text.find("correction.human") != std::string::npos);
    CHECK(text.find("window.geometry") == std::string::npos);
    CHECK(text.find("theme") == std::string::npos);
}

TEST_CASE("A file written before the split is read unchanged", "[settings]") {
    // The old order: display keys first, operation keys after, no banners.
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "theme = dark\nduration-adjust.speed = 12\n").has_value());
    const SettingsRead read = readSettings(files, kPath);
    CHECK(read.settings.theme == Theme::Dark);
    CHECK(read.settings.durationAdjustment.charactersPerSecond == 12.0);
    CHECK(read.diagnostics.empty());
}
```
(Replace `"window.geometry"` and `Theme::Dark` with the real `kGeometryKey` text and enum value found in `settings.cpp` / `theme.hpp`; run `grep -n 'kGeometryKey =' settings.cpp`.)

- [ ] **Step 2:** Run `build/debug/.../subedit_unit_tests "[settings]"`. Expected: FAIL (`found`, `renderOperationSettings` undeclared).
- [ ] **Step 3: Implement.**
  - `settings.hpp`: in `SettingsRead` add `/// True when the file existed and was read. An absent file leaves it false. bool found = false;`. Declare `[[nodiscard]] std::string renderOperationSettings(const Settings& settings);` with a comment saying it is the operation block of `renderSettings`, and that a banner never contains `" = "` so a reader of this text can skip it.
  - `settings.cpp`: in `readSettings`, set `read.found = true;` right after the `content` check succeeds (after the `if (!content) {…}` block).
  - Extract from `renderSettings` the calls `renderDurationAdjustment(out, settings.durationAdjustment); renderCorrectionSettings(out, settings.correction);` into `void renderOperationBlock(std::string& out, const Settings&)` (in the anonymous namespace), and the rest into `renderDisplayBlock`. `renderSettings` = header + `\n` + banner "# Operation settings: shared by the window and the command line (subedit-cli reads these)." + `\n` + operation block + banner "# Display preferences: the window's own, the command line never reads them." + `\n` + display block. `renderOperationSettings` = the operation block alone. Banner text must not contain `" = "`.
- [ ] **Step 4:** Run `"[settings]"` and the whole unit suite for `core/config`. Fix any existing test that pinned the old key order (they are the only legitimate reason to edit an existing expectation; say so in the commit body).
- [ ] **Step 5: Commit** `feat(core): write the settings file as an operation block and a display block`.

---

### Task 4: The catalogue and `OperationSettings`

**Files:**
- Create: `src/lib/subedit/core/config/operation_settings.hpp`, `operation_settings.cpp`
- Modify: `src/lib/subedit/core/CMakeLists.txt` (add `config/operation_settings.cpp`)
- Modify: `src/lib/subedit/core/config/settings.hpp` / `settings.cpp` (expose one parser entry, below)
- Test: `src/test/unit/core/config/operation_settings_test.cpp` (add to `src/test/unit/CMakeLists.txt`)

**Interfaces:**
- Consumes: `renderOperationSettings`, `readSettings`, `SettingsRead::found` (Task 3).
- Produces (all in `subedit::core`):

```cpp
struct OperationSettings {
    DurationAdjustmentSettings durationAdjustment{};
    CorrectionSettings correction{};
    friend bool operator==(const OperationSettings&, const OperationSettings&) = default;
};

struct OperationSettingsRead {
    OperationSettings settings{};
    std::vector<SettingsDiagnostic> diagnostics{};   // operation keys only
    std::optional<FileError> unreadable{};
    bool found = false;
};
[[nodiscard]] OperationSettingsRead readOperationSettings(const FileSystem&, const std::filesystem::path&);
[[nodiscard]] OperationSettings operationPartOf(const Settings&);

enum class SettingSurface { Both, WindowOnly };
struct SettingKey {
    std::string_view key;
    std::string_view description;   // what it is for, one sentence
    std::string_view accepts;       // the closed set, the bounds, or the form
    std::string_view usedBy;        // commands and window actions that read it, e.g. "adjust; Adjust Durations…"
    SettingSurface surface;         // Both when the command line honours it
};
[[nodiscard]] std::span<const SettingKey> operationKeys();
[[nodiscard]] const SettingKey* findOperationKey(std::string_view key);

struct SettingEntry {
    const SettingKey* key;
    std::string value;          // current value, as the file writes it
    std::string defaultValue;
    bool fromFile;              // false: the value is the default
};
[[nodiscard]] std::vector<SettingEntry> describeOperationSettings(const Settings&);

[[nodiscard]] std::expected<void, std::string> setOperationOption(Settings&, std::string_view key, std::string_view value);
[[nodiscard]] std::expected<void, std::string> unsetOperationOption(Settings&, std::string_view key);
```

In `settings.hpp` add (used only by `operation_settings.cpp`): `[[nodiscard]] std::optional<SettingsDiagnostic> applySettingText(Settings& settings, std::string_view key, std::string_view value);` — implemented in `settings.cpp` as: copy into a `SettingsRead scratch{.settings = settings}`, `bool bom = false; applyOption(scratch, bom, key, value);`, if `scratch.diagnostics` is non-empty return its first element, else `settings = scratch.settings` and return `std::nullopt`.

- [ ] **Step 1: Write the failing tests**

```cpp
#include <subedit/core/config/operation_settings.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <set>
#include <sstream>
#include <string>

using namespace subedit::core;

namespace {
const std::filesystem::path kPath = "/cfg/subedit/settings.conf";

// Every `key` the operation block of a default render names.
std::set<std::string> renderedOperationKeys() {
    std::set<std::string> keys;
    std::istringstream in{renderOperationSettings(Settings{})};
    for (std::string line; std::getline(in, line);) {
        if (!line.empty() && line.front() == '#') line.erase(0, 1);
        if (const std::size_t eq = line.find(" = "); eq != std::string::npos)
            keys.insert(line.substr(0, eq));
    }
    return keys;
}
} // namespace

TEST_CASE("The catalogue names every operation key the file writes, bar the activations list",
          "[operation-settings]") {
    std::set<std::string> catalogued;
    for (const SettingKey& one : operationKeys()) catalogued.insert(std::string{one.key});

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

TEST_CASE("Every entry says who uses it, and the command line is named exactly when it honours the key",
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
        if (one.surface == SettingSurface::Both) ++both;
    CHECK(both == 13);
}

TEST_CASE("Reading operation settings ignores a display key without a word", "[operation-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "columns = nonsense\nduration-adjust.speed = 12\n").has_value());
    const OperationSettingsRead read = readOperationSettings(files, kPath);
    CHECK(read.diagnostics.empty());
    CHECK(read.settings.durationAdjustment.charactersPerSecond == 12.0);
    CHECK(read.found);
}

TEST_CASE("An unreadable operation value keeps the default and is reported", "[operation-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = abc\n").has_value());
    const OperationSettingsRead read = readOperationSettings(files, kPath);
    REQUIRE(read.diagnostics.size() == 1);
    CHECK(read.diagnostics[0].key == "duration-adjust.speed");
    CHECK(read.settings == OperationSettings{});
}

TEST_CASE("A missing file is every default and not an error", "[operation-settings]") {
    InMemoryFileSystem files;
    const OperationSettingsRead read = readOperationSettings(files, kPath);
    CHECK_FALSE(read.found);
    CHECK_FALSE(read.unreadable.has_value());
    CHECK(read.settings == OperationSettings{});
}

TEST_CASE("describe walks the catalogue with value, default and source", "[operation-settings]") {
    Settings settings;
    settings.durationAdjustment.charactersPerSecond = 12;
    for (const SettingEntry& entry : describeOperationSettings(settings)) {
        if (entry.key->key == "duration-adjust.speed") {
            CHECK(entry.value == "12");
            CHECK(entry.defaultValue == "15");
            CHECK(entry.fromFile);
        }
        if (entry.key->key == "duration-adjust.gap-ms") {
            CHECK_FALSE(entry.fromFile);
            CHECK(entry.value == entry.defaultValue);
        }
    }
}

TEST_CASE("set stores a valid value and unset puts the default back", "[operation-settings]") {
    Settings settings;
    REQUIRE(setOperationOption(settings, "duration-adjust.speed", "12").has_value());
    CHECK(settings.durationAdjustment.charactersPerSecond == 12.0);
    REQUIRE(unsetOperationOption(settings, "duration-adjust.speed").has_value());
    CHECK(settings.durationAdjustment == DurationAdjustmentSettings{});
}

TEST_CASE("set refuses an unreadable value and leaves the settings alone", "[operation-settings]") {
    Settings settings;
    const Settings before = settings;
    const auto refused = setOperationOption(settings, "duration-adjust.speed", "abc");
    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().find("duration-adjust.speed") != std::string::npos);
    CHECK(settings == before);
}

TEST_CASE("set names a display preference as one, and an unknown key as unknown", "[operation-settings]") {
    Settings settings;
    const auto display = setOperationOption(settings, "theme", "dark");
    REQUIRE_FALSE(display.has_value());
    CHECK(display.error().find("display preference") != std::string::npos);
    const auto unknown = setOperationOption(settings, "no.such.key", "1");
    REQUIRE_FALSE(unknown.has_value());
    CHECK(unknown.error().find("unknown") != std::string::npos);
}

TEST_CASE("set keeps the display preferences already in the settings", "[operation-settings]") {
    Settings settings;
    settings.maximised = true;
    REQUIRE(setOperationOption(settings, "duration-adjust.speed", "12").has_value());
    CHECK(settings.maximised);
}
```

- [ ] **Step 2:** Run `"[operation-settings]"`. Expected: FAIL (header missing).
- [ ] **Step 3: Implement `operation_settings.cpp`.**
  - `operationPartOf(s)` → `{s.durationAdjustment, s.correction}`.
  - `readOperationSettings`: `SettingsRead r = readSettings(files, path);` then copy `unreadable`, `found`, `operationPartOf(r.settings)`, and `diagnostics` filtered by `findOperationKey(d.key) != nullptr`.
  - The catalogue: a `static constexpr std::array` of `SettingKey`. Order = the file's order. Descriptions (English, one sentence) and `accepts`:

`usedBy` is `subedit-cli adjust; Adjust Durations…` for the nine `duration-adjust.*` keys; `subedit-cli correct; Correct Texts…` for the four honoured `correction.*` keys; and `Correct Texts… (window only; correct takes <option> instead)` for the other `correction.*` keys, naming `--tasks`, `--code`, `--max-length`, `--language` or `--skip-length/--skip-lines` as fits. A `Both` entry contains the text `subedit-cli`, a `WindowOnly` one does not (the test above pins it).

| key | description | accepts | surface |
| :-- | :---------- | :------ | :------ |
| `duration-adjust.speed` | Reading speed the durations are brought to. | a positive number of characters per second | Both |
| `duration-adjust.lengthen` | Let the reading speed move an end later. | `true` or `false` | Both |
| `duration-adjust.shorten` | Let the reading speed move an end earlier. | `true` or `false` | Both |
| `duration-adjust.minimum-enabled` | Whether a shortest duration is enforced. | `true` or `false` | Both |
| `duration-adjust.minimum-ms` | Shortest duration of a subtitle. | whole milliseconds, 0 or more | Both |
| `duration-adjust.maximum-enabled` | Whether a longest duration is enforced. | `true` or `false` | Both |
| `duration-adjust.maximum-ms` | Longest duration of a subtitle. | whole milliseconds, above 0 | Both |
| `duration-adjust.gap-enabled` | Whether a least gap before the next subtitle is enforced. | `true` or `false` | Both |
| `duration-adjust.gap-ms` | Least time left before the next subtitle. | whole milliseconds, 0 or more | Both |
| `correction.mentions.enabled` | Whether the window ticks the hearing-impaired task. | `true` or `false` | WindowOnly |
| `correction.mentions.code` | Pattern code the window reads for that task. | a script/language code such as `Zyyy`, `Latn`, `Latn-en` | WindowOnly |
| `correction.common-errors.enabled` / `.code` | same, for common errors | as above | WindowOnly |
| `correction.capitalization.enabled` / `.code` | same, for capitalisation | as above | WindowOnly |
| `correction.line-break.enabled` / `.code` | same, for line breaks | as above | WindowOnly |
| `correction.human` | Apply the patterns of human-made errors. | `true` or `false` | Both |
| `correction.ocr` | Apply the patterns of recognition (OCR) errors. | `true` or `false` | Both |
| `correction.sound-in-brackets` | Whether the window's hearing-impaired task acts on text in brackets. | `true` or `false` | WindowOnly |
| `correction.sound-in-parentheses` | same, in parentheses | `true` or `false` | WindowOnly |
| `correction.line-break.max-length` | Longest line the window's line-break aims at. | a positive number | WindowOnly |
| `correction.line-break.max-lines` | Most lines a subtitle is broken into. | a whole number, 1 or more | Both |
| `correction.line-break.in-ems` | Whether the window measures lines in ems rather than characters. | `true` or `false` | WindowOnly |
| `correction.line-break.skip-on-length` / `skip-max-length` / `skip-on-lines` / `skip-max-lines` | The window's skip gate of the line-break task. | `true`/`false`, a positive number, `true`/`false`, a whole number | WindowOnly |
| `correction.join-split.enabled` / `.join` / `.split` / `.language` | The window's join/split-words task and its dictionary. | `true`/`false`; a dictionary such as `fr_FR`, `en_US` | WindowOnly |
| `correction.remove-blank` | Remove the subtitles the correction empties, instead of leaving them empty. | `true` or `false` | Both |

    Expand the grouped rows into one `SettingKey` each (a unit test enforces the set equals the rendered keys). Before writing, confirm each key's real accepted range from `speedOf` / `millisecondsOf` / `booleanOf` in `settings.cpp` and correct the `accepts` text to what the parser really takes (the 13-keys test and the catalogue-equals-render test pin the rest).
  - `describeOperationSettings(s)`: parse `renderOperationSettings(s)` and `renderOperationSettings(Settings{})` into `key → (value, commented)` maps (strip a leading `#`; split on the first `" = "`; skip lines with no `" = "`); for each catalogue key emit `{&key, value, defaultValue, fromFile = !commented}`.
  - `setOperationOption`: `findOperationKey` null → if the key appears in `renderSettings(Settings{})` text (a display key) return `"<key> is a display preference; set it in the window"`, else `"unknown setting <key>; subedit-cli config list names them"`. Otherwise `applySettingText`; on diagnostic return `"cannot read '<value>' for <key>; subedit-cli config list says what it accepts"`.
  - `unsetOperationOption`: same key checks, then `applySettingText(settings, key, defaultValueOf(key))` where the default comes from `describeOperationSettings(Settings{})`. Unsetting a key already at default succeeds.
- [ ] **Step 4:** Run `"[operation-settings]"` and `"[settings]"`. Expected: PASS. If the catalogue-equals-render test fails, the message lists the difference: fix the table, not the test.
- [ ] **Step 5: Commit** `feat(core): operation settings, their catalogue and a reader that sees only them`.

---

### Task 5: The command line is given the shared settings

**Files:**
- Create: `src/lib/subedit/cli/shared_settings.hpp`, `shared_settings.cpp` (add to `src/lib/subedit/cli/CMakeLists.txt`)
- Modify: `src/exe/cli/commands/command.hpp` (`Declared::run`, `declareWith`)
- Modify: `src/exe/cli/application.cpp` (two global options, loading, passing)
- Test: `src/test/unit/cli/shared_settings_test.cpp` (add to unit CMake)

**Interfaces:**
- Consumes: `readOperationSettings`, `OperationSettings` (Task 4); `platform::resolvedUserSettingsPath()` (Task 2).
- Produces:

```cpp
namespace subedit::cli {
enum class SettingsSource { Disabled, Missing, Unreadable, Read };

struct SettingsChoice {
    bool disabled = false;                       // --no-config
    std::filesystem::path explicitPath{};        // --config FILE, empty when absent
    std::filesystem::path userPath{};            // handed down by main
};

struct SharedSettings {
    core::OperationSettings values{};            // core defaults when disabled/missing/unreadable
    std::filesystem::path path{};                // the file looked at; empty when Disabled
    std::filesystem::path target{};              // explicitPath, else userPath, even when Disabled: what `config set` writes
    SettingsSource source = SettingsSource::Disabled;
    std::vector<core::SettingsDiagnostic> diagnostics{};
};

[[nodiscard]] SharedSettings loadSharedSettings(const core::FileSystem& files, const SettingsChoice& choice);
void announceSettings(const Reporter& reporter, const SharedSettings& shared);   // verbosity 1
[[nodiscard]] Json settingsRecord(const SharedSettings& shared);                 // path string, or null
}
```

`Declared::run` becomes `std::function<ExitCode(core::FileSystem&, const std::optional<core::Encoding>&, const Reporter&, const SharedSettings&)>`. `declareWith` forwards the fourth argument **only** to a `run` that accepts it:

```cpp
[options, run](core::FileSystem& files, const std::optional<core::Encoding>& reading,
               const Reporter& reporter, const SharedSettings& shared) {
    if constexpr (std::is_invocable_v<Run, Options&, core::FileSystem&,
                                      const std::optional<core::Encoding>&, const Reporter&,
                                      const SharedSettings&>)
        return run(*options, files, reading, reporter, shared);
    else
        return run(*options, files, reading, reporter);
}
```
(Add `#include <type_traits>` and the `shared_settings.hpp` include. No other command changes.)

- [ ] **Step 1: Write the failing test**

```cpp
#include <subedit/cli/shared_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace subedit::cli;
using subedit::core::InMemoryFileSystem;

namespace {
const std::filesystem::path kUser = "/u/subedit/settings.conf";
const std::filesystem::path kOther = "/o/other.conf";
}

TEST_CASE("--no-config reads nothing and keeps the core defaults", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    const SharedSettings shared =
        loadSharedSettings(files, {.disabled = true, .userPath = kUser});
    CHECK(shared.source == SettingsSource::Disabled);
    CHECK(shared.values == subedit::core::OperationSettings{});
    CHECK(shared.path.empty());
    CHECK(shared.target == kUser);
}

TEST_CASE("By default the user's file is read", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(shared.source == SettingsSource::Read);
    CHECK(shared.path == kUser);
    CHECK(shared.values.durationAdjustment.charactersPerSecond == 12.0);
}

TEST_CASE("--config names another file and wins over the user's", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = 12\n").has_value());
    REQUIRE(files.writeFile(kOther, "duration-adjust.speed = 20\n").has_value());
    const SharedSettings shared =
        loadSharedSettings(files, {.explicitPath = kOther, .userPath = kUser});
    CHECK(shared.path == kOther);
    CHECK(shared.values.durationAdjustment.charactersPerSecond == 20.0);
}

TEST_CASE("A missing file is defaults, said as missing", "[shared-settings]") {
    InMemoryFileSystem files;
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(shared.source == SettingsSource::Missing);
    CHECK(shared.values == subedit::core::OperationSettings{});
}

TEST_CASE("An unreadable value is reported and the default kept", "[shared-settings]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kUser, "duration-adjust.speed = abc\n").has_value());
    const SharedSettings shared = loadSharedSettings(files, {.userPath = kUser});
    CHECK(shared.source == SettingsSource::Read);
    REQUIRE(shared.diagnostics.size() == 1);
    CHECK(shared.values == subedit::core::OperationSettings{});
}
```

- [ ] **Step 2:** Run `"[shared-settings]"`. Expected: FAIL (header missing).
- [ ] **Step 3: Implement.** `loadSharedSettings`: if `choice.disabled` return `{}` (source Disabled); `path = explicitPath.empty() ? userPath : explicitPath`; if `path.empty()` → `Missing` with the core defaults; else `readOperationSettings`; `unreadable` → `Unreadable`, `!found` → `Missing`, else `Read`. `announceSettings` (all `reporter.say(1, …)`): Disabled → nothing; Read → `"settings: read <path>"` then one `"settings: <key>: cannot read '<value>', default kept"` per diagnostic; Missing → `"settings: <path> not found, defaults used"` (nothing when `path` is empty); Unreadable → `"settings: <path> unreadable, defaults used"`. `settingsRecord`: `Json` string of the path for Read, `Json{}` (null) otherwise — check `Json`'s API in `cli/json.hpp` for the null/string constructors and use them as `constraintsRecord` does.
- [ ] **Step 4: Wire `application.cpp`.** After the `--format` option add:

```cpp
    // **Global, because every command that has a parameter in the file reads it
    // the same way** (ADR 0045). The two exclude each other.
    std::string configFile;
    bool noConfig = false;
    auto* configOption = app.add_option("--config", configFile,
        "Read the operation settings from FILE instead of the user's settings file")->option_text("FILE");
    app.add_flag("--no-config", noConfig,
        "Ignore any settings file: use the built-in defaults")->excludes(configOption);
```
After `core::RealFileSystem files;` and before the dispatch loop:

```cpp
    const SharedSettings shared = loadSharedSettings(
        files, {.disabled = noConfig,
                .explicitPath = configFile,
                .userPath = platform::resolvedUserSettingsPath()});
```
and `command.run(files, encoding, reporter.forCommand(...), shared)`. Add includes `<subedit/cli/shared_settings.hpp>`, `<subedit/platform/locations.hpp>`. `announceSettings` is **not** called here: each command that uses the values calls it (Tasks 6–7), so `inspect` stays silent.
- [ ] **Step 5:** Run `"[shared-settings]"`, then build everything and run the existing e2e `adjust`/`usage` tests (`ctest -R "e2e.*(adjust|usage)"`). Expected: PASS, no output change (the harness moves `XDG_CONFIG_HOME`, so no file is read). Update `usage_test.cpp` only if it pins the global option list; add the two options there.
- [ ] **Step 6: Commit** `feat(cli): give commands the shared settings, with --config and --no-config`.

---

### Task 6: `adjust` reads the file

**Files:**
- Modify: `src/lib/subedit/cli/adjusting.hpp`, `adjusting.cpp` (`constraintsOf`, ~line 87)
- Modify: `src/exe/cli/commands/adjust.cpp`
- Test: `src/test/unit/cli/adjusting_test.cpp`; `src/test/e2e/cli/adjust_test.cpp`

**Interfaces:**
- Consumes: `core::DurationAdjustmentSettings`, `core::constraintsOf(settings)` pattern, `SharedSettings` (Task 5).
- Produces: `std::expected<core::DurationConstraints, std::string> constraintsOf(const AdjustOptions&, const core::DurationAdjustmentSettings& base)`; the old one-argument overload stays and calls it with `{}`.

Rules (the plan's D8, pinned by the tests below):
- `--speed off` → no speed (the existing refusal of `--shorten`/`--no-lengthen` with it stays).
- Otherwise `cps = --speed ?? base.charactersPerSecond`. Directions: `base` speed is *active* when `base.lengthen || base.shorten`. Starting directions are `(base.lengthen, base.shorten)` if active, else `(true, false)`; then `lengthen = start.lengthen && !noLengthen`, `shorten = start.shorten || shorten`. If the base speed is inactive, no `--speed` and no direction flag was given, there is **no speed**.
- `--minimum`/`--gap`: `off` → none; a value → that value; absent → the base (`enabled ? value : none`). `--maximum`: a value, else the base.
- A base speed of `0` (hand-edited) is read as off, as `core::constraintsOf` already does (ADR 0022): do not call `ReadingSpeed::create` with a non-positive value.
- The `isAny` refusal is unchanged.

- [ ] **Step 1: Write the failing unit tests** (`adjusting_test.cpp`; use its existing helpers for building `AdjustOptions` and comparing `DurationConstraints`)

```cpp
TEST_CASE("An option beats the file, which beats the default", "[adjust][settings]") {
    core::DurationAdjustmentSettings file;
    file.charactersPerSecond = 12;
    file.minimumMilliseconds = 2000;

    // File over default: nothing on the command line.
    auto fromFile = constraintsOf(AdjustOptions{}, file);
    REQUIRE(fromFile.has_value());
    CHECK(fromFile->speed->charactersPerSecond() == 12.0);
    CHECK(fromFile->minimum == core::Duration::fromMilliseconds(2000));

    // Option over file.
    auto overridden = constraintsOf(AdjustOptions{.speed = "20", .minimum = "off"}, file);
    REQUIRE(overridden.has_value());
    CHECK(overridden->speed->charactersPerSecond() == 20.0);
    CHECK_FALSE(overridden->minimum.has_value());
}

TEST_CASE("Default settings give exactly the old behaviour", "[adjust][settings]") {
    CHECK(constraintsOf(AdjustOptions{}, core::DurationAdjustmentSettings{}) ==
          constraintsOf(AdjustOptions{}));
}

TEST_CASE("--speed off beats a speed in the file", "[adjust][settings]") {
    core::DurationAdjustmentSettings file;
    file.charactersPerSecond = 12;
    auto constraints = constraintsOf(AdjustOptions{.speed = "off"}, file);
    REQUIRE(constraints.has_value());
    CHECK_FALSE(constraints->speed.has_value());
}

TEST_CASE("A file with the speed switched off keeps it off until asked", "[adjust][settings]") {
    core::DurationAdjustmentSettings file;
    file.lengthen = false;
    file.shorten = false;
    CHECK_FALSE(constraintsOf(AdjustOptions{}, file)->speed.has_value());
    CHECK(constraintsOf(AdjustOptions{.speed = "12"}, file)->speed.has_value());
}

TEST_CASE("A hand-edited speed of zero is read as the speed being off", "[adjust][settings]") {
    core::DurationAdjustmentSettings file;
    file.charactersPerSecond = 0;
    auto constraints = constraintsOf(AdjustOptions{}, file);
    REQUIRE(constraints.has_value());
    CHECK_FALSE(constraints->speed.has_value());
}
```
(`AdjustOptions` uses default member initialisers, so designated initialisers work; add `#include`s as the file does.)

- [ ] **Step 2:** Run `"[adjust][settings]"`. Expected: FAIL (overload missing).
- [ ] **Step 3: Implement.** Rewrite the speed/minimum/maximum/gap blocks of `constraintsOf` as the rules above, keeping `parseSpeed`/`parseLimit` and the messages. Declare the new overload in `adjusting.hpp` with the rules as its comment; the old overload: `return constraintsOf(options, core::DurationAdjustmentSettings{});`. In `adjust.cpp`: change `runAdjust` to take `const SharedSettings& shared` as 5th parameter, call `announceSettings(reporter, shared);` first, then `constraintsOf(options.constraints, shared.values.durationAdjustment)`. Add the settings path to the JSON record: in `adjustAll`'s record (where `constraintsRecord` is set), add a `"settings"` field — thread `const SharedSettings&` into `adjustAll`'s parameter list only if the record is built there; otherwise set it in `runAdjust` through `Reporter`. Read `adjusting.cpp` ~161–end first and follow how `constraints` reach the record; if threading is more than ten lines, record the path in the existing `constraints` object under `"settings_file"` instead (`constraintsRecord` takes an extra `const Json& source`).
- [ ] **Step 4:** Run `"[adjust]"` unit tests. Expected: PASS, including every pre-existing one.
- [ ] **Step 5: Write the e2e tests** in `adjust_test.cpp`, beside the existing cases, reusing `kInput`, `Scratch`, `writeFile`, `invoke` and the way existing cases run `adjust` and read the ends:

```cpp
TEST_CASE("adjust reads the operation settings from --config", "[e2e][adjust][settings]") {
    Scratch scratch;
    const auto input = scratch.path("in.srt");
    const auto config = scratch.path("settings.conf");
    writeFile(input, kInput);
    writeFile(config, "duration-adjust.speed = off\nduration-adjust.minimum-ms = 1500\n");
    // Same as case B2 (--speed off): worked out in the file's header.
    const CliRun run = invoke({"--config", config.string(), "adjust", input.string(),
                               "--output", scratch.path("out.srt").string()});
    CHECK(run.exitCode == 0);
    // assert the four ends like B2: 00:00:01,500 / 00:00:14,000 / 00:00:15,000 / 00:00:16,500
}

TEST_CASE("an option beats --config, and --no-config ignores it", "[e2e][adjust][settings]") { /* --speed 15 with the file's off; then --no-config + the file => case A ends */ }
TEST_CASE("--config and --no-config together are a usage error", "[e2e][settings]") {
    const CliRun run = invoke({"--config", "x", "--no-config", "adjust", "y"});
    CHECK(run.exitCode == 2);   // the manual's usage code; use the named constant the file uses
}
TEST_CASE("a note names the file read at the default verbosity", "[e2e][adjust][settings]") { /* errors contain "settings: read <config>" */ }
```
Fill the commented bodies using the exact helpers of the neighbouring cases (this file is the template; do not invent new helpers). Run `ctest -R e2e.*adjust`. Expected: PASS.
- [ ] **Step 6: Commit** `feat(cli): adjust takes its defaults from the settings file`.

---

### Task 7: `correct` reads the file (three keys)

**Files:**
- Modify: `src/lib/subedit/cli/correcting.hpp`, `correcting.cpp` (`correctionSettingsOf` ~484, `prepareCorrection` ~605)
- Modify: `src/exe/cli/commands/correct.cpp`
- Test: `src/test/unit/cli/correcting_test.cpp` (the existing test file for this unit; `ls src/test/unit/cli | grep correct`), `src/test/e2e/cli/correct_test.cpp`

**Interfaces:**
- Consumes: `core::CorrectionSettings`, `SharedSettings`.
- Produces: `correctionSettingsOf(options, catalogue, const core::CorrectionSettings& shared)` and `prepareCorrection(..., const core::CorrectionSettings& shared)`; the current signatures stay as overloads passing `core::CorrectionSettings{}` so no existing test changes.

Honoured: `correction.human`/`correction.ocr` (when `--classes` is empty), `correction.remove-blank` (`--keep-blank-subtitles` still forces `false`), `correction.line-break.max-lines` (when `--max-lines` is empty). Nothing else from the file reaches `correct`: tasks, codes, length, language and the skip gate stay on the command line (spec D8).

- [ ] **Step 1: Write the failing tests**

```cpp
TEST_CASE("correct takes classes, blank removal and max lines from the file", "[correct][settings]") {
    core::CorrectionSettings file;
    file.human = false;
    file.removeBlankSubtitles = false;
    file.lineBreakMaxLines = 2;
    // `options` as the neighbouring tests build them for the common-errors task.
    auto settings = correctionSettingsOf(commonErrorsOptions(), catalogue(), file);
    REQUIRE(settings.has_value());
    CHECK_FALSE(settings->human);
    CHECK(settings->ocr);
    CHECK_FALSE(settings->removeBlankSubtitles);
}

TEST_CASE("an option beats the file for classes and blank removal", "[correct][settings]") {
    core::CorrectionSettings file;
    file.human = false;
    CorrectionOptions options = commonErrorsOptions();
    options.classes = "human,ocr";
    options.keepBlankSubtitles = false;
    auto settings = correctionSettingsOf(options, catalogue(), file);
    REQUIRE(settings.has_value());
    CHECK(settings->human);
}

TEST_CASE("the file never turns a task on", "[correct][settings]") {
    core::CorrectionSettings file;
    file.lineBreak.enabled = true;
    file.mentions.enabled = true;
    auto settings = correctionSettingsOf(commonErrorsOptions(), catalogue(), file);
    REQUIRE(settings.has_value());
    CHECK_FALSE(settings->lineBreak.enabled);
    CHECK_FALSE(settings->mentions.enabled);
}
```
(`commonErrorsOptions()` / `catalogue()` stand for whatever fixture builders the existing correcting tests use; copy their setup.)

- [ ] **Step 2:** Run `"[correct][settings]"`. Expected: FAIL.
- [ ] **Step 3: Implement.** In the new `correctionSettingsOf` overload keep `core::CorrectionSettings settings;` fresh (so tasks, codes, activations never come from the file), then after `classesInto(...)` succeeds: `if (options.classes.empty()) { settings.human = shared.human; settings.ocr = shared.ocr; }`. Before the `lineBreakInto` call, `if (options.maxLines.empty()) settings.lineBreakMaxLines = shared.lineBreakMaxLines;` — read `lineBreakInto` first to confirm it leaves `lineBreakMaxLines` untouched when `maxLines` is empty and to keep the skip gate's "same limit as `--max-lines`" default consistent (if `skipLines` empty it must follow the *effective* max-lines). Replace the last line with `settings.removeBlankSubtitles = options.keepBlankSubtitles ? false : shared.removeBlankSubtitles;`. In `correct.cpp` `runCorrect` takes `const SharedSettings& shared`, calls `announceSettings`, and passes `shared.values.correction`.
- [ ] **Step 4:** Run `"[correct]"` unit + `ctest -R e2e.*correct`. Expected: PASS.
- [ ] **Step 5:** One e2e case in `correct_test.cpp`: a config with `correction.remove-blank = false` and a subtitle the common-errors task empties → the subtitle is kept empty; same command with `--no-config` removes it. Use the fixture the neighbouring blank-subtitle test uses.
- [ ] **Step 6: Commit** `feat(cli): correct takes three defaults from the settings file`.

---

### Task 8: The `config` commands' logic

**Files:**
- Create: `src/lib/subedit/cli/configuring.hpp`, `configuring.cpp` (add to `src/lib/subedit/cli/CMakeLists.txt`)
- Test: `src/test/unit/cli/configuring_test.cpp`

**Interfaces:**
- Consumes: `describeOperationSettings`, `operationKeys`, `setOperationOption`, `unsetOperationOption`, `readSettings`, `writeSettings` (Tasks 3–4), `Reporter`, `Json`, `ExitCode`.
- Produces:

```cpp
namespace subedit::cli {
/// `config list`: the keys, what they are for and what they accept. No file.
[[nodiscard]] ExitCode listSettingKeys(const Reporter& reporter);
/// `config show`: the path, whether it exists, then every key with value, source and default.
[[nodiscard]] ExitCode showSettings(core::FileSystem& files, const std::filesystem::path& path, const Reporter& reporter);
/// `config set` / `config unset`: rewrite the file atomically, keeping every other setting.
[[nodiscard]] ExitCode setSetting(core::FileSystem& files, const std::filesystem::path& path,
                                  std::string_view key, std::string_view value, const Reporter& reporter);
[[nodiscard]] ExitCode unsetSetting(core::FileSystem& files, const std::filesystem::path& path,
                                    std::string_view key, const Reporter& reporter);
}
```

Output shapes (text; `--format json` writes one `reporter.record(...)` per key for `list`/`show`, plus a first record `{"file": <path>, "exists": <bool>}` for `show`):

```
$ subedit-cli config list
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a positive number of characters per second
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
```
Every description is a sentence that does not repeat the default; the `Default:` line is always printed by `list` (and `[default]`/`[file]` with the default in `show`: `duration-adjust.speed = 12   [file]   (default 15)`).
`show` prints `File: <path> (found)` or `(not found)`, then per key `<key> = <value>   [file]` or `[default]`, with `Window only` keys shown too (they are in the file), no display preference ever. `set`/`unset` say `"<key> = <value>"` / `"<key> reset to its default (<value>)"` at verbosity 1 and write nothing else.

Errors: refusals from `setOperationOption` go through `refuse(message)` (usage exit code); a failed `writeSettings` → the existing I/O exit code with `reporter.failed(...)` (look at how `writeAll` helpers in `cli/writing.cpp` report a `FileError`, and reuse its wording function). `set`/`unset` read with `readSettings`, ignore its diagnostics (the unreadable values are rewritten as defaults; say so at verbosity 2 only), mutate, `writeSettings`. They **never write when the key/value was refused**.

- [ ] **Step 1: Write the failing tests** (in-memory file system; a `Reporter` over `std::ostringstream` errors and `withTextOutput` over another stream, as other `cli` unit tests do — copy the construction from `adjusting_test.cpp`)

```cpp
TEST_CASE("config list names every catalogued key and no value", "[config]") {
    std::ostringstream out, err;
    const Reporter reporter = Reporter{err, 1}.withTextOutput(out);
    CHECK(listSettingKeys(reporter) == ExitCode::Success);
    for (const auto& key : subedit::core::operationKeys())
        CHECK(out.str().find(std::string{key.key}) != std::string::npos);
    CHECK(out.str().find("Accepts:") != std::string::npos);
    CHECK(out.str().find("Default:") != std::string::npos);
    CHECK(out.str().find("Used by: subedit-cli adjust; Adjust Durations") != std::string::npos);
}

TEST_CASE("config show says where it looked and where each value comes from", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = 12\ncolumns = nonsense\n").has_value());
    /* run showSettings; expect "found", "duration-adjust.speed = 12   [file]   (default 15)",
       "duration-adjust.gap-ms = 0   [default]", and no "columns", no "theme" */
}

TEST_CASE("config show on a missing file says not found and shows defaults", "[config]") { /* ... */ }

TEST_CASE("config set writes the value and keeps the display preferences", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "maximised = true\n").has_value());
    CHECK(setSetting(files, kPath, "duration-adjust.speed", "12", reporter) == ExitCode::Success);
    const auto read = subedit::core::readSettings(files, kPath);
    CHECK(read.settings.durationAdjustment.charactersPerSecond == 12.0);
    CHECK(read.settings.maximised);
}

TEST_CASE("config set creates the file and its directory", "[config]") { /* empty fs, path in a new dir */ }

TEST_CASE("config set with a bad value is a usage error and writes nothing", "[config]") {
    InMemoryFileSystem files;
    REQUIRE(files.writeFile(kPath, "duration-adjust.speed = 12\n").has_value());
    CHECK(setSetting(files, kPath, "duration-adjust.speed", "abc", reporter) == ExitCode::Usage);
    CHECK(*files.readFile(kPath) == "duration-adjust.speed = 12\n");
}

TEST_CASE("config set refuses a display key and an unknown key", "[config]") { /* both Usage, file untouched */ }

TEST_CASE("config unset restores the default and leaves it commented", "[config]") {
    /* start with speed 12; unset; file contains "#duration-adjust.speed = 15"; read gives default */
}

TEST_CASE("config unset on a key already at its default succeeds", "[config]") { /* Success */ }
```
Fill the elided bodies completely (assertions as described in each comment); `ExitCode::Usage` is the enumerator name in `cli/exit_code.hpp` (check it).

- [ ] **Step 2:** Run `"[config]"`. Expected: FAIL.
- [ ] **Step 3: Implement** `configuring.cpp` per the output shapes above. `list` iterates `operationKeys()`, taking `Default` from `describeOperationSettings(Settings{})` (match by key). `Used by:` prints the entry's `usedBy` text. `show` reads with `readSettings(files, path)`; `found` tells `(found)`/`(not found)`. JSON: `Json::object().set("key", …).set("value", …).set("source", "file"|"default").set("default", …).set("used_by", …).set("surface", …)`.
- [ ] **Step 4:** Run `"[config]"`. Expected: PASS.
- [ ] **Step 5: Commit** `feat(cli): list, show, set and unset operation settings`.

---

### Task 9: The `config` subcommand and its end-to-end tests

**Files:**
- Create: `src/exe/cli/commands/config.hpp`, `config.cpp` (add to `src/exe/cli/CMakeLists.txt`)
- Modify: `src/exe/cli/commands/table.cpp` (append `Command{.name = "config", .declare = declareConfig}` and the include)
- Test: `src/test/e2e/cli/config_test.cpp` (add to `src/test/e2e/CMakeLists.txt` next to the neighbouring cli tests)

**Interfaces:**
- Consumes: Task 8's four functions; `SharedSettings` (its `path`/explicit path). `config set|unset|show` must act on `--config FILE` when given, else on the user's path. `SharedSettings.path` is empty when `--no-config`; `config set --no-config` is a usage error ("--no-config has nothing to write to"); `config show --no-config` shows the defaults with `File: none (--no-config)`; `config list` ignores both options.

`SharedSettings::target` (Task 5) is the path these commands act on.

- [ ] **Step 1: Write the failing e2e tests** (`config_test.cpp`, modelled on `adjust_test.cpp`; every run passes `--config` a path inside `Scratch`)

```cpp
TEST_CASE("config list works with no file and no option", "[e2e][config]") {
    const CliRun run = invoke({"config", "list"});
    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("duration-adjust.speed"));
    CHECK_THAT(run.output, ContainsSubstring("Accepts:"));
}

TEST_CASE("config set then show then unset round-trips through the file", "[e2e][config]") {
    Scratch scratch;
    const std::string file = scratch.path("settings.conf").string();
    REQUIRE(invoke({"--config", file, "config", "set", "duration-adjust.speed", "12"}).exitCode == 0);
    const CliRun shown = invoke({"--config", file, "config", "show"});
    CHECK_THAT(shown.output, ContainsSubstring("duration-adjust.speed = 12"));
    CHECK_THAT(shown.output, ContainsSubstring(file));
    REQUIRE(invoke({"--config", file, "config", "unset", "duration-adjust.speed"}).exitCode == 0);
    CHECK_THAT(invoke({"--config", file, "config", "show"}).output,
               ContainsSubstring("duration-adjust.speed = 15"));
}

TEST_CASE("config set with a bad value exits with the usage code", "[e2e][config]") { /* exit 2, file absent */ }
TEST_CASE("config set refuses a display preference", "[e2e][config]") { /* theme -> exit 2, message "display preference" */ }
TEST_CASE("config set --no-config is a usage error", "[e2e][config]") { /* exit 2 */ }
TEST_CASE("config show --format json is one object per key", "[e2e][config][json]") { /* every output line parses as an object with "key" */ }
TEST_CASE("a value set by config set reaches adjust", "[e2e][config][adjust]") {
    /* config set duration-adjust.speed off + minimum-ms 1500, then adjust with the same --config
       gives the B2 ends of adjust_test.cpp */
}
```
Fill every elided body.

- [ ] **Step 2:** Run `ctest -R e2e.*config`. Expected: FAIL (no subcommand).
- [ ] **Step 3: Implement `config.cpp`.** CLI11: `CLI::App* config = app.add_subcommand("config", "List, show and change the operation settings")`, `config->require_subcommand(1)`; subcommands `list`, `show`, `set` (positionals `key`, `value`, both required), `unset` (positional `key`). The `Declared::command` pointer must be the `config` app; the `run` closure inspects `config->get_subcommands()` / `parsed()` to dispatch. `declareWith` does not fit (nested); build the `Declared` by hand with the four-argument `run`, as `declareWith` does. Use `shared.target` as the path; `set`/`unset` with `shared.source == SettingsSource::Disabled` → `refuse("--no-config has nothing to write to")`.
- [ ] **Step 4:** Run `ctest -R "e2e.*(config|adjust|usage)"` and `build/…/subedit-cli --help` (the new command appears). Expected: PASS.
- [ ] **Step 5: Commit** `feat(cli): the config command`.

---

### Task 10: Manual, version, gates, PR

Follow CLAUDE.md's order exactly.

**Files:**
- Create: `docs/manual/subedit-cli/config.md`
- Modify: `docs/manual/subedit-cli/index.md` (list `config`), `invocation.md` (global options `--config`, `--no-config`, the priority rule, the verbosity-1 note, where the file is), `adjust.md` and `correct.md` (what the file supplies and the priority), `docs/manual/subedit-gui/` (the page that describes the settings file or `Adjust Durations…`: where the file is, that its operation block is shared with the command line), `docs/i18n/fr/manual/…` if the structure check (#656) requires the same pages (`./src/scripts/gate.sh --list` shows the check names).
- Modify: `CMakeLists.txt` (patch bump `0.15.12` → `0.15.13`, or the next free number: `git -C /home/beber/Projects/subedit tag | sort -V | tail -3` and `gh pr list`).

Per CLAUDE.md the manual lists, for each command: name, what it does and does not do; every argument/option with long/short form and default; accepted values; output; exit codes; errors with their message; **a real example call**. `config.md` must show the real output of `config list` (first key), `config show` and `config set` captured from the built binary, and the catalogue table of the 13 honoured keys. Raccourcis tables: not applicable (no new window action).

- [ ] **Step 1:** Write the manual pages from the built binary's actual output (`build/debug/.../subedit-cli config list | head`).
- [ ] **Step 2: Bump the patch** in `CMakeLists.txt`.
- [ ] **Step 3:** `make manual` (the version example is now stale).
- [ ] **Step 4:** `make check-local` (runs `manual-check`, `check-cli-manual.py`, and `make bench` last — if the machine is busy it records nothing: say so in the PR, once, do not loop).
- [ ] **Step 5:** `make check` once (≈17 min). If a step fails, fix it and resume with `./src/scripts/gate.sh check --from <step>`; do not rerun the whole gate.
- [ ] **Step 6:** Update the rest of the docs not read by the gate: roadmap entry for phase 17 if the roadmap lists phases (`grep -n "16" docs/feuille-de-route.md`), the benchmark journal line from step 4, and the spec's status.
- [ ] **Step 7:** Commit, regenerate the CHANGELOG with the project's script (see how earlier commits `docs(doc): journal des changements` were produced: `grep -rn changelog Makefile src/scripts | head`), push `feat/699`, and open the PR. Title (English, ≤ 72, validated by `./src/scripts/check-commit-message.sh`): `feat(cli): share operation settings with the window, add config`. Body ends with a line `Closes #699` and states which gate steps were run and whether `make bench` recorded a measurement.
- [ ] **Step 8:** After merge, tag `vX.Y.Z` on the merge commit in `main` (CLAUDE.md, "Versions"): do not tag before the user confirms the merge.
