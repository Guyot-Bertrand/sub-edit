# Batch 2 — Commands and window — Implementation plan

> **For the executor:** REQUIRED SUB-SKILL: follow
> superpowers:subagent-driven-development (recommended) or
> superpowers:executing-plans to work through this plan task by task. The steps
> use the checkbox syntax (`- [ ]`) for tracking.

**Goal:** close the five issues of batch 2 defined by #411 — #396, #404,
#398, #397, #409 — in a single pull request, one commit per issue, in this
order.

**Architecture:** five independent fixes that nonetheless touch the same
heavily included files (`main_window.cpp`/`.hpp`, `wording.hpp`,
`command_kind.hpp`), hence the grouping into a single PR decided by #411.
Each task below corresponds to one commit. #404 is the biggest: it
factors into a free function the loop "build, skip the unchanged,
stack, group" that four operations were copying, and fixes along the way a
phantom count in search.

**Tech Stack:** C++23, Qt 6, Catch2, ICU (search).

**Spec:** issue #411 (split into batches) and the five issues themselves —
#396, #404, #398, #397, #409, each with its body and, for #398 and #409, a
decision already settled in a comment. No separate phase spec: these are
end-of-phase-10 corrections.

## Global constraints

- **One commit per issue**, with one `Closes #N` line per issue in the body
  of the pull request (not in the individual commits), in English.
- **C++ is written in English**, identifiers and comments; the commit messages
  and this document are in French.
- **`make check-local` then `make check`** must pass before opening the
  pull request; the patch bump, `make manual` and the bench record follow
  the order described in `CLAUDE.md` — bump only once, at the very end of the
  batch, not after each task.
- **No test may read `reference/gaupol`** or write anything to it.
- The manual (`docs/manual/`) is updated at step 7 of the order in
  `CLAUDE.md`, after the gate, for each task that changes what the
  user sees — hence at the end of the batch, before committing the last
  fix or in a dedicated commit just before the pull request.

---

## Task 1 — #396: the search target survives a structural undo

**Files:**
- Modify: `src/lib/subedit/gui/main_window.cpp` (`MainWindow::openOn`,
  around line 592)
- Modify: `src/lib/subedit/gui/main_window.hpp` (comment of `m_match`,
  line 661)
- Test: `src/test/gui/window_search_test.cpp`

**Exact cause.** `SubtitleTableModel::applied()` does `beginResetModel()` /
`endResetModel()` only for a structural operation (insertion, deletion,
and therefore an undo that replays the inverse of a merge or a split).
`QItemSelectionModel::reset()`, then called by Qt, empties the selection **without
emitting `selectionChanged`** — the connect at line 612 of `main_window.cpp`,
which forgets `m_searchTarget`/`m_match` on a selection change, therefore
never fires in this precise case. The target keeps indices that
no longer exist, and `Project::subtitleAt` throws `std::out_of_range` on the next
`Find Next` or `Replace All`.

Qt's native signal `QAbstractItemModel::modelReset`, for its part, is emitted in every
case — it is exactly what `beginResetModel`/`endResetModel` trigger
— and nothing else hooks onto it today.

- [ ] **Step 1 — write the test that fails today**

In `src/test/gui/window_search_test.cpp`, add, after the existing
cases (the file already has the helper functions `withFour`, `fourIn`,
`searching`, `selectedRows`, `statusOf` — reuse them):

```cpp
TEST_CASE("an undo that resets the model forgets a stale search target",
          "[gui][GUI-SEARCH-03]") {
    // Four subtitles; select the fourth, split it into a fifth. The search
    // then captures a target of {3, 4} before the split is undone.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    selectRow(window, 3);
    window.splitAction()->trigger();
    CHECK(selectedRows(window) == std::vector<int>{3, 4});

    const SearchDialog& dialog = searching(window, "marie");
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});

    // The split is undone: back to four subtitles, and the model was reset
    // rather than told which rows changed — Qt clears the selection without a
    // `selectionChanged`.
    window.undoAction()->trigger();

    // Before the fix, this throws `std::out_of_range` out of `spansAt`.
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});
}
```

- [ ] **Step 2 — verify that it fails**

`./src/scripts/gate.sh check --only build-test` then run the window test
binary filtered on `[GUI-SEARCH-03]` (see `--list-tests` of the binary
concerned for the exact name of the CTest target). Before the fix, the test must
crash (uncaught `std::out_of_range`) rather than fail cleanly on an
assertion — this is the crash #396 describes.

- [ ] **Step 3 — fix `main_window.cpp`**

In `MainWindow::openOn`, right after the existing connect at line 592
(`historyChanged`), add:

```cpp
    // A structural undo or redo resets the model rather than reporting which
    // rows changed — Qt then clears the selection without a
    // `selectionChanged`, which is otherwise what forgets a stale target. This
    // catches that one case directly on Qt's own reset signal.
    connect(model.get(), &QAbstractItemModel::modelReset, this, [this] {
        m_searchTarget.reset();
        m_match.reset();
    });
```

(`QAbstractItemModel` is already included via `<QAbstractItemModel>` in this
file, to be verified — otherwise add the include.)

- [ ] **Step 4 — fix the comment in `main_window.hpp`**

Line 661, replace:

```cpp
    /// The match last found, which `Find Next` starts after and `Replace`
    /// rewrites. Forgotten when the pattern, an option or the document changes.
```

with:

```cpp
    /// The match last found, which `Find Next` starts after and `Replace`
    /// rewrites. Forgotten when the pattern, an option or the document
    /// changes — including a structural undo or redo, which resets the model
    /// rather than reporting the change.
```

- [ ] **Step 5 — verify that the test passes**

Rerun the same targeted test; it must pass without throwing.

- [ ] **Step 6 — commit**

```bash
git -C /home/beber/Projects/subedit add src/lib/subedit/gui/main_window.cpp \
    src/lib/subedit/gui/main_window.hpp src/test/gui/window_search_test.cpp
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
fix(gui): forget the search target on a model reset

Closes #396
EOF
)"
```

(The commit message above is illustrative — `Closes #N` goes in the
body of the pull request, not necessarily in each commit; follow the
convention already in use in the repository for this detail.)

---

## Task 2 — #404: a single text rewrite, and counts that say the same thing

This is the biggest task of the batch. It splits into five movements, in
order: factor the loop, migrate the four simple operations, migrate
the reports to `wording.hpp`, fix the phantom count of
`Replace All`, and two side cleanups that the issue mentions explicitly.

**What stays out of scope, deliberately:** `replaceAll` keeps its own
loop (it needs the ICU engine of `rewrite()` and counts
*matches*, not subtitles — a subtitle with two matches
counts for two in `noticeOfReplaceAll`, as the test
`"replace all is one entry in the history, and says how many"` already fixes:
"replaced 3 matches" for two subtitles touched). The hard-coded
`Document::Main` of `replaceAll` is not touched either: the issue
observes it but does not put it among its "what could be done", and nothing
indicates that a replacement should ever touch the translation.

### 2.1 — Factor the common loop

