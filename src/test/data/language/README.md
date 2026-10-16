# Language fixtures

Cases replayed by `src/scripts/check-comment-language.py --check-fixtures`, a step of the fixtures
gate.

- `lines.json` — single lines and the class the classifier must give them: `fr` or `en`. The
  English lines are the false positives the rules of the script exist to avoid.
- `cases.json` — small repositories built in a scratch directory. `base` is committed first and
  is the revision the list may not outgrow; `files` is then added to the index without a commit.
  A value starting with `@` names a sample of this directory, any other value is the content
  itself. `expect` lists the failure kinds the check must report, once each.
- `injected-comment.cpp.txt` and `injected-titles.txt` — not replayed here: `verify-gates.sh`
  reads them and injects their French into the tree, a C++ comment and two test titles (one per
  line), to prove that the `arch` gate refuses them. They live here so that the script itself
  holds no French.
- The samples — `*.md`, `*.cpp.txt`, `*.txt` — are data: French on purpose, and outside the rule as
  everything under `src/test/data/` is, except the `README.md` you are reading. The C++ samples
  end in `.txt` so that the format gate, which formats every `.cpp` under `src/`, leaves them
  alone.
