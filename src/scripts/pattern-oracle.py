#!/usr/bin/env python3
r"""What Gaupol does with its correction patterns — issue #494.

**The pattern oracle, written in Python because the patterns are.** The files of
`packaging/patterns/` are Gaupol's, written for the `re` module; no C++ engine
reads them as they are, and the one phase 12 chooses will have to prove that it
corrects as Gaupol corrects. This script writes what Gaupol makes of them, case
by case, into `attendus/`; the engine's C++ tests read these expected results
without ever running Python.

**It is not `re.sub`, and that is this script's whole reason to exist.** Gaupol
replaces with a `Finder` that looks for each match **in the already modified
text**, from the end of the last replacement, with its own rule for empty
matches. An oracle that called `re.sub` would only have proven its agreement
with our reading — the lesson of #338.

**The difference is measured, and it is narrow.** Over the 72 records, tens of
thousands of random texts show none for common errors and capitalization:
`Repeat` reruns a pattern until it changes nothing more, and catches what a
`re.sub` would have left. It appears in the cleanup that follows the removal of
mentions, where `^\W*$` and blank lines are removed one after the other; the
two cases that show it are in `hearing-impaired.entrees`.

What the script reproduces, read in Gaupol and named at each place:

- the loading of the files (`aeidon/patternman.py`): whole-line comments,
  stripped lines, `\0` not followed by a digit removed, enablement read from
  the `.conf` files of the same code;
- the cascade of codes, `SkipIn` and `Policy=Replace` (`get_patterns`,
  `_filter_patterns`);
- the search and the replacement (`aeidon/finder.py`);
- the three operations (`aeidon/agents/text.py`): `correct_common_errors`,
  `capitalize`, `remove_hearing_impaired` and its cleanups.

**Most texts have no tags**, and for them Gaupol's parser leaves nothing: it
removes what it did not find, and puts it back in the same place. Where a
correction touches a tag, issue #501 ports Gaupol's parser (`aeidon/parser.py`)
restricted to SubRip tags — the job of phase 10's tag parser, tested
elsewhere, is to do as well.

It **never** reads the Gaupol clone: only the tracked copies, which makes it
replayable on a machine that does not have it.

    ./src/scripts/pattern-oracle.py --write   # write the expected results
    ./src/scripts/pattern-oracle.py --check   # refuse an expected result that drifted
"""

import argparse
import re
import sys
from collections.abc import Callable
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
PATTERNS_ROOT = REPO_ROOT / "src" / "test" / "data" / "motifs"
GAUPOL = REPO_ROOT / "packaging" / "patterns"
INPUTS = PATTERNS_ROOT / "entrees"
EXPECTED = PATTERNS_ROOT / "attendus"

TYPES = ("common-error", "capitalization", "hearing-impaired", "line-break")

# The languages Gaupol ships patterns for: each has its cascade, otherwise
# `SkipIn` and `Policy=Replace` would never be tested. The mentions ship all
# disabled, hence their `tous` ("all") cascade.
REQUIRED_CASCADES = {
    "common-error": ("Latn-en", "Latn-en-US", "Latn-fr", "Latn-fi"),
    "capitalization": ("Latn-en",),
    "hearing-impaired": ("Latn-en tous",),
    "line-break": ("Latn-en",),
}

# What each type calls a changed text, in the words of the input files: a line
# break `coupe` (cuts), it does not `corrige` (correct).
CHANGED_WORD = {
    "common-error": "corrige",
    "capitalization": "corrige",
    "hearing-impaired": "corrige",
    "line-break": "coupe",
}

# An input text, or the removed subtitle an emptying leaves: `None`.
Texts = list[str | None]


class Refusal(Exception):
    """An input that cannot be read."""


# --- The loading: aeidon/patternman.py ------------------------------------------


class Pattern:
    """A record of a pattern file, and the code that carries it."""

    def __init__(self, code: str, rank: int) -> None:
        self.code = code
        self.rank = rank
        self.fields: dict[str, str] = {}
        # `Pattern.enabled` is true as long as a `.conf` says nothing.
        self.enabled = True

    def name(self) -> str:
        return self.fields.get("Name", "")

    def field_list(self, field: str) -> list[str]:
        return [x for x in self.fields.get(field, "").split(";") if x]

    def flags(self) -> int:
        # `Pattern.get_flags`: `getattr(re, name)` for each listed name.
        value = 0
        for name in self.field_list("Flags"):
            value |= getattr(re, name)
        return value


