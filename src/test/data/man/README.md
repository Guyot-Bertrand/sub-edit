# Man page fixtures

Cases replayed by `src/scripts/md-to-man.py --check-fixtures`, a step of the fixtures gate
(issue #683, spec 15, D9).

- `docs/manual/subedit-cli/` and `docs/i18n/fr/manual/subedit-cli/` — a manual of one
  subcommand, `demo`, in English and in French, laid out as the repository's is, so that the
  language of each tree is read from its path. The English `demo.md` holds every construct of the
  converter's subset: a synopsis, a section and a subsection, a list continued on a second line,
  an ordered list, a block quote, a table of three columns with an escaped bar, an example marker
  and a `console` block whose second line starts with a dot and holds a backslash, emphasis,
  inline code, a link, and characters outside ASCII — one groff names, one it does not.
  The French `demo.md` opens with the review caveat of a machine-translated page (#720): its
  marker and its quote, which the page shows after the synopsis, at the head of DESCRIPTION.
- `expected/man1/` and `expected/fr/man1/` — the pages each tree must give, version `1.2.3`,
  manual directory `/usr/share/subedit/manual` (and `/fr`). **Written by hand** from the rules of
  the converter's header, never copied from its output: the replay compares the bytes, and the
  set of files.
- `refused.json` — one edit per construct outside the subset, each made on a scratch copy of the
  English tree: `append` adds text at the end of `page`, `delete` removes it. The converter must
  stop, and its message must name the page (`expect`) and the construct (`reason`).
