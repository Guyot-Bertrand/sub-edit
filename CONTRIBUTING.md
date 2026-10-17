# Contributing to subedit

## External contributions

**The project does not accept external contributions for now.** Pull requests
coming from forks will not be merged.

This is not a judgement on anyone's work: the project is in its foundation
phase, its conventions are still moving, and there is nobody to review. Reporting
a bug through an issue remains useful and welcome.

## Setting up your machine

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
git clone https://github.com/otsaloma/gaupol.git reference/gaupol
./src/scripts/setup-toolchain.sh
```

The reference clone is for consulting Gaupol's behaviour. It is kept
**read-only** by `src/scripts/reference.sh`: reading and `fetch` only, never a
modification.

## The working cycle

A project phase matches a milestone, and opens with a framing issue that
produces its spec. The work is then split into issues attached to that
milestone. See the [roadmap](docs/feuille-de-route.md).

Work happens on `feat/…`, `fix/…`, `docs/…` branches. The `main` branch is
protected against history rewriting.

### Definition of "done"

An issue is closed only when all of this is true:

- [ ] `make check` passes
- [ ] The added or fixed behaviour is covered by tests
- [ ] A benchmark exists if performance is at stake
- [ ] The relevant manual section is up to date
- [ ] The patch of `project(VERSION)` has been incremented in `CMakeLists.txt`
- [ ] `make changelog` has been regenerated
- [ ] The pull request body carries a `Closes #N` line
- [ ] An ADR is written if a decision that is costly to undo was taken

The three mechanically decidable points of this list — the bump, the journal
and the `Closes #N` line — are verified by the pull-request checks job, and so
no longer need to be reviewed by hand. **`Closes`, not a translation of it:**
GitHub closes an issue only on its own keywords, all of them English. This line
is an instruction to GitHub and not prose.

Writing the manual is part of the work, not of a catch-up pass: spelling out
the expected behaviour in prose is a design test.

**Its call examples, however, are not written: they are declared.** A `console`
block of the manual is preceded by a marker that names the command to
illustrate, and `make manual` rewrites the block with what that command really
produces.

````markdown
<!-- example: subedit-cli -->
```console
… rewritten by make manual …
```
````

The marker carries a whole shell line, which allows illustrating a chain —
`subedit-cli file.srt; echo $?` — as a user would write it.

An output copied by hand is documentation that is stale on a reprieve: the only
version number the manual contained was corrected twelve times during phase 2,
without anything checking that it was true. `make check-local` now refuses a
manual whose examples no longer match.

**The manual has two readers, and they do not read the same Markdown.** GitHub
renders it in the browser; `QTextDocument::setMarkdown` renders it in the window
that `Help ▸ Manual` opens. The second is stricter than the first on at least
one point, measured while handling issue #268: **a table separator line needs
at least three characters per column** — `| :--- |` passes everywhere, `| :- |`
makes a table on GitHub and bar-separated text in the window. Three tables of
the manual were in that case, and nothing said so.

There is nothing more to remember: `src/test/gui/real_manual_test.cpp` opens
every real page in a `ManualWindow` and compares the source's tables, images and
cross-references with what the rendering makes of them. A page that renders
badly fails the gate, on the page's name.

## Conventions

### Language

**Everything is written in English; French is a locale only** —
[ADR 0043](docs/adr/0043-english-is-the-language-of-the-repository.md). Code,
comments, scripts, build files, documentation, specs, ADRs, commit messages,
issues, pull requests and test titles are English. French lives in the
catalogue `src/po/fr.po` and the translated manual `docs/i18n/fr/manual/`.

The repository is mixed until the existing French is translated. Two checks of
`check-architecture.sh` hold the rule:

- `check-comment-language.py` refuses French in every tracked file outside the
  two locale trees, except the files named in `.french-allowlist`. The list only
  shrinks: translating a file means deleting its line, and a line that its
  version on `origin/main` does not have is refused.
- test case titles, which Catch2 prints, are held to a stricter rule: a
  non-ASCII character or a French function word fails the gate. A title that
  quotes a French datum — the fixture format has some — goes on the check's
  exemption list, with its reason.

A string the program shows is English in the source and goes through the
translation API — `translate`, `translateIn`, `translatePlural` or Qt's `tr` —
so that a catalogue can translate it. The `untranslated` step of `make check`
counts the user-visible literals that do not, file by file, against
`.untranslated-ratchet`: a new one fails the gate, and the count only goes down.
A literal that is not for a reader — a settings key, an option name, a JSON key,
a format's syntax — is exempted in `.translation-exemptions`, with its reason.

### Commits

[Conventional Commits](https://www.conventionalcommits.org/), verified by a
local hook and by the CI.

```
<type>(<scope>): <description>
```

- **types**: `feat`, `fix`, `perf`, `refactor`, `test`, `docs`, `build`, `ci`,
  `chore`, `revert`
- **scopes**: `build`, `ci`, `cli`, `core`, `doc`, `format`, `gui`, `i18n`,
  `scripts`, `test`, `text`, `video` — the same as the `area:` labels of
  issues
- a header of at most 72 characters, without a trailing period

Since `CHANGELOG.md` is generated from these messages, a sloppy message produces
a sloppy entry.

### Code

The permanent rules are in
[docs/principes-de-conception.md](docs/principes-de-conception.md): typed data
models, explicit memory ownership, abstraction where the variation is known,
testability by construction, measured performance.

Naming and format are enforced mechanically by `.clang-format` and
`.clang-tidy`, not left to vigilance.

**Every clang-tidy exclusion carries its justification in a comment.** An
unjustified exclusion is a defect, not a setting.

## Commands

| Command | Effect |
| :------ | :----- |
| `make build` | builds the `dev` preset |
| `make test` | builds and runs the tests (end-to-end excluded — see `make asan`) |
| `make check` | the complete quality gate |
| `make format` | applies clang-format |
| `make tidy` | static analysis |
| `make asan` | tests under AddressSanitizer and UndefinedBehaviorSanitizer |
| `make coverage` | coverage report |
| `make ratchet` | records the measured coverage as the new ratchet |
| `make bench` | Release benchmarks, recorded in `docs/mesures/performances.md` |
| `make changelog` | regenerates `CHANGELOG.md` |
| `make manual` | regenerates the manual's call examples |
| `make rpm-check` | installs the `.rpm` on a Fedora in a container — weekly, outside the gate |
| `make timings` | the last durations of each gate step, phase and proof, with their load and `JOBS`; flags a latest run above 1.5× the median (`RUNS=N`, `SUITE=check`) |
| `make help` | all targets |

`make check`, `make check-local` and `make verify-gates` end with a table of
what each step or proof cost, and the load average and `JOBS` it ran under; the
long steps also write the time of each of their phases (`· build: 812 s`). Every
measurement is appended to `build/gate-timings.tsv`, a local journal that git
ignores and `make clean` erases — `make timings` reads it.