def read_patterns(kind: str) -> dict[str, list[Pattern]]:
    """All the patterns of `kind`, by code, in file order."""
    by_code: dict[str, list[Pattern]] = {}
    for path in sorted(GAUPOL.glob(f"*.{kind}")):
        code = path.name[: -len(f".{kind}")]
        patterns = by_code.setdefault(code, [])
        content = path.read_text(encoding="utf-8").replace("\r\n", "\n")
        for line in content.split("\n"):
            # `_re_comment` only removes a *whole* comment line: the `#` of
            # music notes, in the middle of a pattern, stays.
            line = re.sub(r"^\s*#.*$", "", line).strip()
            if not line:
                continue
            if line.startswith("["):
                patterns.append(Pattern(code, len(patterns) + 1))
                continue
            name, value = line.split("=", 1)
            name = name[1:] if name.startswith("_") else name
            # Gaupol's issue #70: `\0` protects what GKeyFile and msgfmt
            # would "repair", and disappears on reading.
            patterns[-1].fields[name] = re.sub(r"\\0(?!\d)", "", value)
        config = path.with_name(path.name + ".conf")
        if config.exists():
            entries = re.findall(
                r'<pattern name="([^"]*)" enabled="(true|false)"/>',
                config.read_text(encoding="utf-8"),
            )
            for name, enabled in entries:
                name = name.replace("&quot;", '"').replace("&amp;", "&")
                for pattern in patterns:
                    if pattern.name() == name:
                        pattern.enabled = enabled == "true"
    return by_code


def codes_of(request: str) -> list[str]:
    """`_get_codes`: `Zyyy`, then the script, the language, the country."""
    parts = request.split("-")
    return ["Zyyy"] + ["-".join(parts[: i + 1]) for i in range(len(parts))]


def cascade(by_code: dict[str, list[Pattern]], request: str) -> list[Pattern]:
    """`get_patterns` then `_filter_patterns`, for the code `request`."""
    codes = codes_of(request)
    patterns = []
    for code in codes:
        for pattern in by_code.get(code, []):
            if set(pattern.field_list("SkipIn")) & set(codes):
                continue
            patterns.append(pattern)
    filtered: list[Pattern | None] = []
    for pattern in patterns:
        last = len(filtered) - 1
        for j, kept in enumerate(filtered):
            if kept is not None and kept.name() == pattern.name():
                last = j
                if pattern.fields.get("Policy") == "Replace":
                    filtered[j] = None
        filtered.insert(last + 1, pattern)
        filtered = [x for x in filtered if x is not None]
    return [x for x in filtered if x is not None]


# --- The search and the replacement: aeidon/finder.py ---------------------------


class Finder:
    """`aeidon.Finder`, for a regular expression, ported line by line."""

    def __init__(self, text: str) -> None:
        self.text = text
        self.pattern: re.Pattern[str] | None = None
        self.match: re.Match[str] | None = None
        self.match_span: tuple[int, int] | None = None
        self.pos: int | None = None
        self.replacement = ""

    def set_regex(self, pattern: str, flags: int = re.DOTALL | re.MULTILINE) -> None:
        self.pattern = re.compile(pattern, flags)

    def next(self) -> tuple[int, int]:
        assert self.pattern is not None
        if self.pos is None:
            self.pos = 0
        match = self.pattern.search(self.text, self.pos)
        if match is None:
            raise StopIteration
        # « Avoid getting stuck with zero-length regular expressions. »
        if match.span() == self.match_span == (self.pos, self.pos):
            if self.pos == len(self.text):
                raise StopIteration
            self.pos += 1
            return self.next()
        self.match = match
        self.match_span = match.span()
        self.pos = self.match_span[1]
        return self.match_span

    def replace(self) -> None:
        assert self.match is not None and self.match_span is not None
        a, z = self.match_span
        length = len(self.text)
        self.text = self.text[:a] + self.match.expand(self.replacement) + self.text[z:]
        self.pos = z + len(self.text) - length
        if self.match_span[0] == self.match_span[1]:
            self.match_span = (self.pos, self.pos)

    def replace_all(self) -> int:
        self.pos = 0
        self.match = None
        self.match_span = None
        count = 0
        while True:
            try:
                self.next()
            except StopIteration:
                self.pos = len(self.text)
                self.match_span = None
                break
            self.replace()
            count += 1
        return count


# --- The tag parser: aeidon/parser.py, restricted to SubRip — issue #501 -------