**Files:**
- Create: `src/lib/subedit/core/edit/rewrite_texts.hpp`
- Create: `src/lib/subedit/core/edit/rewrite_texts.cpp`
- Modify: `src/lib/subedit/core/CMakeLists.txt` (add the two files
  to the library)
- Test: `src/test/unit/core/edit/rewrite_texts_test.cpp` (new)

**Interfaces produced**, which tasks 2.2 and 2.3 consume:

```cpp
[[nodiscard]] std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform);

[[nodiscard]] std::size_t rewrittenCount(const Command& command);
```

- [ ] **Step 1 — write the test that fails**

```cpp
#include <subedit/core/edit/rewrite_texts.hpp>

#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::rewrittenCount;
using subedit::core::rewriteTexts;
using subedit::core::Selection;
using subedit::core::SubtitleIndex;

namespace {

constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                             "2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n";

} // namespace

TEST_CASE("rewriteTexts skips what the transformation leaves unchanged",
          "[core][edit]") {
    InMemoryFileSystem files;
    files.addFile("f.srt", kTwo);
    auto opened = openProject(files, "f.srt");
    REQUIRE(opened.has_value());

    const Selection all = Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(1));
    std::unique_ptr<subedit::core::Command> command =
        rewriteTexts(opened->project, all, Document::Main, CommandKind::ChangeCase,
                     [](const std::string& text) { return text; });

    CHECK(command == nullptr);
}

TEST_CASE("rewriteTexts groups every real change under the kind it is given",
          "[core][edit]") {
    InMemoryFileSystem files;
    files.addFile("f.srt", kTwo);
    auto opened = openProject(files, "f.srt");
    REQUIRE(opened.has_value());

    const Selection all = Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(1));
    std::unique_ptr<subedit::core::Command> command =
        rewriteTexts(opened->project, all, Document::Main, CommandKind::ChangeCase,
                     [](const std::string& text) {
                         std::string upper = text;
                         for (char& letter : upper)
                             letter = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));
                         return upper;
                     });

    REQUIRE(command != nullptr);
    CHECK(command->kind() == CommandKind::ChangeCase);
    CHECK(rewrittenCount(*command) == 2);
}
```

(Adapt the exact includes of `InMemoryFileSystem`/`openProject` on the
model of `italics_command_test.cpp`, which already builds a test `Project`
this way — refer to it rather than guess the signatures.)

- [ ] **Step 2 — verify that compilation fails** (the `.hpp` file
  does not exist yet)

- [ ] **Step 3 — write `rewrite_texts.hpp`**

```cpp
#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/document.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace subedit::core {

class Project;
class Selection;

/// Rewrites the text of `document` for every subtitle of `selection` through
/// `transform`, and groups what changed under `kind`.
///
/// **The one loop every text-rewriting operation shared** — italics, letter
/// case, dialogue dashes, cut. Every command is built before any is applied,
/// so that each captures the text as it stands now rather than the one its
/// predecessor left. A subtitle `transform` leaves unchanged contributes
/// neither a command nor a count.
///
/// Returns **nothing when no text changes**. An empty group would apply
/// without doing anything and still push an entry the user would meet in
/// "undo" without understanding it.
[[nodiscard]] std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform);

/// How many subtitles `command` rewrote, read from what it describes rather
/// than counted again — decision D8: the count a caller reports is the count
/// the history would show if the command were undone.
[[nodiscard]] std::size_t rewrittenCount(const Command& command);

} // namespace subedit::core
```

- [ ] **Step 4 — write `rewrite_texts.cpp`**

```cpp
#include <subedit/core/edit/rewrite_texts.hpp>

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform) {
    std::vector<std::unique_ptr<Command>> commands;

    for (const SubtitleIndex index : selection.indices()) {
        const std::string& text = project.subtitleAt(index).text(document);

        std::string written = transform(text);
        if (written == text)
            continue;

        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(written)));
    }

    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(kind, std::move(commands));
}

std::size_t rewrittenCount(const Command& command) {
    std::size_t rewritten = 0;
    for (const Change& change : command.describe())
        rewritten += change.subtitles.count();
    return rewritten;
}

} // namespace subedit::core
```

- [ ] **Step 5 — register the two files in the core's `CMakeLists.txt`**
  (next to the other `core/edit` files) and the test in that of
  `src/test`.

- [ ] **Step 6 — verify that the test passes**

- [ ] **Step 7 — commit** (see the sample message at the end of task 2).

### 2.2 — Migrate italics, case, dashes and cut to `rewriteTexts`

**Files:**
- Modify: `src/lib/subedit/core/edit/italics_command.hpp`/`.cpp`
- Modify: `src/lib/subedit/core/edit/letter_case_command.hpp`/`.cpp`
- Modify: `src/lib/subedit/core/edit/dialogue_dashes_command.cpp`
- Modify: `src/lib/subedit/core/edit/clipboard.cpp`
- Modify: `src/lib/subedit/gui/main_window.cpp` (three sites: lines 1317,
  1340, 1361)
- Existing tests to keep passing without regression:
  `src/test/unit/core/edit/italics_command_test.cpp`,
  `text_commands_test.cpp`, the dashes and cut tests, the corresponding
  window tests.

**Interfaces consumed:** `rewriteTexts(...)` and `rewrittenCount(...)` from
2.1.

- [ ] **Step 1 — adapt `italics_command_test.cpp`**

The line `CHECK(subedit::core::italicisedCount(*command) == 2);` becomes
`CHECK(subedit::core::rewrittenCount(*command) == 2);`, and the include of
`<subedit/core/edit/rewrite_texts.hpp>` is added.

- [ ] **Step 2 — verify that this test fails to compile** (the function
  does not exist yet under this name at this call, `italicisedCount` will be
  removed in the next step — so either this test breaks *after* step 3, or
  both are advanced together; in practice, advancing 1 and 3 in the same
  gesture is simpler here than forcing an artificial intermediate failure).

- [ ] **Step 3 — rewrite `italics_command.cpp`**

```cpp
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/italics.hpp>

#include <algorithm>
#include <string>

namespace subedit::core {

bool wouldItalicise(const Project& project, const Selection& selection, Document document) {
    const SubtitleFormat format = project.sourceFile().format;

    return std::ranges::any_of(selection.indices(), [&](SubtitleIndex index) {
        const std::string& text = project.subtitleAt(index).text(document);
        return !withoutItalics(text, format).empty() && !opensInItalics(text, format);
    });
}

std::unique_ptr<Command>
setItalics(const Project& project, const Selection& selection, Document document, bool italic) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project,
                        selection,
                        document,
                        italic ? CommandKind::Italicise : CommandKind::Unitalicise,
                        [italic, format](const std::string& text) {
                            return italic ? inItalics(text, format) : withoutItalics(text, format);
                        });
}

} // namespace subedit::core
```

Remove `italicisedCount` from the `.hpp` and the `.cpp`.

- [ ] **Step 4 — rewrite `letter_case_command.cpp`** in the same way:

