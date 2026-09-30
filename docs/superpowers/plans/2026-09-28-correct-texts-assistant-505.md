# Correct Texts… Assistant Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build `Tools ▸ Correct Texts…`, the GUI assistant of issue #505 — a wizard that picks a target and tasks, lets the user tune each task's patterns, computes proposed corrections at the core (`proposeCorrections`, issue #504), and lets the user accept, refuse or retouch each change before applying it.

**Architecture:** A `CorrectionController` collaborator (ADR 0034/0035 style: a `View` interface `MainWindow` implements through a nested `CorrectionSide`, wired in the constructor exactly like `ProjectOperations`/`ProjectSearch`) owns a `QWizard` (`CorrectionWizard`). The wizard's pages are thin Qt views over pure, independently-tested functions: target resolution (`correctionTargetsOf`), pattern-code decomposition (`pattern_code.hpp`), a text diff (`core::diffTexts`), and the pattern list per task. The heavy computation (`proposeCorrections`) runs on a background thread from a dedicated progress page so the wizard's own Cancel button gives "abandon" for free. Applying is one `CompositeCommand` per project, going through each project's own `Session`, exactly as `ProjectOperations::apply` already does for a single page.

**Tech Stack:** C++23, Qt 6 (QWizard, QAbstractTableModel, QFutureWatcher/QtConcurrent), ICU (already a dependency via `IcuPatternEngine`), Catch2.

**Spec:** [`docs/specs/12-correction.md`](../../specs/12-correction.md), decision D8 (and D4, D7 it cross-references) — issue body of [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505).

## Global Constraints

- C++ identifiers and comments in English; everything the binary prints (labels, window titles, status bar text) in English — French only in commit messages, this plan, and doc updates.
- A shared sentence between the window and a future CLI (the tally: "N subtitle(s) corrected, M removed") lives in `core/wording.hpp`, per D8.
- `proposeCorrections`/`applyCorrections` touch no project; applying goes through `Session::apply` so undo stays one entry per project (D8).
- A pattern's activation, the two D7 "Sound in…" checkboxes, the line-break limits and the "remove blank subtitles" checkbox are all `CorrectionSettings`, already wired into `Settings`/ADR 0022 persistence (issue #504) — the GUI reads and writes `settings().correction`, it does not reinvent storage.
- No test reads `reference/gaupol`, `~/.config/subedit`, or resolves a real environment variable itself — paths are received, not resolved, inside `core` and inside anything under test (ADR 0022, ADR 0037).
- `make check` is the gate: format, warnings-as-errors, clang-tidy, ASan, coverage, no file left behind. Any test that writes goes through the `Scratch` harness, never a bare filename.
- Every new file: English comments only where the WHY is non-obvious; no comment restating what a well-named identifier already says.

## Review Focus

- **A subtitle a pattern cannot read/translate/terminate on** (`PatternFailure`) must be named in the confirmation page while the rest of the run still applies (GUI-CORRECT-06) — #504 currently drops these on the floor inside `proposeCorrections`; Task 1 fixes that before anything else is built on top of it.
- **A project with no translation, targeted with "Translation" and "all open projects"** must not silently propose garbage from an empty translation column — `correctionTargetsOf` must leave such a project out of the target list entirely.
- **Cancelling the wizard while the background computation is still running** must not crash or apply anything — the computation must not touch any `QObject` or `Project` that the wizard's cancellation may have already torn down.
- **A confirmation table with all rows unchecked** must apply nothing and enter no history — mirrors the existing rule "an operation that changes nothing is not an operation to undo" (`ProjectOperations::removeHearingImpaired`).
- **Retouching the proposed text of a row to equal the original** must not be silently accepted as a no-op change that still creates a command — an accepted row whose edited text equals the original must be excluded from what `applyCorrections` receives, the same way `proposeCorrections` never reports a text a task left untouched.

---

## Task 1: Core — surface pattern failures from `proposeCorrections`

**Files:**
- Modify: `src/lib/subedit/core/text/correction_run.hpp`
- Modify: `src/lib/subedit/core/text/correction_run.cpp`
- Modify: `src/test/unit/core/text/correction_run_test.cpp`

**Interfaces:**
- Consumes: `PatternFailure` (`src/lib/subedit/core/text/common_errors.hpp`), `HearingImpairedCorrection::failures` (`hearing_impaired_correction.hpp`), `CorrectedTexts::failures` (`common_errors.hpp`, reused by `capitalization.hpp`).
- Produces: `struct CorrectionProposal { std::vector<ProposedCorrection> corrections; std::vector<PatternFailure> failures; };` and `[[nodiscard]] CorrectionProposal proposeCorrections(...)` (same parameters as today). Every later task that calls `proposeCorrections` reads `.corrections` and `.failures` off this type.

Gaupol's assistant names a pattern that cannot be read, translated, or that never terminates (GUI-CORRECT-06), but today `runTasks` in `correction_run.cpp` calls `correctHearingImpaired`, `correctCommonErrors` and `correctCapitalization` and throws away their `.failures`. This task makes `proposeCorrections` collect and de-duplicate them (by `kind`+`code`+`rank`, since the same compile failure would otherwise be reported once per target) before any GUI code is written to display them.

- [ ] **Step 1: Write the failing tests**

Add to `src/test/unit/core/text/correction_run_test.cpp`, after the existing `#include` block add `#include <subedit/core/text/common_errors.hpp>` and `using subedit::core::FailureKind; using subedit::core::PatternFailure;`. Then change every existing `const std::vector<ProposedCorrection> proposed = proposeCorrections(...)` declaration to `const CorrectionProposal proposed = proposeCorrections(...)` (mechanical — ten call sites), and update every subsequent `proposed.size()` / `proposed[i]` / `proposed` passed to `applyCorrections` to `proposed.corrections.size()` / `proposed.corrections[i]` / `proposed.corrections`. Add `using subedit::core::CorrectionProposal;` to the `using` block.

Add one new test. It builds its catalogue the way `pattern_catalogue_test.cpp` already does — an `InMemoryFileSystem` holding a hand-written `.common-error` file — rather than the shipped patterns, so it owns the one broken record it needs:

```cpp
TEST_CASE("a pattern that will not compile is named, and the others still apply",
          "[text][assistant]") {
    subedit::core::InMemoryFileSystem files;
    files.addFile("/patterns/Zyyy.common-error",
                  "# -*- conf -*-\n"
                  "\n[Common Error Pattern]\nName=Broken\nClasses=Human;OCR;\nPattern=(\n"
                  "\n[Common Error Pattern]\nName=Double space\nClasses=Human;OCR;\n"
                  "Pattern=  +\nReplacement= \n");
    const PatternCatalogue catalogue =
        subedit::core::readPatternCatalogue(files, "/patterns", {});
    REQUIRE(catalogue.diagnostics().empty()); // both records read fine; only compiling fails

    Project project = projectOf({"Bonjour  Marie"});
    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};

    const std::vector<CorrectionTarget> targets{wholeProject(project)};
    const CharacterLineMeasure measure;
    const CorrectionProposal proposal =
        proposeCorrections(IcuPatternEngine{}, catalogue, settings, measure, targets);

    REQUIRE(proposal.corrections.size() == 1);
    CHECK(proposal.corrections[0].proposed == std::optional<std::string>{"Bonjour Marie"});
    REQUIRE(proposal.failures.size() == 1);
    CHECK(proposal.failures[0].kind == FailureKind::CompileError);
    CHECK(proposal.failures[0].name == "Broken");
}
```

Add `#include <subedit/core/io/in_memory_file_system.hpp>` and `using subedit::core::InMemoryFileSystem;` if not already present in this file's `using` block (they are not, today).

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_unit_test && ./build/debug/src/test/unit/subedit_unit_test "a pattern that will not compile is named*"`
Expected: FAIL to compile (`CorrectionProposal` does not exist yet) — confirms the test exercises new API, not a passing accident.

- [ ] **Step 3: Add `CorrectionProposal` and collect failures**

In `correction_run.hpp`, add after the `CorrectionTarget` struct:

```cpp
#include <subedit/core/text/common_errors.hpp> // for PatternFailure
```

and change the declaration:

```cpp
/// What `proposeCorrections` answered: the changes it would make, and every
/// pattern that could not do its part — named once each, however many texts
/// or targets it was asked to work on (GUI-CORRECT-06).
struct CorrectionProposal {
    std::vector<ProposedCorrection> corrections;
    std::vector<PatternFailure> failures;
};

[[nodiscard]] CorrectionProposal
proposeCorrections(const PatternEngine& engine,
                   const PatternCatalogue& catalogue,
                   const CorrectionSettings& settings,
                   const LineMeasure& measure,
                   std::span<const CorrectionTarget> targets);
```

In `correction_run.cpp`, change `runTasks` to take an out-parameter:

```cpp
void runTasks(const PatternEngine& engine,
              const PatternCatalogue& catalogue,
              const CorrectionSettings& settings,
              const LineMeasure& measure,
              const CorrectionTarget& target,
              SubtitleFormat format,
              std::vector<std::optional<std::string>>& texts,
              std::vector<PatternFailure>& failures) {
    if (settings.mentions.enabled) {
        // ... unchanged selection of patterns ...
        const HearingImpairedCorrection done = correctHearingImpaired(/* unchanged args */);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
        // ... unchanged loop over present.at ...
    }

    if (settings.commonErrors.enabled) {
        // ...
        const CorrectedTexts done = correctCommonErrors(engine, patterns, present.texts, format);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
        // ...
    }

    if (settings.capitalization.enabled) {
        // ...
        const CorrectedTexts done = correctCapitalization(engine, patterns, present.texts, format);
        failures.insert(failures.end(), done.failures.begin(), done.failures.end());
        // ...
    }

    // lineBreak unchanged: breakLines does not report PatternFailure.
}
```

Then in `proposeCorrections`:

```cpp
CorrectionProposal proposeCorrections(const PatternEngine& engine,
                                      const PatternCatalogue& catalogue,
                                      const CorrectionSettings& settings,
                                      const LineMeasure& measure,
                                      std::span<const CorrectionTarget> targets) {
    CorrectionProposal proposal;

    for (const CorrectionTarget& target : targets) {
        // ... unchanged indices/originals/texts setup ...
        runTasks(engine, catalogue, settings, measure, target, format, texts, proposal.failures);
        // ... unchanged loop pushing into proposal.corrections instead of result ...
    }

    // De-duplicate: the same pattern reports the same compile failure once per
    // target it was asked to run on, and the confirmation names it once.
    std::ranges::sort(proposal.failures, {}, [](const PatternFailure& f) {
        return std::tuple{static_cast<int>(f.kind), f.code, f.rank, f.text.value_or(0)};
    });
    proposal.failures.erase(
        std::ranges::unique(proposal.failures,
                            [](const PatternFailure& a, const PatternFailure& b) {
                                return a.kind == b.kind && a.code == b.code && a.rank == b.rank &&
                                       a.name == b.name && a.text == b.text;
                            }).begin(),
        proposal.failures.end());
    return proposal;
}
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_unit_test && ./build/debug/src/test/unit/subedit_unit_test "[assistant]"`
Expected: PASS, all `correction_run_test.cpp` cases including the new one.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/core/text/correction_run.hpp src/lib/subedit/core/text/correction_run.cpp src/test/unit/core/text/correction_run_test.cpp
git commit -m "feat(core): report the patterns proposeCorrections could not apply"
```

## Task 2: Core — a codepoint-level text diff for the confirmation page

**Files:**
- Create: `src/lib/subedit/core/text/text_diff.hpp`
- Create: `src/lib/subedit/core/text/text_diff.cpp`
- Test: `src/test/unit/core/text/text_diff_test.cpp`
- Modify: `src/lib/subedit/CMakeLists.txt` (add the two new files where `correction_run.cpp` is listed — grep the file for `correction_run` to find the right `target_sources` block, for both the library and the unit test target if they are listed separately)

**Interfaces:**
- Produces: `struct DiffSpan { std::string text; bool changed = false; };`, `struct TextDiff { std::vector<DiffSpan> original; std::vector<DiffSpan> proposed; };`, `[[nodiscard]] TextDiff diffTexts(std::string_view original, std::string_view proposed);`. Task 6 (`correction_diff_view`) is this task's only consumer.

Marks what changed between a `ProposedCorrection`'s `original` and `proposed` texts, one span sequence per side, so the confirmation table can render each with the changed part highlighted — the same idea as Gaupol's `MultilineDiffCellRenderer`, reimplemented rather than read from `reference/gaupol` at runtime. A longest-common-subsequence diff over Unicode code points (never bytes, so a span boundary cannot fall inside a multi-byte character).

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/unit/core/text/text_diff_test.cpp
#include <subedit/core/text/text_diff.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
using subedit::core::diffTexts;
} // namespace

TEST_CASE("identical texts answer as one unchanged span each", "[text][diff]") {
    const auto diff = diffTexts("Bonjour", "Bonjour");

    REQUIRE(diff.original.size() == 1);
    CHECK_FALSE(diff.original[0].changed);
    CHECK(diff.original[0].text == "Bonjour");
    REQUIRE(diff.proposed.size() == 1);
    CHECK_FALSE(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "Bonjour");
}

TEST_CASE("two texts with nothing in common are each one changed span", "[text][diff]") {
    const auto diff = diffTexts("abc", "xyz");

    REQUIRE(diff.original.size() == 1);
    CHECK(diff.original[0].changed);
    CHECK(diff.original[0].text == "abc");
    REQUIRE(diff.proposed.size() == 1);
    CHECK(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "xyz");
}

TEST_CASE("text appended at the end is its own changed span, and keeps a "
          "multi-byte character whole",
          "[text][diff]") {
    const auto diff = diffTexts("café", "cafés");

    REQUIRE(diff.original.size() == 1);
    CHECK_FALSE(diff.original[0].changed);
    CHECK(diff.original[0].text == "café");

    REQUIRE(diff.proposed.size() == 2);
    CHECK_FALSE(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "café");
    CHECK(diff.proposed[1].changed);
    CHECK(diff.proposed[1].text == "s");
}

TEST_CASE("text removed from the end is its own changed span, on the original only",
          "[text][diff]") {
    const auto diff = diffTexts("Bonjour!", "Bonjour");

    REQUIRE(diff.original.size() == 2);
    CHECK_FALSE(diff.original[0].changed);
    CHECK(diff.original[0].text == "Bonjour");
    CHECK(diff.original[1].changed);
    CHECK(diff.original[1].text == "!");

    REQUIRE(diff.proposed.size() == 1);
    CHECK_FALSE(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "Bonjour");
}

TEST_CASE("an empty text on one side answers as no spans on that side", "[text][diff]") {
    const auto diff = diffTexts("", "Bonjour");

    CHECK(diff.original.empty());
    REQUIRE(diff.proposed.size() == 1);
    CHECK(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "Bonjour");
}
```

Add this file to the unit test target's sources in `src/lib/subedit/CMakeLists.txt` (or wherever `correction_run_test.cpp` is listed — grep for it) in the same step, so Step 2 below can build it.

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_unit_test 2>&1 | head -30`
Expected: FAIL to compile — `text_diff.hpp` does not exist yet.

- [ ] **Step 3: Implement `diffTexts`**

```cpp
// src/lib/subedit/core/text/text_diff.hpp
#pragma once

// A codepoint-level diff of two texts, for the correction assistant's
// confirmation page — decision D8, issue #505. It says what changed, never
// why: the correction itself is `proposeCorrections`' business.

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// One run of a text: either matches the other side, or was changed.
struct DiffSpan {
    std::string text;
    bool changed = false;

    friend bool operator==(const DiffSpan&, const DiffSpan&) = default;
};

/// `original` and `proposed`, each as spans marking what differs between the
/// two — two sequences, since a changed run need not be the same length on
/// both sides.
struct TextDiff {
    std::vector<DiffSpan> original;
    std::vector<DiffSpan> proposed;
};

/// A longest-common-subsequence diff on Unicode code points: a span never
/// splits one apart.
[[nodiscard]] TextDiff diffTexts(std::string_view original, std::string_view proposed);

} // namespace subedit::core
```

```cpp
// src/lib/subedit/core/text/text_diff.cpp
#include <subedit/core/text/text_diff.hpp>

#include <algorithm>
#include <cstddef>

namespace subedit::core {

namespace {

/// `text` split into its codepoints, each kept as the UTF-8 bytes it was
/// written in — spans are rebuilt by joining runs of these, never by slicing
/// a multi-byte sequence in two. A malformed lead byte is read as one byte,
/// the same width `decodeToUtf8` would already have refused upstream.
[[nodiscard]] std::vector<std::string_view> codepointsOf(std::string_view text) {
    std::vector<std::string_view> points;
    std::size_t i = 0;
    while (i < text.size()) {
        const auto lead = static_cast<unsigned char>(text[i]);
        std::size_t length = 1;
        if ((lead & 0xE0) == 0xC0)
            length = 2;
        else if ((lead & 0xF0) == 0xE0)
            length = 3;
        else if ((lead & 0xF8) == 0xF0)
            length = 4;
        length = std::min(length, text.size() - i);
        points.push_back(text.substr(i, length));
        i += length;
    }
    return points;
}

/// `points[at]`, joined back into whole spans of matched and changed runs.
[[nodiscard]] std::vector<DiffSpan> spansOf(const std::vector<std::string_view>& points,
                                            const std::vector<bool>& changedAt) {
    std::vector<DiffSpan> spans;
    std::size_t start = 0;
    while (start < points.size()) {
        std::size_t end = start;
        while (end < points.size() && changedAt[end] == changedAt[start])
            ++end;
        std::string text;
        for (std::size_t i = start; i < end; ++i)
            text += points[i];
        spans.push_back(DiffSpan{.text = std::move(text), .changed = changedAt[start]});
        start = end;
    }
    return spans;
}

} // namespace

TextDiff diffTexts(std::string_view original, std::string_view proposed) {
    const std::vector<std::string_view> left = codepointsOf(original);
    const std::vector<std::string_view> right = codepointsOf(proposed);

    // Longest common subsequence by dynamic programming — a subtitle line is
    // a handful of words, so the O(n*m) table costs nothing here.
    const std::size_t rows = left.size() + 1;
    const std::size_t cols = right.size() + 1;
    std::vector<std::vector<int>> lcs(rows, std::vector<int>(cols, 0));
    for (std::size_t i = 1; i < rows; ++i) {
        for (std::size_t j = 1; j < cols; ++j) {
            lcs[i][j] = left[i - 1] == right[j - 1] ? lcs[i - 1][j - 1] + 1
                                                    : std::max(lcs[i - 1][j], lcs[i][j - 1]);
        }
    }

    std::vector<bool> leftChanged(left.size(), true);
    std::vector<bool> rightChanged(right.size(), true);
    std::size_t i = left.size();
    std::size_t j = right.size();
    while (i > 0 && j > 0) {
        if (left[i - 1] == right[j - 1]) {
            leftChanged[i - 1] = false;
            rightChanged[j - 1] = false;
            --i;
            --j;
        } else if (lcs[i - 1][j] >= lcs[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    return TextDiff{.original = spansOf(left, leftChanged), .proposed = spansOf(right, rightChanged)};
}

} // namespace subedit::core
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_unit_test && ./build/debug/src/test/unit/subedit_unit_test "[diff]"`
Expected: PASS, all five cases.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/core/text/text_diff.hpp src/lib/subedit/core/text/text_diff.cpp src/test/unit/core/text/text_diff_test.cpp src/lib/subedit/CMakeLists.txt
git commit -m "feat(core): add a codepoint-level text diff for the correction assistant"
```

## Task 3: GUI — resolve where the pattern files live

**Files:**
- Create: `src/lib/subedit/gui/patterns_path.hpp`
- Create: `src/lib/subedit/gui/patterns_path.cpp`
- Test: `src/test/gui/patterns_path_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt` (add `patterns_path.cpp` to the `add_library(subedit_gui STATIC ...)` list)
- Modify: `src/test/gui/CMakeLists.txt` (add `patterns_path_test.cpp` to the test source list)

**Interfaces:**
- Consumes: `core::shippedPatternsPath(const std::filesystem::path&)`, `core::userPatternsPath(std::string_view, std::string_view)` (`src/lib/subedit/core/text/pattern_paths.hpp`, already built by #498/#499, unused so far).
- Produces: `[[nodiscard]] std::filesystem::path installedPatternsPath();` and `[[nodiscard]] std::filesystem::path resolvedUserPatternsPath();`. Task 15 (`main.cpp` wiring) is the only caller.

Nothing in `gui` resolves a pattern directory yet — `core::readPatternCatalogue` deliberately receives its two paths rather than finding them, per ADR 0037, so one piece of code has to do the finding with the real executable and the real environment. `installedManualPath()` and `userSettingsPath()` are the two existing examples of exactly this seam; this task adds the third.

- [ ] **Step 1: Write the failing test**

```cpp
// src/test/gui/patterns_path_test.cpp
#include <subedit/gui/patterns_path.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <string>

