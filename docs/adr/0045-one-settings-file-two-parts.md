# 0045 — One settings file, an operation part and a display part

**Date:** 2026-10-11
**Status:** accepted

Decided at the framing of [#699](https://github.com/Guyot-Bertrand/sub-edit/issues/699). Completes [0022](0022-configuration-au-noyau-et-tolerance-par-option.md); it does not replace it.

## Context

`core::Settings` holds operation parameters (`durationAdjustment`, `correction`, `spellCheck`) next to display preferences (geometry, columns, theme). The window remembers both. The command line never reads the file, so the same operation gives different results on the two surfaces, silently.

## Decision

1. The core exposes the operation part as `OperationSettings`; the file has two blocks: operation settings and display preferences. The file stays one flat file with unchanged keys, so no migration; on writing, each part is a block under a banner comment.
2. The command line reads **only** the operation part (`readOperationSettings`), by default. `--no-config` switches the reading off and `--config FILE` names another file.
3. Priority, strongest first: option on the command, the file, the core default.
4. The command that reads says which file it read at the first verbosity level, and the JSON record carries the path.
5. `config list`, `config show`, `config set` and `config unset` let the command line document, look at and change the operation part. They share one catalogue of keys (key, description, accepted values, default) in the core. Display keys are refused.
6. The standard location is still resolved in one place, now in the Qt-free `subedit_platform`, and handed down by `main`.

## Alternatives dismissed

- **Opt-in reading only** — reproducible on every machine, but a value set in the window or by `config set` would not take effect until each command asked for it. The maintainer preferred convenience, and the announcement of point 4 limits the cost.
- **`[section]` headers** — would break the reading of every existing file, and ADR 0022 chose no migration.
- **Two files** — two paths to document and to keep consistent, for one user.

## Consequences

- Same command line, different answer on two machines is now possible; scripts and bug reports use `--no-config` or `--config` to pin it.
- The command line cannot read a display preference: the type does not give it one.
- `userSettingsPath()` leaves `gui`; its callers change.