```cpp
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>

namespace subedit::core {

std::unique_ptr<Command> setLetterCase(const Project& project,
                                       const Selection& selection,
                                       Document document,
                                       LetterCase wanted) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project, selection, document, CommandKind::ChangeCase,
                        [wanted, format](const std::string& text) {
                            return recased(text, wanted, format);
                        });
}

} // namespace subedit::core
```

Remove `recasedCount` from the `.hpp` and the `.cpp`. In `text_commands_test.cpp`,
`recasedCount(*command)` becomes `rewrittenCount(*command)`.

- [ ] **Step 5 — rewrite `dialogue_dashes_command.cpp`**:

```cpp
std::unique_ptr<Command> setDialogueDashes(const Project& project,
                                           const Selection& selection,
                                           Document document,
                                           bool dashed) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project,
                        selection,
                        document,
                        dashed ? CommandKind::AddDialogueDashes : CommandKind::RemoveDialogueDashes,
                        [dashed, format](const std::string& text) {
                            return dashed ? withDialogueDashes(text, format)
                                         : withoutDialogueDashes(text, format);
                        });
}
```

(`wouldAddDialogueDashes` does not change.)

- [ ] **Step 6 — rewrite `cutTexts` in `clipboard.cpp`**:

```cpp
std::unique_ptr<Command>
cutTexts(const Project& project, const Selection& selection, Document document) {
    return rewriteTexts(project, selection, document, CommandKind::Cut,
                        [](const std::string&) { return std::string{}; });
}
```

(Skipping texts that are already empty is already ensured by `rewriteTexts`, since
`written == text` when both are empty.)

- [ ] **Step 7 — update the three sites of `main_window.cpp`**

Line 1317: `core::italicisedCount(*command)` → `core::rewrittenCount(*command)`.
Line 1340: `core::recasedCount(*command)` → `core::rewrittenCount(*command)`.
Line 1361: `core::recasedCount(*command)` → `core::rewrittenCount(*command)`
(the dashes already borrowed the case's; they now read their
own command, which settles in passing the borrowing that #404 notes).

- [ ] **Step 8 — run the unit and window tests concerned**, make
  them all pass.

- [ ] **Step 9 — commit** (in the same commit as 2.1, or separately depending on
  what makes the review clearest — the two are a single logical
  movement and can be a single issue commit).

### 2.3 — Move the reports to `wording.hpp`

**Files:**
- Modify: `src/lib/subedit/core/wording.hpp`
- Modify: `src/lib/subedit/core/wording.cpp`
- Modify: `src/lib/subedit/gui/main_window.cpp` (same three functions)
- Modify: `CLAUDE.md` (scope of the rule on `wording.hpp`)

**Interfaces produced:**

```cpp
[[nodiscard]] std::string_view nothingToChange();
[[nodiscard]] std::string noticeOfItalics(std::size_t count, bool italic);
[[nodiscard]] std::string noticeOfRecase(std::size_t count);
[[nodiscard]] std::string noticeOfDialogueDashes(std::size_t count, bool dashed);
```

- [ ] **Step 1 — add the declarations to `wording.hpp`**, next to
  `noticeOfReplaceAll` (line 368), with English Doxygen documentation on the model
  of the neighbouring functions:

```cpp
/// What no rewrite had to do — italics, case or dialogue dashes alike. Never
/// followed by a history entry.
[[nodiscard]] std::string_view nothingToChange();

/// What toggling italics did: "N subtitle(s) put in italics" or "… taken out
/// of italics".
[[nodiscard]] std::string noticeOfItalics(std::size_t count, bool italic);

/// What re-casing did: "N subtitle(s) recased".
[[nodiscard]] std::string noticeOfRecase(std::size_t count);

/// What toggling dialogue dashes did: "N subtitle(s) dashed" or "… undashed".
[[nodiscard]] std::string noticeOfDialogueDashes(std::size_t count, bool dashed);
```

- [ ] **Step 2 — implement them in `wording.cpp`**, next to
  `noticeOfReplaceAll`:

```cpp
std::string_view nothingToChange() {
    return "nothing to change";
}

std::string noticeOfItalics(std::size_t count, bool italic) {
    return countOf(count, "subtitle") + (italic ? " put in italics" : " taken out of italics");
}

std::string noticeOfRecase(std::size_t count) {
    return countOf(count, "subtitle") + " recased";
}

std::string noticeOfDialogueDashes(std::size_t count, bool dashed) {
    return countOf(count, "subtitle") + (dashed ? " dashed" : " undashed");
}
```

- [ ] **Step 3 — update `main_window.cpp`**

```cpp
void MainWindow::toggleItalicsOnTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());
    const bool italic = core::wouldItalicise(m_session->project(), target, core::Document::Main);

    std::unique_ptr<core::Command> command =
        core::setItalics(m_session->project(), target, core::Document::Main, italic);
    if (!command) {
        m_prompts->reportOutcome(std::string{core::nothingToChange()});
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(std::move(command), target);
    m_prompts->reportOutcome(core::noticeOfItalics(rewritten, italic));
}
```

(and likewise for `changeCaseOfTarget`/`core::noticeOfRecase`,
`toggleDialogueDashesOnTarget`/`core::noticeOfDialogueDashes`). The three
literal `"nothing to change"` strings disappear in favour of
`core::nothingToChange()`. **Note:** task 3 (#398) will replace these same
`reportOutcome` calls with `statusBar()->showMessage(...)` — do not be surprised
that this code is touched a second time two tasks later; it is
the order of commits that #411 fixes.

- [ ] **Step 4 — fix the scope of the rule in `CLAUDE.md`**

The current passage:

> English also for everything the binary writes to a user — standard
> output, error output, column headers, menu labels, dialog
> messages. The two surfaces say the same words, and
> [`core/wording.hpp`](src/lib/subedit/core/wording.hpp) is the only place
> where they are written.

becomes (specify that `wording.hpp` carries what the *two* surfaces can
write, not the labels specific to one):

> English also for everything the binary writes to a user — standard
> output, error output, column headers, menu labels, dialog
> messages. **What both surfaces can say** — the name
> of a format, an account of an operation, a reason for failure — lives in
> [`core/wording.hpp`](src/lib/subedit/core/wording.hpp), the only place where
> it is written; **a menu or dialog label that belongs to the window and has no
> command-line equivalent stays in the window's file**
> — the rule is about shared words, not about the surface that shows them.

(The exact text above is a proposal; what matters is that the rule
stop asserting that `wording.hpp` is the only place where *every* menu
label is written, which #404 found false by reading — 42 `QStringLiteral`
in `main_window.cpp`.)

- [ ] **Step 5 — tests**, make pass those that today read the literal
  strings in `FakePrompts::outcomes` (they keep doing so
  until task 3, which will make them read the status bar instead); verify
  that they still compare to the same exact words (`"2 subtitles put in
  italics"`, etc. — unchanged, only the place where they are composed moves).

- [ ] **Step 6 — commit.**

### 2.4 — Fix the phantom count of `Replace All`

**Files:**
- Modify: `src/lib/subedit/core/edit/search.cpp` (function `rewrite`, line
  ~243-277)
- Test: `src/test/unit/core/edit/search_test.cpp` (or equivalent file —
  verify its exact name before writing)

**The bug.** `rewrite()` increments `rewritten.count` at each match
found, even when the replacement produces exactly the same text as the one
found (replacing "Marie" with "Marie"). `replaceAll` adds up this count
without ever checking that a text really changed, so that
`replaceAllInTarget` (`main_window.cpp:1629`) announces "replaced 1 match"
without any command entering the history.

- [ ] **Step 1 — write the test that fails**

On the model of the existing tests of `search.cpp` (verify the exact name of the
file and the includes before writing — probably
`src/test/unit/core/edit/search_test.cpp`):

```cpp
TEST_CASE("replacing a match by itself changes nothing and counts nothing",
          "[core][edit][search]") {
    InMemoryFileSystem files;
    files.addFile("f.srt", "1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie.\n\n");
    auto opened = openProject(files, "f.srt");
    REQUIRE(opened.has_value());

    const std::expected<SearchPattern, PatternError> pattern =
        SearchPattern::compile("Marie", SearchOptions{});
    REQUIRE(pattern.has_value());

    const Selection all = Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0));
    const ReplacedAll replaced = replaceAll(opened->project, all, *pattern, "Marie");

    CHECK(replaced.count == 0);
    CHECK(replaced.command == nullptr);
}
```

- [ ] **Step 2 — verify that it fails** (`replaced.count` is 1 today).

- [ ] **Step 3 — fix `rewrite()` in `search.cpp`**

```cpp
    Rewritten rewritten{.text = std::string{text}};
    MarkupParser parser{text, format};

    std::size_t at = only.has_value() ? only->start : 0;
    while (const std::optional<Found> found =
               firstFrom(parser.visible(), compiled, at, replacement)) {
        if (only.has_value() && found->span != *only)
            break;

        // A match that replaces itself by the same text is not a change: not
        // counted, and not what pushes a command past `rewrite`'s caller.
        const std::string_view matched =
            parser.visible().substr(found->span.start, found->span.end - found->span.start);
        const bool changes = found->replacement != matched;

        parser.replace(found->span.start, found->span.end - found->span.start, found->replacement);
        if (changes)
            ++rewritten.count;

        const std::size_t length = MarkupParser{found->replacement, format}.visible().size();
        rewritten.written = {.start = found->span.start, .end = found->span.start + length};
        if (only.has_value())
            break;

        at = found->span.end > found->span.start
                 ? rewritten.written.end
                 : nextCharacter(parser.visible(), rewritten.written.end);
    }

    if (rewritten.count > 0)
        rewritten.text = parser.text();
    return rewritten;
```

(Only the five lines around `++rewritten.count` change; the rest of the
function, quoted here in full for context, does not move.)

- [ ] **Step 4 — verify that the new test passes, and that no existing
  search/replace test breaks** — in particular
  `window_search_test.cpp`, whose "replace all is one entry in the history,
  and says how many" still expects "replaced 3 matches" for a real
  replacement: this test must not move, since its three matches do
  change the text.

- [ ] **Step 5 — commit.**

### 2.5 — Two side cleanups that the issue names explicitly

**a) Remove `replacingAll`, dead code.**

- [ ] Verify that no production caller exists (already confirmed by the
  search: only `markup_parser_test.cpp` calls it).
- [ ] Remove the declaration from `markup_parser.hpp` (line 104) and
  the implementation from `markup_parser.cpp` (lines 597-616).
- [ ] Remove the test case(s) that exercised it in
  `markup_parser_test.cpp`.
- [ ] Verify that compilation and the file's tests still pass.

**b) `SetDurationCommand` becomes a function built on `SetPositionCommand`.**

