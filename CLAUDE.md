# subedit project instructions

## CRITICAL RULE — the reference repository is read-only

`reference/gaupol` is a clone of [Gaupol](https://github.com/otsaloma/gaupol),
present only to be **read**. It is not tracked by git and is not part of the
project.

**No modification, ever.** The following are forbidden, with no exception and
no apparent exception:

- writing, editing, creating or deleting a file under `reference/`;
- any git command that writes to that repository — `add`, `commit`, `checkout`,
  `reset`, `clean`, `stash`, `rm`, `restore`, `branch`, `merge`, `rebase`;
- running a build, a formatter or any tool that produces artefacts in it.

**The only operations allowed:** reading the files, and
`git -C reference/gaupol fetch` followed by `git -C reference/gaupol log`/`diff`
to look at upstream changes. Even updating the working copy (`pull`, `merge`) is
discussed before it is done.

**Reading is for the human, never for the code.** Nothing under `src/lib`,
`src/exe` or `src/test` refers to `reference/`: the clone is absent from CI,
absent from a freshly cloned machine, absent from an archive of the repository.
A test that reads it passes for whoever has it and fails for everyone else — or
reports itself skipped, which is worse, since it stays green while proving
nothing. Versioned test data lives in `src/test/data/`; `src/data/` is not an
option, it is each machine's private corpus and it is ignored by git.
`check-architecture.sh` checks it.

**The content of `src/data/` is never cited in a versioned file.** Neither the
code, nor the documentation, nor a spec, nor an ADR, nor a commit message, nor
the body of an issue or a pull request names a file of the private corpus —
neither its title nor its file name, neither in a table of measurements nor in a
sentence. This holds for `src/data/exemples/` as for the rest.

The reason is twofold. These are **files that do not belong to the project**,
and the repository has no business publishing their inventory. And a figure attributed to a file that
nobody else has is **evidence that nobody can replay**: the reader believes they
are reading a measurement where they only have a testimony.

What remains allowed, and is enough: speaking of the corpus **as a whole** —
“eight of the fifteen files”, “example sets 004 to 006”, “the private corpus
does not settle it”. An observation phrased that way owns up to being one. And
when a figure must become a guarantee rather than an observation, it needs a
fixture in `src/test/data/`, versioned, that everyone can read.

A mistake already made, in August 2026, in the roadmap and a phase spec: five
film titles cited in a table of measurements. Corrected afterwards.

**Practical trap, already hit:** the shell's working directory persists from one
call to the next. A `cd reference/gaupol` done earlier in the session makes any
later command dangerous — `git add -A` then targets the clone, not the project.

> **Consequence: every git command of this project is written with an explicit
> path,** `git -C /home/beber/Projects/subedit …`, or is preceded by a check of
> `pwd`. Never rely on the assumed current directory.

### Mechanical lock

The clone's tree is kept **non-writable**, which makes any write fail at the
file-system level rather than at the level of vigilance.
`src/scripts/reference.sh` drives this lock:

| Command | Effect |
| :------ | :----- |
| `./src/scripts/reference.sh status` | state of the lock, revision, any deviations |
| `./src/scripts/reference.sh fetch` | fetches upstream, **restores the lock automatically** |
| `./src/scripts/reference.sh restore` | restores the clone to the state of its HEAD |
| `./src/scripts/reference.sh lock` / `unlock` | locks, unlocks |

`unlock` is only used deliberately and is closed again immediately after. It is
a guard against mistakes, not a security barrier: the owner can undo a `chmod`.
The point is precisely that an accidental modification is impossible without an
explicit gesture having preceded it.

If the clone turns out to be modified, report it and propose the restoration —
do not do it silently.

## The project

`subedit` rewrites Gaupol — a GTK/Python subtitle editor — in C++23 + Qt 6,
with a goal of feature parity.

| Document | Content |
| :------- | :------ |
| [`docs/feuille-de-route.md`](docs/feuille-de-route.md) | the outline of the MVP, the phases, their scoping and their order |
| [`docs/principes-de-conception.md`](docs/principes-de-conception.md) | permanent rules applicable to all the code |
| [`docs/specs/`](docs/specs/) | one spec per phase, plus the inventory of Gaupol |
| [`docs/configuration-github.md`](docs/configuration-github.md) | locking of the repository, labels, milestones |

## Conventions

- **Language** — **everything is written in English**: C++ (identifiers and
  comments), scripts, build files, documentation, specs, ADRs, issue and pull
  request titles and bodies, commit messages, test titles, and everything the
  binary prints to a user. **French exists as a locale only**: the catalogue
  `src/po/fr.po` and the translated manual `docs/i18n/fr/manual/`. Decided in
  [ADR 0043](docs/adr/0043-english-is-the-language-of-the-repository.md), which
  replaces the split of #312.

  **The repository is mixed until the translation is done.** What exists in
  French is translated in place, in batches, behind a descending allowlist:
  `.french-allowlist` names every tracked file that still contains French, and
  `src/scripts/check-comment-language.py`, run by the `arch` step of the gate,
  refuses French in any other file outside `src/po/` and `docs/i18n/fr/` —
  code, scripts, build files, docs, YAML alike (#660). An entry can be removed
  and never added: the check compares the list with its version at the merge
  base with `origin/main`, and refuses a listed file that no longer contains
  French, so translating a file means deleting its line in the same diff. The
  script's header says what it reads and what it leaves out.
  Meanwhile, **anything new is written in English, in a French file as
  anywhere else**, and a French file that is touched in depth is translated in
  the same diff.

  **What both surfaces can say** — a format name, an account of an operation, a
  reason for failure — lives in
  [`core/wording/`](src/lib/subedit/core/wording/), the only place it is
  written: one header per family (`formats`, `settings`, `analysis`, `counts`,
  `translation`, `conversion`, `editing`, `video`), included for what is said
  and not as a whole, so that changing a family recompiles only its readers
  (#549). **A menu or dialog label that belongs to the window and has no
  command-line equivalent stays in the window's file**: the rule is about shared
  words, not about the surface that shows them.

  **A string the user reads goes through the translation API** — `translate`,
  `translateIn`, `translatePlural` (`core/i18n/messages.hpp`) or Qt's `tr`. The
  `untranslated` step of `check` (`check-translatable-strings.py`, #661) counts,
  file by file, the literals that do not, against `.untranslated-ratchet`, which
  only descends and reaches zero before phase 15 closes. A literal that is not
  for a reader — a key, an option name, a format's syntax — goes in
  `.translation-exemptions`, with its reason. A conversion lowers the ratchet
  with `make untranslated-record`.

  **An installed language stays complete.** The `translations` step of
  `check-local` (#670) merges the template into a copy of every catalogue, as
  `make po` would, and refuses an untranslated, fuzzy or stale entry in a
  language of `src/po/LINGUAS`; a seed is only reported. A changed English
  message therefore means `make po` and its French translation in the same diff.
- **Commits** — Conventional Commits, scopes aligned with the `area:` labels.
- **Pull request title** — **the same grammar, the same limit of 72
  characters.** `.github/workflows/pull-request.yml` passes the title to
  `check-commit-message.sh`, exactly as the `commit-msg` hook passes it a
  message. “Phase 5 review” is therefore not a valid title;
  “docs(doc): review the end of phase 5” is.

  It is checked before opening, in one second:

  ```console
  $ ./src/scripts/check-commit-message.sh "docs(doc): review the end of phase 5"
  ```

  This line was missing, and the silence had a cost: three pull requests in a
  row opened with a non-conforming title in August 2026, three times the same
  red step, and the rule discovered by reading the YAML rather than here. It is
  the same defect as the interface language, identically — a real constraint,
  held by a check, and absent from the only document one reads before acting.
- **Closing an issue** — the body of the PR carries a `Closes #N` line,
  **in English**. It is not prose but an instruction to GitHub, which recognises
  only its own keywords: “Ferme #26” (French for “Closes #26”) is not one, and
  the issue stays open after the merge. The language rule does not apply to it,
  just as it does not apply to the `Co-Authored-By` of a commit. Write a prose
  sentence **as well** if it adds something; the `Closes #N` line is not
  negotiable.
- **Versions** — the `project(VERSION)` of `CMakeLists.txt` is the source of the
  current number; it moves on two occasions:
  - **patch, on every PR** — every PR increments the patch in its own
    diff (`0.1.0` → `0.1.1` → `0.1.2`…);
  - **minor, on every finished milestone** — the patch is reset to zero and the
    minor is bumped: `v0.1.0` closes phase 1, `v0.2.0` will close phase 2, and
    so on up to a `v1.0.0` at the end.

  **Every version gets its tag, patches included** — since #232, and
  `v0.10.9` is the first tagged patch. The `vX.Y.Z` tag is placed **on the merge
  commit in `main`**, once the PR of the bump is merged: it is the commit whose
  `CMakeLists.txt` carries the number. Earlier versions are not tagged after the
  fact.

  **Pushing the tag publishes the version.** `.github/workflows/release.yml`
  builds the `.deb` and the `.rpm` and uploads them to a GitHub release. It
  keeps only the patch releases of the current milestone: when `v0.11.0` is
  published, the releases `0.10.1` to `0.10.N` go away with their packages.
  The `vX.Y.0` all stay, and **no tag is ever deleted**.

  **The bump is done at the last moment** — never at the start of the work. The
  number is only known at that moment: between the first commit and the PR,
  other PRs may have been merged and taken the intended number. Bumping early
  guarantees a conflict on `CMakeLists.txt` and one more rebase.

  “At the last moment” means **just before the last gate, not after**.
  The bump touches `CMakeLists.txt`, which the gate reads — doing it after the
  gate would force running it again, ten minutes for a digit. The order is
  therefore:

  1. code and test;
  2. review, fix;
  3. **bump the patch**;
  4. **`make manual`** — the bump has just made the `--version` example stale,
     which quotes the number. It takes ten seconds, and forgetting it makes the
     next step fail: `check-local` chains `manual-check`, which compares that
     block with what the binary writes. The defect was paid for three times
     before being written down here;
  5. **`make check-local`**, whose chained `make bench` runs on the bumped
     version: **it is that run that records the measurement which stays in the
     log** — unless the machine is busy, in which case nothing is recorded and
     the PR must say so (issue #270);
  6. `make check` — only once, it sees the code *and* the bump;
  7. **update the rest of the documentation and the record of measurements** —
     notes, manual sections the ticket changes, figures recorded at
     step 5. None of that is read by the gate, so none of it reopens it;
  8. commit, regenerate the CHANGELOG, open the PR.

  **`check-local` before `check`, and there is a measurement behind that order.**
  The reverse order held until the end of phase 8, and it cost almost one bench
  record in two: `check-local` chains `make bench` last, so the bench found the
  tail of a quarter of an hour of compilation — six records lost out of thirteen
  in phase 7, three out of six in phase 8. It is also the order the principle
  already wanted, from the cheapest to the most expensive: `check` takes
  seventeen minutes, `check-local` a few.

  **That swap was not enough, and its first run said so** — the bench read
  2.21 on a machine that started at 0.63. The guard compiled the Release tree
  *then* asked whether the machine was free, that is, just after having occupied
  it itself; the one-minute average is the past. The reading now comes before
  the build.
  [ADR 0015](docs/adr/0015-memoire-des-mesures.md).

  **There is therefore no longer a diagnostic run of `make bench` before the
  gate**: the record that stays in the log now precedes it, and a regression
  shows there at the same moment, while there is still time to fix it.

  Step 7 comes after step 6 by construction: it is the only position where the
  documentation costs nothing. The version example, for its part, cannot wait
  until then — it is read by `check-local`, hence step 4, reserved for it.

  The tag and the `project(VERSION)` must carry the same number: **bump the
  CMake before tagging**, otherwise the binary announces a stale version.
  `src/scripts/check-architecture.sh` checks it as soon as a tag points at HEAD.
- **Quality** — `make check` is the gate: format, warnings as errors,
  clang-tidy, tests under ASan, coverage threshold, and **no file left
  behind**. Never announce a piece of work as finished without having run it.

  **CI does not run it, and that has been the rule since #232.** The Actions
  minutes and the cache storage went too fast for a gate per pull
  request: `ci.yml` and `pull-request.yml` only run on demand, and
  no ruleset requires their check any more. The sentence above becomes its only
  guard, and the pull request says what was passed. `Closes #N`, the patch bump
  and the regenerated log are gestures to make, not to watch fail.

  The last check is the most recent and the least obvious: the gate records the
  untracked files before starting and refuses those that appeared in the
  meantime. It is the end-to-end tests that write, and a bare name passed to
  `--output` lands where CTest runs them, that is, at the root of the
  repository. **A test that writes goes through the `Scratch` harness, never
  through a bare name.** An untracked file already present before the gate — a
  new source before its `git add` — does not disturb it; it is the appearance
  that is refused, not the presence.

  **The same check exists outside the repository, and it has its own rule.**
  `check-config-home.sh` wraps the gate the same way and watches
  `~/.config/subedit`, which `git ls-files` does not see. What keeps it green:
  **a test never resolves a configuration location, it is given one.** The only code that resolves one is `platform::resolvedUserSettingsPath()`,
  called by the two `main` functions, which hand the path down;
  `gui::userSettingsPath()` delegates to it — ADR 0022 and 0045. The three harnesses that launch something
  also move `XDG_CONFIG_HOME` to a directory of their own: the interface test
  binary for itself, the end-to-end one for each binary it launches, and
  `check-installation.sh` for what it has just installed.

  **Static analysis no longer has a scope to compute, and no longer a switch.**
  clang-tidy is hooked onto the compile rule of each source, under the
  `tidy` preset — `cmake/Tidy.cmake`, issue #269. What decides to re-analyse a
  file is the build system: its source, a header of its dependency file, or its
  command line. There is therefore no more `TIDY_BASE`, no more “governing
  files”, and no more table of what the gate reads — **a change that alters no
  compilation input costs nothing, mechanically.** To re-analyse everything:
  `rm -rf build/tidy`.

  The price, written rather than kept quiet: the incrementality lives in
  `build/tidy`, so a `make clean` loses it, and CI keeps it through an Actions
  cache.

  **Two inputs escape Ninja, and a key shows them to it**: the content of
  `.clang-tidy` and the version of the binary, neither of which is on the
  command line. `Tidy.cmake` puts their fingerprint in the compile flags, which
  puts them there. Without it, a hardened configuration would leave the tree
  green on a stale answer.

  **`HeaderFilterRegex` had never filtered**, and nobody could see it: the
  pattern `src/.*\.hpp$` matches nothing from the root of the repository, so
  that the gate analysed no header until #269. The preset settled that by
  construction, its current directory being the build tree. A corollary worth
  knowing: **a directory's `.clang-tidy` governs only the `.cpp` files of that
  directory** — for a diagnostic raised in a header, it is the configuration of
  the translation unit that includes it that applies.

  **The gate can be resumed in the middle.** The steps live in
  `src/scripts/gate/`, one per file, and `src/scripts/gate.sh` carries the order:

  ```console
  $ ./src/scripts/gate.sh --list
  $ ./src/scripts/gate.sh check --from coverage
  $ ./src/scripts/gate.sh check --only tidy
  ```

  The `make` targets remain the interface one types, and `verify-gates.sh` still
  invokes them. An unknown step name makes the invocation fail rather than play
  none of them.

  Each step, phase and proof is timed with the load and `JOBS` it ran under,
  and appended to `build/gate-timings.tsv` (local, ignored); `make timings`
  compares the latest run with the median of the previous ones and flags one
  above 1.5×.

  **The version bump, for its part, only counts if a tag points at HEAD.** It is
  the only check that reads the number: `check-architecture.sh` confronts it with
  the tag, and it is inert without a tag. Nothing else can differ from it —
  `version_test` and the end-to-end test of `--version` both derive the number
  from `versionString()` rather than hard-coding it, and the only place where it
  is copied, the `--version` example of the manual, is held by `manual-check`,
  which is in `check-local`.

  So: **a PR that touches only documentation and the patch makes do with
  `make check-local`.** That is the case of every scoping, manual or ADR PR. As
  soon as a `.cpp`, a `.hpp`, a script or a piece of test data moves, the gate
  takes back its rights.
- **User manual** — [`docs/manual/`](docs/manual/). **Every ticket that changes
  what the user sees ends with its update**, once the code is written, reviewed
  and validated — not before, otherwise the manual describes an intention and
  not a piece of software. Concretely, at step 7 of the order described above:
  after the gate, which does not read it. What must appear in it, exhaustively:

  | Element | What is said about it |
  | :------ | :-------------------- |
  | command, subcommand | its name, what it does, and what it does not do |
  | argument, option | its long and short form, whether it is required, its default value |
  | accepted values | the closed set, enumerated; the bounds, for a range |
  | output | what is written, where, and in what form |
  | exit codes | each one, with its meaning |
  | errors | what triggers them and the corresponding message |

  **The window's shortcuts are written in a table with a “Shortcut” column**, the shortcut
  between backticks (`Ctrl+P`), “none” when there is none, and the action named as the
  menu labels it — `check-gui-manual.py` (in `manual-check`) confronts them with the window's
  actions: #612. A key cited in prose is not checked, and must not be that of an action.

  **Each manual tree is checked against the window in its own language** (#672):
  English against the English window, `docs/i18n/<lang>/manual/` against the
  window under `<lang>` (`subedit_list_shortcuts --language`). The label columns
  of the shortcut tables are plain text, written as that window labels them
  (the French pages use `src/po/fr.po`); shortcuts are never translated. The
  screenshots carry translated text, so **each language has its own set**,
  `docs/i18n/<lang>/manual/subedit-gui/captures/`, generated by `make manual`
  for English and every language of `src/po/LINGUAS`.

  An example of a real call accompanies each command. **The manual describes what
  exists, never what is planned** — what is planned goes in the roadmap.

  **The images of the interface manual are generated, never taken by
  hand.** `make manual` builds the real window, shows it without a screen and
  photographs it; the program never writes a reference, it writes
  `<name>.new.png`, and `compare-screenshots.py` promotes or deletes — so that
  an image only enters a diff on the day the interface has changed.
  `check-screenshots.py` catches what neither of the two sees: an image that
  the manual shows and that nothing generates. A new screenshot is added in
  `src/test/tools/screenshots.cpp`, and it is only authoritative under the
  settings that program sets — screenless platform, Fusion style, `DejaVu Sans`;
  it refuses to photograph without that font. ADR 0024.
- **Definition of “done”** — code, tests, **benchmarks replayed and measurements
  recorded**, manual section up to date, patch bumped, CHANGELOG entry
  regenerated.

  The benchmarks are replayed **on every issue**, and not only when performance
  is at stake: a measurement says nothing alone, it only speaks when compared
  with the previous one. They run in `Release`, the shipped mode, and the
  gate does not build them — it is therefore a separate command,
  `make bench`. As long as they take a minute, the question of their cost does
  not arise; it will arise, and will then be dealt with by a subset.

  **“Measurements recorded” admits one absence, and only one** — issue #270.
  `make bench` waits thirty seconds for the load to drop below
  `BENCH_MAX_LOAD`; beyond that it measures, displays, and **records nothing**. A
  log section that cannot be compared with anything takes the place of the one
  that is missing and hides the fact that there was no measurement; a version
  without a section says so. What is then still owed is a sentence in the PR: no
  measurement for this version, and why. Replaying `make bench` when the machine
  is quiet produces it.
