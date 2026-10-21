#!/usr/bin/env python3
"""Generates the `man` pages of `subedit-cli` from its Markdown manual.

**The manual is the single source** — spec 15, D9, ADR 0044, issue #683. The
pages are not written by hand: they are this script's output on a manual tree,
`docs/manual/subedit-cli` or a translation of it, at build time and again at
installation, so no generated page is committed.

    md-to-man.py --manual DIR --output-dir DIR --version V --manual-dir PATH

writes into the output directory

    subedit-cli.1           from `invocation.md`, named after the first sentence of `index.md`
    subedit-cli-<name>.1    from `<name>.md`, one per row of the subcommand table of
                            `invocation.md`, named after its description there

and removes any other `subedit-cli*.1` it finds there, so that a subcommand
removed from the manual leaves no page behind. `--manual-dir` is the directory
the manual is installed in, which the FILES section names: it is known only at
installation, and that is why the pages are generated twice. The output depends
on nothing but its inputs — no date, no clock, no locale: two runs give the same
bytes.

## The subset

The manual is written in a small Markdown, and this converter handles exactly
that and refuses the rest, naming the page and the line. **Every page of the
tree is read**, the ones that become no `man` page included: a construct the
converter does not know fails the build the day it is written, rather than
vanishing from a page nobody rereads.

    headings      `#` is the page's title and is dropped; `##` is a section
                  (`.SH`), `###` and `####` a subsection (`.SS`)
    paragraphs    consecutive lines, filled by groff
    lists         `- item` and `1. item`, one level, continuation lines indented
    block quotes  `> text`, holding paragraphs and lists
    code blocks   fenced with three backticks, any info string (`console`,
                  `bash`...), shown as written; a bare block first in the page is
                  its SYNOPSIS
    tables        GitHub tables; each row becomes a tagged paragraph, the other
                  cells prefixed by their header, because a table of long cells
                  does not fit eighty columns
    comments      `<!-- ... -->` on one line: the example markers of the manual,
                  the language declaration; dropped
    review caveat `<!-- machine-translated -->` then a block quote on the next line, first in
                  a page of a language not yet reviewed (spec 15, D5): the quote opens the
                  DESCRIPTION, after the SYNOPSIS, so the page keeps the form of its call
    inline        code spans, `**bold**`, `*italic*`, `[text](target)` (the
                  text is kept, the target dropped), backslash escapes

Refused: images, raw HTML, indented code blocks, nested lists, setext headings,
horizontal rules, headings deeper than four, unbalanced emphasis.

## The output is ASCII

Every character outside ASCII is written as a groff escape (`\\[u00E9]`, or
`\\(em` and its kin for what groff names). troff reads bytes, not UTF-8, unless
someone calls `preconv` for it, and `man` does not always: a page that is ASCII
from end to end renders the same whichever way it is read, and
`check-man-pages.py` holds that. For each character escaped by its code point the page also
defines a fallback, used only on a device that has no glyph for it — `man` under the `C`
locale renders in ASCII, and would otherwise draw nothing and warn: a ligature then reads as its
letters, an accented letter as its base letter.

`--check-fixtures` replays `src/test/data/man`: the pages a small manual must give, written by
hand, and one refused edit per construct outside the subset — a step of the fixtures gate.

The words a page needs that no manual page carries — NAME, SEE ALSO... — come
from `doc_vocabulary.MAN_WORDS`, per language; a language without them is
refused.
"""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import shutil
import sys
import tempfile
import unicodedata

import doc_vocabulary

REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
FIXTURES = REPO_ROOT / "src/test/data/man"


class Unsupported(Exception):
    """A construct outside the subset, with where it was met."""

    def __init__(self, page: pathlib.Path | str, line: int, what: str) -> None:
        super().__init__(f"{page}:{line}: {what}" if line else f"{page}: {what}")


# ## Blocks