**Files:**
- Modify: `src/lib/subedit/core/edit/set_duration_command.hpp`/`.cpp`
- Modify: `src/lib/subedit/gui/subtitle_table_model.cpp` (only production
  caller)
- Modify: `src/test/unit/core/edit/set_duration_command_test.cpp`

- [ ] Replace the class with a free function:

```cpp
// set_duration_command.hpp
[[nodiscard]] std::unique_ptr<Command>
setDuration(const Project& project, SubtitleIndex index, Duration duration);
```

```cpp
// set_duration_command.cpp
std::unique_ptr<Command>
setDuration(const Project& project, SubtitleIndex index, Duration duration) {
    const Timestamp newEnd = project.subtitleAt(index).start + duration;
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(
        std::make_unique<SetPositionCommand>(project, index, Boundary::End, newEnd));
    return std::make_unique<CompositeCommand>(CommandKind::SetDuration, std::move(commands));
}
```

- [ ] In `subtitle_table_model.cpp`, replace
  `std::make_unique<SetDurationCommand>(project, index, duration)` with
  `setDuration(project, index, duration)` (the return type changes from
  `std::unique_ptr<SetDurationCommand>` to `std::unique_ptr<Command>` — verify
  that the caller does not depend on the concrete type).
- [ ] Adapt `set_duration_command_test.cpp` to the new signature; the
  assertions on `kind()` and `describe()` must keep passing without
  any change of expected values, since `CompositeCommand` delegates them.

- [ ] **Commit 2.5 separately or with 2.4** — they are two cleanups
  independent of each other, both attached to the same issue commit
  #404 if the review prefers a single commit for the whole of task 2.

### Commit message of task 2

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
refactor(core): a single text rewrite for italics, case, dashes and cut

Closes #404
EOF
)"
```

---

## Task 3 — #398: gestures without a dialog speak in the status bar

**Decision already settled** (comment of 2026-09-15 on #398): Italic, the
four Case, Dialogue and their "nothing to change" move to the status bar;
everything else, dialogs included and `Paste Texts` with losses, keeps a
box.

**Files:**
- Modify: `src/lib/subedit/gui/main_window.cpp` (the three functions
  touched in 2.3)
- Modify: `docs/manual/subedit-gui/operations.md` (lines 23-25)
- Modify: `src/test/gui/window_italics_test.cpp`,
  `src/test/gui/window_text_test.cpp` (or equivalent files carrying the
  case/dashes tests — confirm the exact names)

No interface changes: `MainWindow` publicly inherits from `QMainWindow`,
so `statusBar()` is already accessible, including from a test that holds a
`MainWindow&`.

- [ ] **Step 1 — adapt the tests first**

In `window_italics_test.cpp` and `window_text_test.cpp`, replace each
`prompts.outcomes.back() == "..."` concerning Italic/Case/Dialogue with
`window.statusBar()->currentMessage().toStdString() == "..."`. Add
`#include <QStatusBar>` if absent. The expected messages do not change
(same strings as `wording.hpp` produces since task 2.3).

- [ ] **Step 2 — verify that these tests fail** (`FakePrompts::outcomes`
  stays empty since the production code still calls `reportOutcome`).

- [ ] **Step 3 — define a display delay**

In the anonymous namespace of `main_window.cpp`, next to the other
constants of the file:

```cpp
// Long enough to be read without a click, short enough not to survive past
// the next gesture — Qt's own convention for a transient status.
constexpr int kOperationStatusTimeoutMs = 5000;
```

