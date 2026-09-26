#!/usr/bin/env python3
r"""Ce que Gaupol fait de ses motifs de correction — issue #494.

**L'oracle des motifs, écrit en Python parce que les motifs le sont.** Les
fichiers de `src/test/data/motifs/gaupol/` sont ceux de Gaupol, écrits pour le
module `re` ; aucun moteur C++ ne les lit tels quels, et celui que la phase 12
choisira devra prouver qu'il corrige comme Gaupol corrige. Ce script écrit ce
que Gaupol en fait, cas par cas, dans `attendus/` ; les tests C++ du moteur
liront ces attendus sans jamais lancer Python.

**Ce n'est pas `re.sub`, et c'est toute la raison d'être de ce script.** Gaupol
remplace par un `Finder` qui cherche chaque correspondance **dans le texte déjà
modifié**, à partir de la fin du dernier remplacement, avec sa propre règle pour
les correspondances vides. Un oracle qui aurait appelé `re.sub` n'aurait prouvé
que son accord avec notre lecture — la leçon de #338.

**La différence est mesurée, et elle est étroite.** Sur les 72 enregistrements,
des dizaines de milliers de textes tirés au hasard n'en montrent aucune pour les
erreurs courantes et les majuscules : `Repeat` relance un motif jusqu'à ce qu'il
ne change plus rien, et rattrape ce qu'un `re.sub` aurait laissé. Elle apparaît
dans le nettoyage qui suit le retrait des mentions, où `^\W*$` et les lignes
vides se retirent l'une après l'autre ; les deux cas qui la montrent sont dans
`hearing-impaired.entrees`.

Ce que le script reproduit, lu dans Gaupol et nommé à chaque endroit :

- le chargement des fichiers (`aeidon/patternman.py`) : commentaires de ligne
  entière, lignes dépouillées, `\0` non suivi d'un chiffre retiré, activation
  lue dans les `.conf` du même code ;
- la cascade des codes, `SkipIn` et `Policy=Replace` (`get_patterns`,
  `_filter_patterns`) ;
- la recherche et le remplacement (`aeidon/finder.py`) ;
- les trois opérations (`aeidon/agents/text.py`) : `correct_common_errors`,
  `capitalize`, `remove_hearing_impaired` et ses nettoyages.

**Les textes n'ont pas de balises.** Le parseur de Gaupol les retire, puis les
remet en place autour des remplacements ; c'est le travail du parseur de balises
de la phase 10, éprouvé ailleurs. Ici, un texte sans balise passe par le
parseur sans y rien laisser.

Il ne lit **jamais** le clone de Gaupol : seulement les copies versionnées, ce
qui le rend rejouable sur une machine qui ne l'a pas.

    ./src/scripts/pattern-oracle.py --write   # écrit les attendus
    ./src/scripts/pattern-oracle.py --check   # refuse un attendu qui a dérivé
"""

import argparse
import re
import sys
from collections.abc import Callable
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
PATTERNS_ROOT = REPO_ROOT / "src" / "test" / "data" / "motifs"
GAUPOL = PATTERNS_ROOT / "gaupol"
INPUTS = PATTERNS_ROOT / "entrees"
EXPECTED = PATTERNS_ROOT / "attendus"

TYPES = ("common-error", "capitalization", "hearing-impaired", "line-break")

# Les langues dont Gaupol livre des motifs : chacune a sa cascade, sans quoi
# `SkipIn` et `Policy=Replace` ne seraient jamais éprouvés. Les mentions sont
# livrées toutes désactivées, d'où leur cascade `tous`.
REQUIRED_CASCADES = {
    "common-error": ("Latn-en", "Latn-en-US", "Latn-fr", "Latn-fi"),
    "capitalization": ("Latn-en",),
    "hearing-impaired": ("Latn-en tous",),
    "line-break": ("Latn-en",),
}

# Ce que chaque type appelle un texte changé : un découpage coupe, il ne
# corrige pas.
CHANGED_WORD = {
    "common-error": "corrige",
    "capitalization": "corrige",
    "hearing-impaired": "corrige",
    "line-break": "coupe",
}