# `SubRip.tag` (aeidon/markups/subrip.py): any tag between angle brackets,
# case ignored. It is the only markup the oracle ports — the 184 cases of #494
# had none, and the framing of #501 restricts the port to SubRip.
TAG = re.compile(r"<.*?>", re.DOTALL | re.MULTILINE | re.IGNORECASE)


def subrip_clean(text: str) -> str:
    """`SubRip.clean`: the four tag cleanups, in its order."""
    flags = re.DOTALL | re.MULTILINE | re.IGNORECASE
    text = re.sub(r"<([a-z]+)[^<]*?>( *)</\1>", r"\2", text, flags=flags)
    text = re.sub(r"</([a-z]+)>( *)<\1>", r"\2", text, flags=flags)
    text = re.sub(r" ?(<(?!/)[^>]+?>) ", r" \1", text, flags=flags)
    text = re.sub(r" (</[^>]+?>) ?", r"\1 ", text, flags=flags)
    return text


class Parser(Finder):
    """`aeidon.Parser`, ported for SubRip tags — issue #501.

    Holds the tags apart from the visible text (`self.text`, inherited from
    `Finder`) while a correction searches and replaces it, then puts them back.
    **A margin** — an identical tag at the head and the tail of every line,
    like an `<i>` around every line of a subtitle — is handled apart: it wraps
    the whole text rather than following each replacement, exactly as
    `_set_margins` decides.
    """

    def __init__(self, text: str) -> None:
        self._margins: list[str] = []
        self._tags: list[list] = []
        if "\n" in text:
            self._set_margins(text)
        if not self._margins:
            self._set_tags(text)
        super().__init__(TAG.sub("", text))

    def _set_margins(self, text: str) -> None:
        """`_set_margins`: a head tag and a tail tag common to every line,
        and nothing else between them."""
        lines = text.split("\n")
        line = lines[0]
        start_tag = ""
        while True:
            match = TAG.match(line)
            if match is None:
                break
            a, z = match.span()
            start_tag += line[a:z]
            line = line[z:]
        if not start_tag:
            return
        if not all(x.startswith(start_tag) for x in lines):
            return
        end_tag = ""
        while True:
            match = None
            for match in TAG.finditer(line):
                pass
            if match is None:
                break
            a, z = match.span()
            if z != len(line):
                return
            end_tag = line[a:z] + end_tag
            line = line[:a]
        if not all(x.endswith(end_tag) for x in lines):
            return
        for line in (x[len(start_tag) : -len(end_tag)] for x in lines):
            if TAG.search(line) is not None:
                return
        self._margins = [start_tag, end_tag]

    def _set_tags(self, text: str) -> None:
        """`_set_tags`: each tag, its position in the visible text."""
        for match in TAG.finditer(text):
            a, z = match.span()
            self._tags.append([a, text[a:z]])

    def _shift_tags(self, pos: int, shift: int, orig_text: str) -> None:
        """`_shift_tags`, ported as is: shifts the tags after `pos`."""
        if not shift:
            return
        if not self._tags:
            return
        opening = True
        if pos < len(orig_text):
            if orig_text[pos].isspace():
                opening = False
        elif pos == len(orig_text):
            opening = False
        closing = not opening
        pos_with_tags = pos
        for tag_pos, tag in self._tags:
            if shift > 0 and closing:
                if tag_pos < pos_with_tags:
                    pos_with_tags += len(tag)
            elif tag_pos <= pos_with_tags:
                pos_with_tags += len(tag)
        between_length = 0
        for i, (tag_pos, tag) in enumerate(self._tags):
            orig_end = pos_with_tags - shift + between_length
            if shift < 0 and pos_with_tags < tag_pos < orig_end:
                self._tags[i][0] = pos_with_tags + between_length
                between_length += len(tag)
            elif tag_pos >= pos_with_tags:
                self._tags[i][0] += shift

    def replace(self) -> None:
        """`Parser.replace`: `Finder`'s, then the shift of the tags."""
        assert self.match_span is not None
        a = self.match_span[0]
        orig_text = self.text
        super().replace()
        self._shift_tags(a, len(self.text) - len(orig_text), orig_text)

    def get_text(self) -> str:
        """`get_text`: the tags put back, the margin if there is one, cleaned."""
        text = self.text
        for pos, tag in self._tags:
            text = text[:pos] + tag + text[pos:]
        if self._margins:
            text = text.replace("\n", f"{self._margins[1]}\n{self._margins[0]}")
            text = self._margins[0] + text + self._margins[1]
        return subrip_clean(text)


# --- The three operations: aeidon/agents/text.py --------------------------------