TEST_CASE("the installed patterns directory sits beside the manual, under the executable",
          "[gui][patterns]") {
    const std::filesystem::path installed = subedit::gui::installedPatternsPath();

    // `<bin>/../share/subedit/patterns` — the layout GNUInstallDirs produces,
    // the same one `installedManualPath()` already reaches for the manual.
    CHECK(installed.filename() == "patterns");
    CHECK(installed.parent_path().filename() == "subedit");
    CHECK(installed.parent_path().parent_path().filename() == "share");
}

TEST_CASE("the user's patterns directory follows XDG_DATA_HOME when it is set",
          "[gui][patterns]") {
    // `qputenv`/`qgetenv` would move the *process* environment, which a test
    // run in parallel with others must not do — `core::userPatternsPath` is a
    // pure function of the strings it is given, so this test reads the
    // process's actual variables through it without setting anything.
    const std::string xdg = std::getenv("XDG_DATA_HOME") != nullptr ? std::getenv("XDG_DATA_HOME") : "";
    const std::string home = std::getenv("HOME") != nullptr ? std::getenv("HOME") : "";
    const std::filesystem::path expected = subedit::core::userPatternsPath(xdg, home);

    CHECK(subedit::gui::resolvedUserPatternsPath() == expected);
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile — `patterns_path.hpp` does not exist yet.

- [ ] **Step 3: Implement the two resolvers**

```cpp
// src/lib/subedit/gui/patterns_path.hpp
#pragma once

#include <filesystem>

namespace subedit::gui {

/// Where the shipped patterns are installed, worked out from the executable —
/// never from a path frozen at build time, the same rule `installedManualPath()`
/// follows and for the same reason: the configure prefix and the install
/// prefix are not the same.
[[nodiscard]] std::filesystem::path installedPatternsPath();

/// Where a user drops patterns of her own, read from the real environment —
/// the one place allowed to, since `core::userPatternsPath` itself takes the
/// two strings rather than reading them (ADR 0037, ADR 0022's own rule for
/// `userSettingsPath()`).
[[nodiscard]] std::filesystem::path resolvedUserPatternsPath();

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/patterns_path.cpp
#include <subedit/gui/patterns_path.hpp>

#include <subedit/core/text/pattern_paths.hpp>

#include <QByteArray>
#include <QCoreApplication>
#include <QString>

namespace subedit::gui {

std::filesystem::path installedPatternsPath() {
    const QString binaries = QCoreApplication::applicationDirPath();
    return core::shippedPatternsPath(std::filesystem::path{binaries.toStdString()});
}

std::filesystem::path resolvedUserPatternsPath() {
    const QByteArray xdg = qgetenv("XDG_DATA_HOME");
    const QByteArray home = qgetenv("HOME");
    return core::userPatternsPath(std::string_view{xdg.constData(), static_cast<std::size_t>(xdg.size())},
                                  std::string_view{home.constData(), static_cast<std::size_t>(home.size())});
}

} // namespace subedit::gui
```

Check `core::shippedPatternsPath`'s actual layout (`<prefix>/bin` → `<prefix>/share/subedit/patterns`, per its own doc comment in `pattern_paths.hpp`) against `installedManualPath()`'s `.. / "share" / "subedit" / "manual"` — if `shippedPatternsPath` already does the `applicationDirPath() / ".." / ...` normalisation internally (read its `.cpp` before writing this step for real), this function is only the `QCoreApplication::applicationDirPath()` call and the delegation; do not re-derive the layout by hand if the core function already owns it.

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[patterns]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/patterns_path.hpp src/lib/subedit/gui/patterns_path.cpp src/test/gui/patterns_path_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): resolve the installed and user pattern directories"
```

## Task 4: GUI — resolve the correction target across one or every open project

**Files:**
- Create: `src/lib/subedit/gui/correction_target.hpp`
- Create: `src/lib/subedit/gui/correction_target.cpp`
- Test: `src/test/gui/correction_target_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `ProjectPage` (`project_page.hpp`: `.session->project()`, `.tableSelection`), `subedit::gui::selectionOf` (`target.hpp`), `core::CorrectionTarget`, `core::Selection::all`, `core::Document`, `core::Project::translationFile()`.
- Produces: `enum class CorrectionScope { Selection, CurrentProject, AllProjects };` and `[[nodiscard]] std::vector<core::CorrectionTarget> correctionTargetsOf(CorrectionScope scope, core::Document document, std::span<const std::unique_ptr<ProjectPage>> pages, std::size_t currentPage);`. Task 10 (`correction_target_page`) and Task 15 (`correction_controller`) are the callers.

D8 names three scopes — the selection, the current project, every open project — that GUI code has no precedent for combining, unlike `targetOf`/`selectionOf` in `target.hpp` which only ever look at one project. This is the first place that resolves "every open project" into `CorrectionTarget`s; `ProjectSearch`'s own `replaceAllAcrossProjects` walks `View::projectCount()`/`View::project(int)` the same way but never turns the walk into a reusable value, so there is nothing to call, only a shape to follow.

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/gui/correction_target_test.cpp
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/project_page.hpp>

#include <QItemSelectionModel>
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::correctionTargetsOf;
using subedit::gui::CorrectionScope;
using subedit::gui::ProjectPage;

constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                            "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n";

[[nodiscard]] std::unique_ptr<ProjectPage> pageOn(const char* content) {
    InMemoryFileSystem files;
    files.addFile("/film.srt", content);
    auto opened = openProject(files, "/film.srt");
    REQUIRE(opened.has_value());
    return ProjectPage::make(std::move(opened->project));
}

} // namespace

TEST_CASE("the selection scope reads the current page's own selection, and nothing else",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo));
    pages.push_back(pageOn(kTwo));
    pages[0]->tableSelection->select(pages[0]->model->index(1, 0),
                                     QItemSelectionModel::Select | QItemSelectionModel::Rows);

    const auto targets =
        correctionTargetsOf(CorrectionScope::Selection, Document::Main, pages, 0);

    REQUIRE(targets.size() == 1);
    CHECK(targets[0].project == &pages[0]->session->project());
    CHECK(targets[0].selection.count() == 1);
}

TEST_CASE("the current-project scope takes the whole file of the page on screen",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo));
    pages.push_back(pageOn(kTwo));

    const auto targets =
        correctionTargetsOf(CorrectionScope::CurrentProject, Document::Main, pages, 1);

    REQUIRE(targets.size() == 1);
    CHECK(targets[0].project == &pages[1]->session->project());
    CHECK(targets[0].selection.count() == 2);
}

TEST_CASE("the all-projects scope takes every open project, whole", "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo));
    pages.push_back(pageOn(kTwo));

    const auto targets =
        correctionTargetsOf(CorrectionScope::AllProjects, Document::Main, pages, 0);

    REQUIRE(targets.size() == 2);
    CHECK(targets[0].project == &pages[0]->session->project());
    CHECK(targets[1].project == &pages[1]->session->project());
}

TEST_CASE("a project with no translation is left out when the target is the translation",
          "[gui][correction]") {
    std::vector<std::unique_ptr<ProjectPage>> pages;
    pages.push_back(pageOn(kTwo)); // no translation file opened

    const auto targets =
        correctionTargetsOf(CorrectionScope::AllProjects, Document::Translation, pages, 0);

    CHECK(targets.empty());
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement `correctionTargetsOf`**

```cpp
// src/lib/subedit/gui/correction_target.hpp
#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/text/correction_run.hpp>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace subedit::gui {

struct ProjectPage;

/// The three targets D8 names for the correction assistant's first page.
enum class CorrectionScope {
    Selection,     ///< the rows selected in the page on screen
    CurrentProject, ///< the whole file of the page on screen
    AllProjects,   ///< every open project, whole
};

/// Resolves `scope` and `document` into the concrete `CorrectionTarget`s
/// `proposeCorrections` takes — the window's job, never `proposeCorrections`'
/// own (issue #504's own doc comment on `CorrectionTarget`).
///
/// A project with no translation of its own is left out when `document` is
/// `Translation`: proposing corrections to an empty column that only follows
/// the main text would report every line of it as "changed" from nothing.
[[nodiscard]] std::vector<core::CorrectionTarget>
correctionTargetsOf(CorrectionScope scope,
                    core::Document document,
                    std::span<const std::unique_ptr<ProjectPage>> pages,
                    std::size_t currentPage);

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_target.cpp
#include <subedit/gui/correction_target.hpp>

#include <subedit/core/model/project.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/target.hpp>

namespace subedit::gui {

namespace {

[[nodiscard]] bool carries(const core::Project& project, core::Document document) {
    return document == core::Document::Main || project.translationFile().has_value();
}

} // namespace

std::vector<core::CorrectionTarget>
correctionTargetsOf(CorrectionScope scope,
                    core::Document document,
                    std::span<const std::unique_ptr<ProjectPage>> pages,
                    std::size_t currentPage) {
    std::vector<core::CorrectionTarget> targets;

    if (scope == CorrectionScope::AllProjects) {
        for (const std::unique_ptr<ProjectPage>& page : pages) {
            core::Project& project = page->session->project();
            if (!carries(project, document))
                continue;
            targets.push_back(core::CorrectionTarget{
                .project = &project, .selection = core::Selection::all(project), .document = document});
        }
        return targets;
    }

    ProjectPage& page = *pages[currentPage];
    core::Project& project = page.session->project();
    if (!carries(project, document))
        return targets;

    const core::Selection selection = scope == CorrectionScope::Selection
                                          ? selectionOf(*page.tableSelection)
                                          : core::Selection::all(project);
    if (selection.count() == 0)
        return targets;

    targets.push_back(
        core::CorrectionTarget{.project = &project, .selection = selection, .document = document});
    return targets;
}

} // namespace subedit::gui
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction]"`
Expected: PASS, all four cases.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_target.hpp src/lib/subedit/gui/correction_target.cpp src/test/gui/correction_target_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): resolve the correction assistant's target across open projects"
```

## Task 5: GUI — decompose a pattern code into script, language, country

**Files:**
- Create: `src/lib/subedit/gui/pattern_code.hpp`
- Create: `src/lib/subedit/gui/pattern_code.cpp`
- Test: `src/test/gui/pattern_code_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `core::PatternCatalogue::patterns()`, `core::CorrectionPattern::code`/`kind()`, `core::PatternKind` (`pattern_catalogue.hpp`, `correction_pattern.hpp`).
- Produces: `struct PatternCodeParts { std::string script; std::string language; std::string country; };`, `[[nodiscard]] PatternCodeParts splitCode(std::string_view code);`, `[[nodiscard]] std::string joinCode(const PatternCodeParts& parts);`, `[[nodiscard]] std::vector<std::string> scriptsOf(const core::PatternCatalogue&, core::PatternKind);`, `[[nodiscard]] std::vector<std::string> languagesOf(const core::PatternCatalogue&, core::PatternKind, std::string_view script);`, `[[nodiscard]] std::vector<std::string> countriesOf(const core::PatternCatalogue&, core::PatternKind, std::string_view script, std::string_view language);`. Task 7 (`pattern_code_selector`) is the consumer.

A code is `Script[-language[-COUNTRY]]`, or `Zyyy` for every script (`correction_settings.hpp`'s own doc comment on `TaskSettings::code`) — there is no ISO-15924/639/3166 registry anywhere in this codebase, and building one to populate three generic combo boxes would be scope the spec never asks for. **Decision for this plan**: the three combos on each task page (script, language, country) are populated from the codes the catalogue actually carries for that pattern kind, decomposed, rather than from an external locale database — cascading, so the language combo only offers what exists under the chosen script and the country combo only what exists under the chosen script and language. This is a deliberate simplification over Gaupol's `LocalePage`, worth a line in the spec's "Précisé par #505" once merged (Task 16).

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/gui/pattern_code_test.cpp
#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_code.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::PatternKind;
using subedit::gui::joinCode;
using subedit::gui::PatternCodeParts;
using subedit::gui::splitCode;
} // namespace