# Un texte d'entrée, ou le sous-titre retiré qu'un vidage laisse : `None`.
Texts = list[str | None]


class Refusal(Exception):
    """Une entrée qui ne se lit pas."""


# --- Le chargement : aeidon/patternman.py ---------------------------------------


class Pattern:
    """Un enregistrement d'un fichier de motifs, et le code qui le porte."""

    def __init__(self, code: str, rank: int) -> None:
        self.code = code
        self.rank = rank
        self.fields: dict[str, str] = {}
        # `Pattern.enabled` vaut vrai tant qu'un `.conf` ne dit rien.
        self.enabled = True

    def name(self) -> str:
        return self.fields.get("Name", "")

    def field_list(self, field: str) -> list[str]:
        return [x for x in self.fields.get(field, "").split(";") if x]

    def flags(self) -> int:
        # `Pattern.get_flags` : `getattr(re, nom)` pour chaque nom listé.
        value = 0
        for name in self.field_list("Flags"):
            value |= getattr(re, name)
        return value


def read_patterns(kind: str) -> dict[str, list[Pattern]]:
    """Tous les motifs de `kind`, par code, dans l'ordre des fichiers."""
    by_code: dict[str, list[Pattern]] = {}
    for path in sorted(GAUPOL.glob(f"*.{kind}")):
        code = path.name[: -len(f".{kind}")]
        patterns = by_code.setdefault(code, [])
        content = path.read_text(encoding="utf-8").replace("\r\n", "\n")
        for line in content.split("\n"):
            # `_re_comment` ne retire qu'une ligne *entière* de commentaire : le
            # `#` des notes de musique, au milieu d'un motif, reste.
            line = re.sub(r"^\s*#.*$", "", line).strip()
            if not line:
                continue
            if line.startswith("["):
                patterns.append(Pattern(code, len(patterns) + 1))
                continue
            name, value = line.split("=", 1)
            name = name[1:] if name.startswith("_") else name
            # Issue #70 de Gaupol : `\0` protège ce que GKeyFile et msgfmt
            # « répareraient », et disparaît à la lecture.
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
    """`_get_codes` : `Zyyy`, puis l'écriture, la langue, le pays."""
    parts = request.split("-")
    return ["Zyyy"] + ["-".join(parts[: i + 1]) for i in range(len(parts))]


def cascade(by_code: dict[str, list[Pattern]], request: str) -> list[Pattern]:
    """`get_patterns` puis `_filter_patterns`, pour le code `request`."""
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


# --- La recherche et le remplacement : aeidon/finder.py -------------------------


class Finder:
    """`aeidon.Finder`, pour une expression, porté ligne à ligne."""

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


# --- Les trois opérations : aeidon/agents/text.py -------------------------------


def correct_common_errors(texts: list[str], patterns: list[Pattern]) -> Texts:
    """`correct_common_errors` : dans l'ordre, et `Repeat` reboucle."""
    corrected: Texts = []
    for text in texts:
        finder = Finder(text)
        for pattern in patterns:
            finder.set_regex(pattern.fields["Pattern"], pattern.flags())
            finder.replacement = pattern.fields.get("Replacement", "")
            count = finder.replace_all()
            while pattern.fields.get("Repeat") == "True" and count:
                count = finder.replace_all()
        corrected.append(finder.text)
    return corrected


CAPITALIZABLE = re.compile(r"^\W*(?<!\.\.\.)(?<!…)\w")


def capitalize_first(finder: Finder, pos: int) -> bool:
    """`_capitalize_first` : le premier caractère de mot à partir de `pos`."""
    match = CAPITALIZABLE.search(finder.text[pos:])
    if match is not None:
        i = pos + match.end() - 1
        text = finder.text
        finder.text = text[:i] + text[i : i + 1].capitalize() + text[i + 1 :]
    return match is not None