- [ ] **Step 4 — replace the three `reportOutcome` calls concerned**

```cpp
void MainWindow::toggleItalicsOnTarget() {
    ...
    if (!command) {
        statusBar()->showMessage(QString::fromStdString(std::string{core::nothingToChange()}),
                                 kOperationStatusTimeoutMs);
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(std::move(command), target);
    statusBar()->showMessage(QString::fromStdString(core::noticeOfItalics(rewritten, italic)),
                             kOperationStatusTimeoutMs);
}
```

(same gesture for `changeCaseOfTarget`/`noticeOfRecase` and
`toggleDialogueDashesOnTarget`/`noticeOfDialogueDashes`). The fifteen other
calls to `reportOutcome`/`reportFailure` in the file — `Adjust Durations…`,
`Remove Hearing-Impaired Mentions…`, `Paste Texts`, search, etc. — do not
change: they keep their dialog.

- [ ] **Step 5 — verify that the tests pass.**

- [ ] **Step 6 — update the manual**

`docs/manual/subedit-gui/operations.md`, lines 23-25, replace:

```
**What an operation did is told in an information box**, which has to be
closed — even after a `Ctrl+I`. The messages are quoted in the section of
each operation.
```

with:

```
**What an operation did is told in an information box that has to be
closed, except for `Italic`, `Case` and `Dialogue`**: these three open
no dialog, and their report — "nothing to change" included — is displayed
in the status bar, with nothing to close. The messages are quoted in the
section of each operation.
```

The message blocks quoted further down (lines 393-395, 399, 466-468, 471-472)
do not change words, only medium — nothing to fix in their
content.

- [ ] **Step 7 — commit.**

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
feat(gui): gestures without a dialog speak in the status bar