def correct_common_errors(texts: list[str], patterns: list[Pattern]) -> Texts:
    """`correct_common_errors`: in order, and `Repeat` loops again."""
    corrected: Texts = []
    for text in texts:
        finder = Parser(text)
        for pattern in patterns:
            finder.set_regex(pattern.fields["Pattern"], pattern.flags())
            finder.replacement = pattern.fields.get("Replacement", "")
            count = finder.replace_all()
            while pattern.fields.get("Repeat") == "True" and count:
                count = finder.replace_all()
        corrected.append(finder.get_text())
    return corrected


CAPITALIZABLE = re.compile(r"^\W*(?<!\.\.\.)(?<!…)\w")


def capitalize_first(finder: Finder, pos: int) -> bool:
    """`_capitalize_first`: the first word character from `pos`."""
    match = CAPITALIZABLE.search(finder.text[pos:])
    if match is not None:
        i = pos + match.end() - 1
        text = finder.text
        finder.text = text[:i] + text[i : i + 1].capitalize() + text[i + 1 :]
    return match is not None


def capitalize_matches(finder: Finder, pattern: Pattern, cap_next: bool) -> bool:
    """`_capitalize_text` — recursive in Gaupol, iterative here, same walk."""
    while True:
        try:
            a, z = finder.next()
        except StopIteration:
            return cap_next
        if pattern.fields.get("Capitalize") == "Start":
            capitalize_first(finder, a)
        if pattern.fields.get("Capitalize") == "After":
            cap_next = not capitalize_first(finder, z)


def capitalize(texts: list[str], patterns: list[Pattern]) -> Texts:
    """`capitalize` over consecutive subtitles, the first of index 0."""
    capitalized: Texts = []
    cap_next = False
    for index, text in enumerate(texts):
        finder = Parser(text)
        if cap_next or index == 0:
            capitalize_first(finder, 0)
            cap_next = False
        for pattern in patterns:
            finder.set_regex(pattern.fields["Pattern"], pattern.flags())
            finder.pos = 0
            cap_next = capitalize_matches(finder, pattern, cap_next)
        capitalized.append(finder.get_text())
    return capitalized


LEFTOVERS = (
    (r"(^\s+|\s+$)", ""),
    (r" {2,}", " "),
    (r"^\W*$", ""),
    (r"(^\n|\n$)", ""),
    (r"^([\-\–\—])(\S)", r"\1 \2"),
    (r"^[\-\–\—] (.*?^[^\-\–\—])", r"\1"),
    (r"\A[\-\–\—] ([^\n]*)\Z", r"\1"),
)


def remove_hearing_impaired(texts: list[str], patterns: list[Pattern]) -> Texts:
    """`remove_hearing_impaired`: `None` for a removed subtitle.

    **The cleanups of `_remove_leftover_hi` only touch the texts the patterns
    changed** — a text left untouched keeps its double spaces.

    **Two parsers, as `_remove_leftover_hi` uses a second one** — issue #501:
    the cleanups reread the text already recomposed by the first, tags put
    back, rather than going on with the same one. On a text without tags, both
    come to the same.
    """
    removed: Texts = []
    for text in texts:
        finder = Parser(text)
        for pattern in patterns:
            finder.set_regex(pattern.fields["Pattern"], pattern.flags())
            finder.replacement = pattern.fields.get("Replacement", "")
            finder.replace_all()
        corrected = finder.get_text()
        if corrected == text:
            removed.append(text)
            continue
        cleaner = Parser(corrected)
        for expression, replacement in LEFTOVERS:
            cleaner.set_regex(expression)
            cleaner.replacement = replacement
            cleaner.replace_all()
        # `remove_blank`, which the assistant checks by default.
        removed.append(cleaner.get_text() or None)
    return removed


# --- Line breaking: aeidon/liner.py ---------------------------------------------

# The three penalties of `aeidon/test/test_liner.py`, which its five cases use:
# the author's intent, written by him, beside the ones Gaupol ships.
TEST_PENALTIES = (
    (r"( )- ", re.DOTALL | re.MULTILINE, 1, -1000.0),
    (r"[,.;:!?]( )", re.DOTALL | re.MULTILINE, 1, -100.0),
    (r"\b(by|the|into|a)( )", re.DOTALL | re.MULTILINE, 2, 1000.0),
)

# A penalty: the expression, the group that holds the space, the value.
Penalty = tuple[re.Pattern[str], int, float]


