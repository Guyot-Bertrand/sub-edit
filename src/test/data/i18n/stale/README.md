# Stale-translation fixture

Replayed by `src/scripts/check-translations.py --check-fixtures`, a step of the fixtures gate (issue #670, spec 15,
D10). It stands in for `src/po/`: each case copies `po/` to a scratch directory, writes the `LINGUAS` it names, and
runs the check there.

- `po/subedit.pot` — a template of eight entries, a plural among them, in the layout `make pot` writes.
- `po/de.po` — complete and well formed: nothing to find.
- `po/fr.po` — a catalogue the template has moved past: an empty entry (`Close`), a fuzzy one, a sentence whose
  English changed since the last merge (`subtitle` became `subtitles`), an entry the template added since and the
  catalogue lacks, a translated entry the template dropped, and an entry already obsolete.
- `po/pl.po` — complete but malformed: a placeholder renumbered (`%1` → `%2`), a mnemonic lost, and a plural with two
  forms where the header says three.
- `cases.json` — one case per `LINGUAS`: the verdict, the counts of each language (translated, untranslated, fuzzy,
  obsolete, on the merged copy), and every finding as `[language, kind, msgid, fatal]`.

The same catalogues are installed in one case and seeds in another, so that the one difference between the two runs
is whether a finding fails. `fr` merged as installed (fuzzy matching) finds the changed sentence `CHANGED`; merged as
a seed (none) it finds it `NEW`, and the old sentence `GONE`. The replay also checks that the report names each
failing entry and that the catalogue directory is left as it was.