TEST_CASE("splitting and joining a code round-trip at every depth", "[gui][pattern-code]") {
    CHECK(splitCode("Zyyy") == PatternCodeParts{.script = "Zyyy"});
    CHECK(splitCode("Latn") == PatternCodeParts{.script = "Latn"});
    CHECK(splitCode("Latn-en") == PatternCodeParts{.script = "Latn", .language = "en"});
    CHECK(splitCode("Latn-en-US") ==
          PatternCodeParts{.script = "Latn", .language = "en", .country = "US"});

    CHECK(joinCode(PatternCodeParts{.script = "Latn", .language = "en", .country = "US"}) ==
          "Latn-en-US");
    CHECK(joinCode(PatternCodeParts{.script = "Latn", .language = "en"}) == "Latn-en");
    CHECK(joinCode(PatternCodeParts{.script = "Latn"}) == "Latn");
    CHECK(joinCode(PatternCodeParts{}) == "Zyyy");
}
```

For `scriptsOf`/`languagesOf`/`countriesOf`, use the same `pattern_catalogue_test.cpp` `InMemoryFileSystem` + `readPatternCatalogue` pattern from Task 1's test, writing a `.common-error` file that ships records coded `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` and `Latn-fr` — check the exact required keys (`Name=`, `Classes=`, `Pattern=`) by re-reading `pattern_catalogue_test.cpp`'s `commonErrors()` helper (Task 1 already found it) rather than guessing the format again. Add:

```cpp
TEST_CASE("the three combos read what the catalogue actually carries, cascading",
          "[gui][pattern-code]") {
    // catalogue built as above, with Zyyy, Latn, Latn-en, Latn-en-US, Latn-fr

    CHECK(scriptsOf(catalogue, PatternKind::CommonError) == std::vector<std::string>{"Zyyy", "Latn"});
    CHECK(languagesOf(catalogue, PatternKind::CommonError, "Latn") ==
          std::vector<std::string>{"en", "fr"});
    CHECK(countriesOf(catalogue, PatternKind::CommonError, "Latn", "en") ==
          std::vector<std::string>{"US"});
    // A script with only a bare code offers no language at all.
    CHECK(languagesOf(catalogue, PatternKind::CommonError, "Zyyy").empty());
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/pattern_code.hpp
#pragma once

#include <subedit/core/text/correction_pattern.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

namespace subedit::gui {

/// A code's three parts — `Script[-language[-COUNTRY]]`, or all empty for the
/// bare `Zyyy`.
struct PatternCodeParts {
    std::string script;
    std::string language;
    std::string country;

    friend bool operator==(const PatternCodeParts&, const PatternCodeParts&) = default;
};

[[nodiscard]] PatternCodeParts splitCode(std::string_view code);

/// `Zyyy` for an empty script — the one code that names no script at all.
[[nodiscard]] std::string joinCode(const PatternCodeParts& parts);

/// The scripts the catalogue carries records of `kind` under, `Zyyy` first —
/// **not an external locale list**: what is not in the catalogue is not
/// offered, since nothing would come of choosing it.
[[nodiscard]] std::vector<std::string> scriptsOf(const core::PatternCatalogue& catalogue,
                                                 core::PatternKind kind);

/// The languages the catalogue carries under `script`, for `kind`.
[[nodiscard]] std::vector<std::string>
languagesOf(const core::PatternCatalogue& catalogue, core::PatternKind kind, std::string_view script);

/// The countries the catalogue carries under `script`-`language`, for `kind`.
[[nodiscard]] std::vector<std::string> countriesOf(const core::PatternCatalogue& catalogue,
                                                    core::PatternKind kind,
                                                    std::string_view script,
                                                    std::string_view language);

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/pattern_code.cpp
#include <subedit/gui/pattern_code.hpp>

#include <subedit/core/text/pattern_catalogue.hpp>

#include <algorithm>

namespace subedit::gui {

PatternCodeParts splitCode(std::string_view code) {
    if (code == "Zyyy")
        return {};
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
template <typename Read>
[[nodiscard]] std::vector<std::string>
distinctOf(const core::PatternCatalogue& catalogue, core::PatternKind kind, Read read) {
    std::vector<std::string> found;
    for (const core::CorrectionPattern& pattern : catalogue.patterns()) {
        if (pattern.kind() != kind)
            continue;
        std::string value = read(splitCode(pattern.code));
        if (!value.empty() && !std::ranges::contains(found, value))
            found.push_back(value);
    }
    return found;
}

} // namespace

std::vector<std::string> scriptsOf(const core::PatternCatalogue& catalogue, core::PatternKind kind) {
    std::vector<std::string> scripts;
    for (const core::CorrectionPattern& pattern : catalogue.patterns()) {
        if (pattern.kind() != kind)
            continue;
        const std::string script = splitCode(pattern.code).script;
        if (!std::ranges::contains(scripts, script))
            scripts.push_back(script);
    }
    return scripts;
}

std::vector<std::string>
languagesOf(const core::PatternCatalogue& catalogue, core::PatternKind kind, std::string_view script) {
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
```

Note `scriptsOf` reads `Zyyy` as `splitCode("Zyyy").script == ""` (empty), which the `!value.empty()` guard in `distinctOf` would drop — `scriptsOf` therefore cannot reuse `distinctOf` as written (it needs the bare `Zyyy` script kept, unlike an empty language or country, which genuinely means "none"). It is written above as its own loop for exactly that reason; keep it that way rather than trying to fold it into `distinctOf`.

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[pattern-code]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/pattern_code.hpp src/lib/subedit/gui/pattern_code.cpp src/test/gui/pattern_code_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): decompose pattern codes into script, language and country"
```

## Task 6: GUI — render a text diff as rich text

**Files:**
- Create: `src/lib/subedit/gui/correction_diff_view.hpp`
- Create: `src/lib/subedit/gui/correction_diff_view.cpp`
- Test: `src/test/gui/correction_diff_view_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `core::DiffSpan`, `core::TextDiff` (Task 2).
- Produces: `[[nodiscard]] QString correctionDiffHtml(const std::vector<core::DiffSpan>& spans);`. Task 12 (`correction_result_model`) is the consumer, for both the original and the proposed column.

**Decision for this plan**: the changed part is marked **bold**, not coloured. A colour needs to read correctly against both the light and the dark palette (ADR 0024 already tests both), and this project has no established mapping from "changed" to a specific colour anywhere else; bold reads correctly under either, for free, and still satisfies D8's "the difference marked".

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/gui/correction_diff_view_test.cpp
#include <subedit/core/text/text_diff.hpp>
#include <subedit/gui/correction_diff_view.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::DiffSpan;
using subedit::gui::correctionDiffHtml;
} // namespace

TEST_CASE("an unchanged span is written plainly", "[gui][diff-view]") {
    const std::vector<DiffSpan> spans{DiffSpan{.text = "Bonjour", .changed = false}};
    CHECK(correctionDiffHtml(spans).toStdString() == "Bonjour");
}

TEST_CASE("a changed span is wrapped in bold", "[gui][diff-view]") {
    const std::vector<DiffSpan> spans{DiffSpan{.text = "Bonjour ", .changed = false},
                                      DiffSpan{.text = "!", .changed = true}};
    CHECK(correctionDiffHtml(spans).toStdString() == "Bonjour <b>!</b>");
}

TEST_CASE("text that looks like markup is escaped before it is wrapped", "[gui][diff-view]") {
    const std::vector<DiffSpan> spans{DiffSpan{.text = "<i>", .changed = true}};
    CHECK(correctionDiffHtml(spans).toStdString() == "<b>&lt;i&gt;</b>");
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/correction_diff_view.hpp
#pragma once

#include <subedit/core/text/text_diff.hpp>

#include <QString>

#include <vector>

namespace subedit::gui {

/// `spans` as rich text, the changed runs in bold — a rendering of `TextDiff`,
/// never a computation of one: `core::diffTexts` decides what changed, this
/// only decides how to show it.
[[nodiscard]] QString correctionDiffHtml(const std::vector<core::DiffSpan>& spans);

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_diff_view.cpp
#include <subedit/gui/correction_diff_view.hpp>

namespace subedit::gui {

QString correctionDiffHtml(const std::vector<core::DiffSpan>& spans) {
    QString html;
    for (const core::DiffSpan& span : spans) {
        const QString escaped = QString::fromStdString(span.text).toHtmlEscaped();
        html += span.changed ? "<b>" + escaped + "</b>" : escaped;
    }
    return html;
}

} // namespace subedit::gui
```

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[diff-view]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_diff_view.hpp src/lib/subedit/gui/correction_diff_view.cpp src/test/gui/correction_diff_view_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): render a text diff as rich text for the confirmation table"
```

## Task 7: GUI — the script/language/country widget

**Files:**
- Create: `src/lib/subedit/gui/pattern_code_selector.hpp`
- Create: `src/lib/subedit/gui/pattern_code_selector.cpp`
- Test: `src/test/gui/pattern_code_selector_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: Task 5's `splitCode`/`joinCode`/`scriptsOf`/`languagesOf`/`countriesOf`.
- Produces: `class PatternCodeSelector final : public QWidget` with `[[nodiscard]] std::string code() const;`, `void setCode(std::string_view code);`, `signals: void codeChanged();`. Task 9 (`correction_task_page`) is the consumer — one instance per task page.

Three cascading `QComboBox`es over one catalogue and one `PatternKind`, wrapping Task 5's pure functions the way `OperationDialog` wraps `targetLabel()` — a thin widget with no logic of its own to test beyond wiring.

- [ ] **Step 1: Write the failing test**

```cpp
// src/test/gui/pattern_code_selector_test.cpp
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_code_selector.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::PatternCodeSelector;

/// Zyyy (bare), Latn-en-US, Latn-fr — enough to exercise every cascade.
/// Build the same way Task 1 and Task 5 do: `InMemoryFileSystem` plus a
/// hand-written `.common-error` file, re-checking the exact `Name=`/`Classes=`/
/// `Pattern=` keys against `pattern_catalogue_test.cpp`'s `commonErrors()`
/// helper before writing this literally.
PatternCatalogue smallCatalogue();
} // namespace

TEST_CASE("it opens on the first script the catalogue carries, unfiltered below it",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};

    CHECK(selector.code() == "Zyyy");
}

TEST_CASE("setCode selects each combo and cascades the ones under it", "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};

    selector.setCode("Latn-en-US");

    CHECK(selector.code() == "Latn-en-US");
}

TEST_CASE("moving to a script with no languages resets to the bare code",
          "[gui][pattern-code-selector]") {
    const PatternCatalogue catalogue = smallCatalogue();
    PatternCodeSelector selector{catalogue, PatternKind::CommonError};
    selector.setCode("Latn-en-US");

    selector.setCode("Zyyy");

    CHECK(selector.code() == "Zyyy");
}
```

Write `smallCatalogue()` in the anonymous namespace using Task 1's `InMemoryFileSystem` + `readPatternCatalogue("/patterns", {})` recipe, with three `[Common Error Pattern]` records coded `Zyyy`, `Latn-en-US` and `Latn-fr`.

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/pattern_code_selector.hpp
#pragma once

#include <subedit/core/text/correction_pattern.hpp>

#include <QWidget>

#include <string>
#include <string_view>

class QComboBox;

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

namespace subedit::gui {

/// The script, language and country a task page's patterns are chosen for —
/// three combos cascading over what `catalogue` carries for `kind`, never
/// over an external locale list (Task 5).
class PatternCodeSelector final : public QWidget {
    Q_OBJECT

public:
    PatternCodeSelector(const core::PatternCatalogue& catalogue,
                        core::PatternKind kind,
                        QWidget* parent = nullptr);

    [[nodiscard]] std::string code() const;
    void setCode(std::string_view code);

signals:
    void codeChanged();

private:
    void refreshLanguages();
    void refreshCountries();

    const core::PatternCatalogue* m_catalogue;
    core::PatternKind m_kind;
    QComboBox* m_script;
    QComboBox* m_language;
    QComboBox* m_country;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/pattern_code_selector.cpp
#include <subedit/gui/pattern_code_selector.hpp>

#include <subedit/gui/pattern_code.hpp>

#include <QComboBox>
#include <QHBoxLayout>
#include <QString>

namespace subedit::gui {

PatternCodeSelector::PatternCodeSelector(const core::PatternCatalogue& catalogue,
                                         core::PatternKind kind,
                                         QWidget* parent)
    : QWidget(parent), m_catalogue(&catalogue), m_kind(kind), m_script(new QComboBox{this}),
      m_language(new QComboBox{this}), m_country(new QComboBox{this}) {
    auto* layout = new QHBoxLayout{this};
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_script);
    layout->addWidget(m_language);
    layout->addWidget(m_country);

    m_language->addItem(QStringLiteral("(every language)"), QString{});
    m_country->addItem(QStringLiteral("(every country)"), QString{});
    for (const std::string& script : scriptsOf(*m_catalogue, m_kind))
        m_script->addItem(QString::fromStdString(script), QString::fromStdString(script));

    connect(m_script, &QComboBox::currentIndexChanged, this, [this] {
        refreshLanguages();
        emit codeChanged();
    });
    connect(m_language, &QComboBox::currentIndexChanged, this, [this] {
        refreshCountries();
        emit codeChanged();
    });
    connect(m_country, &QComboBox::currentIndexChanged, this, &PatternCodeSelector::codeChanged);

    refreshLanguages();
}

void PatternCodeSelector::refreshLanguages() {
    const QString script = m_script->currentData().toString();
    const QSignalBlocker block{m_language};
    while (m_language->count() > 1)
        m_language->removeItem(1);
    for (const std::string& language : languagesOf(*m_catalogue, m_kind, script.toStdString()))
        m_language->addItem(QString::fromStdString(language), QString::fromStdString(language));
    m_language->setCurrentIndex(0);
    refreshCountries();
}

void PatternCodeSelector::refreshCountries() {
    const QString script = m_script->currentData().toString();
    const QString language = m_language->currentData().toString();
    const QSignalBlocker block{m_country};
    while (m_country->count() > 1)
        m_country->removeItem(1);
    for (const std::string& country :
        countriesOf(*m_catalogue, m_kind, script.toStdString(), language.toStdString()))
        m_country->addItem(QString::fromStdString(country), QString::fromStdString(country));
    m_country->setCurrentIndex(0);
}

std::string PatternCodeSelector::code() const {
    return joinCode(PatternCodeParts{
        .script = m_script->currentData().toString().toStdString(),
        .language = m_language->currentData().toString().toStdString(),
        .country = m_country->currentData().toString().toStdString(),
    });
}

void PatternCodeSelector::setCode(std::string_view code) {
    const PatternCodeParts parts = splitCode(code);
    const int scriptIndex = m_script->findData(QString::fromStdString(parts.script));
    m_script->setCurrentIndex(scriptIndex >= 0 ? scriptIndex : 0); // triggers refreshLanguages
    const int languageIndex = m_language->findData(QString::fromStdString(parts.language));
    m_language->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0); // triggers refreshCountries
    const int countryIndex = m_country->findData(QString::fromStdString(parts.country));
    m_country->setCurrentIndex(countryIndex >= 0 ? countryIndex : 0);
}

} // namespace subedit::gui
```

`QComboBox::setCurrentIndex` does not emit `currentIndexChanged` when the index does not actually change (e.g. going from index 0 to index 0) — if `scriptIndex` is already the current index, `refreshLanguages` will not fire from the signal and `setCode` would leave stale language/country lists. Call `refreshLanguages()`/`refreshCountries()` explicitly after each `setCurrentIndex` in `setCode` rather than relying on the signal, to make this correct regardless of the previous state:

```cpp
void PatternCodeSelector::setCode(std::string_view code) {
    const PatternCodeParts parts = splitCode(code);
    const int scriptIndex = m_script->findData(QString::fromStdString(parts.script));
    m_script->setCurrentIndex(scriptIndex >= 0 ? scriptIndex : 0);
    refreshLanguages();
    const int languageIndex = m_language->findData(QString::fromStdString(parts.language));
    m_language->setCurrentIndex(languageIndex >= 0 ? languageIndex : 0);
    refreshCountries();
    const int countryIndex = m_country->findData(QString::fromStdString(parts.country));
    m_country->setCurrentIndex(countryIndex >= 0 ? countryIndex : 0);
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[pattern-code-selector]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/pattern_code_selector.hpp src/lib/subedit/gui/pattern_code_selector.cpp src/test/gui/pattern_code_selector_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the script/language/country widget of a task page"
```

## Task 8: Core+GUI — a shared "is this pattern enabled" function, and the pattern list widget

**Files:**
- Modify: `src/lib/subedit/core/config/correction_settings.hpp` (add the declaration)
- Create: `src/lib/subedit/core/config/pattern_activation.cpp` (the extracted function — or add it beside `correction_settings.hpp`'s existing `.cpp` if one already exists; grep `src/lib/subedit/core/config/CMakeLists.txt` for a `correction_settings.cpp`, and place it there instead of a new file if it does)
- Modify: `src/lib/subedit/core/text/correction_run.cpp` (use the extracted function, delete the private copy)
- Test: `src/test/unit/core/config/pattern_activation_test.cpp` (or add cases to wherever `correction_settings` is already tested)
- Create: `src/lib/subedit/gui/pattern_list.hpp`
- Create: `src/lib/subedit/gui/pattern_list.cpp`
- Test: `src/test/gui/pattern_list_test.cpp`
- Modify: `src/lib/subedit/core/CMakeLists.txt`, `src/lib/subedit/gui/CMakeLists.txt`, both test `CMakeLists.txt`

**Interfaces:**
- Produces (core): `[[nodiscard]] bool patternEnabled(const core::CorrectionPattern& pattern, const core::CorrectionSettings& settings);` — D2's own rule (a `PatternActivation` override, or the shipped `enabled` when there is none), read today only inside `correction_run.cpp`'s private `isEnabled`.
- Produces (gui): `class PatternList final : public QWidget` with `void setCode(std::string_view code, const core::CorrectionSettings& settings);` and `[[nodiscard]] std::vector<core::PatternActivation> activations() const;`. Task 9 is the consumer, one instance per task page.

`correction_run.cpp`'s `isEnabled` already computes exactly what a pattern-list checkbox needs to open on (D2's activation rule); `pattern_list.cpp` needs the same rule and must not duplicate it, so it is promoted to a small core function first. `catalogue.cascade(kind, code)`'s own doc comment says a pattern name can repeat across the cascade — "a name found again is filed after the one it repeats" — and `CorrectionPattern::name`'s own comment: "several records may share a name, and share a single tick box." A box therefore represents every record of that name the current cascade holds, not one record — checked when any of them is currently enabled, and toggling it writes one `PatternActivation` per underlying record (each keeping its own `.code`, since D2's override is looked up by the *record's* code, not the cascade's request).

- [ ] **Step 1: Write the failing test for `patternEnabled`**

```cpp
// src/test/unit/core/config/pattern_activation_test.cpp (or appended to the
// existing correction_settings test file if the grep above finds one)
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionPattern;
using subedit::core::CorrectionSettings;
using subedit::core::patternEnabled;
using subedit::core::PatternActivation;
using subedit::core::PatternKind;
} // namespace

TEST_CASE("a pattern with no override keeps its shipped default", "[core][pattern-activation]") {
    CorrectionPattern pattern;
    pattern.code = "Zyyy";
    pattern.name = "Letter I";
    pattern.enabled = false;

    CHECK_FALSE(patternEnabled(pattern, CorrectionSettings{}));
}

TEST_CASE("an override by kind, code and name wins over the shipped default",
          "[core][pattern-activation]") {
    CorrectionPattern pattern;
    pattern.code = "Zyyy";
    pattern.name = "Letter I";
    pattern.fields = subedit::core::CommonErrorFields{};
    pattern.enabled = false;

    CorrectionSettings settings;
    settings.patternActivations.push_back(PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = true});

    CHECK(patternEnabled(pattern, settings));
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_unit_test 2>&1 | head -30`
Expected: FAIL to compile — `patternEnabled` is not declared yet.

- [ ] **Step 3: Extract `patternEnabled`**

In `correction_settings.hpp`, add after the `CorrectionSettings` struct:

```cpp
/// Whether `pattern` applies under `settings` — decision D2: an explicit
/// override by kind, code and name, or the shipped `.conf` default when there
/// is none. **Class filtering (D4) is not this function's**: it answers
/// activation alone, the same split `correction_run.cpp`'s own `isEnabled`
/// and `classesAllow` already keep apart.
[[nodiscard]] bool patternEnabled(const CorrectionPattern& pattern, const CorrectionSettings& settings);
```

Move the body of `correction_run.cpp`'s anonymous-namespace `isEnabled` into wherever `correction_settings.cpp` lives (check `src/lib/subedit/core/config/CMakeLists.txt` for the exact existing `.cpp`, or create `correction_settings.cpp` if `correction_settings.hpp` is currently header-only), renamed to `patternEnabled`:

```cpp
bool patternEnabled(const CorrectionPattern& pattern, const CorrectionSettings& settings) {
    for (const PatternActivation& activation : settings.patternActivations) {
        if (activation.kind == pattern.kind() && activation.code == pattern.code &&
            activation.name == pattern.name)
            return activation.enabled;
    }
    return pattern.enabled;
}
```

Then in `correction_run.cpp`, delete the private `isEnabled` and change its one call site (inside `selectedPatterns`) to call `patternEnabled` instead — `#include <subedit/core/config/correction_settings.hpp>` is already there.

- [ ] **Step 4: Run every existing correction test to verify nothing broke, and the new test passes**

Run: `cmake --build build/debug --target subedit_unit_test && ./build/debug/src/test/unit/subedit_unit_test "[assistant],[pattern-activation],[pattern]"`
Expected: PASS — the `correction_run_test.cpp` cases that depend on activation overrides (grep it for `PatternActivation` to name them) must still pass unchanged, proving the extraction did not change behaviour.

- [ ] **Step 5: Commit the extraction on its own**

```bash
git add src/lib/subedit/core/config/correction_settings.hpp src/lib/subedit/core/config/*.cpp src/lib/subedit/core/text/correction_run.cpp src/test/unit/core/config/pattern_activation_test.cpp src/lib/subedit/core/CMakeLists.txt
git commit -m "refactor(core): share the pattern-activation rule with the gui"
```

- [ ] **Step 6: Write the failing `PatternList` test**

```cpp
// src/test/gui/pattern_list_test.cpp
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::PatternKind;
using subedit::core::readPatternCatalogue;
using subedit::gui::PatternList;

/// Two common-error records under `Zyyy`: "Letter I" (enabled by default) and
/// "Musical notes" (`Enabled=False` in the `.conf`, mirroring the real
/// shipped file — reread issue #504's memory note on disabled-by-default
/// records before writing the literal file).
PatternCatalogue twoRecords();
} // namespace

TEST_CASE("each box opens on the shipped default when settings carry no override",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};

    list.setCode("Zyyy", CorrectionSettings{});

    CHECK(list.activations().empty()); // nothing overridden yet: every box agrees with its default
}

TEST_CASE("unchecking a box on by default produces one activation, disabled",
          "[gui][pattern-list]") {
    const PatternCatalogue catalogue = twoRecords();
    PatternList list{catalogue, PatternKind::CommonError};
    list.setCode("Zyyy", CorrectionSettings{});

    // Drive it as the widget test convention elsewhere in this suite does:
    // find the QCheckBox named after the pattern's name and click it. Check
    // `findChild<QCheckBox*>` by `setObjectName(QString::fromStdString(name))`
    // set in the widget's own construction (add this if the implementation
    // below does not already set it).

    const auto activations = list.activations();
    REQUIRE(activations.size() == 1);
    CHECK(activations[0].name == "Letter I");
    CHECK_FALSE(activations[0].enabled);
}
```

- [ ] **Step 7: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 8: Implement `PatternList`**

```cpp
// src/lib/subedit/gui/pattern_list.hpp
#pragma once

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <QWidget>

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

class QCheckBox;
class QVBoxLayout;

namespace subedit::gui {

/// One tick box per pattern *name* a cascade holds — several records may
/// share a name and share a box (`CorrectionPattern::name`).
class PatternList final : public QWidget {
    Q_OBJECT

public:
    PatternList(const core::PatternCatalogue& catalogue, core::PatternKind kind, QWidget* parent = nullptr);

    /// Rebuilds the boxes for `code`'s cascade, each opened from `settings`.
    void setCode(std::string_view code, const core::CorrectionSettings& settings);

    /// One `PatternActivation` per underlying record whose box now disagrees
    /// with that record's shipped default.
    [[nodiscard]] std::vector<core::PatternActivation> activations() const;

signals:
    void changed();

private:
    struct Entry {
        std::string name;
        QCheckBox* box;
        std::vector<const core::CorrectionPattern*> records;
    };

    const core::PatternCatalogue* m_catalogue;
    core::PatternKind m_kind;
    QVBoxLayout* m_layout;
    std::vector<Entry> m_entries;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/pattern_list.cpp
#include <subedit/gui/pattern_list.hpp>

#include <subedit/core/text/pattern_catalogue.hpp>

#include <QCheckBox>
#include <QString>
#include <QVBoxLayout>

namespace subedit::gui {

PatternList::PatternList(const core::PatternCatalogue& catalogue, core::PatternKind kind, QWidget* parent)
    : QWidget(parent), m_catalogue(&catalogue), m_kind(kind), m_layout(new QVBoxLayout{this}) {
    m_layout->setContentsMargins(0, 0, 0, 0);
}

void PatternList::setCode(std::string_view code, const core::CorrectionSettings& settings) {
    for (const Entry& entry : m_entries)
        entry.box->deleteLater();
    m_entries.clear();

    for (const core::CorrectionPattern* record : m_catalogue->cascade(m_kind, code)) {
        const auto found = std::ranges::find_if(
            m_entries, [record](const Entry& entry) { return entry.name == record->name; });
        if (found != m_entries.end()) {
            found->records.push_back(record);
            found->box->setChecked(found->box->isChecked() || core::patternEnabled(*record, settings));
            continue;
        }
        auto* box = new QCheckBox{QString::fromStdString(record->name), this};
        box->setObjectName(QString::fromStdString(record->name));
        box->setChecked(core::patternEnabled(*record, settings));
        connect(box, &QCheckBox::toggled, this, &PatternList::changed);
        m_layout->addWidget(box);
        m_entries.push_back(Entry{.name = record->name, .box = box, .records = {record}});
    }
}

std::vector<core::PatternActivation> PatternList::activations() const {
    std::vector<core::PatternActivation> activations;
    for (const Entry& entry : m_entries) {
        for (const core::CorrectionPattern* record : entry.records) {
            if (record->enabled == entry.box->isChecked())
                continue; // agrees with the shipped default: no override to write
            activations.push_back(core::PatternActivation{
                .kind = record->kind(), .code = record->code, .name = record->name,
                .enabled = entry.box->isChecked()});
        }
    }
    return activations;
}

} // namespace subedit::gui
```

Add `#include <algorithm>` for `std::ranges::find_if`.

- [ ] **Step 9: Run the test to verify it passes**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[pattern-list]"`
Expected: PASS. If Step 6's "unchecking a box" scenario cannot drive the checkbox through `findChild<QCheckBox*>(objectName)->toggle()` cleanly, adjust the test to do exactly that (the `setObjectName` call above exists for this).

- [ ] **Step 10: Commit**

```bash
git add src/lib/subedit/gui/pattern_list.hpp src/lib/subedit/gui/pattern_list.cpp src/test/gui/pattern_list_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the pattern list widget of a task page"
```

## Task 9: GUI — the four task pages

**Files:**
- Create: `src/lib/subedit/gui/correction_task_page.hpp`
- Create: `src/lib/subedit/gui/correction_task_page.cpp`
- Test: `src/test/gui/correction_task_page_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `PatternCodeSelector` (Task 7), `PatternList` (Task 8), `core::TaskSettings`/`core::CorrectionSettings` (already in `correction_settings.hpp`).
- Produces: `class CorrectionTaskPage : public QWizardPage` (abstract) with `[[nodiscard]] std::string code() const;`, `[[nodiscard]] std::vector<core::PatternActivation> activations() const;`, `virtual void applySettings(const core::CorrectionSettings&) = 0;`; and the four concrete subclasses `MentionsPage`, `CommonErrorsPage`, `CapitalizationPage`, `LineBreakPage`, each with their own extra accessors. Task 14 (`correction_wizard`) instantiates all four and wires the wizard to only show a task's page when Task 10's target page has that task checked; Task 15 (`correction_controller`) reads every accessor once the wizard finishes and combines it with `CorrectionTargetPage::taskChecked(...)` to build each `core::TaskSettings.enabled`.

One shared shape (D8: "a page per task — mentions, common errors, capitalization — with the writing, the language and the country, and the list of patterns by name"), four thin subclasses that add what D4/D7/D5 name for their own task: common errors' Human/OCR classes, mentions' two "Sound in…" checkboxes, line-break's length/lines/unit.

**Whether a task runs lives on the target page alone (Task 10), never here.** D8 checks tasks once, on the first page ("the page of tasks and the target: the tasks to check"); a task's own page only configures it, and Task 14's wizard skips a task's page entirely when its box on the target page is unchecked. A second "enabled" checkbox on this page would be a second place to say the same thing, and the two could disagree. `core::TaskSettings.enabled` is therefore assembled by Task 15 from `CorrectionTargetPage::taskChecked(...)`, not read off this page.

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/gui/correction_task_page_test.cpp
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_task_page.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionSettings;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::readPatternCatalogue;
using subedit::gui::CommonErrorsPage;
using subedit::gui::MentionsPage;

/// One `Zyyy` common-error record and one `Zyyy` hearing-impaired record,
/// built the way Task 1/5/8 already build small catalogues.
PatternCatalogue smallCatalogue();
} // namespace

TEST_CASE("a common-errors page opens on its own code and classes", "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = smallCatalogue();
    CommonErrorsPage page{catalogue};

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};
    settings.human = true;
    settings.ocr = false;
    page.applySettings(settings);

    CHECK(page.code() == "Zyyy");
    CHECK(page.human());
    CHECK_FALSE(page.ocr());
}

TEST_CASE("a mentions page opens on the two D7 sound checkboxes, decoupled from patterns",
          "[gui][correction-task-page]") {
    const PatternCatalogue catalogue = smallCatalogue();
    MentionsPage page{catalogue};

    CorrectionSettings settings;
    settings.soundInBrackets = true;
    page.applySettings(settings);

    CHECK(page.soundInBrackets());
    CHECK_FALSE(page.soundInParentheses());
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement the base page**

```cpp
// src/lib/subedit/gui/correction_task_page.hpp
#pragma once

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <QWizardPage>

#include <string>
#include <vector>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QVBoxLayout;

namespace subedit::gui {

class PatternCodeSelector;
class PatternList;

/// D8's shared shape of a task page: whether the task runs, the code its
/// patterns are chosen for, and the patterns themselves by name.
class CorrectionTaskPage : public QWizardPage {
    Q_OBJECT

public:
    CorrectionTaskPage(const QString& title,
                       const core::PatternCatalogue& catalogue,
                       core::PatternKind kind,
                       QWidget* parent = nullptr);

    /// Opens the page on `settings` — every subclass overrides this to read
    /// its own `TaskSettings` field and any task-specific extra it owns.
    virtual void applySettings(const core::CorrectionSettings& settings) = 0;

    [[nodiscard]] std::string code() const;
    [[nodiscard]] std::vector<core::PatternActivation> activations() const;

protected:
    /// Sets the shared widgets from `code`; a subclass calls this first from
    /// its own `applySettings`, then sets what it owns on top. Whether the
    /// task itself runs is not this page's concern — see this task's own note
    /// above.
    void applyBase(const std::string& code, const core::CorrectionSettings& settings);

    /// Where a subclass adds its own rows, between the code selector and the
    /// pattern list.
    [[nodiscard]] QVBoxLayout* extraLayout() const { return m_extraLayout; }

private:
    const core::PatternCatalogue* m_catalogue;
    core::PatternKind m_kind;
    core::CorrectionSettings m_settings; // kept so a code change re-opens the pattern list correctly
    PatternCodeSelector* m_selector;
    QVBoxLayout* m_extraLayout;
    PatternList* m_list;
};

/// GUI-HEARING-03: the two "Sound in…" checkboxes of D7, decoupled from the
/// pattern list — neither names a compiled pattern.
class MentionsPage final : public CorrectionTaskPage {
public:
    explicit MentionsPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;

    [[nodiscard]] bool soundInBrackets() const;
    [[nodiscard]] bool soundInParentheses() const;

private:
    QCheckBox* m_brackets;
    QCheckBox* m_parentheses;
};

/// D4: the Human/OCR class filter.
class CommonErrorsPage final : public CorrectionTaskPage {
public:
    explicit CommonErrorsPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;

    [[nodiscard]] bool human() const;
    [[nodiscard]] bool ocr() const;

private:
    QCheckBox* m_human;
    QCheckBox* m_ocr;
};

/// No extra of its own — the shared shape is the whole page.
class CapitalizationPage final : public CorrectionTaskPage {
public:
    explicit CapitalizationPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;
};

/// D5: the line-break limits, in characters or in ems.
class LineBreakPage final : public CorrectionTaskPage {
public:
    explicit LineBreakPage(const core::PatternCatalogue& catalogue, QWidget* parent = nullptr);

    void applySettings(const core::CorrectionSettings& settings) override;

    [[nodiscard]] double maxLength() const;
    [[nodiscard]] int maxLines() const;
    /// True for ems, false for characters — the unit chosen, never carried by
    /// `CorrectionSettings` itself (D5: the em measure is a `gui`-only type).
    [[nodiscard]] bool useEms() const;

private:
    QDoubleSpinBox* m_maxLength;
    QSpinBox* m_maxLines;
    QComboBox* m_unit;
};

} // namespace subedit::gui
```

- [ ] **Step 4: Implement the base page and the four subclasses**

```cpp
// src/lib/subedit/gui/correction_task_page.cpp
#include <subedit/gui/correction_task_page.hpp>

#include <subedit/gui/pattern_code_selector.hpp>
#include <subedit/gui/pattern_list.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace subedit::gui {

CorrectionTaskPage::CorrectionTaskPage(const QString& title,
                                       const core::PatternCatalogue& catalogue,
                                       core::PatternKind kind,
                                       QWidget* parent)
    : QWizardPage(parent), m_catalogue(&catalogue), m_kind(kind),
      m_selector(new PatternCodeSelector{catalogue, kind, this}), m_extraLayout(new QVBoxLayout{}),
      m_list(new PatternList{catalogue, kind, this}) {
    setTitle(title);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(m_selector);
    layout->addLayout(m_extraLayout);
    layout->addWidget(m_list);

    connect(m_selector, &PatternCodeSelector::codeChanged, this,
            [this] { m_list->setCode(code(), m_settings); });
}

std::string CorrectionTaskPage::code() const { return m_selector->code(); }

std::vector<core::PatternActivation> CorrectionTaskPage::activations() const {
    return m_list->activations();
}

void CorrectionTaskPage::applyBase(const std::string& code, const core::CorrectionSettings& settings) {
    m_settings = settings;
    m_selector->setCode(code);
    m_list->setCode(code, settings);
}

MentionsPage::MentionsPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(QStringLiteral("Mentions"), catalogue, core::PatternKind::HearingImpaired, parent),
      m_brackets(new QCheckBox{QStringLiteral("Sound in brackets"), this}),
      m_parentheses(new QCheckBox{QStringLiteral("Sound in parentheses"), this}) {
    extraLayout()->addWidget(m_brackets);
    extraLayout()->addWidget(m_parentheses);
}

void MentionsPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.mentions.code, settings);
    m_brackets->setChecked(settings.soundInBrackets);
    m_parentheses->setChecked(settings.soundInParentheses);
}

bool MentionsPage::soundInBrackets() const { return m_brackets->isChecked(); }
bool MentionsPage::soundInParentheses() const { return m_parentheses->isChecked(); }

CommonErrorsPage::CommonErrorsPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(QStringLiteral("Common Errors"), catalogue, core::PatternKind::CommonError, parent),
      m_human(new QCheckBox{QStringLiteral("Human"), this}),
      m_ocr(new QCheckBox{QStringLiteral("OCR"), this}) {
    extraLayout()->addWidget(m_human);
    extraLayout()->addWidget(m_ocr);
}

void CommonErrorsPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.commonErrors.code, settings);
    m_human->setChecked(settings.human);
    m_ocr->setChecked(settings.ocr);
}