def capitalize_matches(finder: Finder, pattern: Pattern, cap_next: bool) -> bool:
    """`_capitalize_text` — récursif chez Gaupol, itératif ici, même parcours."""
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
    """`capitalize` sur des sous-titres consécutifs, le premier d'indice 0."""
    capitalized: Texts = []
    cap_next = False
    for index, text in enumerate(texts):
        finder = Finder(text)
        if cap_next or index == 0:
            capitalize_first(finder, 0)
            cap_next = False
        for pattern in patterns:
            finder.set_regex(pattern.fields["Pattern"], pattern.flags())
            finder.pos = 0
            cap_next = capitalize_matches(finder, pattern, cap_next)
        capitalized.append(finder.text)
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
    """`remove_hearing_impaired` : `None` pour un sous-titre retiré.

    **Les nettoyages de `_remove_leftover_hi` ne touchent que les textes que les
    motifs ont changés** — un texte laissé intact garde ses espaces doubles.
    """
    removed: Texts = []
    for text in texts:
        finder = Finder(text)
        for pattern in patterns:
            finder.set_regex(pattern.fields["Pattern"], pattern.flags())
            finder.replacement = pattern.fields.get("Replacement", "")
            finder.replace_all()
        if finder.text == text:
            removed.append(text)
            continue
        for expression, replacement in LEFTOVERS:
            finder.set_regex(expression)
            finder.replacement = replacement
            finder.replace_all()
        # `remove_blank`, que l'assistant coche par défaut.
        removed.append(finder.text or None)
    return removed


# --- Le découpage de lignes : aeidon/liner.py -----------------------------------

# Les trois pénalités de `aeidon/test/test_liner.py`, que ses cinq cas utilisent :
# l'intention de l'auteur, écrite par lui, à côté de celles que Gaupol livre.
TEST_PENALTIES = (
    (r"( )- ", re.DOTALL | re.MULTILINE, 1, -1000.0),
    (r"[,.;:!?]( )", re.DOTALL | re.MULTILINE, 1, -100.0),
    (r"\b(by|the|into|a)( )", re.DOTALL | re.MULTILINE, 2, 1000.0),
)

# Une pénalité : l'expression, le groupe qui tient l'espace, la valeur.
Penalty = tuple[re.Pattern[str], int, float]


def penalties_of(patterns: list[Pattern]) -> list[Penalty]:
    """`_get_penalties` puis `Liner.set_penalties`."""
    return [
        (re.compile(x.fields["Pattern"], x.flags()), int(x.fields["Group"]), float(x.fields["Penalty"]))
        for x in patterns
    ]


class Liner:
    """`aeidon.Liner`, porté ligne à ligne, en unité caractère.

    `_list_possible_breaks` est mémorisé chez Gaupol par l'identité de l'objet ;
    ici par ses arguments, pour un seul découpage : les réglages ne changent
    pas pendant un appel, et le résultat est le même.
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
        """`_calculate_demerit`, terme à terme."""
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
        """`_detect_penalties` : chaque espace reçoit la somme de ses pénalités."""
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
        """`_list_possible_breaks`, triés par pénalité croissante."""
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
        """`_break_lines` : les coupures et leur démérite, ou rien de valide."""
        breaks = self.possible_breaks(boxes, penalties, nlines)
        best: list[int] | None = None
        best_demerit = float(sys.maxsize)
        if len(" ".join(boxes)) <= self.max_length:
            # Une ligne seule, valide, est l'étalon qu'il faut battre.
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
        """`set_text` puis `break_lines`.

        **Rien n'est coupé tant que le nombre de lignes essayé n'atteint pas
        `max_lines`** : un texte trop court en mots pour les atteindre est rendu
        tel quel, qu'il tienne ou non dans la longueur.
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
    """`Latn-en:2 24/3`, `cascade Latn-en 24/3`, `essai 40/3`, `aucune 24/2`."""
    selection, _, settings = target.rpartition(" ")
    length, _, lines = settings.partition("/")
    if not selection or not length.isdigit() or not lines.isdigit():
        raise Refusal(f"{where} : une cible de découpage finit par « longueur/lignes »")
    return selection, int(length), int(lines)


