# 0027 — ICU to read and write encodings

**Date:** 2026-09-02
**Status:** accepted

Decided while scoping phase 8, issue #288.

## Context

Phase 8 must read files that are not UTF-8. Two distinct needs follow from it,
and they are easily confused:

**Converting** — turning bytes in a given encoding into text, and the reverse on
writing. Without it, nothing is possible.

**Detecting** — proposing an encoding when nobody has said which. A subtitle
file carries no encoding declaration: neither a header nor a naming convention.
The only explicit mark is the BOM, and it exists only for Unicode formats.

The roadmap posed the open question: "Gaupol relies on `charset-normalizer`,
which has no direct equivalent in C++. Candidates: ICU, `uchardet`,
`compact_enc_det`."

**The starting state matters, and it is peculiar.** `subedit-cli` has **no
external dependency** — `ldd` shows neither Qt nor ICU. Seven phases left it
that way. `subedit-gui`, for its part, already pulls in `libicui18n`,
`libicuuc` and `libicudata` through Qt.

## What was measured

The three routes were run **against the same corpus**, before choosing — it is
what the phase initialisation (#287) had required, and what
`score-encoding-detection.py` is for. The labelled corpus is that of #289: nine
fixtures whose table states the encoding.

| Route | Labelled corpus | What it can do |
| :---- | --------------: | :------------- |
| **ICU**, `ucsdet` and `ucnv` | **9/9** | detect **and** convert |
| `uchardet` | **9/9** | detect only |
| hand-written tables, prototype | 6/9 | whatever is written |
| `file -b --mime-encoding`, control | 6/9 | — |

The table prototype — BOM, strict UTF-8, then a vote on the share of high bytes
that land on a letter — gets it wrong exactly where it is hard: it reads Latin-1
as CP1252, CP1252 as Latin-2, CP1250 as CP1252.

On the private corpus, without labels, all three propose an encoding under
which the file decodes entirely in 71 cases out of 71 — which proves nothing
beyond necessity: a single-byte encoding decodes almost anything, and that is
precisely where mojibake comes from. They contradict each other on 19 files out
of 71, all in the Latin family.

## Decision

**ICU, for conversion as for detection, as a core dependency.**

Three reasons, in the order of their weight.

**Conversion decides, not detection.** Detection without conversion is useless,
and ICU is the only one of the three candidates that converts. Choosing
`uchardet` would force conversion to be written alongside — hence choosing
between ICU and tables anyway, but after adding one more dependency.

**Iso-functionality fixes the set of encodings, and it cannot be written by
hand.** Gaupol carries ninety-seven. Writing ninety-seven tables — among them
Big5, Shift-JIS and GB18030, which are not tables but state machines — is
endless and pointless work, for a worse result than what a distribution library
has been doing for twenty years.

**The real cost is smaller than it looks for the window, and full for the
command line.** `subedit-gui` already loads ICU through Qt: its size at startup
does not move. `subedit-cli` gains its first dependency from it, and that is the
real price of this decision.

## What the decision costs, written rather than left unsaid

**`subedit-cli` stops being self-contained.** It was a real property and it was
written nowhere — so it was never a promise, only a fact. It is lost, and this
paragraph is here so that it is known to have been lost deliberately.

**Two more package names, one per family.** `libicu74` on Debian and Ubuntu,
`libicu` on Fedora — and ADR 0023 already says that this is where the package
can be wrong without anything showing it at build time. #266 paid for it once.
What catches it now exists: `check-rpm.sh` installs the `.rpm` on a real
Fedora, every week.

**The weight.** `libicudata` is some thirty megabytes. It is already on every
machine that runs Qt, KDE, Firefox or LibreOffice; it is not necessarily on a
bare server where someone would want only `subedit-cli`.

## Alternatives dismissed

**`uchardet` to detect, hand-written tables to convert.** It is the cheapest
route in dependencies — `libuchardet` is about a hundred kilobytes — and it
detects as well as ICU on the corpus. Dismissed for conversion: it caps the set
of encodings at what is written, which contradicts iso-functionality. It
remains the right answer the day ICU's weight becomes a real problem rather
than a theoretical nuisance.

**`compact_enc_det`.** Missing from the Ubuntu and Fedora repositories, hence
outside the rule of ADR 0004 — dependencies come from the distribution.

**Writing detection by hand, ICU for conversion only.** The prototype scores
6/9 on nine made-up fixtures, that is, on the easy case. Adding ICU and then not
using it for what it does best would be paying without taking.

**Qt for conversion, in the window only.** `QStringDecoder` exists and the
window already has Qt — but the core is not allowed to depend on it, it is an
architectural invariant that `check-architecture.sh` holds. Two conversion
paths, one for the window and the other for the command line, would be two ways
of reading the same file: exactly what sharing the core exists to prevent.

## Consequences

**Detection is a proposal, never a certainty.** Latin-1 and CP1252 are
indistinguishable on a text that does not use the `0x80–0x9f` range: the bytes
do not carry the answer. A detector that decides is guessing, and it is
`latin1.srt` in the labelled corpus that shows it — all three routes give an
answer there, and none can prove it. What best-effort reading with diagnostics
(ADR 0008) requires here: the chosen encoding is written in the diagnostics,
and the user can contradict it.

**The score will be taken again.** `score-encoding-detection.py` takes the
detector as an argument: the day someone wants to replace ICU, the comparison
will be redone on the same corpus, in the same terms, without rewriting
anything.
