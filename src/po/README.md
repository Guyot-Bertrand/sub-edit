# Message catalogues

| File | What it is |
| :--- | :--------- |
| `subedit.pot` | the template: every literal passed to `translate`, `translateIn` or `translatePlural` in `src/lib` and `src/exe`. Generated, never edited by hand |
| `<lang>.po` | one catalogue per language, merged from the template |
| `LINGUAS` | the languages that are **compiled and installed**, one per line — the one list of them |
| `UNREVIEWED` | the installed languages whose translation is machine-written and **not yet reviewed** by a reader of the language, one per line |

A `.po` that is not in `LINGUAS` stays in the repository without being built or shipped: a language ships when it is
complete — no untranslated and no fuzzy entry —, with its manuals (spec 15, D5). French is reviewed by the maintainer;
every other language ships machine-translated, listed in `UNREVIEWED` and saying so, until a reader of it signs it off.
A line of either list is a bare code, `de` or `pt_BR`; a comment is a line of its own, starting with `#`.

## Adding a language

Nothing in the code, the build or the gate names a language: each one is read from `LINGUAS` and from the files of
the language. For a code `<lang>` whose seed is `<lang>.po`:

1. **The catalogue**: delete the seed marker line of `<lang>.po` (`seed-catalogues.py: a seed…` and the line after it),
   complete every entry as French was (#668), credit Gaupol's translators as the header already does, and add the
   review caveat line below. `msgfmt --check --statistics -o /dev/null src/po/<lang>.po` reports no untranslated and no
   fuzzy entry.
2. **The manuals**: `docs/i18n/<lang>/manual/`, page for page the English tree, each page opening with the review
   caveat below; the code blocks and the inline code byte for byte the English ones.
3. **The vocabulary of the manual checks**: `docs/i18n/<lang>/vocabulary.json` — the words of the tables' headers and
   of the man pages, read by `doc_vocabulary.py` (its docstring lists the keys); French's table is the model.
4. **The two lists**: add `<lang>` to `LINGUAS` and to `UNREVIEWED`.
5. **The English manual**: the `## Language` sections of `docs/manual/subedit-cli/invocation.md` and
   `docs/manual/subedit-gui/invocation.md` name the language as installed and machine-translated, with the line
   `<!-- machine-translated: <lang> -->`, and the pages that list the installed languages (`installation.md`,
   `invocation.md`, `help.md`) name it; the translated manuals follow.
6. **`make manual`** takes the language's screenshots and refreshes the `--version` example, which lists the installed
   languages. It needs Qt's own catalogue for the language (`qtbase_<lang>.qm`) on the machine.

What then runs for the language with no line to write: the build compiles and installs its catalogue, manual and man
pages; `translations` holds it complete and its caveat in place; the structure check, `check-cli-manual.py`,
`check-gui-manual.py` and the screenshots check read its tree; the shipped-language tests of the command line and of the
window, and the man page tests, run for it (`SUBEDIT_SHIPPED_LANGUAGES`, the lines of `LINGUAS`, given to those test
files alone); `check-installation.sh` and `check-rpm.sh` look for its files in the tree and in the packages.

## The review caveat

A language of `UNREVIEWED` says it is machine-translated in three places, each in a fixed form that
`src/scripts/check-review-caveat.py` (the `translations` step) checks; a language that is not in the list may carry
none of them, so that signing a language off is a diff of a few lines, each one refused if forgotten.

| Where | What, exactly |
| :---- | :------------ |
| the catalogue `<lang>.po` | in the header's comments, before the first `msgid`, the line `# Machine translation, not reviewed by a reader of the language (spec 15, D5).` — `make po` keeps it |
| every page of `docs/i18n/<lang>/manual/` | right after the `#` title, the line `<!-- machine-translated -->`, then on the very next line a block quote in the language saying that the page is a machine translation awaiting a reader's review — prose only, no inline code and no link, so that the structure check has nothing to compare with the English page; a man page shows it at the head of its DESCRIPTION |
| each `## Language` section of the English manual | the line `<!-- machine-translated: <lang> -->`, beside the sentence that says the language is machine-translated |

French is reviewed, so it is not in `UNREVIEWED` and carries none of them. **To sign a language off**, once a reader of
it has said so in its issue: delete its line in `UNREVIEWED`, the header line of its catalogue, the marker and its quote
on every page, and its line in the Language sections, with the sentence beside it.

## French

`fr.po` is complete and installed (#668): a catalogue taken over, never a seed. It started from Gaupol's French — the
156 entries `seed-catalogues.py --output-dir <scratch>` gives it — and the other 737 were written for subedit. An entry
carried from Gaupol says so in a `# From Gaupol.` comment, and the header credits Gaupol's translators, the upstream
revision and the licence. It has no seed marker, so the script never writes it, and `make po` merges it **with** fuzzy
matching: a changed English sentence comes back flagged `fuzzy`, which a translator reviews before the language ships
again. Check it with:

```console
$ msgfmt --check --statistics -o /dev/null src/po/fr.po
893 translated messages.
```

Conventions: the terms of the French manual (*sous-titre*, *décaler*, *caler*, *cadence d'images*, `i/s`, *plage*,
*motif*, *réglages*); a plain space before `:`, `;`, `?` and `!`; `…` where the source has it; one `&` per label, its
letter unique within its menu or dialog; option names, values typed, format names and the `%N` arguments kept as they
are.

## Workflow

```console
$ make pot        # rewrite subedit.pot from the code
$ make po         # merge the template into every catalogue, seeds included
$ make pot-check  # fail when subedit.pot is not what the code produces (part of check-local)
```

`make pot` is reproducible: same sources, same bytes. Sources are sorted, locations name the file only (a line number
would change at every insertion), and `POT-Creation-Date` is the template value.

## The gate: translations that fall behind

```console
$ make translations   # part of check-local, right after pot-check
```

The `translations` step (`src/scripts/check-translations.py`, #670) merges the template into a scratch copy of every
catalogue — `update-po.sh --into`, the same merge as `make po` — and compares the result with the committed file by
content: entries, translations and flags, never line wrapping, references or the header. **A language of `LINGUAS`
must come out complete**; a seed is only reported. The report is a table, one line per language — translated,
untranslated, fuzzy and obsolete entries of the merged copy, and the share of the template it translates — then a
line per seed with what else it has, then, for an installed language that fails, its first ten entries by kind:

| Kind | What it means | What to do |
| :--- | :------------ | :--------- |
| `NO-FILE` | `LINGUAS` names a language that has no `.po` | add the catalogue, or remove the line |
| `UNTRANSLATED` | the entry has no translation, or a plural form is empty | translate it |
| `NEW` | the template has the entry and the committed catalogue does not | `make po`, then translate it |
| `FUZZY` | the committed entry is flagged `fuzzy` | review it, then remove the flag |
| `CHANGED` | the merge brings the entry back flagged `fuzzy`: its English changed | `make po`, then review it |
| `GONE` | the template no longer has an entry the catalogue translates | `make po`, which makes it obsolete |
| `DIFFERS` | the merge changes the entry's flags | `make po` |
| `PLACEHOLDERS` | the `%N` arguments are not the source's — each plural form a subset, together all of them | fix the translation |
| `ACCELERATOR` | the source has an `&` mnemonic and the translation not, the reverse, or two | fix the translation |
| `PLURAL-FORMS` | a plural has not as many forms as the header's `Plural-Forms` | fix the forms or the header |

Obsolete entries are counted and never fail: `make po` keeps them. A seed's accelerator mismatches are real — Gaupol
put no mnemonic where we have one — and wait for the translator who takes the seed over.

## The seeds from Gaupol

Every `<lang>.po` that `LINGUAS` does not name is a **seed** (#667): our template, with the translations of Gaupol's
catalogue of that language carried over where Gaupol has exactly the same message — `_` accelerators turned into `&`,
Python placeholders into `%1`, fuzzy entries never carried. The rules are in the docstring of
`src/scripts/seed-catalogues.py`; how much each seed translates is in
[`docs/mesures/catalogue-seeds.md`](../../docs/mesures/catalogue-seeds.md).

**Origin and licence.** The translations are the work of Gaupol's translators (<https://github.com/otsaloma/gaupol>,
`po/`), distributed under the GNU General Public License, version 3 or later, as subedit is. Each seed's header names
the upstream file and revision and credits the translators that file credits.

`make po` merges the template into the seeds too, without fuzzy matching: a new message appears empty, and their
translations are kept. **To re-seed** from a newer Gaupol, with the read-only clone of `reference/gaupol` fetched
(`./src/scripts/reference.sh fetch`):

```console
$ ./src/scripts/seed-catalogues.py reference/gaupol/po \
      --revision "$(git -C reference/gaupol rev-parse HEAD)" --report docs/mesures/catalogue-seeds.md
```

The script only reads the clone, writes the same bytes for the same inputs, and never touches a language of `LINGUAS`
nor a seed whose header has lost its `seed-catalogues.py` marker line: deleting that line is how a translator takes a
seed over. Nothing of the gate runs it on Gaupol's files; the `fixtures` step replays it on
`src/test/data/i18n/seed/`.

## Build and install

`cmake/Translations.cmake` compiles each language of `LINGUAS` with `msgfmt --check` into
`<build>/share/subedit/locale/<lang>/LC_MESSAGES/subedit.mo`, and installs it under
`<prefix>/share/subedit/locale/<lang>/LC_MESSAGES/`. The `.deb` and the `.rpm` carry the same tree. A seed is never
compiled, and `check-installation.sh` refuses an installed catalogue of a language `LINGUAS` does not name.

The gettext tools (`msgfmt`, `xgettext`, `msgmerge`) are build-time tools only; `./src/scripts/setup-toolchain.sh`
installs them. Nothing of gettext is needed at run time: the program reads the `.mo` itself (ADR 0042).