HEADING = re.compile(r"^(#{1,6})\s+(.*?)\s*#*\s*$")
FENCE = re.compile(r"^```([A-Za-z0-9_+-]*)\s*$")
BULLET = re.compile(r"^([-*+])\s+(.*)$")
ORDERED = re.compile(r"^(\d+)\.\s+(.*)$")
COMMENT = re.compile(r"^<!--.*-->\s*$")
# The marker of a page not yet reviewed by a reader of its language: the block quote on the next
# line says so in the language (spec 15, D5; `check-review-caveat.py`).
CAVEAT_MARKER = "<!-- machine-translated -->"
TABLE_SEPARATOR = re.compile(r"^\|?\s*:?-+:?\s*(\|\s*:?-+:?\s*)*\|?\s*$")
RULE = re.compile(r"^\s*([-*_])(\s*\1){2,}\s*$")


def split_row(line: str) -> list[str]:
    """The cells of a table row, `\\|` being a bar inside a cell rather than a border."""
    body = line.strip()
    body = body.removeprefix("|")
    if body.endswith("|") and not body.endswith("\\|"):
        body = body[:-1]
    cells = re.split(r"(?<!\\)\|", body)
    return [cell.strip().replace("\\|", "|") for cell in cells]


def parse_blocks(lines: list[tuple[int, str]], page: pathlib.Path | str) -> list[tuple]:
    """The blocks of a run of numbered lines, or `Unsupported` on the first construct the
    subset does not have."""
    blocks: list[tuple] = []
    index = 0

    def refuse(at: int, what: str) -> None:
        raise Unsupported(page, at, what)

    while index < len(lines):
        number, line = lines[index]
        stripped = line.strip()

        if not stripped:
            index += 1
            continue

        if fence := FENCE.match(line):
            body: list[str] = []
            index += 1
            while index < len(lines) and lines[index][1].rstrip() != "```":
                body.append(lines[index][1])
                index += 1
            if index == len(lines):
                refuse(number, "a code block that is never closed")
            index += 1
            blocks.append(("code", fence.group(1), body))
            continue

        if line.startswith("```"):
            refuse(number, "a fence with text after its info string")

        if heading := HEADING.match(line):
            level = len(heading.group(1))
            if level > 4:
                refuse(number, f"a heading of level {level}: the subset stops at four")
            blocks.append(("heading", level, heading.group(2), number))
            index += 1
            continue

        if COMMENT.match(stripped):
            index += 1
            continue

        if stripped.startswith("<"):
            refuse(number, "raw HTML: only a one-line <!-- comment --> is in the subset")

        if line.startswith(("    ", "\t")):
            refuse(number, "an indented code block: fence it with three backticks")

        if RULE.match(line):
            refuse(number, "a horizontal rule")

        if stripped.startswith(">"):
            caveat = index > 0 and lines[index - 1][1].strip() == CAVEAT_MARKER
            quoted: list[tuple[int, str]] = []
            while index < len(lines) and lines[index][1].strip().startswith(">"):
                text = lines[index][1].strip()[1:]
                quoted.append((lines[index][0], text[1:] if text.startswith(" ") else text))
                index += 1
            blocks.append(("caveat" if caveat else "quote", parse_blocks(quoted, page)))
            continue

        if stripped.startswith("|"):
            if index + 1 >= len(lines) or not TABLE_SEPARATOR.match(lines[index + 1][1]):
                refuse(number, "a table row with no separator row under its header")
            header = split_row(line)
            rows: list[list[str]] = []
            index += 2
            while index < len(lines) and lines[index][1].strip().startswith("|"):
                cells = split_row(lines[index][1])
                if len(cells) != len(header):
                    refuse(lines[index][0], f"a table row of {len(cells)} cells under a header of {len(header)}")
                rows.append(cells)
                index += 1
            blocks.append(("table", header, rows))
            continue

        bullet = BULLET.match(line)
        ordered = ORDERED.match(line)
        if bullet or ordered:
            kind = "bullet" if bullet else "ordered"
            items: list[tuple[str, str]] = []
            while index < len(lines):
                current = lines[index][1]
                item_bullet = BULLET.match(current)
                item_ordered = ORDERED.match(current)
                if kind == "bullet" and item_bullet:
                    marker, text = "", item_bullet.group(2)
                elif kind == "ordered" and item_ordered:
                    marker, text = item_ordered.group(1), item_ordered.group(2)
                else:
                    break
                index += 1
                # The continuation lines: indented, until a blank line or the next item.
                while index < len(lines) and lines[index][1].startswith(" ") and lines[index][1].strip():
                    continuation = lines[index][1].strip()
                    if BULLET.match(continuation) or ORDERED.match(continuation):
                        refuse(lines[index][0], "a nested list: the subset has one level")
                    text += " " + continuation
                    index += 1
                items.append((marker, text))
                # A blank line between two items keeps the list going.
                if (index + 1 < len(lines) and not lines[index][1].strip()
                        and (BULLET.match(lines[index + 1][1]) if kind == "bullet"
                             else ORDERED.match(lines[index + 1][1]))):
                    index += 1
            blocks.append(("list", kind, items))
            continue

        if line.startswith(" "):
            refuse(number, "an indented line outside a list item")

        paragraph: list[str] = []
        while index < len(lines):
            current = lines[index][1]
            if (not current.strip() or FENCE.match(current) or HEADING.match(current)
                    or current.strip().startswith(("|", ">", "<!--")) or BULLET.match(current)
                    or ORDERED.match(current)):
                break
            if re.match(r"^(=+|-+)\s*$", current):
                refuse(lines[index][0], "a setext heading: write it with #")
            paragraph.append(current.strip())
            index += 1
        blocks.append(("paragraph", join_lines(paragraph), number))

    return blocks