def penalties_of(patterns: list[Pattern]) -> list[Penalty]:
    """`_get_penalties` then `Liner.set_penalties`."""
    return [
        (re.compile(x.fields["Pattern"], x.flags()), int(x.fields["Group"]), float(x.fields["Penalty"]))
        for x in patterns
    ]


class Liner:
    """`aeidon.Liner`, ported line by line, in character units.

    `_list_possible_breaks` is memoised in Gaupol by the object's identity;
    here by its arguments, for a single breaking: the settings do not change
    during a call, and the result is the same.
    """

    def __init__(self, penalties: list[Penalty], max_length: int, max_lines: int) -> None:
        self.penalties = penalties
        self.max_length = max_length
        self.max_lines = max_lines
        self.memo: dict[tuple[tuple[str, ...], tuple[float, ...], int], list[int]] = {}

    @staticmethod
    def boxes_to_lines(boxes: list[str], breaks: list[int]) -> list[str]:
        edges = [0] + [x + 1 for x in breaks] + [len(boxes)]
        return [" ".join(boxes[edges[i] : edges[i + 1]]) for i in range(len(edges) - 1)]

    def demerit(self, boxes: list[str], penalties: list[float], breaks: list[int]) -> float:
        """`_calculate_demerit`, term by term."""
        nlines = len(breaks) + 1
        chosen = [penalties[i] for i in breaks]
        lengths = [len(x) for x in self.boxes_to_lines(boxes, breaks)]
        mean = sum(lengths) / len(lengths)
        width = self.max_length
        return (
            sum(chosen)
            + 50 * sum(((x - mean) / width) ** 2 for x in lengths)
            + 50
            * sum(
                ((lengths[i] - lengths[i + 1]) / width) ** 2
                for i in range(len(lengths) - 1)
                if lengths[i] > lengths[i + 1]
            )
            + 100 * (nlines - 1) ** 3
            + 1000 * max(0, nlines - self.max_lines) ** 3
        )

    def detect_penalties(self, finder: Finder, boxes: list[str]) -> list[float]:
        """`_detect_penalties`: each space gets the sum of its penalties."""
        text = " ".join(boxes)
        at_space = [0.0] * len(text)
        for regex, group, value in self.penalties:
            finder.pattern = regex
            finder.pos = 0
            while True:
                try:
                    finder.next()
                except StopIteration:
                    break
                assert finder.match is not None
                at_space[finder.match.span(group)[0]] += value
        penalties = [0.0] * len(boxes)
        pos = -1
        for i in range(len(boxes) - 1):
            pos = pos + 1 + len(boxes[i])
            penalties[i] = at_space[pos]
        return penalties

    def possible_breaks(self, boxes: list[str], penalties: list[float], nlines: int) -> list[int]:
        """`_list_possible_breaks`, sorted by increasing penalty."""
        key = (tuple(boxes), tuple(penalties), nlines)
        if key not in self.memo:
            self.memo[key] = self._possible_breaks(boxes, penalties, nlines)
        return self.memo[key]

    def _possible_breaks(self, boxes: list[str], penalties: list[float], nlines: int) -> list[int]:
        breaks = list(range(len(boxes) - (nlines - 1)))
        breakpen = penalties[: len(breaks)]
        if nlines == 1:
            return []
        keep = [False] * len(breaks)
        if nlines == 2:
            for i in range(len(breaks)):
                keep[i] = max(len(x) for x in self.boxes_to_lines(boxes, [i])) <= self.max_length
        else:
            for i in range(len(breaks)):
                if len(self.boxes_to_lines(boxes, [i])[0]) > self.max_length:
                    break
                keep[i] = bool(self.possible_breaks(boxes[i + 1 :], penalties[i + 1 :], nlines - 1))
        kept = [(breakpen[i], breaks[i]) for i in range(len(breaks)) if keep[i]]
        return [b for _, b in sorted(kept)]

    def break_among(
        self, boxes: list[str], penalties: list[float], nlines: int
    ) -> tuple[list[int] | None, float]:
        """`_break_lines`: the breaks and their demerit, or nothing valid."""
        breaks = self.possible_breaks(boxes, penalties, nlines)
        best: list[int] | None = None
        best_demerit = float(sys.maxsize)
        if len(" ".join(boxes)) <= self.max_length:
            # A single valid line is the yardstick to beat.
            best = []
            best_demerit = self.demerit(boxes, penalties, [])
        if nlines == 1:
            return best, best_demerit
        if nlines == 2:
            for i in breaks:
                if penalties[i] > best_demerit:
                    break
                demerit = self.demerit(boxes, penalties, [i])
                if demerit < best_demerit:
                    best, best_demerit = [i], demerit
            return best, best_demerit
        for i in breaks:
            negative = sorted(x for x in penalties[i + 1 :] if x < 0)
            negative_sum = sum(negative[: min(len(negative), nlines - 2)])
            if penalties[i] + negative_sum > best_demerit:
                break
            later, _ = self.break_among(boxes[i + 1 :], penalties[i + 1 :], nlines - 1)
            if later is None:
                continue
            candidate = [i] + [i + 1 + x for x in later]
            demerit = self.demerit(boxes, penalties, candidate)
            if demerit < best_demerit:
                best, best_demerit = candidate, demerit
        return best, best_demerit

    def break_lines(self, text: str) -> str:
        """`set_text` then `break_lines`.

        **Nothing is broken until the number of lines tried reaches
        `max_lines`**: a text too short in words to reach it is returned as is,
        whether or not it fits in the length.
        """
        finder = Finder(text.strip())
        finder.text = finder.text.replace("\n", " ")
        finder.set_regex(" {2,}")
        finder.replacement = " "
        finder.replace_all()
        boxes = finder.text.split(" ")
        if len(boxes) == 1:
            return finder.text
        penalties = self.detect_penalties(finder, boxes)
        best: list[int] | None = None
        best_demerit = float(sys.maxsize)
        for nlines in range(min(2, self.max_lines), min(10, len(boxes)) + 1):
            breaks, demerit = self.break_among(boxes, penalties, nlines)
            if breaks is None:
                continue
            if demerit < best_demerit:
                best, best_demerit = breaks, demerit
            if nlines < self.max_lines:
                continue
            assert best is not None
            result = finder.text
            pos = -1
            for i in range(len(boxes)):
                pos = pos + 1 + len(boxes[i])
                if i in best:
                    result = result[:pos] + "\n" + result[pos + 1 :]
            return result
        return finder.text


