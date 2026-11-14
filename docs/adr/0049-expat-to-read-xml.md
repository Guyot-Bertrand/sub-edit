# 0049 — Expat to read XML

**Date:** 2026-11-14
**Status:** accepted

Decided while scoping phase 17, [#837](https://github.com/Guyot-Bertrand/sub-edit/issues/837); the decision is D5 of
[its spec](../specs/17-text-formats.md).

## Context

Phase 17 reads and writes TTML, and TTML is XML: namespaces (`tts:`, `ttp:`, `ttm:`), entities, character references,
comments, CDATA sections, a prolog. Nine formats were read so far by grammars written by hand, and none was XML.

**The starting state, measured on 2026-11-14.** `subedit_core` links ICU and enchant, nothing else; `subedit-cli` has no
XML library. `subedit-gui` already loads **both** `libexpat` and `libxml2`, without the project asking for either:
expat through fontconfig, libxml2 through libmpv's dependencies (libavformat among them). Both development packages
are on the machine: expat 2.6.1, libxml2 2.9.14.

**What is written, not only read.** The writer of TTML produces XML from the model, and needs nothing but escaping;
the decision is about reading.

## Decision

**The core reads XML with expat**, linked through `find_package(EXPAT)` like ICU (ADR 0004), behind one reader of
the `format` directory. Nothing outside it includes `expat.h`.

- The parser is created namespace-aware (`XML_ParserCreateNS`), so `tts:fontStyle` arrives as its namespace and
  local name, whatever prefix the file chose.
- It is fed the **UTF-8 text the encoding layer has already decoded** (phase 8, ADR 0027), its encoding forced to
  UTF-8: the XML declaration's `encoding` does not decide twice.
- **No external entity is ever loaded**: no handler is set for them, and expat loads none without one. Expat's
  protection against entity expansion (since 2.4.0) stays on.
- The writer of TTML does not use it: it escapes `&`, `<`, `>` and the quote, and writes.

The packages gain one dependency: `libexpat1` for the `.deb`, `expat` for the `.rpm`.

## Alternatives dismissed

- **libxml2.** Also present, and richer: a tree, XPath, validation. None of it is needed: a TTML reading is one pass
  over the document, keeping the `head`'s styles and walking `body`. Its larger surface has a longer security history,
  and its defaults load DTDs unless told otherwise — a mistake to avoid rather than a feature to use.
- **Qt's `QXmlStreamReader`.** The core has no Qt and the command line does not link it (the minimal executables of
  the design principles). Bringing QtCore into `subedit-cli` for one format is the wrong price.
- **A parser written by hand**, as the hearing-impaired mentions were (ADR 0017). That grammar was the project's own;
  XML's is not. Namespaces, entities, CDATA and character references are each a place to be wrong, and a file from a
  streaming service uses all of them. ADR 0017 wrote a parser because no library spoke the grammar; one does here.
- **pugixml, tinyxml2.** Not installed, a new package for the user, and a DOM where a pass is enough.

## Consequences

- `subedit-cli` links `libexpat` (190 KB). `ldd` shows it, and the packages declare it.
- **SAMI is not read with it.** SAMI looks like HTML and is rarely well-formed — unclosed `<P>`, attributes without
  quotes — and an XML parser would refuse most real files. Its reader is written by hand, tolerant, and that is D4 of
  the spec.
- A malformed TTML file is refused with expat's line and column, in the same form as the other formats' reading
  errors.

**What makes it cheap to undo:** one reader includes expat. Replacing it means rewriting that file, not the format.