# ## Inline

# What groff names, rather than its code point: these render on every device, the ASCII one
# included, where a `\[uXXXX]` that the device has no glyph for draws nothing.
NAMED = {
    "\u2014": r"\(em", "\u2013": r"\(en", "\u2018": r"\(oq", "\u2019": r"\(cq",
    "\u201c": r"\(lq", "\u201d": r"\(rq", "\u00ab": r"\(Fo", "\u00bb": r"\(Fc",
    "\u00a0": r"\~", "\u202f": r"\~", "\u2026": r"\&.\|.\|.", "\u2192": r"\(->",
    "\u2190": r"\(<-", "\u2265": r"\(>=", "\u2264": r"\(<=", "\u00d7": r"\(mu",
    "\u2260": r"\(!=", "\u2022": r"\(bu", "\u00b7": r"\(pc", "\u25b8": r"\(->",
}


def is_cjk(char: str) -> bool:
    """A Han character, a Japanese or Korean letter, or a CJK or full-width sign: text in these
    scripts has no spaces, so groff has nowhere to break a line unless it is told."""
    return ("\u2e80" <= char <= "\u9fff" or "\uac00" <= char <= "\ud7af"
            or "\uf900" <= char <= "\ufaff" or "\uff00" <= char <= "\uffef")


# A sign that cannot start a line, and one that cannot end it.
NOT_LINE_START = "，。；：！？）、”’》」』】…—"
NOT_LINE_END = "（“‘《「『【"


def join_lines(lines: list[str]) -> str:
    """The lines of a paragraph as one: a space between them, except between two characters of
    a script written without spaces, where a space would show."""
    joined = lines[0]
    for line in lines[1:]:
        if joined and line and is_cjk(joined[-1]) and is_cjk(line[0]):
            joined += line
        else:
            joined += " " + line
    return joined


def escape(text: str, code: bool = False) -> str:
    """Text as troff reads it: backslashes doubled, non-ASCII escaped, and in code every
    hyphen a minus, so that an option is typed back as it reads."""
    out: list[str] = []
    for position, char in enumerate(text):
        if char == "\\":
            out.append(r"\e")
        elif char == "-" and code:
            out.append(r"\-")
        elif ord(char) < 128:
            out.append(char)
        elif char in NAMED:
            out.append(NAMED[char])
        else:
            out.append(f"\\[u{ord(char):04X}]")
            # `\:` is a break groff may take, with no width: between two CJK characters, unless
            # the sign that follows cannot start a line or the one before cannot end it.
            following = text[position + 1:position + 2]
            if (is_cjk(char) and following and is_cjk(following)
                    and following not in NOT_LINE_START and char not in NOT_LINE_END):
                out.append(r"\:")
    return "".join(out)


# A link, its text possibly holding code spans; an image is the same with a `!` in front.
LINK = re.compile(r"(!?)\[((?:[^\[\]`]|`[^`]*`)*)\]\(([^()\s]*)\)")

INLINE = re.compile(
    r"(?P<code>`+)"                          # a code span opens
    r"|(?P<strong>\*\*)"
    r"|(?P<em>\*)"
    r"|(?P<escaped>\\[!-/:-@\[-`{-~])"       # a backslash escape of punctuation
)