def line_break_settings(target: str, where: str) -> tuple[str, int, int]:
    """`Latn-en:2 24/3`, `cascade Latn-en 24/3`, `essai 40/3` (test), `aucune 24/2` (none)."""
    selection, _, settings = target.rpartition(" ")
    length, _, lines = settings.partition("/")
    if not selection or not length.isdigit() or not lines.isdigit():
        raise Refusal(f"{where}: a line-break target ends with \"length/lines\"")
    return selection, int(length), int(lines)


def line_break_penalties(by_code: dict[str, list[Pattern]], selection: str, where: str) -> list[Penalty]:
    if selection == "aucune":
        return []
    if selection == "essai":
        return [(re.compile(p, f), g, v) for p, f, g, v in TEST_PENALTIES]
    return penalties_of(patterns_of(by_code, selection, where))


def expected_line_breaks() -> tuple[str, list[str]]:
    """The `.cas` of the line breaks, and what is wrong in its inputs.

    **Each pattern must decide at least one case**: its breaking differs from
    the one obtained with no penalty at all, under the same settings. Otherwise
    an engine that ignored it would pass all the same.
    """
    kind = "line-break"
    by_code = read_patterns(kind)
    lines = [
        "# Generated by src/scripts/pattern-oracle.py — do not edit by hand.",
        "# What Gaupol does with the \"line-break\" patterns: see ../LISEZMOI.md.",
        "",
    ]
    decisive: set[str] = set()
    cascades = set()
    problems = []
    for case in read_cases(kind):
        selection, length, count = line_break_settings(case.target, case.where)
        penalties = line_break_penalties(by_code, selection, case.where)
        outputs = [Liner(penalties, length, count).break_lines(t) for t in case.texts]
        changed = outputs != case.texts
        if (case.expectation == "coupe") != changed:
            what = "changes" if changed else "does not change"
            problems.append(f"{case.where}: announced \"{case.expectation}\", and the text {what}")
        if selection.startswith("cascade "):
            cascades.add(selection.split(" ", 1)[1])
        elif selection not in ("aucune", "essai"):
            bare = [Liner([], length, count).break_lines(t) for t in case.texts]
            if bare != outputs:
                decisive.add(selection)
        lines.extend(cas_lines(case, outputs))
    for code, patterns in by_code.items():
        for pattern in patterns:
            if f"{code}:{pattern.rank}" not in decisive:
                problems.append(
                    f"{kind}.entrees: \"{code}:{pattern.rank}\" ({pattern.name()}) "
                    "decides no case — its breaking is that of no penalty"
                )
    for request in REQUIRED_CASCADES[kind]:
        if request not in cascades:
            problems.append(f"{kind}.entrees: no cascade \"{request}\"")
    return "\n".join(lines) + "\n", problems


