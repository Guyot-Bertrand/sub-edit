# Gaupol's correction patterns

Gaupol's twelve pattern files and their twelve activation `.conf` files, which
`subedit` installs into `share/subedit/patterns` and reads as they are —
[ADR 0037](../../docs/adr/0037-lire-les-motifs-de-gaupol-tels-quels.md).

Copied from Gaupol's repository at revision `f433bae3d23ef3572411fa9adca3fbab5cb1b3f9`
(2026-07-28), from its `aeidon/data/patterns` directory. **Gaupol is
under GPL-3, like this project**: copying is permitted, and this file is its
attribution — the patterns are the work of Gaupol's authors, not of this repository.

**Never touch them up by hand.** They are what the engine will have to read, and a
"repaired" pattern here would cease to be Gaupol's. An update is done by
recopying the whole set from a more recent revision of Gaupol, changing the
revision above, then rerunning the oracle: the diff of the expected files then says what
the update changes.

**What reads them**: the core's reader (`core/text/pattern_catalogue`), and
`src/scripts/pattern-oracle.py`, which writes what Gaupol does with them into
`src/test/data/motifs/attendus/`. There is only one copy, and it is this one.

**This file is installed with them**: the reader looks only at files whose
name ends with a pattern type or with `.conf`, and this one is neither.