def inline(text: str, where: str, plain: bool = False) -> str:
    """A paragraph's text in troff. `plain` drops the fonts, for a heading or a NAME line."""

    def link_text(match: re.Match) -> str:
        if match.group(1):
            raise Unsupported(where, 0, f"an image: {match.group(0)[:60]}")
        return match.group(2)

    text = LINK.sub(link_text, text)
    out: list[str] = []
    bold = 0
    italic = 0
    current = "R"

    def font(code_span: bool = False) -> None:
        nonlocal current
        if plain:
            return
        wanted = ("B" if bold or code_span else "") + ("I" if italic else "") or "R"
        if wanted != current:
            out.append(f"\\f[{wanted}]")
            current = wanted

    position = 0
    while position < len(text):
        match = INLINE.search(text, position)
        if match is None:
            out.append(escape(text[position:]))
            break
        out.append(escape(text[position:match.start()]))
        position = match.end()
        if ticks := match.group("code"):
            end = text.find(ticks, position)
            if end == -1:
                raise Unsupported(where, 0, f"a code span that is never closed: {text[:60]}")
            span = text[position:end]
            if len(ticks) > 1:
                span = span.strip()
            font(code_span=True)
            out.append(escape(span, code=True))
            font()
            position = end + len(ticks)
        elif match.group("strong"):
            if bold:
                bold -= 1
            elif "**" in text[position:]:
                bold += 1
            else:
                raise Unsupported(where, 0, f"a ** that is never closed: {text[:60]}")
            font()
        elif match.group("em"):
            if italic:
                italic -= 1
            elif "*" in text[position:].replace("**", ""):
                italic += 1
            else:
                out.append("*")
                continue
            font()
        elif escaped := match.group("escaped"):
            out.append(escape(escaped[1]))
    if bold or italic:
        raise Unsupported(where, 0, f"emphasis that is never closed: {text[:60]}")
    if not plain and current != "R":
        out.append(r"\f[R]")
    return "".join(out)


def text_line(rendered: str) -> str:
    """A line of text that troff will not take for a request."""
    return "\\&" + rendered if rendered.startswith((".", "'")) else rendered


def quoted(argument: str) -> str:
    """A macro argument, quoted, its double quotes escaped."""
    return '"' + argument.replace('"', r"\(dq") + '"'


# ## Pages

def render_blocks(blocks: list[tuple], where: str, out: list[str]) -> None:
    for block in blocks:
        kind = block[0]
        if kind == "heading":
            _, level, title, number = block
            label = inline(title, f"{where}:{number}", plain=True)
            if level == 2:
                out.append(".SH " + quoted(upper(label)))
            else:
                out.append(".SS " + quoted(label))
        elif kind == "paragraph":
            _, text, number = block
            out.append(".PP")
            out.append(text_line(inline(text, f"{where}:{number}")))
        elif kind == "code":
            out.append(".PP")
            out.append(".RS 4")
            out.append(".EX")
            out.extend(text_line(escape(line, code=True)) for line in block[2])
            out.append(".EE")
            out.append(".RE")
        elif kind == "list":
            _, list_kind, items = block
            for marker, text in items:
                if list_kind == "bullet":
                    out.append(r".IP \(bu 2")
                else:
                    out.append(f".IP {marker}. 4")
                out.append(text_line(inline(text, where)))
        elif kind in ("quote", "caveat"):
            out.append(".RS 4")
            render_blocks(block[1], where, out)
            out.append(".RE")
        elif kind == "table":
            _, header, rows = block
            for row in rows:
                out.append(".TP")
                out.append(text_line(inline(row[0], where)))
                for position, cell in enumerate(row[1:], start=1):
                    if position > 1:
                        out.append(".br")
                    label = inline(header[position], where, plain=True)
                    value = inline(cell, where)
                    if len(row) > 2 and label:
                        out.append(text_line(f"{label}: {value}"))
                    else:
                        out.append(text_line(value))