def line_break_penalties(by_code: dict[str, list[Pattern]], selection: str, where: str) -> list[Penalty]:
    if selection == "aucune":
        return []
    if selection == "essai":
        return [(re.compile(p, f), g, v) for p, f, g, v in TEST_PENALTIES]
    return penalties_of(patterns_of(by_code, selection, where))


def expected_line_breaks() -> tuple[str, list[str]]:
    """Le `.cas` des découpages, et ce qui ne va pas dans ses entrées.

    **Chaque motif doit décider au moins un cas** : son découpage diffère de
    celui qu'on obtient sans aucune pénalité, aux mêmes réglages. Sans quoi un
    moteur qui l'ignorerait passerait quand même.
    """
    kind = "line-break"
    by_code = read_patterns(kind)
    lines = [
        "# Engendré par src/scripts/pattern-oracle.py — ne pas éditer à la main.",
        "# Ce que Gaupol fait des motifs « line-break » : voir ../LISEZMOI.md.",
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
            what = "change" if changed else "ne change pas"
            problems.append(f"{case.where} : annoncé « {case.expectation} », et le texte {what}")
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
                    f"{kind}.entrees : « {code}:{pattern.rank} » ({pattern.name()}) "
                    "ne décide aucun cas — son découpage est celui d'aucune pénalité"
                )
    for request in REQUIRED_CASCADES[kind]:
        if request not in cascades:
            problems.append(f"{kind}.entrees : pas de cascade « {request} »")
    return "\n".join(lines) + "\n", problems


OPERATIONS: dict[str, Callable[[list[str], list[Pattern]], Texts]] = {
    "common-error": correct_common_errors,
    "capitalization": capitalize,
    "hearing-impaired": remove_hearing_impaired,
}


# --- Les entrées et les attendus ------------------------------------------------


def quoted_texts(field: str, where: str) -> list[str]:
    """Un ou plusieurs textes entre guillemets, séparés par `;`.

    Plusieurs textes sont des sous-titres consécutifs : la remise en majuscule
    porte son état de l'un à l'autre, et c'est la seule façon de l'éprouver.
    """
    texts = []
    i = 0
    while True:
        while i < len(field) and field[i] == " ":
            i += 1
        if i >= len(field) or field[i] != '"':
            raise Refusal(f"{where} : un texte s'écrit entre guillemets")
        text = ""
        i += 1
        while True:
            if i >= len(field):
                raise Refusal(f"{where} : un guillemet n'est pas refermé")
            if field[i] == '"':
                i += 1
                break
            if field[i] == "\\":
                i += 1
                escaped = field[i] if i < len(field) else ""
                if escaped not in ("n", "t", '"', "\\"):
                    raise Refusal(f"{where} : échappement inconnu « \\{escaped} »")
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
            raise Refusal(f"{where} : deux textes se séparent par « ; »")
        i += 1


def escaped(text: str) -> str:
    return (
        text.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t")
    )


class Case:
    """Une ligne d'un fichier d'entrées."""

    def __init__(self, where: str, target: str, expectation: str, label: str, texts: list[str]) -> None:
        self.where = where
        self.target = target
        self.expectation = expectation
        self.label = label
        self.texts = texts


def read_cases(kind: str) -> list[Case]:
    """Les cas écrits à la main pour `kind`."""
    path = INPUTS / f"{kind}.entrees"
    cases = []
    for number, line in enumerate(path.read_text(encoding="utf-8").split("\n"), 1):
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        where = f"{path.name}:{number}"
        fields = [x.strip() for x in line.split("|", 3)]
        if len(fields) != 4:
            raise Refusal(f"{where} : quatre champs — cible | attente | libellé | textes")
        target, expectation, label, texts = fields
        # Les espaces d'alignement ne font pas partie de la cible.
        target = " ".join(target.split())
        if expectation not in (CHANGED_WORD[kind], "intact"):
            raise Refusal(f"{where} : l'attente est « {CHANGED_WORD[kind]} » ou « intact »")
        cases.append(Case(where, target, expectation, label, quoted_texts(texts, where)))
    return cases