OPERATIONS: dict[str, Callable[[list[str], list[Pattern]], Texts]] = {
    "common-error": correct_common_errors,
    "capitalization": capitalize,
    "hearing-impaired": remove_hearing_impaired,
}


# --- The inputs and the expected results ----------------------------------------


def quoted_texts(field: str, where: str) -> list[str]:
    """One or more quoted texts, separated by `;`.

    Several texts are consecutive subtitles: capitalization carries its state
    from one to the next, and it is the only way to test it.
    """
    texts = []
    i = 0
    while True:
        while i < len(field) and field[i] == " ":
            i += 1
        if i >= len(field) or field[i] != '"':
            raise Refusal(f"{where}: a text is written between double quotes")
        text = ""
        i += 1
        while True:
            if i >= len(field):
                raise Refusal(f"{where}: a double quote is not closed")
            if field[i] == '"':
                i += 1
                break
            if field[i] == "\\":
                i += 1
                escaped = field[i] if i < len(field) else ""
                if escaped not in ("n", "t", '"', "\\"):
                    raise Refusal(f"{where}: unknown escape \"\\{escaped}\"")
                text += {"n": "\n", "t": "\t"}.get(escaped, escaped)
            else:
                text += field[i]
            i += 1
        texts.append(text)
        while i < len(field) and field[i] == " ":
            i += 1
        if i >= len(field):
            return texts
        if field[i] != ";":
            raise Refusal(f"{where}: two texts are separated by \";\"")
        i += 1


def escaped(text: str) -> str:
    return (
        text.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t")
    )


class Case:
    """A line of an input file."""

    def __init__(self, where: str, target: str, expectation: str, label: str, texts: list[str]) -> None:
        self.where = where
        self.target = target
        self.expectation = expectation
        self.label = label
        self.texts = texts


def read_cases(kind: str) -> list[Case]:
    """The cases written by hand for `kind`."""
    path = INPUTS / f"{kind}.entrees"
    cases = []
    for number, line in enumerate(path.read_text(encoding="utf-8").split("\n"), 1):
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        where = f"{path.name}:{number}"
        fields = [x.strip() for x in line.split("|", 3)]
        if len(fields) != 4:
            raise Refusal(f"{where}: four fields — target | expectation | label | texts")
        target, expectation, label, texts = fields
        # The alignment spaces are not part of the target.
        target = " ".join(target.split())
        if expectation not in (CHANGED_WORD[kind], "intact"):
            raise Refusal(f"{where}: the expectation is \"{CHANGED_WORD[kind]}\" or \"intact\"")
        cases.append(Case(where, target, expectation, label, quoted_texts(texts, where)))
    return cases


def patterns_of(by_code: dict[str, list[Pattern]], target: str, where: str) -> list[Pattern]:
    """What a target designates.

    `Latn-en:3` — a single record, enabled or not; `cascade Latn-en` — the
    whole cascade, as Gaupol enables it by default; `cascade Latn-en tous` —
    the same, everything checked, which the mentions need.
    """
    if target.startswith("cascade "):
        request, _, everything = target.split(" ", 1)[1].partition(" ")
        if everything not in ("", "tous"):
            raise Refusal(f"{where}: \"cascade <code>\" or \"cascade <code> tous\"")
        patterns = [x for x in cascade(by_code, request) if everything or x.enabled]
        if not patterns:
            raise Refusal(f"{where}: no enabled pattern for \"{request}\"")
        return patterns
    code, _, rank = target.partition(":")
    if not rank.isdigit() or code not in by_code or not 1 <= int(rank) <= len(by_code[code]):
        raise Refusal(f"{where}: no record \"{target}\"")
    return [by_code[code][int(rank) - 1]]


def cas_lines(case: "Case", outputs: Texts) -> list[str]:
    """The `.cas` lines of a case: a text of a sequence carries its rank."""
    lines = []
    for k, (given, output) in enumerate(zip(case.texts, outputs), 1):
        rank = f" [{k}/{len(case.texts)}]" if len(case.texts) > 1 else ""
        name = f"{case.target} {case.expectation} — {case.label}{rank}"
        if output is None:
            expected = "supprimé"
        elif output == given:
            expected = "="
        else:
            expected = f'"{escaped(output)}"'
        lines.append(f'{name} | "{escaped(given)}" | {expected}')
    return lines


