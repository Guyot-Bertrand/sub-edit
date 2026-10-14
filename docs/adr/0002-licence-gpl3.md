# 0002 — Publish subedit under GPL-3.0-or-later

**Date:** 2026-08-04
**Status:** accepted

## Context

Gaupol is published under GPL-3.0-or-later. Its code is not reused — subedit is
an independent rewrite — but Gaupol ships **data** under the same licence, which
represent years of linguistic work:

- the correction patterns (`aeidon/data/patterns/`): common errors classified as
  Human/OCR, capitalisation, removal of hearing-impaired annotations, line-break
  penalties, broken down by script, language and country;
- the formats' default headers (`aeidon/data/headers/`);
- the interface translations (`po/`, 20 locales).

Rebuilding this corpus from scratch would be a considerable effort, for a result
of lower quality, and would take the project away from its goal of feature
parity.

## Decision

subedit is published under **GPL-3.0-or-later**, which allows Gaupol's data to
be reused directly, with attribution.

## Alternatives dismissed

- **A permissive licence (MIT, Apache-2.0) with rewritten patterns** — would
  offer complete freedom of use, but would require rebuilding all the linguistic
  work. The correction engine is the part richest in domain value: starting it
  over from scratch would degrade the product with no benefit to its user.
- **A permissive licence without a correction engine** — would mean giving up
  feature parity, which is the project's goal.

## Consequences

Every distribution of subedit must come with its sources. The project cannot be
built into a proprietary product — which is neither a goal nor a constraint for
a subtitle editor meant for personal use.

Dependencies must stay GPL-3.0 compatible: Qt under LGPLv3, dynamically linked,
is; this rules out, on the other hand, static linking with Qt.

The origin of the reused data must be credited explicitly in the repository.