def upper(rendered: str) -> str:
    """A section title in capitals, the escapes of its non-ASCII letters included."""
    def escaped(match: re.Match) -> str:
        return f"\\[u{ord(chr(int(match.group(1), 16)).upper()[0]):04X}]"
    capitals = re.sub(r"\\\[u([0-9A-F]{4})\]", escaped, rendered)
    # Only what is outside an escape: `\(em` stays `\(em`.
    return re.sub(r"(\\\[[^\]]*\]|\\\([A-Za-z<>=!\-]{2}|\\.)|([^\\]+)",
                  lambda m: m.group(1) or m.group(2).upper(), capitals)


def numbered(text: str) -> list[tuple[int, str]]:
    return list(enumerate(text.splitlines(), start=1))


def links_to(blocks: list[tuple], names: list[str]) -> list[str]:
    """The subcommands whose page a page links to, in the order of their first link."""
    found: list[str] = []

    def visit(text: str) -> None:
        for target in re.findall(r"\]\(([a-z0-9-]+)\.md(?:#[^)]*)?\)", text):
            if target in names and target not in found:
                found.append(target)

    for block in blocks:
        if block[0] == "paragraph":
            visit(block[1])
        elif block[0] == "list":
            for _, text in block[2]:
                visit(text)
        elif block[0] == "table":
            for row in block[2]:
                for cell in row:
                    visit(cell)
        elif block[0] in ("quote", "caveat"):
            for name in links_to(block[1], names):
                if name not in found:
                    found.append(name)
    return found


def first_sentence(blocks: list[tuple], where: str) -> str:
    for block in blocks:
        if block[0] == "paragraph":
            text = inline(block[1], where, plain=True)
            match = re.match(r"(.+?[.!?])(\s|$)", text)
            sentence = (match.group(1) if match else text).rstrip(".")
            # A NAME line reads as a phrase after its dash: "subedit-cli - the command-line tool".
            # An acronym or a name in capitals keeps them: only a capitalised word is lowered.
            word = sentence.split(" ", 1)[0]
            if len(word) == 1 or word[1:].islower():
                sentence = sentence[0].lower() + sentence[1:]
            return sentence
    raise Unsupported(where, 1, "no paragraph to name the page after")


def page(name: str, description: str, blocks: list[tuple], source: str, words: dict,
         version: str, manual_dir: str, manual_title: str, see_also: list[str]) -> str:
    out = [
        f".\\\" Generated by src/scripts/md-to-man.py from {escape(source)}; edit the manual, not this page.",
        f".TH {name.upper()} 1 \"\" \"subedit {version}\" {quoted(escape(words['manual_title']))}",
        # No hyphenation: a page names commands and options, which a hyphen would make untypable.
        ".nh",
        *([".na"] if any(is_cjk(char) for char in description + words["name"]) else []),
        ".SH " + quoted(escape(words["name"])),
        f"{escape(name, code=True)} \\- {description}",
    ]
    # The review caveat first in the page is not what the call looks like: it opens the description.
    caveats = []
    while blocks and blocks[0][0] == "caveat":
        caveats.append(blocks[0])
        blocks = blocks[1:]
    # A bare code block before any section is the form of the call: the synopsis.
    if blocks and blocks[0][0] == "code" and blocks[0][1] == "":
        out.append(".SH " + quoted(escape(words["synopsis"])))
        out.append(".nf")
        out.extend(text_line(escape(line, code=True)) for line in blocks[0][2])
        out.append(".fi")
        blocks = blocks[1:]
    blocks = caveats + blocks
    if blocks and blocks[0][0] != "heading":
        out.append(".SH " + quoted(escape(words["description"])))
    render_blocks(blocks, source, out)
    out.append(".SH " + quoted(escape(words["files"])))
    out.append(".TP")
    out.append(".I " + escape(manual_dir))
    out.append(text_line(manual_title))
    out.append(".SH " + quoted(escape(words["see_also"])))
    references = [f".BR {escape(other, code=True)} (1)" for other in see_also]
    out.extend(reference + ("," if position < len(references) - 1 else "")
               for position, reference in enumerate(references))
    # A fallback for every character the page escapes, used only on a device that has no glyph
    # for it: `man` under an ASCII locale, `groff` on its default device. Without it, troff warns
    # and draws nothing; with it, a ligature reads as its letters and a subscript as its digit.
    fallbacks = []
    for code in sorted(set(re.findall(r"\\\[u([0-9A-F]{4,6})\]", "\n".join(out)))):
        letters = "".join(c for c in unicodedata.normalize("NFKD", chr(int(code, 16))) if ord(c) < 128)
        fallbacks.append(f".if !c\\[u{code}] .char \\[u{code}] {escape(ASCII_FALLBACK.get(code, letters) or '?')}")
    out[3:3] = fallbacks
    return "\n".join(out) + "\n"