def expected_of(kind: str) -> tuple[str, list[str]]:
    """The `.cas` of `kind`, and what is wrong in its inputs."""
    if kind == "line-break":
        return expected_line_breaks()
    by_code = read_patterns(kind)
    operation = OPERATIONS[kind]
    lines = [
        "# Generated by src/scripts/pattern-oracle.py — do not edit by hand.",
        f"# What Gaupol does with the \"{kind}\" patterns: see ../LISEZMOI.md.",
        "",
    ]
    covered: dict[str, set[str]] = {}
    cascades = set()
    problems = []
    for case in read_cases(kind):
        outputs = operation(case.texts, patterns_of(by_code, case.target, case.where))
        changed = outputs != case.texts
        # A case that does not do what it announces is a case that proves nothing.
        if (case.expectation == "corrige") != changed:
            what = "changes" if changed else "does not change"
            problems.append(f"{case.where}: announced \"{case.expectation}\", and the text {what}")
        if case.target.startswith("cascade "):
            cascades.add(case.target.split(" ", 1)[1])
        else:
            covered.setdefault(case.target, set()).add(case.expectation)
        lines.extend(cas_lines(case, outputs))

    # Each record has a case it corrects and a case it leaves, and each shipped
    # language has its cascade: otherwise the file does not prove what its name
    # promises.
    for code, patterns in by_code.items():
        for pattern in patterns:
            target = f"{code}:{pattern.rank}"
            missing = {"corrige", "intact"} - covered.get(target, set())
            if missing:
                problems.append(
                    f"{kind}.entrees: \"{target}\" ({pattern.name()}) "
                    f"has no {' nor '.join(sorted(missing))} case"
                )
    for request in REQUIRED_CASCADES[kind]:
        if request not in cascades:
            problems.append(f"{kind}.entrees: no cascade \"{request}\"")
    return "\n".join(lines) + "\n", problems


# The codes asked of the cascade: each of the levels, a country, a language shipped
# without a country, and two codes for which no file exists.
CASCADE_REQUESTS = ("Latn", "Latn-en", "Latn-en-US", "Latn-fr", "Latn-fi", "Latn-de", "Cyrl")


def expected_cascades() -> str:
    """What the cascade of each code returns, for the core's reader — issue #498.

    One line per type and per code: the records in the order Gaupol applies
    them, `<code>:<rank>`, followed by `+` if they are enabled by default and
    by `-` otherwise. It is what `get_patterns` returns, and what the C++ reader
    must return in turn, without ever running Python.
    """
    lines = [
        "# Generated by src/scripts/pattern-oracle.py — do not edit by hand.",
        "# The cascade of each code: `<code>:<rank>`, `+` enabled by default, `-` not.",
        "# See ../LISEZMOI.md.",
        "",
    ]
    for kind in TYPES:
        by_code = read_patterns(kind)
        for request in CASCADE_REQUESTS:
            items = [f"{x.code}:{x.rank}{'+' if x.enabled else '-'}" for x in cascade(by_code, request)]
            lines.append(f"{kind} {request} | {' '.join(items)}")
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true", help="write the expected results")
    mode.add_argument("--check", action="store_true", help="refuse an expected result that drifted")
    arguments = parser.parse_args()

    failures = []
    for kind in TYPES:
        try:
            content, problems = expected_of(kind)
        except Refusal as refusal:
            failures.append(str(refusal))
            continue
        failures.extend(problems)
        path = EXPECTED / f"{kind}.cas"
        if arguments.write:
            # Nothing is written for a type whose inputs do not hold.
            if not problems:
                EXPECTED.mkdir(exist_ok=True)
                path.write_text(content, encoding="utf-8")
        elif not path.exists() or path.read_text(encoding="utf-8") != content:
            failures.append(
                f"{path.relative_to(REPO_ROOT)} is not what the oracle writes: "
                "rerun \"src/scripts/pattern-oracle.py --write\" and review the diff"
            )

    cascades = expected_cascades()
    path = EXPECTED / "cascades.txt"
    if arguments.write:
        if not failures:
            path.write_text(cascades, encoding="utf-8")
    elif not path.exists() or path.read_text(encoding="utf-8") != cascades:
        failures.append(
            f"{path.relative_to(REPO_ROOT)} is not what the oracle writes: "
            "rerun \"src/scripts/pattern-oracle.py --write\" and review the diff"
        )

    if failures:
        for failure in failures:
            print(f"✗ {failure}", file=sys.stderr)
        return 1
    print("✓ the expected results of the patterns are what Gaupol makes of them")
    return 0


if __name__ == "__main__":
    sys.exit(main())
