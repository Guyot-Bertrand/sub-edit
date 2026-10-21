# Plan — issue #509, `Tools ▸ Check Spelling…` (D6)

Branch `feat/509`. Spec: `docs/specs/12-correction.md` (D6, GUI-SPELL-01/02/03). Model to read: Gaupol's
`SpellCheckNavigator` and `SpellCheckDialog`
(`reference/gaupol/aeidon/spell.py`, `reference/gaupol/gaupol/dialogs/spell_check.py`, READ-ONLY;
never `cd` into `reference/`).

## Design decisions

1. **Core (`core/text/`)**:
   - `SpellCheckNavigator`: faithful port of Gaupol's class, positions in **UTF-8 bytes** (the
     `tokenizeForSpelling` splitter returns bytes). It owns a `SpellChecker`.
   - `SpellCheckWalk`: the walk over several targets (`CorrectionTarget`, already there) — subtitle after
     subtitle, project after project — on top of the navigator. **No Qt.** It produces
     `ProposedCorrection`s (existing type): the application goes through `core::applyCorrections`, hence one
     history entry per project, with no new command.
2. **Closing in the middle** applies what has been *done*: the texts already left **and** the gestures (replace,
   join…) already made on the current text; not the edits typed into the text area and not
   validated ("Save and resume"), nor the rest of the walk. **A deviation from Gaupol, to be stated**: it
   loses the current text.
3. **Two windows, like Gaupol**: `Tools ▸ Check Spelling…` (the walk) and
   `Tools ▸ Spell-Check Settings…` (language, target, document — Gaupol's `LanguageDialog`). The latter
   is **always enabled**: it is what gets out of a language without a dictionary, without which the greyed-out
   entry would be a dead end.
4. **Without a dictionary for the configured language**, `Check Spelling…` is **greyed out** and its tooltip / its
   status line says `core::noDictionaryFor(language)` ("no dictionary for fr") — GUI-SPELL-02, already
   cited at #508, to be cited again here. The empty configured language = the system's, resolved by
   `spellLanguageFor` (already there, `join_split_page.hpp`).
5. **Settings**: three keys `spell-check.language`, `spell-check.target`, `spell-check.document`
   (Gaupol: `language`, `target`, `field`), added to `Settings` on the model of `correction.*`;
   unknown value → default + diagnostic. The window size is not retained (out of scope).
6. **The replacement list** is written on closing (`saveSpellReplacements`), under
   `spellConfigDirectory()` received from the window — never resolved by the controller (ADR 0022).
7. **The walk places the view**: each unknown word brings back the project's tab and selects the row
   (`View::reveal(project, index)`), as `preview` does for the film.
8. Tests **on the double** (`WordListSpellProvider`), never on the machine's dictionaries.

## Tasks (sequential)

- **T1** `SpellCheckNavigator` + unit tests (GUI-SPELL-01 on the core side).
- **T2** `SpellCheckWalk` + tests (multi-target, an unchanged text produces nothing, closing in the middle).
- **T3** `spell-check.*` settings (Settings, read/write, diagnostics, tests, `docs/` of the keys).
- **T4** `SpellCheckSettingsDialog` + `SpellCheckDialog` (widgets) + widget tests.
- **T5** `SpellCheckController`, the two actions in `window_actions`, `MainWindow` wiring
  (`CorrectionSide` or a dedicated side), GUI-SPELL-02 greying-out, controller tests, one history entry
  per project, replacement list written.
- **T6** Spec (D6 "Specified by #509", GUI-SPELL-01/02 entered and cited), manual + screenshot
  (`src/test/tools/screenshots.cpp`), then bump / gate by the coordinator.

## Instructions to each implementer

- C++23; **C++ comments in English**, everything else in French; identifiers in English; displayed
  labels in English (shared words → `core/wording.hpp`).
- Commits: Conventional Commits in French, `feat(core): …` / `feat(gui): …`, ending with
  `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`. **Never `amend`.** Every git command:
  `git -C /home/beber/Projects/subedit …`. Never `cd` into `reference/`.
- Build: `cmake --preset dev`; binaries `build/dev/bin/subedit_core_test`, `subedit_gui_test`.
  Verify in targeted steps (`./src/scripts/gate.sh <target> --only <step>`), **no full `make check`**
  (the coordinator runs it once).
- `moc` does not always parse `<concepts>`/`<compare>`: forward declaration in the
  `Q_OBJECT` headers, real inclusion in the `.cpp`.
- TDD: a failing test, then the code.
- Never name a file of `src/data/`; fixtures live in `src/test/data/`.
- No version bump, no CHANGELOG, no manual before T6.
