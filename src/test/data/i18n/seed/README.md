# Catalogue seed fixture

Replayed by `src/scripts/seed-catalogues.py --check-fixtures`, a step of the fixtures gate (issue #667,
spec 15, D5). It stands in for Gaupol's catalogues, which a fresh clone does not have.

- `gaupol/` — catalogues written the way Gaupol's are: `_` accelerators, Python placeholders flagged
  `python-brace-format`, a fuzzy entry, an untranslated one, an obsolete one, an entry our template
  does not have, and plurals with the right and the wrong number of forms. `fr` and `nl` exist to be
  refused.
- `template.pot` — a small template in the layout `make pot` writes, with a context, translator
  comments and a message long enough to be wrapped.
- `po/` — the state of `src/po/` before the run: `LINGUAS` installs `fr`, whose catalogue must not
  change; `nl.po` has lost its seed marker, so a translator owns it; `de.po` is an older seed, which
  the run overwrites.
- `expected/` — the seeds and the report the run must write, byte for byte. They were checked by
  hand, entry by entry, against the rules of the script's docstring.
- `cases.json` — the revision passed to the run, the files that must be left alone and why, and the
  translation each named entry must end with (`""` for empty, `null` for absent, a list for the
  plural forms).

The replay also runs the script twice and in a second directory (the bytes must not move), runs the
merge `make po` applies to a seed (it must change nothing), and passes each seed to `msgfmt --check`.
