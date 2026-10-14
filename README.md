# subedit

A subtitle editor, rewriting [Gaupol](https://github.com/otsaloma/gaupol) in
C++23 and Qt 6.

> **Status: phase 7 of seventeen, the last one before the MVP.** Both programs
> exist and are usable. `subedit-cli` carries seven subcommands — inspect,
> convert, shift, transform, change the frame rate, snap to a grid, remove
> hearing-impaired mentions. `subedit-gui` opens, displays, edits, inserts,
> deletes, undoes, saves, plays the associated film and remembers its settings.
> Both install, and ship as `.deb` and `.rpm`. What remains is the manual
> reachable from the window. The [roadmap](docs/feuille-de-route.md) describes
> the seventeen phases: nine lead to the MVP, the following ones complete
> feature parity with Gaupol.

## Why

Gaupol is an excellent subtitle editor, written in Python with GTK. subedit
aims at **feature parity** in a compiled native language, with performance as a
cross-cutting design goal — data structures and algorithms chosen for the job
from the start, rather than optimisation after the fact.

## Building

Dependencies: CMake ≥ 3.28, a C++23 compiler (GCC 13 will do), and the
verification toolchain.

```bash
./src/scripts/setup-toolchain.sh   # ninja, clang-tidy, clang-format, gcovr, ccache, git-cliff, Qt 6, libmpv, ffmpeg
make build                         # compile
make release                       # the two optimised binaries, and nothing else
make test                          # compile and run the tests (end-to-end excluded — see make asan)
make check                         # the full quality gate
make help                          # every target
```

## Layout

```
src/lib/subedit/<lib>/    libraries — all the useful code lives here
src/exe/<binary>/         executables — a main and wiring, nothing else
src/test/{unit,bench}/    tests and benchmarks, mirroring the libraries
src/test/tools/           the programs the tests launch, and the one that
                          photographs the window for the manual
src/scripts/              automation
docs/                     specs, decisions, manuals
reference/gaupol          reference clone, read-only, not committed
```

Internal includes are always qualified from `src/lib`:

```cpp
#include <subedit/core/version.hpp>
```

## Quality

`make check` is the gate, and the CI runs nothing else: formatting, warnings as
errors, clang-tidy, architecture invariants, tests under AddressSanitizer and
UndefinedBehaviorSanitizer, coverage ratchet. Local and remote cannot diverge.

## Documentation

| Document | Contents |
| :------- | :------- |
| [Roadmap](docs/feuille-de-route.md) | the outline of the MVP, the phases, their scope and their order |
| [Design principles](docs/principes-de-conception.md) | permanent rules that apply to all the code |
| [Specs](docs/specs/) | one spec per phase, and the inventory of Gaupol's features |
| [Decisions](docs/adr/) | what was decided, and why the other options were set aside |
| [Manual](docs/manual/index.md) | where to start, then one manual per executable |

## License

GPL-3.0-or-later, see [LICENSE](LICENSE).

This choice makes it possible to reuse Gaupol's linguistic data — correction
patterns, format headers, translations — which represent years of work. See
[ADR 0002](docs/adr/0002-licence-gpl3.md).

subedit is an independent rewrite: no Gaupol code is reused.
