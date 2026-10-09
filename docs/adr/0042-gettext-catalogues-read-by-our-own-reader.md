# 0042 — Keep messages in gettext catalogues, read by our own reader

**Date:** 2026-10-09
**Status:** accepted

Decided at the framing of phase 15 ([#655](https://github.com/Guyot-Bertrand/sub-edit/issues/655)), which the roadmap had left as an open question: "Qt Linguist or gettext?".

## Context

The interface, the command line and the shared wording of `core/wording/` must speak several languages. Three facts, checked in [#654](https://github.com/Guyot-Bertrand/sub-edit/issues/654), decide the question:

- **`core` and `cli` include no Qt header.** `core/wording/` is shared by the window and the command line; `QObject::tr` cannot reach it.
- **Gaupol's catalogues are gettext `.po` files**: 566 messages, no `msgctxt`, no named placeholders, four plurals, 20 locales, 50 % to 99 % translated depending on the locale. 34 % of our window literals have an identical msgid there; 66 % are new.
- **Neither toolchain is installed** here or in `setup-toolchain.sh`, so provisioning is needed whatever is chosen.

The libc `gettext` functions translate only when the locale is *generated on the machine*. A CI runner or a fresh install without `fr_FR.UTF-8` gets English silently, and a test of the French interface cannot run there.

## Decision

Messages are kept in GNU `.po` catalogues, compiled to `.mo` by `msgfmt` at build time, and read at run time by a **reader of our own in `core/i18n/`**, Qt-free, used by `core`, `cli` and `gui`. The reader evaluates the `Plural-Forms` expression and resolves the language from `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG` itself, without asking the system whether the locale exists. The msgid is the English text; arguments are positional (`%1`..`%9`).

## Alternatives dismissed

- **Qt Linguist (`.ts`) everywhere** — would put Qt in `core`, against the minimal-executable principle, or leave a second mechanism for it.
- **Qt Linguist for the window and something else for `core` and `cli`** — two catalogues, two workflows, two toolchains to keep in step.
- **Converting Gaupol's `.po` into `.ts`** — faithful only where strings match (34 %), and `core` stays out.
- **libc `gettext` as it is** — locale-dependent and untestable on a machine without the locale; silent failure for the user who sets only `LANGUAGE`.
- **A catalogue format of our own** — gives up the tools translators already use and Gaupol's twenty locales.

## Consequences

- One mechanism and one syntax of arguments everywhere; Gaupol's catalogues become seeds with one mechanical conversion (`_` → `&`).
- **We own a reader**: a few hundred lines, which must be safe on a corrupt or hostile `.mo` (checked under ASan) and are tested under any locale. This is the cost of the decision.
- The gettext tools become a build-time dependency (`gettext` package); nothing of gettext is needed at run time.
- Qt's own strings (standard buttons) keep using Qt's catalogues.
- Reconsider if the reader's maintenance proves heavier than adopting a gettext library that does not depend on the system locale.
