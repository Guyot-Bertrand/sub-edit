# Translatable-string fixtures

Cases replayed by `src/scripts/check-translatable-strings.py --check-fixtures`, a step of the
fixtures gate (issue #661, spec 15, D10).

- `*.cpp.txt` — annotated C++. A line ending in `// expect: <verdict>` holds literals that must
  all get that verdict: `counted` (the ratchet counts it), `translated` (it goes through
  `translate`, `translateIn`, `translatePlural` or `tr`), `not-prose` (it has no word), or
  `exempt:<category>` (the category of `.translation-exemptions` that the first matching entry
  names). `// expect: none` marks a line that must hold no literal at all — a quotation in a
  comment, an `#include`. A line `// path: <path>` gives the repository path the code after it
  is judged as, which is what proves the exemptions that hold in one file only. Every category
  of the table must be proven by at least one marked literal, or the replay fails.
- `cases.json` — small repositories built in a scratch directory, each with the repository's own
  `.translation-exemptions` copied in first. `base` is committed first and is the revision whose
  ratchet total may not be exceeded; `files` is then added to the index without a commit. A
  value starting with `@` names a sample of this directory, any other value is the content
  itself. `expect` lists the failure kinds the check must report, once each.
- The C++ samples end in `.txt` so that the format gate, which formats every `.cpp` under `src/`,
  leaves them alone, and so that nothing compiles them.