bool CommonErrorsPage::human() const { return m_human->isChecked(); }
bool CommonErrorsPage::ocr() const { return m_ocr->isChecked(); }

CapitalizationPage::CapitalizationPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(QStringLiteral("Capitalization"), catalogue, core::PatternKind::Capitalization, parent) {}

void CapitalizationPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.capitalization.code, settings);
}

LineBreakPage::LineBreakPage(const core::PatternCatalogue& catalogue, QWidget* parent)
    : CorrectionTaskPage(QStringLiteral("Line Break"), catalogue, core::PatternKind::LineBreak, parent),
      m_maxLength(new QDoubleSpinBox{this}), m_maxLines(new QSpinBox{this}),
      m_unit(new QComboBox{this}) {
    m_maxLength->setRange(1.0, 1000.0);
    m_maxLines->setRange(1, 100);
    m_unit->addItem(QStringLiteral("Characters"));
    m_unit->addItem(QStringLiteral("Ems"));

    auto* form = new QFormLayout{};
    form->addRow(QStringLiteral("Maximum length:"), m_maxLength);
    form->addRow(QStringLiteral("Maximum lines:"), m_maxLines);
    form->addRow(QStringLiteral("Unit:"), m_unit);
    extraLayout()->addLayout(form);
}

void LineBreakPage::applySettings(const core::CorrectionSettings& settings) {
    applyBase(settings.lineBreak.code, settings);
    m_maxLength->setValue(settings.lineBreakMaxLength);
    m_maxLines->setValue(settings.lineBreakMaxLines);
}

double LineBreakPage::maxLength() const { return m_maxLength->value(); }
int LineBreakPage::maxLines() const { return m_maxLines->value(); }
bool LineBreakPage::useEms() const { return m_unit->currentIndex() == 1; }

} // namespace subedit::gui
```

`CorrectionSettings` does not carry the chosen unit (D5: the em measure is a `gui`-only type, so nothing at the core names it) — `useEms()` therefore always opens unchecked (`Characters`, index 0) regardless of what a previous session used. Note this as a known gap for the spec's "Précisé par #505" (Task 16): the unit itself is not remembered between sessions, only the length and line limits are. This is acceptable for D8's own list of what the assistant retains, which does not name a unit either — do not silently add a setting for it here; if the user wants it remembered, that is a follow-up issue, not a reason to block this one.

- [ ] **Step 5: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-task-page]"`
Expected: PASS.

- [ ] **Step 6: Commit**

```bash
git add src/lib/subedit/gui/correction_task_page.hpp src/lib/subedit/gui/correction_task_page.cpp src/test/gui/correction_task_page_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the four task pages of the correction assistant"
```

## Task 10: GUI — the target/task page

**Files:**
- Create: `src/lib/subedit/gui/correction_target_page.hpp`
- Create: `src/lib/subedit/gui/correction_target_page.cpp`
- Test: `src/test/gui/correction_target_page_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `CorrectionScope` (Task 4), `core::Document`, `core::CorrectionTask` (`correction_run.hpp`).
- Produces: `class CorrectionTargetPage final : public QWizardPage` with `[[nodiscard]] CorrectionScope scope() const;`, `[[nodiscard]] core::Document document() const;`, `[[nodiscard]] bool taskChecked(core::CorrectionTask task) const;`, `void setTaskChecked(core::CorrectionTask task, bool checked);`. Task 14 instantiates it and reads `taskChecked` to decide which task pages to show; Task 15 reads it and Task 4's `correctionTargetsOf` after Finish. `setTaskChecked` exists for Task 14's own test and for `CorrectionController` (Task 15) to open the wizard on the tasks the settings last had checked, per D8's "what the assistant retains: the tasks checked."

The wizard's first page — D8: "the target is the one Gaupol has — the selection, the current project, every open project — and the document is chosen on the first page, text or translation, translation only offered if a project carries one." Two independent radio groups (target, document) plus the four task checkboxes GUI-CORRECT-01 names.

- [ ] **Step 1: Write the failing test**

```cpp
// src/test/gui/correction_target_page_test.cpp
#include <subedit/gui/correction_target_page.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionTask;
using subedit::core::Document;
using subedit::gui::CorrectionScope;
using subedit::gui::CorrectionTargetPage;
} // namespace

TEST_CASE("the selection target is unavailable, and current project is the default, "
          "when nothing is selected",
          "[gui][correction-target-page]") {
    CorrectionTargetPage page{/*selectionAvailable=*/false, /*translationAvailable=*/false};

    CHECK(page.scope() == CorrectionScope::CurrentProject);
}

TEST_CASE("the translation document is offered only when a target carries one",
          "[gui][correction-target-page]") {
    CorrectionTargetPage withTranslation{true, true};
    CorrectionTargetPage without{true, false};

    // Without a translation available, asking for it anyway still answers
    // Main: the radio was never enabled, so it cannot have been chosen.
    CHECK(withTranslation.document() == Document::Main); // opens on Main either way
    CHECK(without.document() == Document::Main);
}

TEST_CASE("all four tasks are offered and none is checked by default",
          "[gui][correction-target-page]") {
    CorrectionTargetPage page{true, false};

    CHECK_FALSE(page.taskChecked(CorrectionTask::Mentions));
    CHECK_FALSE(page.taskChecked(CorrectionTask::CommonErrors));
    CHECK_FALSE(page.taskChecked(CorrectionTask::Capitalization));
    CHECK_FALSE(page.taskChecked(CorrectionTask::LineBreak));
}

TEST_CASE("setTaskChecked opens a task pre-checked", "[gui][correction-target-page]") {
    CorrectionTargetPage page{true, false};

    page.setTaskChecked(CorrectionTask::CommonErrors, true);

    CHECK(page.taskChecked(CorrectionTask::CommonErrors));
    CHECK_FALSE(page.taskChecked(CorrectionTask::Mentions));
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/correction_target_page.hpp
#pragma once

#include <subedit/core/model/document.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_target.hpp>

#include <QWizardPage>

class QCheckBox;
class QRadioButton;

namespace subedit::gui {

/// The first page of the assistant — which tasks, on which target, on which
/// document (D8).
class CorrectionTargetPage final : public QWizardPage {
    Q_OBJECT

public:
    /// `selectionAvailable`: the page on screen has rows selected. `translationAvailable`:
    /// at least one candidate project carries a translation.
    CorrectionTargetPage(bool selectionAvailable, bool translationAvailable, QWidget* parent = nullptr);

    [[nodiscard]] CorrectionScope scope() const;
    [[nodiscard]] core::Document document() const;
    [[nodiscard]] bool taskChecked(core::CorrectionTask task) const;
    void setTaskChecked(core::CorrectionTask task, bool checked);

private:
    QRadioButton* m_selection;
    QRadioButton* m_currentProject;
    QRadioButton* m_allProjects;
    QRadioButton* m_text;
    QRadioButton* m_translation;
    QCheckBox* m_mentions;
    QCheckBox* m_commonErrors;
    QCheckBox* m_capitalization;
    QCheckBox* m_lineBreak;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_target_page.cpp
#include <subedit/gui/correction_target_page.hpp>

#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

namespace subedit::gui {

CorrectionTargetPage::CorrectionTargetPage(bool selectionAvailable, bool translationAvailable, QWidget* parent)
    : QWizardPage(parent) {
    setTitle(QStringLiteral("Tasks and Target"));

    auto* targetGroup = new QGroupBox{QStringLiteral("Target"), this};
    m_selection = new QRadioButton{QStringLiteral("Selection"), targetGroup};
    m_selection->setEnabled(selectionAvailable);
    m_currentProject = new QRadioButton{QStringLiteral("Current Project"), targetGroup};
    m_currentProject->setChecked(true);
    m_allProjects = new QRadioButton{QStringLiteral("All Open Projects"), targetGroup};
    auto* targetLayout = new QVBoxLayout{targetGroup};
    targetLayout->addWidget(m_selection);
    targetLayout->addWidget(m_currentProject);
    targetLayout->addWidget(m_allProjects);

    auto* documentGroup = new QGroupBox{QStringLiteral("Document"), this};
    m_text = new QRadioButton{QStringLiteral("Text"), documentGroup};
    m_text->setChecked(true);
    m_translation = new QRadioButton{QStringLiteral("Translation"), documentGroup};
    m_translation->setEnabled(translationAvailable);
    auto* documentLayout = new QVBoxLayout{documentGroup};
    documentLayout->addWidget(m_text);
    documentLayout->addWidget(m_translation);

    auto* taskGroup = new QGroupBox{QStringLiteral("Tasks"), this};
    m_mentions = new QCheckBox{QStringLiteral("Mentions"), taskGroup};
    m_commonErrors = new QCheckBox{QStringLiteral("Common Errors"), taskGroup};
    m_capitalization = new QCheckBox{QStringLiteral("Capitalization"), taskGroup};
    m_lineBreak = new QCheckBox{QStringLiteral("Line Break"), taskGroup};
    auto* taskLayout = new QVBoxLayout{taskGroup};
    taskLayout->addWidget(m_mentions);
    taskLayout->addWidget(m_commonErrors);
    taskLayout->addWidget(m_capitalization);
    taskLayout->addWidget(m_lineBreak);

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(targetGroup);
    layout->addWidget(documentGroup);
    layout->addWidget(taskGroup);
}

CorrectionScope CorrectionTargetPage::scope() const {
    if (m_selection->isChecked())
        return CorrectionScope::Selection;
    if (m_allProjects->isChecked())
        return CorrectionScope::AllProjects;
    return CorrectionScope::CurrentProject;
}

core::Document CorrectionTargetPage::document() const {
    return m_translation->isChecked() ? core::Document::Translation : core::Document::Main;
}

bool CorrectionTargetPage::taskChecked(core::CorrectionTask task) const {
    switch (task) {
    case core::CorrectionTask::Mentions:
        return m_mentions->isChecked();
    case core::CorrectionTask::CommonErrors:
        return m_commonErrors->isChecked();
    case core::CorrectionTask::Capitalization:
        return m_capitalization->isChecked();
    case core::CorrectionTask::LineBreak:
        return m_lineBreak->isChecked();
    }
    return false; // unreachable: every enumerator is handled above
}

void CorrectionTargetPage::setTaskChecked(core::CorrectionTask task, bool checked) {
    switch (task) {
    case core::CorrectionTask::Mentions:
        m_mentions->setChecked(checked);
        return;
    case core::CorrectionTask::CommonErrors:
        m_commonErrors->setChecked(checked);
        return;
    case core::CorrectionTask::Capitalization:
        m_capitalization->setChecked(checked);
        return;
    case core::CorrectionTask::LineBreak:
        m_lineBreak->setChecked(checked);
        return;
    }
}

} // namespace subedit::gui
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-target-page]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_target_page.hpp src/lib/subedit/gui/correction_target_page.cpp src/test/gui/correction_target_page_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the target and task page of the correction assistant"
```

## Task 11: GUI — the progress page

**Files:**
- Create: `src/lib/subedit/gui/correction_progress_page.hpp`
- Create: `src/lib/subedit/gui/correction_progress_page.cpp`
- Test: `src/test/gui/correction_progress_page_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt` (this test needs `Qt6::Test` — check the existing `target_link_libraries` for `subedit_gui_test` already links it, since `cell_delegates_test.cpp` uses `QSignalSpy` there already; if it does not, add it)

**Interfaces:**
- Consumes: `core::CorrectionProposal` (Task 1).
- Produces: `class CorrectionProgressPage final : public QWizardPage` with `void setComputation(std::function<core::CorrectionProposal()> compute);`, `[[nodiscard]] const core::CorrectionProposal& result() const;`, `signals: void aboutToCompute();`. Task 14 places it between the task pages and the confirmation page; Task 15 connects `aboutToCompute` to a slot that reads the wizard's other pages (scope, document, per-task codes and activations) into plain values on the GUI thread and calls `setComputation` with a closure over those plain values, right before this page's own `initializePage()` hands that closure to the background thread.

**Why `aboutToCompute` exists rather than reading pages inside `compute` directly**: `compute` runs on a background thread (`QtConcurrent::run`), and Qt widgets — every wizard page included — may only be read from the GUI thread. `aboutToCompute()` is emitted synchronously, at the very start of `initializePage()`, before anything moves to a background thread; a listener connected to it is still running on the GUI thread and can safely call into `CorrectionTargetPage`/`CorrectionTaskPage` accessors there, building the closure `compute` will later run from data alone. This page itself stays generic — it names no other page type, only a signal a caller can hook.

`proposeCorrections` reports neither progress nor a way to cancel (Task 1's header is unchanged in that respect — deliberately, see Task 5's own note on not reopening #504's settled API further than needed). D8's "progression, and the ability to abandon the computation" is built entirely at this layer instead: the computation runs on a background thread via `QtConcurrent::run`, the page shows an indeterminate bar while it is not done, and the wizard's own Cancel button is the abandon gesture — closing the wizard discards `CorrectionProgressPage` and its `QFutureWatcher` without ever reading a result that has not arrived. This is safe specifically because `proposeCorrections` touches no project and the closure captures only long-lived objects (Task 15 makes sure of that: the engine, catalogue and settings it captures belong to `CorrectionController`, which outlives every wizard it opens).

- [ ] **Step 1: Write the failing test**

This is the suite's first background-thread page, so its test is also the suite's first use of `QSignalSpy::wait` — there is no existing example to follow for the "wait for an async Qt signal" shape; write it from the documented behaviour of `QSignalSpy::wait` (blocks pumping the event loop until the signal fires or the timeout elapses, returns whether it fired).

```cpp
// src/test/gui/correction_progress_page_test.cpp
#include <subedit/gui/correction_progress_page.hpp>

#include <QSignalSpy>
#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionProposal;
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionProgressPage;
} // namespace

TEST_CASE("the page is not complete until its background computation finishes",
          "[gui][correction-progress-page]") {
    CorrectionProgressPage page;
    page.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(ProposedCorrection{});
        return result;
    });
    CHECK_FALSE(page.isComplete());

    const QSignalSpy spy{&page, &CorrectionProgressPage::completeChanged};
    page.initializePage();
    REQUIRE(spy.wait(2000));

    CHECK(page.isComplete());
    CHECK(page.result().corrections.size() == 1);
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/correction_progress_page.hpp
#pragma once