Closes #398
EOF
)"
```

---

## Task 4 — #397: commit the cell being edited before a gesture without a dialog

**What the issue asks first: replay, do not assume.** The two
scenarios are described as "plausible, not replayed" — the task therefore starts
with two tests that establish, before any fix, what really
happens.

**Files:**
- Modify: `src/lib/subedit/gui/main_window.cpp` (new private method, and
  seven call sites)
- Modify: `src/lib/subedit/gui/main_window.hpp` (private declaration)
- Test: `src/test/gui/window_italics_test.cpp` (`Ctrl+I` keyboard scenario),
  new file or dedicated section for the second scenario if a mouse
  gesture is needed to tell them apart

- [ ] **Step 1 — write the "one line" scenario**

On the model of `window_player_test.cpp:415-434`, which already opens an editor
with `window.table()->edit(index)`:

```cpp
TEST_CASE("Ctrl+I while a cell is being edited commits the edit first",
          "[gui][GUI-EDIT-01]") {
    InMemoryFileSystem files = withFile("f.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "f.srt"), prompts};
    window.show();

    const QModelIndex edited = window.table()->model()->index(0, kTextColumn);
    window.table()->edit(edited);
    REQUIRE(window.table()->isEditing());

    auto* editor = qobject_cast<QWidget*>(window.table()->indexWidget(edited));
    // The delegate's editor is a QPlainTextEdit for the text column; typed
    // characters land wherever Qt's own focus already sits — a persistent
    // editor opened by `edit()` takes it as part of opening.
    QTest::keyClicks(editor, " tout");

    window.italicAction()->trigger();

    CHECK_FALSE(window.table()->isEditing());
    CHECK(window.table()
              ->model()
              ->data(edited, Qt::DisplayRole)
              .toString()
              .toStdString()
              .find("tout") != std::string::npos);
}
```

(Adapt the exact name of the delegate/editing widget and the include of
`<QTest>`/`<QPlainTextEdit>` relying on `cell_delegates.hpp` — verify
on the spot which widget `TextDelegate::createEditor` builds before writing this
test, rather than guess.)

- [ ] **Step 2 — run this test and note what really happens**
  (this is the "reading" the issue asks for before fixing: either the typing
  is lost — bug 1 — or the italic applied is overwritten — bug 2 — or nothing
  breaks because Qt already commits by itself in this precise case). Record the
  observed result in the commit message or in the description of the
  pull request, since it is information the issue itself did not
  have.

- [ ] **Step 3 — add the common method**

In `main_window.hpp`, private section (next to the other gesture helper
methods):

```cpp
/// Commits and closes the cell editor open on the table, if any — the same
/// validation a modal dialog already gets by taking the focus away.
void commitCellEditor();
```

Dans `main_window.cpp` :

```cpp
void MainWindow::commitCellEditor() {
    // Ahead of every gesture without a dialog, so that the target it reads
    // and the command it builds see the edit already applied rather than
    // racing it — issue #397. Giving the table itself the focus is what a
    // click elsewhere already does, and the delegate's own focus-out handling
    // — Qt's, unmodified — takes it from there: it validates rather than
    // discards, the same answer a dialog already gave.
    if (m_table->isEditing())
        m_table->setFocus();
}
```

- [ ] **Step 4 — call `commitCellEditor()` at the head of the seven gestures without
  a dialog**

`toggleItalicsOnTarget()`, `changeCaseOfTarget()`, `toggleDialogueDashesOnTarget()`,
`cutTexts()`, `pasteTexts()`, `mergeSubtitles()`, `splitSubtitle()` — one line
`commitCellEditor();` as the first statement of each.

- [ ] **Step 5 — verify that the test of step 1 now passes**, and
  that no existing test of these seven gestures regresses (in particular those
  that check the selection or the focus after the gesture).

- [ ] **Step 6 — write a second test if step 2 showed a second distinct
  bug** (the issue's "several lines" scenario, where the italic applied
  would be overwritten rather than the typing lost) — otherwise, note explicitly in
  the pull request that this second scenario does not reproduce and why, rather
  than write a test that proves nothing.

- [ ] **Step 7 — commit.**

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
fix(gui): commit the cell being edited before a gesture without a dialog

Closes #397
EOF
)"
```

---

## Task 5 — #409: duration adjustment validates its speed and keeps its settings

**Decision already settled** (comment of 2026-09-15 on #409): four parts —
a `ReadingSpeed` validated at construction, the four defaults in the core,
the form state distinct from the request, and persistence in
`settings.conf` like the two search options.

### 5.1 — `ReadingSpeed` validated at construction

**Files:**
- Modify: `src/lib/subedit/core/edit/duration_adjustment.hpp`/`.cpp`
- Modify: `src/lib/subedit/gui/duration_adjust_dialog.cpp` (two
  constructions, lines 98 and 132)
- Modify: `src/test/unit/core/edit/duration_adjustment_test.cpp` (five
  constructions), `src/test/gui/window_durations_test.cpp` (one)

Follow the pattern already in use for `FrameRate`
(`src/lib/subedit/core/time/frame_rate.hpp:46-73`): private constructor,
public factory returning `std::optional`.

- [ ] **Step 1 — write the test that fails**

```cpp
TEST_CASE("a reading speed of zero or less is refused", "[core][edit]") {
    CHECK_FALSE(core::ReadingSpeed::create(0.0, true, false).has_value());
    CHECK_FALSE(core::ReadingSpeed::create(-1.0, true, false).has_value());
    CHECK(core::ReadingSpeed::create(15.0, true, false).has_value());
}
```

- [ ] **Step 2 — verify that it fails to compile** (`create` does not
  exist).

- [ ] **Step 3 — rewrite `ReadingSpeed`** in `duration_adjustment.hpp`:

```cpp
struct ReadingSpeed {
    /// Compiles `charactersPerSecond`, or refuses zero and the negatives.
    [[nodiscard]] static std::optional<ReadingSpeed>
    create(double charactersPerSecond, bool lengthen, bool shorten);

    /// Visible characters per second. Strictly positive — `create` refuses
    /// anything else.
    [[nodiscard]] double charactersPerSecond() const { return m_charactersPerSecond; }

    [[nodiscard]] bool lengthen() const { return m_lengthen; }

    [[nodiscard]] bool shorten() const { return m_shorten; }

    friend bool operator==(const ReadingSpeed&, const ReadingSpeed&) = default;

private:
    ReadingSpeed(double charactersPerSecond, bool lengthen, bool shorten)
        : m_charactersPerSecond(charactersPerSecond), m_lengthen(lengthen), m_shorten(shorten) {}

    double m_charactersPerSecond;
    bool m_lengthen;
    bool m_shorten;
};
```

**Careful**: this change turns public fields into accessors, and
therefore breaks every existing aggregate construction (`ReadingSpeed{...}`), not
only the two of the dialog. Update at once, in the same commit:
`duration_adjustment.cpp` (functions `boundsOf`, `endFor`, `countSacrifices`,
which read `speed.charactersPerSecond`, `speed.lengthen`, `speed.shorten` —
switch to the accessors), and the `.value_or(ReadingSpeed{})` of
`duration_adjust_dialog.cpp:98`, which must become
`.value_or(*ReadingSpeed::create(core::kDefaultReadingSpeed, true, false))` or
an equivalent default constant built once.

- [ ] **Step 4 — in `duration_adjustment.cpp`**, replace each access
  `speed.charactersPerSecond`/`.lengthen`/`.shorten` with
  `speed.charactersPerSecond()`/`.lengthen()`/`.shorten()`.

- [ ] **Step 5 — in `duration_adjust_dialog.cpp`**, lines 98-101:

```cpp
    const core::ReadingSpeed speed =
        initial.speed.value_or(*core::ReadingSpeed::create(core::kDefaultReadingSpeed, true, false));
    m_speed->setValue(speed.charactersPerSecond());
    m_lengthen->setChecked(initial.speed.has_value() && speed.lengthen());
    m_shorten->setChecked(initial.speed.has_value() && speed.shorten());
```

and lines 129-135 (`constraints()`), taking into account that the speed
entered in a `QDoubleSpinBox` bounded to `[1.0, 99.0]` (line 93 of the same
file) can never produce a refused `create()` in practice — the
`.value()` is therefore safe there, with a comment that says so rather than a silence
that would suggest an oversight:
```cpp
    std::optional<core::ReadingSpeed> speed;
    if (m_lengthen->isChecked() || m_shorten->isChecked()) {
        // The box is bounded to [1, 99]: `create` cannot refuse what it holds.
        speed = core::ReadingSpeed::create(
            m_speed->value(), m_lengthen->isChecked(), m_shorten->isChecked());
    }
```

- [ ] **Step 6 — adapt the six existing tests** (`duration_adjustment_test.cpp`
  lines 96, 110, 124, 140, 177; `window_durations_test.cpp` line 144) to the
  new constructor: `ReadingSpeed{...}` → `*ReadingSpeed::create(...)`.

- [ ] **Step 7 — verify that everything compiles and the tests pass.**

### 5.2 — The default maximum migrates to the core

**Files:**
- Modify: `src/lib/subedit/core/edit/duration_adjustment.hpp`
- Modify: `src/lib/subedit/gui/duration_adjust_dialog.cpp`

- [ ] Add to `duration_adjustment.hpp`, next to
  `kDefaultMinimumMilliseconds`:

```cpp
/// Gaupol's default maximum duration, in milliseconds — off by default, and
/// this is only what a switched-off field shows.
inline constexpr std::int64_t kDefaultMaximumMilliseconds = 6000;
```

- [ ] In `duration_adjust_dialog.cpp`, remove
  `constexpr double kDefaultMaximumSeconds = 6.0;` (line 32) and its use
  (line 110), replace it with
  `secondsOf(core::Duration::fromMilliseconds(core::kDefaultMaximumMilliseconds))`.
  The other three defaults (`kDefaultReadingSpeed`, `kDefaultMinimumMilliseconds`,
  the zero of `gap`) are already in the core — nothing to change there.

- [ ] Verify compilation.

### 5.3 — The form state, distinct from the request

**Files:**
- Modify: `src/lib/subedit/gui/duration_adjust_dialog.hpp`/`.cpp`
- Modify: `src/lib/subedit/gui/main_window.cpp`/`.hpp`
- Test: `src/test/gui/window_durations_test.cpp`

**The exact problem** (confirmed by reading): `main_window.hpp:652` stores a
`core::DurationConstraints` — whose unchecked optional fields are
`std::nullopt` — as the `initial` of the next opening. The value that an
unchecked box held before being unchecked is never kept anywhere
outside the Qt widget destroyed with the dialog. Decision #409: separate a
"what the form shows" type — four values, always present, and
their boxes — from `DurationConstraints`, which remains the request the core
receives.

- [ ] **Step 1 — write the test that fails**

```cpp
TEST_CASE("an unchecked minimum keeps its value across two openings of the dialog",
          "[gui][GUI-DUR-01]") {
    InMemoryFileSystem files = withFile("f.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "f.srt"), prompts};
    window.show();

    prompts.fill = [](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        adjust.minimumBox()->setValue(2.0);
        adjust.minimumCheck()->setChecked(false);
    };
    prompts.nextRun = true;
    window.adjustDurationsAction()->trigger();

    prompts.fill = [](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        CHECK(adjust.minimumBox()->value() == 2.0);
    };
    window.adjustDurationsAction()->trigger();
}
```

(Verify the exact name of the action — `adjustDurationsAction()` is a
guess to be confirmed in `main_window.hpp` before writing this test; if
the accessor does not exist yet, add it like the other public `*Action()`
of the file.)

- [ ] **Step 2 — verify that it fails** (the field shows 1.5 s, the default,
  and not 2.0 s).

- [ ] **Step 3 — create the form state type**

In `src/lib/subedit/core/config/duration_adjustment_settings.hpp` (new
file, alongside `search_options.hpp`, whose pattern it follows):

```cpp
#pragma once

#include <subedit/core/edit/duration_adjustment.hpp>

#include <cstdint>

namespace subedit::core {

/// The form of `Adjust Durations…`, as the window keeps it from one opening to
/// the next and from one session to the next — decision of issue #409.
///
/// **Every value is always here, whether its case is checked or not.**
/// `DurationConstraints` turns an unchecked case into `std::nullopt`, which is
/// right for the core and wrong for a form: unchecking a field must not erase
/// the number it held, or checking it back on asks the user for it again.
struct DurationAdjustmentSettings {
    double charactersPerSecond = kDefaultReadingSpeed;
    bool lengthen = true;
    bool shorten = false;

    bool minimumEnabled = true;
    std::int64_t minimumMilliseconds = kDefaultMinimumMilliseconds;

    bool maximumEnabled = false;
    std::int64_t maximumMilliseconds = kDefaultMaximumMilliseconds;

    bool gapEnabled = true;
    std::int64_t gapMilliseconds = 0;

    friend bool operator==(const DurationAdjustmentSettings&,
                           const DurationAdjustmentSettings&) = default;
};

/// The request the core receives, built from what the form shows — an
/// unchecked field becomes absent, never zero by accident.
[[nodiscard]] DurationConstraints constraintsOf(const DurationAdjustmentSettings& settings);

} // namespace subedit::core
```

and its `.cpp`:

```cpp
#include <subedit/core/config/duration_adjustment_settings.hpp>

namespace subedit::core {

DurationConstraints constraintsOf(const DurationAdjustmentSettings& settings) {
    std::optional<ReadingSpeed> speed;
    if (settings.lengthen || settings.shorten) {
        // A settings file edited by hand could carry zero: the core has
        // nothing useful to do with it, so it is read as the speed being off
        // rather than refused outright — the tolerance ADR 0022 already
        // chooses for every other option.
        speed = ReadingSpeed::create(settings.charactersPerSecond, settings.lengthen, settings.shorten);
    }

    return DurationConstraints{
        .speed = speed,
        .minimum = settings.minimumEnabled
                      ? std::optional{Duration::fromMilliseconds(settings.minimumMilliseconds)}
                      : std::nullopt,
        .maximum = settings.maximumEnabled
                      ? std::optional{Duration::fromMilliseconds(settings.maximumMilliseconds)}
                      : std::nullopt,
        .gap = settings.gapEnabled ? std::optional{Duration::fromMilliseconds(settings.gapMilliseconds)}
                                   : std::nullopt};
}

} // namespace subedit::core
```

- [ ] **Step 4 — the dialog takes `DurationAdjustmentSettings` as input and
  output**

`DurationAdjustDialog`: the constructor receives
`const core::DurationAdjustmentSettings& initial` instead of
`const core::DurationConstraints&`. A new public method:

```cpp
[[nodiscard]] core::DurationAdjustmentSettings settings() const;
```

replaces `constraints()` as what the window reads after `exec()`;
`isComplete()` calls `core::constraintsOf(settings()).isAny()`. The initial
filling of the fields (current lines 98-112) reads directly
from `initial.*` rather than from `value_or`s — no more need for `value_or`
since `DurationAdjustmentSettings` has no optional field.

- [ ] **Step 5 — `MainWindow` keeps the state rather than the request**

`main_window.hpp:652`: `core::DurationConstraints m_durationConstraints;`
becomes `core::DurationAdjustmentSettings m_durationSettings;`.
`main_window.cpp:1255-1272` (`adjustDurationsOfTarget`):

```cpp
void MainWindow::adjustDurationsOfTarget() {
    commitCellEditor();
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    DurationAdjustDialog dialog{target.count(), m_durationSettings, this};
    if (!m_prompts->run(dialog))
        return;

    m_durationSettings = dialog.settings();

    core::DurationAdjustment adjustment = core::adjustDurations(
        m_session->project(), target, core::constraintsOf(m_durationSettings));
    if (adjustment.command != nullptr)
        applyOperation(std::move(adjustment.command), target);

    m_prompts->reportOutcome(core::noticeOfAdjustment(adjustment.adjusted, adjustment.sacrificed));
}
```

(`commitCellEditor()` appears here only if task 4 has already been integrated —
`Adjust Durations…` has a dialog, so #397 did not strictly
require it; adding it is harmless and consistent, but is not required by
#397. Leave it out if one prefers not to mix the two tasks.)

- [ ] **Step 6 — verify that the test of step 1 passes**, and that the existing
  tests of `window_durations_test.cpp` still pass (adapt any
  constructions of `DurationConstraints` into
  `DurationAdjustmentSettings` where the test built the initial state).

### 5.4 — Persistence in `settings.conf`

**Files:**
- Modify: `src/lib/subedit/core/config/settings.hpp` (add the field)
- Modify: `src/lib/subedit/core/config/settings.cpp` (keys, reading,
  writing)
- Modify: `src/lib/subedit/gui/main_window.cpp` (`applySettings`,
  `settings()`)
- Modify: `docs/manual/subedit-gui/operations.md` (lines 222-223) and
  `docs/manual/subedit-gui/preferences.md` (lines 170, 177-182)
- Test: `src/test/unit/core/config/settings_test.cpp` (confirm the exact
  name)

Follow exactly the pattern of `SearchOptions` (`settings.hpp:119`,
`settings.cpp:34-35,256-259,384-385`).

- [ ] **Step 1 — write the test that fails**, on the model of the existing
  tests of `search.regex`/`search.ignore-case` in
  `settings_test.cpp` — a write/read round trip that checks that
  `durationAdjustment` survives, and that a file that does not mention it
  falls back on the defaults.

- [ ] **Step 2 — add the field to `Settings`**

```cpp
/// The form of `Adjust Durations…`, kept from one opening of the dialog to
/// the next and from one session to the next — issue #409, the same
/// treatment as `search` just above.
DurationAdjustmentSettings durationAdjustment{};
```

(with the include of `<subedit/core/config/duration_adjustment_settings.hpp>`
in `settings.hpp`.)

- [ ] **Step 3 — the keys**, next to `kSearchIgnoreCaseKey`:

```cpp
constexpr std::string_view kDurationSpeedKey = "duration-adjust.speed";
constexpr std::string_view kDurationLengthenKey = "duration-adjust.lengthen";
constexpr std::string_view kDurationShortenKey = "duration-adjust.shorten";
constexpr std::string_view kDurationMinimumEnabledKey = "duration-adjust.minimum-enabled";
constexpr std::string_view kDurationMinimumKey = "duration-adjust.minimum-ms";
constexpr std::string_view kDurationMaximumEnabledKey = "duration-adjust.maximum-enabled";
constexpr std::string_view kDurationMaximumKey = "duration-adjust.maximum-ms";
constexpr std::string_view kDurationGapEnabledKey = "duration-adjust.gap-enabled";
constexpr std::string_view kDurationGapKey = "duration-adjust.gap-ms";
```

**A decimal number must be read differently from an integer**: `speed` is a
`double`, whereas `integerOf`/`booleanOf` cover the rest. Add a
small function next to `integerOf`:

```cpp
[[nodiscard]] std::optional<double> decimalOf(std::string_view text) {
    double value = 0.0;
    const char* const first = std::to_address(text.begin());
    const char* const last = std::to_address(text.end());
    const std::from_chars_result read = std::from_chars(first, last, value);
    if (read.ec != std::errc{} || read.ptr != last)
        return std::nullopt;
    return value;
}
```

- [ ] **Step 4 — `applyOption`**

The function's comment already says that the seventh option made it
cross the complexity threshold the gate tolerates — adding nine more
in a row would almost certainly cross it again. **Before adding the
branches, run `clang-tidy` on `settings.cpp` alone** to measure the effect:

```console
$ ./src/scripts/gate.sh check --only tidy
```

If the threshold is exceeded after the addition, do not pile up additional
`else if`s: group the nine keys of the duration adjustment in a
dedicated function `applyDurationAdjustmentOption(read, key, value)` called
from `applyOption` by a single `else if (key.starts_with("duration-adjust."))`
— a decomposition rather than inflating the existing function, in the
same spirit as what was already done to make it fit under the threshold at the
seventh setting.

```cpp
void applyDurationAdjustmentOption(SettingsRead& read, std::string_view key, std::string_view value) {
    const auto take = [&read, key, value](auto parsed, auto& field) {
        if (!parsed.has_value()) {
            read.diagnostics.push_back({.key = std::string{key}, .value = std::string{value}});
            return;
        }
        field = *std::move(parsed);
    };

    DurationAdjustmentSettings& duration = read.settings.durationAdjustment;
    if (key == kDurationSpeedKey)
        take(decimalOf(value), duration.charactersPerSecond);
    else if (key == kDurationLengthenKey)
        take(booleanOf(value), duration.lengthen);
    else if (key == kDurationShortenKey)
        take(booleanOf(value), duration.shorten);
    else if (key == kDurationMinimumEnabledKey)
        take(booleanOf(value), duration.minimumEnabled);
    else if (key == kDurationMinimumKey)
        take(integerOf(value), duration.minimumMilliseconds);
    else if (key == kDurationMaximumEnabledKey)
        take(booleanOf(value), duration.maximumEnabled);
    else if (key == kDurationMaximumKey)
        take(integerOf(value), duration.maximumMilliseconds);
    else if (key == kDurationGapEnabledKey)
        take(booleanOf(value), duration.gapEnabled);
    else if (key == kDurationGapKey)
        take(integerOf(value), duration.gapMilliseconds);
}
```

(`integerOf` returns an `int`; `minimumMilliseconds` etc. are
`std::int64_t` — verify the conversion, or change `integerOf` into a
template covering both sizes if the gate requires it. A detail to settle at
implementation rather than here: both options are reasonable, and the
choice depends on what `clang-tidy`/`cppcoreguidelines` accept.)

Then, in `applyOption`:

```cpp
    else if (key.starts_with("duration-adjust."))
        applyDurationAdjustmentOption(read, key, value);
```

- [ ] **Step 5 — `renderSettings`**, next to the `search` block:

```cpp
    const DurationAdjustmentSettings duration = settings.durationAdjustment;
    const DurationAdjustmentSettings durationDefaults;
    writeOption(out, kDurationSpeedKey, std::to_string(duration.charactersPerSecond),
                duration.charactersPerSecond == durationDefaults.charactersPerSecond);
    writeOption(out, kDurationLengthenKey, duration.lengthen ? "true" : "false",
                duration.lengthen == durationDefaults.lengthen);
    writeOption(out, kDurationShortenKey, duration.shorten ? "true" : "false",
                duration.shorten == durationDefaults.shorten);
    writeOption(out, kDurationMinimumEnabledKey, duration.minimumEnabled ? "true" : "false",
                duration.minimumEnabled == durationDefaults.minimumEnabled);
    writeOption(out, kDurationMinimumKey, std::to_string(duration.minimumMilliseconds),
                duration.minimumMilliseconds == durationDefaults.minimumMilliseconds);
    writeOption(out, kDurationMaximumEnabledKey, duration.maximumEnabled ? "true" : "false",
                duration.maximumEnabled == durationDefaults.maximumEnabled);
    writeOption(out, kDurationMaximumKey, std::to_string(duration.maximumMilliseconds),
                duration.maximumMilliseconds == durationDefaults.maximumMilliseconds);
    writeOption(out, kDurationGapEnabledKey, duration.gapEnabled ? "true" : "false",
                duration.gapEnabled == durationDefaults.gapEnabled);
    writeOption(out, kDurationGapKey, std::to_string(duration.gapMilliseconds),
                duration.gapMilliseconds == durationDefaults.gapMilliseconds);
```

- [ ] **Step 6 — `main_window.cpp`**, in `applySettings` (next to
  line 1830):

```cpp
    m_durationSettings = settings.durationAdjustment;
```

and in `settings()` (next to line 1866):

```cpp
    settings.durationAdjustment = m_durationSettings;
```

- [ ] **Step 7 — verify that the test of step 1 passes, and that
  `make check` stays green on `settings.cpp`** (complexity included).

- [ ] **Step 8 — update the manual**

`docs/manual/subedit-gui/operations.md`, lines 222-223, replace:

```
The dialog reopens on the values of the last adjustment, for as long as the window
stays open; it does not keep them from one session to the next.
```

with:

```
The dialog reopens on the values of the last adjustment, **and keeps them from one
session to the next** — like the two search options.
```

`docs/manual/subedit-gui/preferences.md`:

- line 170, "Five settings" becomes "Six settings", and the list gains
  the duration adjustment:

```
**Six retained settings are chosen in a dialog**, each in the one where
it is used: the theme, here; the side of an insertion, in the insertion
dialog; the two search options, in the search
dialog; the encoding and the byte order mark, in `Save As…`; the
reading speed, the durations and the gap of the duration adjustment, in its
own dialog.
```

- lines 177-182, the sentence about `Adjust Durations…` as the only exception
  disappears since it is no longer one:

```
And **no number that belongs to a document is kept from one session to
the next**: an offset duration, two transformation reference points are true
of one file and false of the next. The dialogs ask for them again at each
launch, deliberately. The settings of `Adjust Durations…` are an exception on
the same footing as the two search options: a reading speed, a minimum
duration or a gap are choices of method rather than measurements
of a file, and persist from one session to the next for that reason.
```

- [ ] **Step 9 — commit.**

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
feat(core): validate the reading speed and keep the adjustment settings

Closes #409
EOF
)"
```

---

## End of batch — what remains owed, once the five commits are in place

Follow the order already described in `CLAUDE.md`:

- [ ] reread the whole diff of the five tasks;
- [ ] bump the patch in `CMakeLists.txt` (only once, at the end);
- [ ] `make manual`;
- [ ] `make check-local`, record the bench;
- [ ] `make check`;
- [ ] verify that all the documentation touched is up to date (manual,
  `CLAUDE.md`) — done along the way in the tasks above, to be reread one last time;
- [ ] regenerate the `CHANGELOG`;
- [ ] open the pull request with the five `Closes #N` lines (#396, #404,
  #398, #397, #409) in its body, title at the dominant scope (probably
  `feat(gui):` or `fix(gui):` depending on what dominates — five issues touching
  both `core` and `gui`, to be settled when writing the title) and under the limit of
  72 characters, verified by
  `./src/scripts/check-commit-message.sh "<title>"` before opening it.