# What no decomposition gives in ASCII, for the fallbacks above.
ASCII_FALLBACK = {"2032": "'", "2033": "''", "2212": "-"}


def without_title(blocks: list[tuple]) -> list[tuple]:
    """The blocks after the `#` title, which the NAME section replaces."""
    if blocks and blocks[0][0] == "heading" and blocks[0][1] == 1:
        return blocks[1:]
    return blocks


def generate(manual: pathlib.Path, output_dir: pathlib.Path, version: str, manual_dir: str,
             tables: dict[str, dict], words_by_language: dict[str, dict]) -> list[pathlib.Path]:
    """Writes every page of a manual tree and returns their paths, in the order written."""
    invocation = manual / "invocation.md"
    index = manual / "index.md"
    for required in (invocation, index):
        if not required.is_file():
            raise Unsupported(required, 0, "missing: the man pages are generated from it")

    language = doc_vocabulary.language_of(invocation, invocation.read_text(encoding="utf-8"))
    if language not in tables or language not in words_by_language:
        raise Unsupported(invocation, 0, f"the language \"{language}\" has no vocabulary "
                                         f"(docs/i18n/{language}/vocabulary.json, or doc_vocabulary.py)")
    words = words_by_language[language]

    # Every page is parsed, the ones that give no man page included: the subset is the manual's.
    parsed: dict[str, list[tuple]] = {}
    for markdown in sorted(manual.glob("*.md")):
        parsed[markdown.stem] = parse_blocks(numbered(markdown.read_text(encoding="utf-8")), markdown)
        # Rendered into nothing, for its inline constructs: an image in a page that gives no man
        # page must stop the build as one in a page that does.
        render_blocks(parsed[markdown.stem], str(markdown), [])

    # The subcommands and their descriptions: the first table of `invocation.md` headed by the
    # language's subcommand word, as `check-cli-manual.py` reads it.
    subcommands: list[tuple[str, str]] = []
    for block in parsed["invocation"]:
        if block[0] == "table" and block[1][0] == tables[language]["subcommand_header"]:
            for row in block[2]:
                if match := re.match(r"^\[?`([a-z][a-z0-9-]*)`\]?", row[0]):
                    subcommands.append((match.group(1), inline(row[1], str(invocation), plain=True)))
            break
    if not subcommands:
        raise Unsupported(invocation, 0, "no subcommand table: the pages cannot be named")
    names = [name for name, _ in subcommands]

    index_blocks = without_title(parsed["index"])
    manual_title = inline(parsed["index"][0][2], str(index), plain=True) \
        if parsed["index"] and parsed["index"][0][0] == "heading" else "subedit-cli"
    def relative(path: pathlib.Path) -> str:
        """The page's path from `docs/`, the same wherever the tree is: the output names it."""
        match = re.search(r"(?:^|/)(docs/.*)$", path.as_posix())
        return match.group(1) if match else path.name

    output_dir.mkdir(parents=True, exist_ok=True)
    written: list[pathlib.Path] = []

    top = page("subedit-cli", first_sentence(index_blocks, str(index)),
               without_title(parsed["invocation"]), relative(invocation), words, version,
               manual_dir, manual_title,
               [f"subedit-cli-{name}" for name in names] + ["subedit-gui"])
    written.append(output_dir / "subedit-cli.1")
    written[-1].write_text(top, encoding="ascii")

    for name, description in subcommands:
        source = manual / f"{name}.md"
        if name not in parsed:
            raise Unsupported(source, 0, f"missing: `{name}` is in the subcommand table of invocation.md")
        blocks = without_title(parsed[name])
        see_also = ["subedit-cli"] + [f"subedit-cli-{other}" for other in links_to(blocks, names)
                                      if other != name]
        text = page(f"subedit-cli-{name}", description, blocks, relative(source),
                    words, version, manual_dir, manual_title, see_also)
        written.append(output_dir / f"subedit-cli-{name}.1")
        written[-1].write_text(text, encoding="ascii")

    for stale in output_dir.glob("subedit-cli*.1"):
        if stale not in written:
            stale.unlink()
    return written