#include <subedit/core/text/correction_run.hpp>

#include <QFutureWatcher>
#include <QWizardPage>

#include <functional>

namespace subedit::gui {

/// Runs a correction computation on a background thread while the wizard
/// shows progress — D8. Abandoning is the wizard's own Cancel button: nothing
/// here interrupts the computation, it is simply never read.
class CorrectionProgressPage final : public QWizardPage {
    Q_OBJECT

public:
    explicit CorrectionProgressPage(QWidget* parent = nullptr);

    /// What to run once the page is shown — set before the wizard opens.
    void setComputation(std::function<core::CorrectionProposal()> compute);

    void initializePage() override;
    [[nodiscard]] bool isComplete() const override;

    /// What the computation answered — empty until `isComplete()`.
    [[nodiscard]] const core::CorrectionProposal& result() const { return m_result; }

signals:
    /// Emitted synchronously, on the GUI thread, at the very start of
    /// `initializePage()` — the moment for a listener to read the wizard's
    /// other pages and call `setComputation` before the background thread
    /// starts.
    void aboutToCompute();

private:
    std::function<core::CorrectionProposal()> m_compute;
    QFutureWatcher<core::CorrectionProposal> m_watcher;
    core::CorrectionProposal m_result;
    bool m_done = false;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_progress_page.cpp
#include <subedit/gui/correction_progress_page.hpp>

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QtConcurrentRun>

namespace subedit::gui {

CorrectionProgressPage::CorrectionProgressPage(QWidget* parent) : QWizardPage(parent) {
    setTitle(QStringLiteral("Progress"));

    auto* bar = new QProgressBar{this};
    bar->setRange(0, 0); // indeterminate: proposeCorrections reports no progress of its own

    auto* layout = new QVBoxLayout{this};
    layout->addWidget(new QLabel{QStringLiteral("Computing the proposed corrections…"), this});
    layout->addWidget(bar);

    connect(&m_watcher, &QFutureWatcher<core::CorrectionProposal>::finished, this, [this] {
        m_result = m_watcher.result();
        m_done = true;
        emit completeChanged();
        if (wizard() != nullptr)
            wizard()->next(); // auto-advance to the confirmation page
    });
}

void CorrectionProgressPage::setComputation(std::function<core::CorrectionProposal()> compute) {
    m_compute = std::move(compute);
}

void CorrectionProgressPage::initializePage() {
    m_done = false;
    m_result = {};
    emit aboutToCompute(); // GUI thread: a listener may call setComputation here
    m_watcher.setFuture(QtConcurrent::run(m_compute));
}

bool CorrectionProgressPage::isComplete() const { return m_done; }

} // namespace subedit::gui
```

`Qt6::Concurrent` must be linked for `QtConcurrentRun` — check `target_link_libraries(subedit_gui PUBLIC subedit::core Qt6::Widgets)` in `src/lib/subedit/gui/CMakeLists.txt` (read earlier while exploring): add `Qt6::Concurrent` to that line, and confirm `find_package(Qt6 ... Concurrent)` is requested at the top-level `CMakeLists.txt` (grep it; add `Concurrent` to the component list if it is missing).

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-progress-page]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_progress_page.hpp src/lib/subedit/gui/correction_progress_page.cpp src/test/gui/correction_progress_page_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt CMakeLists.txt
git commit -m "feat(gui): compute proposed corrections on a background thread"
```

## Task 12: GUI — the confirmation table model and its diff-rendering delegate

**Files:**
- Create: `src/lib/subedit/gui/correction_result_model.hpp`
- Create: `src/lib/subedit/gui/correction_result_model.cpp`
- Test: `src/test/gui/correction_result_model_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `core::ProposedCorrection` (Task 1), `core::diffTexts` (Task 2), `correctionDiffHtml` (Task 6).
- Produces: `class CorrectionResultModel final : public QAbstractTableModel` (columns `Accept`, `Original`, `Proposed`; `void markAll(bool accepted);`, `[[nodiscard]] std::vector<core::ProposedCorrection> acceptedCorrections() const;`) and `class CorrectionDiffDelegate final : public QStyledItemDelegate` (paints the `Original`/`Proposed` columns' rich text — `QTableView::setItemDelegate` does not interpret HTML in `Qt::DisplayRole` on its own). Task 13 (`correction_confirmation_page`) is the consumer.

D8's confirmation table: a checkbox per row (`Mark All`/`Unmark All` drive every row at once), the original and proposed text each with the difference marked (Task 6's rich text needs a delegate to actually paint as rich text — `QStyledItemDelegate`'s default painting treats `Qt::DisplayRole` as plain text), and the proposed text editable in place.

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/gui/correction_result_model_test.cpp
#include <subedit/gui/correction_result_model.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionResultModel;

std::vector<ProposedCorrection> twoRows() {
    return {ProposedCorrection{.original = "Bonjour  Marie", .proposed = std::string{"Bonjour Marie"}},
           ProposedCorrection{.original = "Au revoir", .proposed = std::nullopt}};
}
} // namespace

TEST_CASE("every row is accepted by default", "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    CHECK(model.rowCount() == 2);
    CHECK(model.acceptedCorrections().size() == 2);
}

TEST_CASE("unchecking a row's accept box drops it from what is applied",
          "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.setData(model.index(0, CorrectionResultModel::Accept), Qt::Unchecked, Qt::CheckStateRole);

    const auto accepted = model.acceptedCorrections();
    REQUIRE(accepted.size() == 1);
    CHECK(accepted[0].original == "Au revoir");
}

TEST_CASE("markAll checks or unchecks every row at once", "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.markAll(false);
    CHECK(model.acceptedCorrections().empty());

    model.markAll(true);
    CHECK(model.acceptedCorrections().size() == 2);
}

TEST_CASE("retouching the proposed text changes what is applied", "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.setData(model.index(0, CorrectionResultModel::Proposed),
                  QStringLiteral("Bonjour, Marie"),
                  Qt::EditRole);

    const auto accepted = model.acceptedCorrections();
    REQUIRE(accepted.size() == 2);
    CHECK(accepted[0].proposed == std::optional<std::string>{"Bonjour, Marie"});
}

TEST_CASE("retouching the proposed text back to the original drops the row",
          "[gui][correction-result-model]") {
    CorrectionResultModel model{twoRows()};

    model.setData(model.index(0, CorrectionResultModel::Proposed),
                  QStringLiteral("Bonjour  Marie"), // exactly the original
                  Qt::EditRole);

    const auto accepted = model.acceptedCorrections();
    REQUIRE(accepted.size() == 1);
    CHECK(accepted[0].original == "Au revoir"); // only the second row remains
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/correction_result_model.hpp
#pragma once

#include <subedit/core/text/correction_run.hpp>

#include <QAbstractTableModel>
#include <QStyledItemDelegate>

#include <optional>
#include <string>
#include <vector>

namespace subedit::gui {

/// The confirmation table of D8 — one row per proposed change, an accept box,
/// the original and the proposed text, both with their difference marked, the
/// proposed text editable in place.
class CorrectionResultModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column { Accept = 0, Original = 1, Proposed = 2 };

    explicit CorrectionResultModel(std::vector<core::ProposedCorrection> corrections,
                                   QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /// `Mark All` / `Unmark All`.
    void markAll(bool accepted);

    [[nodiscard]] const core::ProposedCorrection& correctionAt(int row) const {
        return m_rows.at(static_cast<std::size_t>(row)).correction;
    }

    /// The accepted rows, `proposed` replaced by what was typed for a row
    /// that was retouched — never a row retouched back to exactly its
    /// original text, which is not a change to apply.
    [[nodiscard]] std::vector<core::ProposedCorrection> acceptedCorrections() const;

private:
    struct Row {
        core::ProposedCorrection correction;
        bool accepted = true;
        std::optional<std::string> retouched;

        /// What the Proposed column shows and edits — the retouched text if
        /// there is one, otherwise the proposal itself, blank for a removal.
        [[nodiscard]] std::string proposedText() const {
            if (retouched.has_value())
                return *retouched;
            return correction.proposed.value_or(std::string{});
        }
    };

    std::vector<Row> m_rows;
};

/// Paints `Qt::DisplayRole` as the rich text Task 6 produces — `QStyledItemDelegate`
/// on its own treats it as plain text, HTML tags and all.
class CorrectionDiffDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_result_model.cpp
#include <subedit/gui/correction_result_model.hpp>

#include <subedit/core/text/text_diff.hpp>
#include <subedit/gui/correction_diff_view.hpp>

#include <QPainter>
#include <QTextDocument>

#include <utility>

namespace subedit::gui {

CorrectionResultModel::CorrectionResultModel(std::vector<core::ProposedCorrection> corrections,
                                             QObject* parent)
    : QAbstractTableModel(parent) {
    m_rows.reserve(corrections.size());
    for (core::ProposedCorrection& correction : corrections)
        m_rows.push_back(Row{.correction = std::move(correction)});
}

int CorrectionResultModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(m_rows.size());
}

int CorrectionResultModel::columnCount(const QModelIndex& /*parent*/) const { return 3; }

QVariant CorrectionResultModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return {};
    const Row& row = m_rows.at(static_cast<std::size_t>(index.row()));

    if (index.column() == Accept && role == Qt::CheckStateRole)
        return row.accepted ? Qt::Checked : Qt::Unchecked;

    if (index.column() == Original && role == Qt::DisplayRole) {
        const core::TextDiff diff = core::diffTexts(row.correction.original, row.proposedText());
        return correctionDiffHtml(diff.original);
    }
    if (index.column() == Proposed) {
        if (role == Qt::EditRole)
            return QString::fromStdString(row.proposedText());
        if (role == Qt::DisplayRole) {
            const core::TextDiff diff = core::diffTexts(row.correction.original, row.proposedText());
            return correctionDiffHtml(diff.proposed);
        }
    }
    return {};
}

bool CorrectionResultModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid())
        return false;
    Row& row = m_rows.at(static_cast<std::size_t>(index.row()));

    if (index.column() == Accept && role == Qt::CheckStateRole) {
        row.accepted = value.toInt() == Qt::Checked;
        emit dataChanged(index, index, {Qt::CheckStateRole});
        return true;
    }
    if (index.column() == Proposed && role == Qt::EditRole) {
        row.retouched = value.toString().toStdString();
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

Qt::ItemFlags CorrectionResultModel::flags(const QModelIndex& index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags common = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == Accept)
        return common | Qt::ItemIsUserCheckable;
    if (index.column() == Proposed)
        return common | Qt::ItemIsEditable;
    return common;
}

QVariant CorrectionResultModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};
    switch (section) {
    case Accept:
        return QStringLiteral("Accept");
    case Original:
        return QStringLiteral("Original");
    case Proposed:
        return QStringLiteral("Corrected Text");
    default:
        return {};
    }
}

void CorrectionResultModel::markAll(bool accepted) {
    for (Row& row : m_rows)
        row.accepted = accepted;
    if (!m_rows.empty())
        emit dataChanged(index(0, Accept), index(static_cast<int>(m_rows.size()) - 1, Accept),
                         {Qt::CheckStateRole});
}

std::vector<core::ProposedCorrection> CorrectionResultModel::acceptedCorrections() const {
    std::vector<core::ProposedCorrection> result;
    for (const Row& row : m_rows) {
        if (!row.accepted)
            continue;
        core::ProposedCorrection correction = row.correction;
        if (row.retouched.has_value())
            correction.proposed = *row.retouched;
        if (correction.proposed.has_value() && *correction.proposed == correction.original)
            continue; // retouched back to the original: nothing to apply
        result.push_back(std::move(correction));
    }
    return result;
}

void CorrectionDiffDelegate::paint(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->save();

    QTextDocument document;
    document.setHtml(opt.text);
    opt.text.clear();
    if (opt.widget != nullptr)
        opt.widget->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    painter->translate(opt.rect.topLeft());
    document.setTextWidth(opt.rect.width());
    document.drawContents(painter);
    painter->restore();
}

QSize CorrectionDiffDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QTextDocument document;
    document.setHtml(index.data(Qt::DisplayRole).toString());
    document.setTextWidth(option.rect.width() > 0 ? option.rect.width() : 200);
    return QSize{static_cast<int>(document.idealWidth()), static_cast<int>(document.size().height())};
}

} // namespace subedit::gui
```

This is the standard Qt "rich text delegate" shape (`QTextDocument::setHtml` + `drawContents` over a painted, text-cleared base item) — verify it actually renders bold correctly on screen during Task 16's screenshot step; a delegate is easy to get subtly wrong (word wrap width, vertical centering) in ways no unit test below catches, only a look at the captured PNG will.

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-result-model]"`
Expected: PASS, all five cases.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_result_model.hpp src/lib/subedit/gui/correction_result_model.cpp src/test/gui/correction_result_model_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the confirmation table model and its diff delegate"
```

## Task 13: GUI — the confirmation page

**Files:**
- Create: `src/lib/subedit/gui/correction_confirmation_page.hpp`
- Create: `src/lib/subedit/gui/correction_confirmation_page.cpp`
- Test: `src/test/gui/correction_confirmation_page_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: `CorrectionResultModel`/`CorrectionDiffDelegate` (Task 12), `CorrectionProgressPage` (Task 11), `core::PatternFailure` (Task 1).
- Produces: `class CorrectionConfirmationPage final : public QWizardPage` with `void setProgressPage(const CorrectionProgressPage* progress);`, `void setRemoveBlankSubtitlesDefault(bool);`, `[[nodiscard]] CorrectionResultModel* resultModel() const;`, `[[nodiscard]] bool removeBlankSubtitles() const;`, `signals: void previewRequested(int row);`. Task 14 wires `setProgressPage` once both pages exist; Task 15 owns `resultModel()->acceptedCorrections()`, `removeBlankSubtitles()` and the `previewRequested` slot.

The last page: `Mark All`/`Unmark All`, the table (Task 12's model with Task 12's delegate on the two rich-text columns), `Preview`, the "remove blank subtitles" checkbox (D8: checked by default), and — GUI-CORRECT-06 — the patterns the run could not apply, named. `Preview` only knows a row was asked for; whether that row's project actually has a video, and placing playback on it, is `CorrectionController`'s business (Task 15) through `previewRequested`, the same separation `ProjectOperations`/`VideoPane` already keep.

- [ ] **Step 1: Write the failing tests**

```cpp
// src/test/gui/correction_confirmation_page_test.cpp
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QSignalSpy>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionProposal;
using subedit::core::FailureKind;
using subedit::core::PatternFailure;
using subedit::core::PatternKind;
using subedit::core::ProposedCorrection;
using subedit::gui::CorrectionConfirmationPage;
using subedit::gui::CorrectionProgressPage;
} // namespace

TEST_CASE("the page builds its table from the progress page's result", "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.corrections.push_back(ProposedCorrection{.original = "a", .proposed = std::string{"b"}});
        return result;
    });
    const QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    REQUIRE(page.resultModel() != nullptr);
    CHECK(page.resultModel()->rowCount() == 1);
}

TEST_CASE("a named pattern failure is shown when the run left one behind",
          "[gui][correction-confirmation-page]") {
    CorrectionProgressPage progress;
    progress.setComputation([] {
        CorrectionProposal result;
        result.failures.push_back(PatternFailure{.kind = FailureKind::CompileError,
                                                  .code = "Zyyy",
                                                  .rank = 1,
                                                  .name = "Broken"});
        return result;
    });
    const QSignalSpy spy{&progress, &CorrectionProgressPage::completeChanged};
    progress.initializePage();
    REQUIRE(spy.wait(2000));

    CorrectionConfirmationPage page;
    page.setProgressPage(&progress);
    page.initializePage();

    CHECK(page.findChild<QLabel*>(QStringLiteral("abandonedPatterns"))->text().contains(QStringLiteral("Broken")));
}

TEST_CASE("the remove-blank-subtitles box opens on the default it is given",
          "[gui][correction-confirmation-page]") {
    CorrectionConfirmationPage page;
    page.setRemoveBlankSubtitlesDefault(false);

    CHECK_FALSE(page.removeBlankSubtitles());
}
```

- [ ] **Step 2: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/correction_confirmation_page.hpp
#pragma once

#include <subedit/core/text/correction_run.hpp>

#include <QWizardPage>

class QCheckBox;
class QLabel;
class QPushButton;
class QTableView;

namespace subedit::gui {

class CorrectionProgressPage;
class CorrectionResultModel;

/// The last page — the table of proposed changes, `Mark All`/`Unmark All`,
/// `Preview`, the remove-blank-subtitles case, and the patterns the run could
/// not apply, named (D8, GUI-CORRECT-02/03/06).
class CorrectionConfirmationPage final : public QWizardPage {
    Q_OBJECT

public:
    explicit CorrectionConfirmationPage(QWidget* parent = nullptr);

