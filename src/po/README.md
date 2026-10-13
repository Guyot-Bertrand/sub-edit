# Message catalogues

| File | What it is |
| :--- | :--------- |
| `subedit.pot` | the template: every literal passed to `translate`, `translateIn` or `translatePlural` in `src/lib` and `src/exe`. Generated, never edited by hand |
| `<lang>.po` | one catalogue per language, merged from the template |
| `LINGUAS` | the languages that are **compiled and installed**, one per line — the one list of them |

A `.po` that is not in `LINGUAS` stays in the repository without being built or shipped: a language ships when it is
complete — no untranslated and no fuzzy entry — and has been reviewed by someone who reads it (spec 15, D5). Today
that is French alone.

## French

`fr.po` is complete and installed (#668): a catalogue taken over, never a seed. It started from Gaupol's French — the
156 entries `seed-catalogues.py --output-dir <scratch>` gives it — and the other 734 were written for subedit. An entry
carried from Gaupol says so in a `# From Gaupol.` comment, and the header credits Gaupol's translators, the upstream
revision and the licence. It has no seed marker, so the script never writes it, and `make po` merges it **with** fuzzy
matching: a changed English sentence comes back flagged `fuzzy`, which a translator reviews before the language ships
again. Check it with:

```console
$ msgfmt --check --statistics -o /dev/null src/po/fr.po
890 translated messages.
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