def check_fixtures() -> int:
    """Replays `src/test/data/man`: each tree gives the pages written by hand, byte for byte,
    and each edit outside the subset is refused, naming the page and the construct."""
    failures: list[str] = []
    tables = doc_vocabulary.load_tables(None)
    words = doc_vocabulary.load_man_words(None)
    trees = (("docs/manual/subedit-cli", "man1", "/usr/share/subedit/manual"),
             ("docs/i18n/fr/manual/subedit-cli", "fr/man1", "/usr/share/subedit/manual/fr"))
    with tempfile.TemporaryDirectory() as scratch:
        for tree, subdir, manual_dir in trees:
            output = pathlib.Path(scratch) / subdir
            try:
                generate(FIXTURES / tree, output, "1.2.3", manual_dir, tables, words)
            except Unsupported as error:
                failures.append(f"{tree}: refused: {error}")
                continue
            expected_dir = FIXTURES / "expected" / subdir
            written = sorted(page.name for page in output.glob("*.1"))
            expected = sorted(page.name for page in expected_dir.glob("*.1"))
            if written != expected:
                failures.append(f"{subdir}: wrote {written}, expected {expected}")
            for name in sorted(set(written) & set(expected)):
                got = (output / name).read_text(encoding="ascii").splitlines()
                want = (expected_dir / name).read_text(encoding="ascii").splitlines()
                for number, (line, wanted) in enumerate(zip(got, want), start=1):
                    if line != wanted:
                        failures.append(f"{subdir}/{name}:{number}: wrote {line!r}, expected {wanted!r}")
                        break
                else:
                    if len(got) != len(want):
                        failures.append(f"{subdir}/{name}: {len(got)} lines, expected {len(want)}")

        cases = json.loads((FIXTURES / "refused.json").read_text(encoding="utf-8"))
        for number, case in enumerate(cases):
            copy = pathlib.Path(scratch) / f"case-{number}"
            shutil.copytree(FIXTURES / "docs/manual/subedit-cli", copy)
            page_path = copy / case["page"]
            if case.get("delete"):
                page_path.unlink()
            else:
                page_path.write_text(page_path.read_text(encoding="utf-8") + case["append"], encoding="utf-8")
            try:
                generate(copy, pathlib.Path(scratch) / f"out-{number}", "1.2.3", "/x", tables, words)
            except Unsupported as error:
                message = str(error)
                if case["expect"] not in message or case["reason"] not in message:
                    failures.append(f"{case['name']}: refused for another reason: {message}")
                continue
            failures.append(f"{case['name']}: let through")

    if failures:
        for failure in failures:
            print(failure, file=sys.stderr)
        print(f"\n{len(failures)} man page fixture(s) failed", file=sys.stderr)
        return 1
    print(f"the man page fixtures hold: {len(trees)} trees, {len(cases)} refusals")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--manual", type=pathlib.Path,
                        help="a subedit-cli manual tree: docs/manual/subedit-cli or a translation")
    parser.add_argument("--output-dir", type=pathlib.Path)
    parser.add_argument("--version", help="the version the pages announce")
    parser.add_argument("--manual-dir",
                        help="where the manual is installed, which the FILES section names")
    parser.add_argument("--check-fixtures", action="store_true",
                        help="replay the cases of src/test/data/man, and nothing else")
    parser.add_argument("--vocabulary", type=pathlib.Path,
                        help="a JSON file {language: table} that adds to the shipped tables")
    arguments = parser.parse_args()

    if arguments.check_fixtures:
        return check_fixtures()
    if None in (arguments.manual, arguments.output_dir, arguments.version, arguments.manual_dir):
        parser.error("--manual, --output-dir, --version and --manual-dir are required")

    try:
        written = generate(arguments.manual, arguments.output_dir, arguments.version,
                           arguments.manual_dir, doc_vocabulary.load_tables(arguments.vocabulary),
                           doc_vocabulary.load_man_words(arguments.vocabulary))
    except Unsupported as error:
        print(f"md-to-man.py: {error}", file=sys.stderr)
        return 1
    print(f"{len(written)} man page(s) written to {arguments.output_dir}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