    /// `progress` must outlive this page — the wizard owns both.
    void setProgressPage(const CorrectionProgressPage* progress) { m_progress = progress; }
    void setRemoveBlankSubtitlesDefault(bool removeBlank);

    void initializePage() override;

    [[nodiscard]] CorrectionResultModel* resultModel() const { return m_model; }
    [[nodiscard]] bool removeBlankSubtitles() const;

signals:
    /// The row at `row` (into `resultModel()`) was asked to be previewed.
    void previewRequested(int row);

private:
    const CorrectionProgressPage* m_progress = nullptr;
    QTableView* m_table;
    CorrectionResultModel* m_model = nullptr;
    QPushButton* m_markAll;
    QPushButton* m_unmarkAll;
    QPushButton* m_preview;
    QCheckBox* m_removeBlank;
    QLabel* m_abandoned;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_confirmation_page.cpp
#include <subedit/gui/correction_confirmation_page.hpp>

#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QTableView>
#include <QVBoxLayout>

namespace subedit::gui {

namespace {

[[nodiscard]] QString reasonOf(core::FailureKind kind) {
    switch (kind) {
    case core::FailureKind::Untranslatable:
        return QStringLiteral("cannot be translated");
    case core::FailureKind::CompileError:
        return QStringLiteral("will not compile");
    case core::FailureKind::InvalidReplacement:
        return QStringLiteral("has an invalid replacement");
    case core::FailureKind::TimedOut:
        return QStringLiteral("timed out");
    case core::FailureKind::TooManyPasses:
        return QStringLiteral("never settled");
    case core::FailureKind::TooLong:
        return QStringLiteral("grew the text too long");
    }
    return {}; // unreachable: every enumerator is handled above
}

[[nodiscard]] QString abandonedText(const std::vector<core::PatternFailure>& failures) {
    QStringList lines;
    for (const core::PatternFailure& failure : failures)
        lines << QString::fromStdString(failure.name) + " (" + reasonOf(failure.kind) + ")";
    return QStringLiteral("Not applied: ") + lines.join(QStringLiteral(", "));
}

} // namespace

CorrectionConfirmationPage::CorrectionConfirmationPage(QWidget* parent)
    : QWizardPage(parent), m_table(new QTableView{this}), m_markAll(new QPushButton{QStringLiteral("Mark All"), this}),
      m_unmarkAll(new QPushButton{QStringLiteral("Unmark All"), this}),
      m_preview(new QPushButton{QStringLiteral("Preview"), this}),
      m_removeBlank(new QCheckBox{QStringLiteral("Remove all blank subtitles"), this}),
      m_abandoned(new QLabel{this}) {
    setTitle(QStringLiteral("Confirmation"));
    m_removeBlank->setChecked(true);
    m_abandoned->setObjectName(QStringLiteral("abandonedPatterns"));
    m_abandoned->setWordWrap(true);
    m_abandoned->hide();

    auto* buttons = new QHBoxLayout{};
    buttons->addWidget(m_markAll);
    buttons->addWidget(m_unmarkAll);
    buttons->addStretch();
    buttons->addWidget(m_preview);

    auto* layout = new QVBoxLayout{this};
    layout->addLayout(buttons);
    layout->addWidget(m_table);
    layout->addWidget(m_removeBlank);
    layout->addWidget(m_abandoned);

    connect(m_markAll, &QPushButton::clicked, this, [this] { m_model->markAll(true); });
    connect(m_unmarkAll, &QPushButton::clicked, this, [this] { m_model->markAll(false); });
    connect(m_preview, &QPushButton::clicked, this, [this] {
        const QModelIndexList selected = m_table->selectionModel()->selectedRows();
        if (!selected.isEmpty())
            emit previewRequested(selected.first().row());
    });
}

void CorrectionConfirmationPage::setRemoveBlankSubtitlesDefault(bool removeBlank) {
    m_removeBlank->setChecked(removeBlank);
}

void CorrectionConfirmationPage::initializePage() {
    delete m_model;
    m_model = new CorrectionResultModel{m_progress->result().corrections, this};
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    auto* delegate = new CorrectionDiffDelegate{m_table};
    m_table->setItemDelegateForColumn(CorrectionResultModel::Original, delegate);
    m_table->setItemDelegateForColumn(CorrectionResultModel::Proposed, delegate);

    const bool anyFailure = !m_progress->result().failures.empty();
    m_abandoned->setVisible(anyFailure);
    if (anyFailure)
        m_abandoned->setText(abandonedText(m_progress->result().failures));
}

bool CorrectionConfirmationPage::removeBlankSubtitles() const { return m_removeBlank->isChecked(); }

} // namespace subedit::gui
```

The test drives `initializePage()` directly rather than through a real `QWizard::next()`, matching how Task 8's `PatternList` test drove `setCode` directly rather than through a whole wizard flow — Task 14's own end-to-end test is what proves the pages actually chain correctly inside a real `QWizard`.

- [ ] **Step 4: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-confirmation-page]"`
Expected: PASS.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_confirmation_page.hpp src/lib/subedit/gui/correction_confirmation_page.cpp src/test/gui/correction_confirmation_page_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the confirmation page of the correction assistant"
```

## Task 14: GUI — assemble the wizard, skipping unchecked tasks

**Files:**
- Create: `src/lib/subedit/gui/correction_wizard.hpp`
- Create: `src/lib/subedit/gui/correction_wizard.cpp`
- Test: `src/test/gui/correction_wizard_test.cpp`
- Modify: `src/lib/subedit/gui/CMakeLists.txt`, `src/test/gui/CMakeLists.txt`

**Interfaces:**
- Consumes: every page from Tasks 9–13.
- Produces: `class CorrectionWizard final : public QWizard` with page IDs `TargetId`, `MentionsId`, `CommonErrorsId`, `CapitalizationId`, `LineBreakId`, `ProgressId`, `ConfirmationId`, and accessors `targetPage()`, `mentionsPage()`, `commonErrorsPage()`, `capitalizationPage()`, `lineBreakPage()`, `progressPage()`, `confirmationPage()` (each returning a reference to the concrete page type, not `QWizardPage`). Task 15 (`correction_controller`) is the only consumer: it builds one `CorrectionWizard`, sets `progressPage().setComputation(...)`, and reads every page back once `QWizard::exec()` returns `true`.

D8: "the same order of pages" as Gaupol — target/tasks, then one page per *checked* task, then progress, then confirmation. `QWizard::nextId()` is overridden to walk the four task pages in order and skip any whose box is unchecked on the target page — the mechanism this task's own note in Task 9 promised.

- [ ] **Step 1: Write the failing test**

```cpp
// src/test/gui/correction_wizard_test.cpp
#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_wizard.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {
using subedit::core::CorrectionSettings;
using subedit::core::CorrectionTask;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::readPatternCatalogue;
using subedit::gui::CorrectionWizard;

PatternCatalogue emptyCatalogue() {
    InMemoryFileSystem files;
    return readPatternCatalogue(files, "/patterns", {});
}
} // namespace

TEST_CASE("advancing past the target page goes straight to progress when no task is checked",
          "[gui][correction-wizard]") {
    const PatternCatalogue catalogue = emptyCatalogue();
    CorrectionWizard wizard{catalogue, CorrectionSettings{}, /*selectionAvailable=*/true,
                           /*translationAvailable=*/false};
    wizard.show();
    REQUIRE(wizard.currentId() == CorrectionWizard::TargetId);

    wizard.next();

    CHECK(wizard.currentId() == CorrectionWizard::ProgressId);
}

TEST_CASE("advancing past the target page visits only the checked tasks, in order",
          "[gui][correction-wizard]") {
    const PatternCatalogue catalogue = emptyCatalogue();
    CorrectionWizard wizard{catalogue, CorrectionSettings{}, true, false};
    wizard.show();
    wizard.targetPage().setTaskChecked(CorrectionTask::LineBreak, true);
    wizard.targetPage().setTaskChecked(CorrectionTask::Mentions, true);

    wizard.next();
    CHECK(wizard.currentId() == CorrectionWizard::MentionsId); // Gaupol's order, not the check order

    wizard.next();
    CHECK(wizard.currentId() == CorrectionWizard::LineBreakId);

    wizard.next();
    CHECK(wizard.currentId() == CorrectionWizard::ProgressId);
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -30`
Expected: FAIL to compile.

- [ ] **Step 3: Implement**

```cpp
// src/lib/subedit/gui/correction_wizard.hpp
#pragma once

#include <subedit/core/config/correction_settings.hpp>

#include <QWizard>

namespace subedit::core {
class PatternCatalogue;
} // namespace subedit::core

namespace subedit::gui {

class CapitalizationPage;
class CommonErrorsPage;
class CorrectionConfirmationPage;
class CorrectionProgressPage;
class CorrectionTargetPage;
class LineBreakPage;
class MentionsPage;

/// `Tools ▸ Correct Texts…` — D8, Gaupol's own order of pages, a task's page
/// shown only when it is checked on the target page.
class CorrectionWizard final : public QWizard {
    Q_OBJECT

public:
    enum PageId {
        TargetId,
        MentionsId,
        CommonErrorsId,
        CapitalizationId,
        LineBreakId,
        ProgressId,
        ConfirmationId,
    };

    CorrectionWizard(const core::PatternCatalogue& catalogue,
                     const core::CorrectionSettings& settings,
                     bool selectionAvailable,
                     bool translationAvailable,
                     QWidget* parent = nullptr);

    [[nodiscard]] int nextId() const override;

    [[nodiscard]] CorrectionTargetPage& targetPage() const { return *m_target; }
    [[nodiscard]] MentionsPage& mentionsPage() const { return *m_mentions; }
    [[nodiscard]] CommonErrorsPage& commonErrorsPage() const { return *m_commonErrors; }
    [[nodiscard]] CapitalizationPage& capitalizationPage() const { return *m_capitalization; }
    [[nodiscard]] LineBreakPage& lineBreakPage() const { return *m_lineBreak; }
    [[nodiscard]] CorrectionProgressPage& progressPage() const { return *m_progress; }
    [[nodiscard]] CorrectionConfirmationPage& confirmationPage() const { return *m_confirmation; }

private:
    CorrectionTargetPage* m_target;
    MentionsPage* m_mentions;
    CommonErrorsPage* m_commonErrors;
    CapitalizationPage* m_capitalization;
    LineBreakPage* m_lineBreak;
    CorrectionProgressPage* m_progress;
    CorrectionConfirmationPage* m_confirmation;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_wizard.cpp
#include <subedit/gui/correction_wizard.hpp>

#include <subedit/core/text/correction_run.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>

#include <array>
#include <algorithm>

namespace subedit::gui {

namespace {
constexpr std::array<int, 4> kTaskPages{CorrectionWizard::MentionsId, CorrectionWizard::CommonErrorsId,
                                        CorrectionWizard::CapitalizationId, CorrectionWizard::LineBreakId};
constexpr std::array<core::CorrectionTask, 4> kTasks{
    core::CorrectionTask::Mentions, core::CorrectionTask::CommonErrors,
    core::CorrectionTask::Capitalization, core::CorrectionTask::LineBreak};
} // namespace

CorrectionWizard::CorrectionWizard(const core::PatternCatalogue& catalogue,
                                   const core::CorrectionSettings& settings,
                                   bool selectionAvailable,
                                   bool translationAvailable,
                                   QWidget* parent)
    : QWizard(parent), m_target(new CorrectionTargetPage{selectionAvailable, translationAvailable}),
      m_mentions(new MentionsPage{catalogue}), m_commonErrors(new CommonErrorsPage{catalogue}),
      m_capitalization(new CapitalizationPage{catalogue}), m_lineBreak(new LineBreakPage{catalogue}),
      m_progress(new CorrectionProgressPage{}), m_confirmation(new CorrectionConfirmationPage{}) {
    setWindowTitle(QStringLiteral("Correct Texts"));
    setPage(TargetId, m_target);
    setPage(MentionsId, m_mentions);
    setPage(CommonErrorsId, m_commonErrors);
    setPage(CapitalizationId, m_capitalization);
    setPage(LineBreakId, m_lineBreak);
    setPage(ProgressId, m_progress);
    setPage(ConfirmationId, m_confirmation);
    setStartId(TargetId);

    for (std::size_t i = 0; i < kTasks.size(); ++i)
        m_target->setTaskChecked(kTasks[i], settings_taskSettingsOf(settings, kTasks[i]).enabled);
    m_mentions->applySettings(settings);
    m_commonErrors->applySettings(settings);
    m_capitalization->applySettings(settings);
    m_lineBreak->applySettings(settings);
    m_confirmation->setProgressPage(m_progress);
    m_confirmation->setRemoveBlankSubtitlesDefault(settings.removeBlankSubtitles);
}

int CorrectionWizard::nextId() const {
    if (currentId() == ConfirmationId)
        return -1;
    if (currentId() == ProgressId)
        return ConfirmationId;

    // TargetId or one of the four task pages: walk forward from here to the
    // next checked task, Gaupol's own order.
    const auto here = currentId() == TargetId
                          ? kTaskPages.begin() - 1
                          : std::ranges::find(kTaskPages, currentId());
    for (auto it = here + 1; it != kTaskPages.end(); ++it) {
        const std::size_t index = static_cast<std::size_t>(it - kTaskPages.begin());
        if (m_target->taskChecked(kTasks[index]))
            return *it;
    }
    return ProgressId;
}

} // namespace subedit::gui
```

`settings_taskSettingsOf` above is not a real function — replace it with a small local lambda in the constructor (or four explicit lines) that picks `settings.mentions`/`settings.commonErrors`/`settings.capitalization`/`settings.lineBreak` by task, since `CorrectionSettings` has no array of `TaskSettings` to index:

```cpp
    const auto taskSettingsOf = [&settings](core::CorrectionTask task) -> const core::TaskSettings& {
        switch (task) {
        case core::CorrectionTask::Mentions:
            return settings.mentions;
        case core::CorrectionTask::CommonErrors:
            return settings.commonErrors;
        case core::CorrectionTask::Capitalization:
            return settings.capitalization;
        case core::CorrectionTask::LineBreak:
            return settings.lineBreak;
        }
        return settings.mentions; // unreachable
    };
    for (const core::CorrectionTask task : kTasks)
        m_target->setTaskChecked(task, taskSettingsOf(task).enabled);
```

Use this in place of the `settings_taskSettingsOf` line above.

- [ ] **Step 4: Run the test to verify it passes**

Run: `cmake --build build/debug --target subedit_gui_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-wizard]"`
Expected: PASS, both cases.

- [ ] **Step 5: Commit**

```bash
git add src/lib/subedit/gui/correction_wizard.hpp src/lib/subedit/gui/correction_wizard.cpp src/test/gui/correction_wizard_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): assemble the correction assistant's wizard"
```

## Task 15: Core+GUI — the controller: rework `applyCorrections`, then wire everything to `MainWindow`

This is the integration task: it makes `applyCorrections` fit the ordinary `Session::apply` road instead of mutating outside it (the decision from this plan's design conversation), then builds `CorrectionController` — the collaborator that opens the wizard, resolves the target, feeds the background computation, and applies what is accepted — and finally wires a `Tools ▸ Correct Texts…` action into `MainWindow`, the way `ProjectOperations`/`ProjectSearch` already are (ADR 0034/0035).

**Files:**
- Modify: `src/lib/subedit/core/text/correction_run.hpp`, `src/lib/subedit/core/text/correction_run.cpp`, `src/test/unit/core/text/correction_run_test.cpp` (Step 1)
- Modify: `src/lib/subedit/core/wording.hpp`, `src/lib/subedit/core/wording.cpp` (Step 1: add `noticeOfCorrection`)
- Create: `src/lib/subedit/gui/correction_controller.hpp`, `src/lib/subedit/gui/correction_controller.cpp`
- Test: `src/test/gui/correction_controller_test.cpp`
- Modify: `src/lib/subedit/gui/main_window.hpp`, `src/lib/subedit/gui/main_window.cpp`, `src/lib/subedit/gui/window_actions.hpp`, `src/lib/subedit/gui/window_actions.cpp`
- Modify: `src/exe/gui/main.cpp` (resolve and hand over the pattern catalogue, Task 3's paths)
- Test: `src/test/gui/window_correction_test.cpp` (the registry's eight cases)
- Modify: every relevant `CMakeLists.txt`

**Interfaces:**
- Consumes: everything from Tasks 1–14.
- Produces (core): `[[nodiscard]] std::string noticeOfCorrection(std::size_t corrected, std::size_t removed);` in `wording.hpp`; `applyCorrections` keeps its signature but no longer applies its own commands.
- Produces (gui): `class CorrectionController final` with a nested `View`, `CorrectionController(Prompts& prompts, View& view)`, `void open();`, `[[nodiscard]] const core::CorrectionSettings& settings() const;`, `void setSettings(core::CorrectionSettings settings);`. `MainWindow::correctTextsAction()` and (for tests) `MainWindow::correctionController()`.

### Step 1: rework `applyCorrections` so it composes without applying

`Session::project()` is documented const — "the only road to a change is a command, and the compiler holds it" — but #504's `applyCorrections` currently calls `composite->apply(*group.project)` directly, mutating outside any session, and its own doc comment claims the result is "applied but not yet inscribed to a history." Nothing in `Session`/`History` can inscribe an already-applied command without re-running it, and adding that would weaken the one invariant every other operation in this codebase relies on. Rather than extend `Session`, `applyCorrections` is brought in line with every other operation instead: it composes each project's `CompositeCommand` and hands it back **unapplied** — the caller (`CorrectionController`, Step 5 below) applies it the ordinary way, `Session::apply(command)`, exactly like `ProjectOperations::apply`.

- [ ] **Step 1a: Update the existing test to the new contract**

In `src/test/unit/core/text/correction_run_test.cpp`, replace the test named `"applying accepted corrections is one command per project, undone as one"` (find it — it is the second `TEST_CASE` in the file, right after the one Task 1 already touched) with:

```cpp
TEST_CASE("applying accepted corrections composes one command per project, without applying it",
          "[text][assistant]") {
    Project first = projectOf({"Bonjour  Marie"});
    Project second = projectOf({"Au  revoir"});

    CorrectionSettings settings;
    settings.commonErrors = {.enabled = true, .code = "Zyyy"};
    const std::vector<CorrectionTarget> targets{wholeProject(first), wholeProject(second)};
    const CharacterLineMeasure measure;
    const CorrectionProposal proposed =
        proposeCorrections(IcuPatternEngine{}, shippedPatterns(), settings, measure, targets);
    REQUIRE(proposed.corrections.size() == 2);

    const std::vector<AppliedCorrection> composed = applyCorrections(proposed.corrections, true);

    REQUIRE(composed.size() == 2);
    CHECK(composed[0].project == &first);
    CHECK(composed[1].project == &second);
    // Composed, not applied: the projects are exactly what they were.
    CHECK(textsOf(first) == std::vector<std::string>{"Bonjour  Marie"});
    CHECK(textsOf(second) == std::vector<std::string>{"Au  revoir"});

    // tallyOf reads the composed commands before anyone has applied them — the
    // same order `ProjectOperations::removeHearingImpaired` already uses for
    // its own tally.
    const CorrectionTally tally = tallyOf(composed);
    CHECK(tally.corrected == 2);
    CHECK(tally.removed == 0);

    // Applying each command by hand does what it says — proof the command
    // itself is complete and correct without `applyCorrections` having run it.
    composed[0].command->apply(first);
    composed[1].command->apply(second);
    CHECK(textsOf(first) == std::vector<std::string>{"Bonjour Marie"});
    CHECK(textsOf(second) == std::vector<std::string>{"Au revoir"});
}
```

Also grep the rest of the file for any other assertion that reads a project's text right after `applyCorrections` without an explicit `command->apply(...)` in between (the removal test, if there is one, is the likely other case) and add the same explicit `apply` call before it — the rest of the test's own logic (what gets removed vs. blanked) does not change, only *when* it takes effect.

- [ ] **Step 1b: Run the test to verify it fails**

Run: `cmake --build build/debug --target subedit_unit_test 2>&1 | head -30`
Expected: FAIL — the current `applyCorrections` still mutates directly, so `textsOf(first)` already reads the corrected text before `command->apply` is ever called by the test.

- [ ] **Step 1c: Remove the direct `apply` call**

In `correction_run.cpp`, delete the line `composite->apply(*group.project);` from `applyCorrections` — everything else in that function (grouping by project, building `SetTextCommand`/`RemoveCommand`/`CompositeCommand`) stays exactly as it is. Update the doc comment on `applyCorrections` in `correction_run.hpp`:

```cpp
/// Composes `accepted` — a subset of what `proposeCorrections` answered —
/// into **one `CompositeCommand` per project, not yet applied**: the caller
/// runs it the ordinary way, `Session::apply(command)`, so undoing it is one
/// gesture per project and the session's own history stays the only road to a
/// change (`Session::project()`'s own rule). `tallyOf` below reads a composed
/// command before it is applied — the same order every other tally in this
/// codebase already uses.
```

- [ ] **Step 1d: Run the tests to verify they pass**

Run: `cmake --build build/debug --target subedit_unit_test && ./build/debug/src/test/unit/subedit_unit_test "[assistant]"`
Expected: PASS.

- [ ] **Step 1e: Add `noticeOfCorrection` to `core/wording.hpp`**

D8: "the phrase lives in `core/wording.hpp`, since phase 13 will say the same." Add, near `noticeOfSplit`:

```cpp
// wording.hpp
/// What the correction assistant applied: "N subtitle(s) corrected, M removed".
[[nodiscard]] std::string noticeOfCorrection(std::size_t corrected, std::size_t removed);
```

```cpp
// wording.cpp
std::string noticeOfCorrection(std::size_t corrected, std::size_t removed) {
    return countOf(corrected, "subtitle") + " corrected, " + std::to_string(removed) + " removed";
}
```

Add a one-line test beside `noticeOfSplit`'s own test in `wording_test.cpp` (grep for it to match its exact style): `CHECK(noticeOfCorrection(3, 1) == "3 subtitles corrected, 1 removed");` and `CHECK(noticeOfCorrection(1, 0) == "1 subtitle corrected, 0 removed");` (singular/plural, `countOf`'s own rule).

- [ ] **Step 1f: Run, then commit Step 1 on its own**

Run: `cmake --build build/debug --target subedit_unit_test && ./build/debug/src/test/unit/subedit_unit_test "[assistant],[wording]"`
Expected: PASS.

```bash
git add src/lib/subedit/core/text/correction_run.hpp src/lib/subedit/core/text/correction_run.cpp src/test/unit/core/text/correction_run_test.cpp src/lib/subedit/core/wording.hpp src/lib/subedit/core/wording.cpp src/test/unit/core/wording_test.cpp
git commit -m "refactor(core): compose corrections without applying them"
```

### Step 2: write the failing `CorrectionController` tests

Following `project_operations_test.cpp`'s own shape exactly: a `Desk`/`Bench`-style `View` double, `ProjectPage::make`, `FakePrompts`. The wizard's async progress page means the `fill` callback (run synchronously inside `FakePrompts::run`, before it returns) must itself drive the wizard through every page, including waiting for the background computation with `QSignalSpy::wait` — nothing here needs a new seam, `fill` already receives the live `QDialog&` (a `CorrectionWizard&`, once cast) and the test's own event loop is real.

```cpp
// src/test/gui/correction_controller_test.cpp
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/gui/correction_controller.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/project_page.hpp>

#include <QCheckBox>
#include <QFont>
#include <QSignalSpy>
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::PatternCatalogue;
using subedit::core::Project;
using subedit::core::readPatternCatalogue;
using subedit::core::SubtitleIndex;
using subedit::gui::CorrectionController;
using subedit::gui::CorrectionWizard;
using subedit::gui::ProjectPage;
using subedit::test::FakePrompts;

/// One `Zyyy` common-error record, "Double space" — the one task exercised
/// below. Built the way Task 1/5/8 already build small catalogues.
PatternCatalogue smallCatalogue();

[[nodiscard]] std::unique_ptr<ProjectPage> pageOn(const char* content) {
    InMemoryFileSystem files;
    files.addFile("/film.srt", content);
    auto opened = openProject(files, "/film.srt");
    REQUIRE(opened.has_value());
    return ProjectPage::make(std::move(opened->project));
}

/// The window, as the controller sees it.
class Desk final : public CorrectionController::View {
public:
    QWidget parent;
    PatternCatalogue catalogue;
    std::vector<std::unique_ptr<ProjectPage>> pages;
    std::size_t shown = 0;
    std::vector<std::string> announced;
    std::vector<std::pair<core::Project*, SubtitleIndex>> previewed;

    explicit Desk(PatternCatalogue catalogueIn) : catalogue(std::move(catalogueIn)) {}

    [[nodiscard]] QWidget* dialogParent() override { return &parent; }
    [[nodiscard]] std::span<const std::unique_ptr<ProjectPage>> pages() const override { return pages; }
    [[nodiscard]] std::size_t shownProject() const override { return shown; }
    [[nodiscard]] const PatternCatalogue& patternCatalogue() const override { return catalogue; }
    [[nodiscard]] QFont applicationFont() const override { return QFont{}; }
    void announce(const std::string& message) override { announced.push_back(message); }
    void preview(core::Project& project, SubtitleIndex index) override {
        previewed.emplace_back(&project, index);
    }
};

/// Drives a `CorrectionWizard` from the target page straight through to the
/// confirmation page, checking one task and waiting for its computation.
void driveToConfirmation(QDialog& dialog) {
    auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
    wizard.targetPage().setTaskChecked(core::CorrectionTask::CommonErrors, true);
    wizard.commonErrorsPage().applySettings(wizard.settingsUsedForTest()); // see note below
    wizard.next(); // target -> common errors
    wizard.next(); // common errors -> progress

    const QSignalSpy spy{&wizard.progressPage(), &subedit::gui::CorrectionProgressPage::completeChanged};
    REQUIRE(spy.wait(2000));
}

} // namespace

TEST_CASE("Finish applies the accepted corrections and announces the tally",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = &driveToConfirmation;
    CorrectionController controller{prompts, desk};

    controller.open();

    CHECK(desk.pages[0]->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
         "Bonjour Marie");
    CHECK(desk.pages[0]->session->canUndo());
    REQUIRE(desk.announced.size() == 1);
    CHECK(desk.announced[0] == "1 subtitle corrected, 0 removed");
}

TEST_CASE("cancelling the wizard leaves every project and the settings untouched",
          "[gui][correction-controller]") {
    Desk desk{smallCatalogue()};
    desk.pages.push_back(pageOn("1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n"));
    FakePrompts prompts;
    prompts.nextRun = false; // Cancel
    CorrectionController controller{prompts, desk};
    const core::CorrectionSettings before = controller.settings();

    controller.open();

    CHECK(desk.pages[0]->session->project().subtitleAt(SubtitleIndex::fromValue(0)).mainText ==
         "Bonjour  Marie");
    CHECK(desk.announced.empty());
    CHECK(controller.settings() == before);
}
```

The `driveToConfirmation` sketch above leans on wizard/page accessors this plan has already defined (`setTaskChecked`, `applySettings`, `next()`) except `wizard.settingsUsedForTest()`, which does not exist — replace that line with whatever actually gets a `CommonErrorsPage` showing the "Double space" pattern checked: since `smallCatalogue()` ships it `Enabled` by default and `applySettings` was already called once by `CorrectionWizard`'s own constructor (from `CorrectionController::open()`, with `desk`'s starting `CorrectionSettings{}`), the box should already be checked without calling `applySettings` again — delete that line and verify the test still passes; only add it back if the shipped default turns out to need an explicit override once this is actually run.

- [ ] **Step 3: Run the tests to verify they fail**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -40`
Expected: FAIL to compile — `correction_controller.hpp` does not exist yet.

### Step 4: implement `CorrectionController`

```cpp
// src/lib/subedit/gui/correction_controller.hpp
#pragma once

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <QFont>

#include <memory>
#include <span>
#include <string>
#include <vector>

class QWidget;

namespace subedit::core {
class PatternCatalogue;
class Project;
} // namespace subedit::core

namespace subedit::gui {

class Prompts;
struct ProjectPage;

/// `Tools ▸ Correct Texts…` end to end — D8: opens the wizard, resolves the
/// target, computes proposals in the background, applies what is accepted.
class CorrectionController final {

public:
    class View {

    public:
        virtual ~View() = default;

        [[nodiscard]] virtual QWidget* dialogParent() = 0;

        /// Every open project, in tab order — the same road `ProjectSearch`
        /// walks for "every open project", but as a span: Task 4's
        /// `correctionTargetsOf` is written against `MainWindow::m_pages`'s
        /// own storage.
        [[nodiscard]] virtual std::span<const std::unique_ptr<ProjectPage>> pages() const = 0;

        [[nodiscard]] virtual std::size_t shownProject() const = 0;

        [[nodiscard]] virtual const core::PatternCatalogue& patternCatalogue() const = 0;

        /// The font the ems measure calibrates against — `QApplication::font()`
        /// in production, `DejaVu Sans` under a test (D5's own rule).
        [[nodiscard]] virtual QFont applicationFont() const = 0;

        virtual void announce(const std::string& message) = 0;

        /// Places playback on `index` of `project`'s own page — does nothing
        /// if that project has no film. `Preview`, D8.
        virtual void preview(core::Project& project, core::SubtitleIndex index) = 0;

    protected:
        View() = default;
        View(const View&) = default;
        View(View&&) = default;
        View& operator=(const View&) = default;
        View& operator=(View&&) = default;
    };

    /// `prompts` and `view` must outlive this.
    CorrectionController(Prompts& prompts, View& view);

    /// Opens the assistant. Settings persist only if it was finished, never
    /// if it was cancelled.
    void open();

    [[nodiscard]] const core::CorrectionSettings& settings() const { return m_settings; }
    void setSettings(core::CorrectionSettings settings) { m_settings = std::move(settings); }

private:
    Prompts* m_prompts;
    View* m_view;
    core::CorrectionSettings m_settings;
};

} // namespace subedit::gui
```

```cpp
// src/lib/subedit/gui/correction_controller.cpp
#include <subedit/gui/correction_controller.hpp>

#include <subedit/core/model/project.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/correction_target_page.hpp>
#include <subedit/gui/correction_task_page.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/ems_line_measure.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/target.hpp>

#include <QDialog>

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

namespace subedit::gui {

namespace {

[[nodiscard]] const core::TaskSettings&
taskSettingsFor(const core::CorrectionSettings& settings, core::CorrectionTask task) {
    switch (task) {
    case core::CorrectionTask::Mentions:
        return settings.mentions;
    case core::CorrectionTask::CommonErrors:
        return settings.commonErrors;
    case core::CorrectionTask::Capitalization:
        return settings.capitalization;
    case core::CorrectionTask::LineBreak:
        return settings.lineBreak;
    }
    return settings.mentions; // unreachable
}

/// Merges `from` into `into` by (kind, code, name), replacing an existing
/// entry or adding a new one — never clearing a stale entry for a code this
/// wizard run never showed (a known limitation, see Task 9's own note on
/// `PatternList`).
void mergeActivations(std::vector<core::PatternActivation>& into,
                      const std::vector<core::PatternActivation>& from) {
    for (const core::PatternActivation& activation : from) {
        const auto found = std::ranges::find_if(into, [&](const core::PatternActivation& existing) {
            return existing.kind == activation.kind && existing.code == activation.code &&
                   existing.name == activation.name;
        });
        if (found != into.end())
            *found = activation;
        else
            into.push_back(activation);
    }
}

/// Reads every page of `wizard` into `previous`'s shape — the settings to
/// compute with while the wizard is open, and to persist once it finishes.
[[nodiscard]] core::CorrectionSettings settingsOf(const CorrectionWizard& wizard,
                                                  const core::CorrectionSettings& previous) {
    core::CorrectionSettings settings = previous;
    settings.mentions = {.enabled = wizard.targetPage().taskChecked(core::CorrectionTask::Mentions),
                         .code = wizard.mentionsPage().code()};
    settings.commonErrors = {
        .enabled = wizard.targetPage().taskChecked(core::CorrectionTask::CommonErrors),
        .code = wizard.commonErrorsPage().code()};
    settings.capitalization = {
        .enabled = wizard.targetPage().taskChecked(core::CorrectionTask::Capitalization),
        .code = wizard.capitalizationPage().code()};
    settings.lineBreak = {.enabled = wizard.targetPage().taskChecked(core::CorrectionTask::LineBreak),
                          .code = wizard.lineBreakPage().code()};
    settings.human = wizard.commonErrorsPage().human();
    settings.ocr = wizard.commonErrorsPage().ocr();
    settings.soundInBrackets = wizard.mentionsPage().soundInBrackets();
    settings.soundInParentheses = wizard.mentionsPage().soundInParentheses();
    settings.lineBreakMaxLength = wizard.lineBreakPage().maxLength();
    settings.lineBreakMaxLines = wizard.lineBreakPage().maxLines();
    settings.removeBlankSubtitles = wizard.confirmationPage().removeBlankSubtitles();
    mergeActivations(settings.patternActivations, wizard.mentionsPage().activations());
    mergeActivations(settings.patternActivations, wizard.commonErrorsPage().activations());
    mergeActivations(settings.patternActivations, wizard.capitalizationPage().activations());
    mergeActivations(settings.patternActivations, wizard.lineBreakPage().activations());
    return settings;
}

[[nodiscard]] ProjectPage* pageOwning(std::span<const std::unique_ptr<ProjectPage>> pages,
                                      const core::Project& project) {
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        if (&page->session->project() == &project)
            return page.get();
    }
    return nullptr;
}

} // namespace

CorrectionController::CorrectionController(Prompts& prompts, View& view)
    : m_prompts(&prompts), m_view(&view) {}

void CorrectionController::open() {
    const std::span<const std::unique_ptr<ProjectPage>> pages = m_view->pages();
    const std::size_t shown = m_view->shownProject();

    const bool selectionAvailable = selectionOf(*pages[shown]->tableSelection).count() != 0;
    bool translationAvailable = false;
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        if (page->session->project().translationFile().has_value()) {
            translationAvailable = true;
            break;
        }
    }

    CorrectionWizard wizard{
        m_view->patternCatalogue(), m_settings, selectionAvailable, translationAvailable, m_view->dialogParent()};

    core::IcuPatternEngine engine;
    QObject::connect(&wizard.progressPage(), &CorrectionProgressPage::aboutToCompute, &wizard,
                     [this, &wizard, &engine, pages, shown] {
                         const core::CorrectionSettings current = settingsOf(wizard, m_settings);
                         const CorrectionScope scope = wizard.targetPage().scope();
                         const core::Document document = wizard.targetPage().document();
                         std::vector<core::CorrectionTarget> targets =
                             correctionTargetsOf(scope, document, pages, shown);

                         const std::shared_ptr<const core::LineMeasure> measure =
                             wizard.lineBreakPage().useEms()
                                 ? std::static_pointer_cast<const core::LineMeasure>(
                                       std::make_shared<EmsLineMeasure>(m_view->applicationFont()))
                                 : std::static_pointer_cast<const core::LineMeasure>(
                                       std::make_shared<core::CharacterLineMeasure>());

                         wizard.progressPage().setComputation(
                             [this, &engine, current, targets = std::move(targets), measure] {
                                 return core::proposeCorrections(
                                     engine, m_view->patternCatalogue(), current, *measure, targets);
                             });
                     });

    QObject::connect(&wizard.confirmationPage(), &CorrectionConfirmationPage::previewRequested, &wizard,
                     [this, &wizard](int row) {
                         const core::ProposedCorrection& correction =
                             wizard.confirmationPage().resultModel()->correctionAt(row);
                         m_view->preview(*correction.project, correction.index);
                     });

    if (!m_prompts->run(wizard))
        return; // cancelled: settings and every project stay as they were

    m_settings = settingsOf(wizard, m_settings);

    const std::vector<core::ProposedCorrection> accepted =
        wizard.confirmationPage().resultModel()->acceptedCorrections();
    std::vector<core::AppliedCorrection> composed =
        core::applyCorrections(accepted, wizard.confirmationPage().removeBlankSubtitles());
    const core::CorrectionTally tally = core::tallyOf(composed); // before the commands are moved below

    for (core::AppliedCorrection& one : composed) {
        ProjectPage* owner = pageOwning(pages, *one.project);
        if (owner == nullptr)
            continue; // its project closed while the wizard was open
        owner->model->applied(owner->session->apply(std::move(one.command)));
    }

    m_view->announce(core::noticeOfCorrection(tally.corrected, tally.removed));
}

} // namespace subedit::gui
```

Two things to verify once this actually builds, that could not be checked from reading alone:

1. **The lambda connected to `aboutToCompute` captures `&wizard` and `&engine`, both local to `open()`.** This is safe only because `m_prompts->run(wizard)` — a `QDialog::exec()` underneath in production — blocks until the wizard closes, so the connection is never invoked after `open()`'s locals start going out of scope. Confirm `QtPrompts::run` (the production `Prompts` implementation — find it, likely `qt_prompts.cpp`) really does call `dialog.exec()` and nothing asynchronous.
2. **A background computation that is still running when Cancel is pressed keeps running with its own copy of the closure**, which holds `core::CorrectionTarget`s pointing at live `core::Project` objects. If the very project a target points at is closed in the brief window between Cancel and that orphaned computation finishing, the read is a dangling pointer. This is a narrow, real risk this plan accepts rather than engineers around (see this task's own note above) — do not silently fix it with something ad hoc; if it needs closing, that is a follow-up issue with its own design conversation, not a line added here under time pressure.

- [ ] **Step 5: Run the `CorrectionController` tests, fixing what only building reveals**

Run: `cmake --build build/debug --target subedit_gui_test 2>&1 | head -60`

This is the task most likely to need real back-and-forth with the compiler — the `Desk`/`driveToConfirmation` sketch above has at least one placeholder line already flagged. Iterate: build, read the first error, fix it, rebuild, until `./build/debug/src/test/gui/subedit_gui_test "[correction-controller]"` passes both cases.

- [ ] **Step 6: Commit the controller**

```bash
git add src/lib/subedit/gui/correction_controller.hpp src/lib/subedit/gui/correction_controller.cpp src/test/gui/correction_controller_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): add the correction assistant's controller"
```

### Step 7: wire `Tools ▸ Correct Texts…` into `MainWindow`

Follow `ProjectSearch`/`ProjectOperations`'s exact wiring shape (`main_window.cpp:193-330`, already read while designing this plan): a private nested `CorrectionSide final : public CorrectionController::View`, one member, one line in the constructor, one action.

- [ ] **Step 7a: add the action**

In `window_actions.hpp`, add `QAction* correctTexts = nullptr;` beside `hearingImpaired` in the `Tools` section. In `window_actions.cpp`'s constructor initializer list, add (right after `hearingImpaired(...)`):

```cpp
correctTexts(buildAction(owner, QStringLiteral("&Correct Texts…"), {})),
```

In the same file's menu-building function (`tools->addAction(hearingImpaired);` — grep it), add right after:

```cpp
tools->addAction(correctTexts);
```

- [ ] **Step 7b: add `m_patterns` and its setter to `MainWindow`**

In `main_window.hpp`, add a private member `core::PatternCatalogue m_patterns;` (default-constructed: empty, both vectors empty — a `MainWindow` built without one, as every existing test still does, simply offers no patterns, same graceful-empty shape `CorrectionSettings{}` already has) and a public setter, next to `setManualPath`:

```cpp
/// The patterns the assistant offers — resolved once, in `main.cpp`, the
/// same road `installedManualPath()`/`setManualPath` already take (ADR 0022).
/// Left empty by default: every existing test constructs a window without
/// calling this, and an empty catalogue simply offers no pattern to check.
void setPatternCatalogue(core::PatternCatalogue catalogue) { m_patterns = std::move(catalogue); }
```

Add `#include <subedit/core/text/pattern_catalogue.hpp>` to `main_window.hpp`.

- [ ] **Step 7c: add the `CorrectionSide` and the collaborator**

In `main_window.cpp`, right after `OperationsSide` (around line 330, already read), add:

```cpp
/// What the correction assistant asks of the window — issue #505.
class MainWindow::CorrectionSide final : public CorrectionController::View {

public:
    explicit CorrectionSide(MainWindow& window) : m_window(&window) {}

    [[nodiscard]] QWidget* dialogParent() override { return m_window; }

    [[nodiscard]] std::span<const std::unique_ptr<ProjectPage>> pages() const override {
        return m_window->m_pages;
    }

    [[nodiscard]] std::size_t shownProject() const override {
        return static_cast<std::size_t>(m_window->m_currentPage);
    }

    [[nodiscard]] const core::PatternCatalogue& patternCatalogue() const override {
        return m_window->m_patterns;
    }

    [[nodiscard]] QFont applicationFont() const override { return QApplication::font(); }

    void announce(const std::string& message) override {
        m_window->statusBar()->showMessage(QString::fromStdString(message), kOperationStatusTimeoutMs);
    }

    void preview(core::Project& project, core::SubtitleIndex index) override {
        for (const std::unique_ptr<ProjectPage>& page : m_window->m_pages) {
            if (&page->session->project() != &project)
                continue;
            page->tableSelection->select(page->model->index(static_cast<int>(index.value()), 0),
                                         QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            m_window->m_video->placeAtSelection(*page);
            return;
        }
    }

private:
    MainWindow* m_window;
};
```

In the constructor, next to `m_operationsSide`/`m_operations` (around `main_window.cpp:575-576`, already read):

```cpp
m_correctionSide = std::make_unique<CorrectionSide>(*this);
m_correction = std::make_unique<CorrectionController>(*m_prompts, *m_correctionSide);
```

and the action:

```cpp
connect(m_actions->correctTexts, &QAction::triggered, this, [this] { m_correction->open(); });
```

Add `std::unique_ptr<CorrectionSide> m_correctionSide;` and `std::unique_ptr<CorrectionController> m_correction;` to `main_window.hpp`'s private members, and `class CorrectionSide;` to its forward declarations, alongside `class OperationsSide;`. Add `#include <subedit/gui/correction_controller.hpp>` to `main_window.cpp`.

- [ ] **Step 7d: `refreshActions`, settings, and a test accessor**

In `refreshActions()` (`main_window.cpp:1093`, already read), right after `m_actions->hearingImpaired->setEnabled(anything);`:

```cpp
m_actions->correctTexts->setEnabled(anything);
```

In `applySettings`/`settings()` (grep for `m_operations->setDurationSettings`/`m_operations->durationSettings()`, already read at `main_window.cpp:1438`/`1472`), add the same pair:

```cpp
m_correction->setSettings(settings.correction);
```
```cpp
settings.correction = m_correction->settings();
```

For tests, add to `main_window.hpp`, near `hearingImpairedAction()`:

```cpp
[[nodiscard]] QAction* correctTextsAction() const { return m_actions->correctTexts; }
```

- [ ] **Step 8: resolve the catalogue in `main.cpp`**

In `src/exe/gui/main.cpp`, right after `window.setManualPath(subedit::gui::installedManualPath());` (already read):

```cpp
// The patterns the assistant offers — Task 3's two resolvers, ADR 0022's
// own rule: resolved here, with the real executable and the real
// environment, and nowhere else.
window.setPatternCatalogue(subedit::core::readPatternCatalogue(
    files, subedit::gui::installedPatternsPath(), subedit::gui::resolvedUserPatternsPath()));
```

Add `#include <subedit/gui/patterns_path.hpp>` and `#include <subedit/core/text/pattern_catalogue.hpp>` to `main.cpp`.

- [ ] **Step 9: write the registry's end-to-end tests**

One new file, `src/test/gui/window_correction_test.cpp`, following `window_hearing_impaired_test.cpp`'s and `window_tabs_test.cpp`'s own shape — a real `MainWindow`, `window.correctTextsAction()->trigger()`, `FakePrompts` with a `fill` that drives the wizard. Eight cases, one per registry entry:

```cpp
// src/test/gui/window_correction_test.cpp
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/correction_wizard.hpp>
#include <subedit/gui/main_window.hpp>

#include <QSignalSpy>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::CorrectionTask;
using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::OpenedFile;
using subedit::gui::CorrectionWizard;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

[[nodiscard]] InMemoryFileSystem withFile(const char* content) {
    InMemoryFileSystem files;
    files.addFile("film.srt", content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* path = "film.srt") {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// One text, one common error to fix.
constexpr const char* kOneError = "1\n00:00:01,000 --> 00:00:02,000\nBonjour  Marie\n\n";

/// Common-error task, no target/document override needed (defaults are
/// current-project/text): checks the task, advances to progress, waits.
void checkCommonErrorsAndAdvance(CorrectionWizard& wizard) {
    wizard.targetPage().setTaskChecked(CorrectionTask::CommonErrors, true);
    wizard.next();
    const QSignalSpy spy{&wizard.progressPage(), &subedit::gui::CorrectionProgressPage::completeChanged};
    wizard.next();
    REQUIRE(spy.wait(2000));
}

} // namespace

TEST_CASE("GUI-CORRECT-01: the assistant applies checked tasks to the chosen target and document",
          "[gui][GUI-CORRECT-01]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) { checkCommonErrorsAndAdvance(dynamic_cast<CorrectionWizard&>(dialog)); };
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(window.table()
             ->model()
             ->data(window.table()->model()->index(0, 4), Qt::DisplayRole)
             .toString()
             .toStdString() == "Bonjour Marie");
}

TEST_CASE("GUI-CORRECT-02: a changed text shows its original, and is accepted, refused or retouched",
          "[gui][GUI-CORRECT-02]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        auto& wizard = dynamic_cast<CorrectionWizard&>(dialog);
        checkCommonErrorsAndAdvance(wizard);
        // The confirmation page is current now: refuse the one row.
        wizard.confirmationPage().resultModel()->markAll(false);
    };
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(window.table()
             ->model()
             ->data(window.table()->model()->index(0, 4), Qt::DisplayRole)
             .toString()
             .toStdString() == "Bonjour  Marie"); // refused: unchanged
}

TEST_CASE("GUI-CORRECT-03: applying makes one history entry and the status bar says the count",
          "[gui][GUI-CORRECT-03]") {
    InMemoryFileSystem files = withFile(kOneError);
    FakePrompts prompts;
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) { checkCommonErrorsAndAdvance(dynamic_cast<CorrectionWizard&>(dialog)); };
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.correctTextsAction()->trigger();

    CHECK(window.undoAction()->isEnabled());
    window.undoAction()->trigger();
    CHECK(window.table()
             ->model()
             ->data(window.table()->model()->index(0, 4), Qt::DisplayRole)
             .toString()
             .toStdString() == "Bonjour  Marie"); // one undo puts it all back
}
```

`GUI-CORRECT-03` above checks undo rather than the status bar directly because `MainWindow`'s status bar text is not already exposed to tests the way `FakePrompts::outcomes` is for `reportOutcome` (`announce` goes to `statusBar()->showMessage`, read via `window.statusBar()->currentMessage()` if that accessor exists — check `status_line_test.cpp` or `window_actions_test.cpp` for how an existing test reads an announced message, e.g. `window.statusBar()->currentMessage().toStdString()`, and add that assertion here too if it does).

Continue with the remaining five, each isolating one thing the earlier three do not already cover:

- **`GUI-CORRECT-04`** — open the assistant twice: the first run checks a specific pattern off in the common-errors page's `PatternList` (find it via `wizard.commonErrorsPage().findChild<PatternList*>()`... actually `PatternList` is itself a private member the page does not expose; instead drive it through `wizard.commonErrorsPage().activations()` after unchecking one box found by `findChild<QCheckBox*>(QString::fromStdString(patternName))` — `PatternList::setCode` already gives each box that `objectName`, Task 8) and finishes; the second run reopens the assistant and checks that pattern's box is still unchecked — proof the choice was retained (`CorrectionSettings.patternActivations`, D2/D8).
- **`GUI-CORRECT-05`** — a text a Human-classed pattern and an OCR-classed pattern would both touch (reuse `common-error.cas`'s own fixtures if two such patterns exist there, or write two records into a small in-memory catalogue the way Task 1 does): uncheck OCR, leave Human checked, and show only the Human-classed change lands (D4).
- **`GUI-CORRECT-06`** — an in-memory catalogue (`InMemoryFileSystem` + `readPatternCatalogue`, Task 1's own recipe) carrying one pattern that will not compile (`Pattern=(`) alongside one that does: after Finish, assert the confirmation page's `abandonedPatterns` label (found via `findChild<QLabel*>`, Task 13) named the broken one, **and** the other correction still applied. This needs `MainWindow::setPatternCatalogue` called before `window.show()`, so the window under test does not read the shipped patterns.
- **`GUI-CORRECT-07`**: same in-memory catalogue idea, but through the **user** directory rather than a compile failure — `readPatternCatalogue(files, shipped, user)` already merges both (Task 1's own dependency); write one record into each and confirm both apply.
- **`GUI-HEARING-03`**: check Mentions, tick "Sound in brackets", on a text with `#music#` and one with `[bracketed]` — confirm both the balayage-based bracket removal (already covered by `GUI-HEARING-01`, not retested here) and the moteur-based `#…#` removal fire from the same run.

Each of these needs its own small `InMemoryFileSystem`/catalogue fixture; build them the way `pattern_catalogue_test.cpp` and Task 1 already do, not by reading `reference/gaupol` or the shipped files (`SUBEDIT_PATTERNS_DIR`) for anything the test needs to assert a specific outcome from — the shipped files can change, a fixture the test itself wrote cannot.

- [ ] **Step 10: Run every test written across this task**

Run: `cmake --build build/debug --target subedit_gui_test subedit_unit_test && ./build/debug/src/test/gui/subedit_gui_test "[correction-controller],[GUI-CORRECT],[GUI-HEARING-03]" && ./build/debug/src/test/unit/subedit_unit_test "[assistant],[wording]"`
Expected: PASS, every case.

- [ ] **Step 11: Commit the wiring and the registry tests**

```bash
git add src/lib/subedit/gui/main_window.hpp src/lib/subedit/gui/main_window.cpp src/lib/subedit/gui/window_actions.hpp src/lib/subedit/gui/window_actions.cpp src/exe/gui/main.cpp src/test/gui/window_correction_test.cpp src/lib/subedit/gui/CMakeLists.txt src/test/gui/CMakeLists.txt
git commit -m "feat(gui): wire Correct Texts… into the window"
```

## Task 16: finishing — screenshot, manual, registry, version

Everything in Tasks 1–15 is code and tests. This task is the project's own "definition of done" (root `CLAUDE.md`): a screenshot `make manual` generates, the manual section, the registry entries, the patch bump, and the regenerated changelog — in that order, because the order is why each step it depends on is already done by the time it runs.

- [ ] **Step 1: Add the confirmation screenshot**

In `src/test/tools/screenshots.cpp`, follow the exact `SplitProjectDialog`/`DurationAdjustDialog` shape already read while designing this plan (construct the widget pre-filled with the state the manual describes, `capture(widget, widget, directory, name)`, light then dark). The confirmation page is what GUI-CORRECT-02/03's own criterion ("une capture de la confirmation engendrée par `make manual`") asks for — build a `CorrectionResultModel` directly (no need for a whole wizard or window) with two or three rows showing a mix of accepted/refused/removed, set it on a bare `QTableView` with `CorrectionDiffDelegate` on its two rich-text columns, and capture that:

```cpp
{
    subedit::gui::applyTheme(subedit::core::Theme::Light);
    std::vector<subedit::core::ProposedCorrection> rows{
        subedit::core::ProposedCorrection{.original = "Bonjour  Marie", .proposed = std::string{"Bonjour Marie"}},
        subedit::core::ProposedCorrection{.original = "[Bruit de pas]", .proposed = std::nullopt}};
    subedit::gui::CorrectionResultModel model{rows};
    QTableView table;
    table.setModel(&model);
    auto* delegate = new subedit::gui::CorrectionDiffDelegate{&table};
    table.setItemDelegateForColumn(subedit::gui::CorrectionResultModel::Original, delegate);
    table.setItemDelegateForColumn(subedit::gui::CorrectionResultModel::Proposed, delegate);
    written = capture(table, table, directory, "correction") && written;
}
{
    subedit::gui::applyTheme(subedit::core::Theme::Dark);
    // same construction again — `capture` needs a freshly-shown widget per
    // theme, the same reason every existing pair above constructs its dialog
    // twice rather than reusing one across both captures
    written = capture(table, table, directory, "correction-sombre") && written;
}
```

Read the actual surrounding structure of `screenshots.cpp` before writing this in — in particular how `capture` expects to be handed a widget that is not yet shown (it likely calls `show()`/`resize()` itself, check an existing block) and whether the file's own top-level `written` accumulator is named that or something else at the point this is inserted.

- [ ] **Step 2: `make manual` and `compare-screenshots.py`**

Run: `make manual`
This builds `subedit_screenshots`, runs it (offscreen, Fusion style, DejaVu Sans — ADR 0024), and writes `correction.new.png`/`correction-sombre.new.png` since no reference exists yet. Then run whatever `make manual` itself already chains for promotion (check the `Makefile` target — it likely calls `compare-screenshots.py` itself); if it does not, run it directly to promote the two `.new.png` files.

- [ ] **Step 3: Write the manual section**

New file `docs/manual/subedit-gui/correct-texts.md` (or a new section of `operations.md` if that turns out to be the establish

ed home for `Tools` entries once you check how `Remove Hearing-Impaired Mentions…`'s own section is laid out there — match whichever convention the file already uses). Cover, per the table `CLAUDE.md` itself requires: the command (`Tools ▸ Correct Texts…`), its pages in order, every option each page offers with its default, what the confirmation table's three columns and four buttons do, and a real example (a short scenario: open a file, check Common Errors, finish, one text corrected). Link it from `docs/manual/index.md`'s own table, next to the `Remove Hearing-Impaired Mentions…` row (`docs/manual/index.md:27`, already read).

- [ ] **Step 4: Registry — `docs/exigences.md` and `docs/specs/12-correction.md`**

In `docs/exigences.md`, change the `état` column of `GUI-CORRECT-01` through `GUI-CORRECT-07` and `GUI-HEARING-03` (lines 199–206, already read) from `prévue` to whatever the column's own vocabulary uses for "done" (check a row already marked done elsewhere in the same file for the exact word).

In `docs/specs/12-correction.md`, add a `> **Précisé par #505.**` block after D8's existing `> **Précisé par #504.**` block (the spec's own established convention — five examples already in the file, read while designing this plan). It should name, briefly, the real decisions this plan made that D8 itself left open:
- the three per-task combos (script/language/country) are populated from what the catalogue actually carries, not an external locale registry (Task 5);
- the changed part of a diff is marked bold, not coloured (Task 6);
- `applyCorrections` was rewritten to compose without applying, so every correction goes through `Session::apply` like every other operation (Task 15, Step 1) — and why, in one sentence (`Session::project()`'s own const rule);
- "abandon" during progress is the wizard's own Cancel button, with the accepted, narrow risk that a background computation may still be reading a project closed in the moment right after cancelling (Task 15's own note);
- the chosen unit (characters/ems) for line-breaking is not remembered between sessions, only the two numeric limits are (Task 9's own note).

- [ ] **Step 5: Bump the patch, rebuild the manual's `--version` example, `check-local`, `check`**

Follow the root `CLAUDE.md`'s own ordered list exactly — it is not repeated here, since duplicating it is exactly the drift the document itself warns about:
1. `CMakeLists.txt`'s `project(VERSION)` — patch only.
2. `make manual` again (the bump just made the `--version` example in the manual stale).
3. `make check-local`, which chains `make bench` — read what it reports, and if the machine was busy, say so in the PR rather than record a diagnostic run.
4. `make check` once — it sees the code and the bump together.

- [ ] **Step 6: Update the rest of the documentation and the measurement log**

Whatever `make bench`'s successful run (Step 5.3) recorded, and any manual section this task's own Step 3 did not already cover once the real screenshots and final wording are in front of you.

- [ ] **Step 7: Commit, regenerate the changelog, open the PR**

```bash
git add -A
git commit -m "docs: relire la fin de l'issue #505"
```

Regenerate the changelog per the project's own convention (check how the previous entry — issue #504's — was produced; grep `CHANGELOG.md`'s own generation note or script). Open the PR with `Closes #505` in English on its own line, a French description of what was built, and the benchmark note from Step 5.3 if the machine was busy.

---

## Plan-wide notes for whoever executes this

- **Every "verify a pattern by reading the real file first" instruction in this plan is deliberate, not filler.** This plan was written by reading the real `hearing_impaired_dialog`, `project_operations`, `project_search`, `target.hpp`, `session.hpp`, `correction_run.cpp`, `pattern_catalogue.hpp`, `window_hearing_impaired_test.cpp`, `window_tabs_test.cpp`, `screenshots.cpp` and more — but line numbers and exact surrounding code drift the moment another PR merges. Where a step says "grep for X" or "read Y before writing this," do that first; do not assume the snippet quoted here is still byte-for-byte what is on `main`.
- **The one real design fork this plan resolved with the user** (not a default the plan picked on its own): `applyCorrections` was reworked to compose without applying, rather than extending `Session`/`History` with a new "record an already-applied command" method — see Task 15, Step 1's own explanation. Everything downstream of that (the controller's apply loop) depends on this choice.
- **Five smaller decisions this plan made on its own, each flagged where it is made, each worth a sentence in the spec's "Précisé par #505"**: pattern codes are enumerated from the catalogue rather than an external locale list (Task 5); the diff is bold, not coloured (Task 6); a task's own page has no "enabled" checkbox of its own, only the target page's box decides (Task 9/10); the line-break unit is not persisted (Task 9); a stale `PatternActivation` for a code this wizard run never showed is not cleared, only ever replaced or added to (Task 15's `mergeActivations`).
- **What Task 15's own two numbered caveats mean for review**: read them before approving that task's PR, specifically — they are not rhetorical.