def patterns_of(by_code: dict[str, list[Pattern]], target: str, where: str) -> list[Pattern]:
    """Ce qu'une cible désigne.

    `Latn-en:3` — un enregistrement seul, actif ou non ; `cascade Latn-en` — la
    cascade entière, telle que Gaupol l'active par défaut ; `cascade Latn-en
    tous` — la même, tout coché, ce qu'il faut pour les mentions.
    """
    if target.startswith("cascade "):
        request, _, everything = target.split(" ", 1)[1].partition(" ")
        if everything not in ("", "tous"):
            raise Refusal(f"{where} : « cascade <code> » ou « cascade <code> tous »")
        patterns = [x for x in cascade(by_code, request) if everything or x.enabled]
        if not patterns:
            raise Refusal(f"{where} : aucun motif actif pour « {request} »")
        return patterns
    code, _, rank = target.partition(":")
    if not rank.isdigit() or code not in by_code or not 1 <= int(rank) <= len(by_code[code]):
        raise Refusal(f"{where} : pas d'enregistrement « {target} »")
    return [by_code[code][int(rank) - 1]]


def cas_lines(case: "Case", outputs: Texts) -> list[str]:
    """Les lignes `.cas` d'un cas : un texte d'une suite porte son rang."""
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
    """Le `.cas` de `kind`, et ce qui ne va pas dans ses entrées."""
    if kind == "line-break":
        return expected_line_breaks()
    by_code = read_patterns(kind)
    operation = OPERATIONS[kind]
    lines = [
        "# Engendré par src/scripts/pattern-oracle.py — ne pas éditer à la main.",
        f"# Ce que Gaupol fait des motifs « {kind} » : voir ../LISEZMOI.md.",
        "",
    ]
    covered: dict[str, set[str]] = {}
    cascades = set()
    problems = []
    for case in read_cases(kind):
        outputs = operation(case.texts, patterns_of(by_code, case.target, case.where))
        changed = outputs != case.texts
        # Un cas qui ne fait pas ce qu'il annonce est un cas qui ne prouve rien.
        if (case.expectation == "corrige") != changed:
            what = "change" if changed else "ne change pas"
            problems.append(f"{case.where} : annoncé « {case.expectation} », et le texte {what}")
        if case.target.startswith("cascade "):
            cascades.add(case.target.split(" ", 1)[1])
        else:
            covered.setdefault(case.target, set()).add(case.expectation)
        lines.extend(cas_lines(case, outputs))

    # Chaque enregistrement a un cas qu'il corrige et un cas qu'il laisse, et
    # chaque langue livrée a sa cascade : sans quoi le fichier ne prouve pas ce
    # que son nom promet.
    for code, patterns in by_code.items():
        for pattern in patterns:
            target = f"{code}:{pattern.rank}"
            missing = {"corrige", "intact"} - covered.get(target, set())
            if missing:
                problems.append(
                    f"{kind}.entrees : « {target} » ({pattern.name()}) "
                    f"n'a pas de cas {' ni '.join(sorted(missing))}"
                )
    for request in REQUIRED_CASCADES[kind]:
        if request not in cascades:
            problems.append(f"{kind}.entrees : pas de cascade « {request} »")
    return "\n".join(lines) + "\n", problems


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--write", action="store_true", help="écrit les attendus")
    mode.add_argument("--check", action="store_true", help="refuse un attendu qui a dérivé")
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
            # Rien n'est écrit pour un type dont les entrées ne tiennent pas.
            if not problems:
                EXPECTED.mkdir(exist_ok=True)
                path.write_text(content, encoding="utf-8")
        elif not path.exists() or path.read_text(encoding="utf-8") != content:
            failures.append(
                f"{path.relative_to(REPO_ROOT)} n'est pas ce que l'oracle écrit : "
                "relancer « src/scripts/pattern-oracle.py --write » et relire le diff"
            )

    if failures:
        for failure in failures:
            print(f"✗ {failure}", file=sys.stderr)
        return 1
    print("✓ les attendus des motifs sont ce que Gaupol en fait")
    return 0


if __name__ == "__main__":
    sys.exit(main())
