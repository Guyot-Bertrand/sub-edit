#!/usr/bin/env python3
"""Score une détection d'encodage, au lieu de la croire.

**Une détection est un classifieur, pas une fonction.** Elle a un taux de
succès, pas un résultat juste ou faux, et « la détection marche » sans chiffre
est exactement le genre d'affirmation que trois relectures de suite ont
démentie — l'icône que trois lecteurs sur quatre acceptaient, le `.rpm`
irréprochable à l'inspection et refusé à l'installation, les raccourcis mesurés
sur une plateforme qu'aucun utilisateur n'a.

**Le détecteur est un argument, et c'est tout l'intérêt.** Le cadrage de la
phase 8 (#288) a trois voies — ICU, uchardet, des tables écrites — et le choix
entre elles est un choix de taux. Le mesurer après avoir choisi, c'est choisir
sans mesurer. Chaque voie se passe donc au même corpus, sans que ce script ait à
connaître aucune d'elles :

    ./src/scripts/score-encoding-detection.py --detector 'uchardet {}'
    ./src/scripts/score-encoding-detection.py --detector 'file -b --mime-encoding {}'
    ./src/scripts/score-encoding-detection.py --detector './build/dev/bin/probe {}' --prive

`{}` est remplacé par le chemin du fichier ; ce que la commande écrit sur sa
sortie standard est lu comme le nom de l'encodage.

## Les deux corpus, et ce que chacun peut dire

**Le corpus étiqueté** — `src/test/data/encodages/`, fabriqué par
`encoding-fixtures.py`. L'étiquette vient avec le fichier, gratuitement : la
table dit dans quel encodage il a été écrit. C'est le seul des deux qui donne un
**taux**, et il donne aussi le détail des confusions — confondre deux latins ne
coûte pas ce que coûte confondre un latin et un cyrillique.

**Le corpus privé** — `src/data/`, qui porte des fichiers réels, et des fichiers
réels mal encodés sont précisément ce qu'un corpus fabriqué ne contient pas.
**Il n'a pas d'étiquettes**, donc il ne donne aucun taux : ce serait inventer une
vérité. Ce qu'il donne est ce qui se vérifie sans elle — la répartition des
réponses, combien décodent réellement le fichier de bout en bout, et sur quoi
deux détecteurs se contredisent.

**Il ne sait pas nommer un fichier**, et ce n'est pas une politesse : la règle du
dépôt interdit de citer le corpus privé, et un outil qui *peut* le faire finit
par le faire. Rien ici ne porte un chemin jusqu'à la sortie.

## Où la détection cesse de savoir — `--par-longueur`

Le corpus étiqueté donne **un** taux, sur douze fichiers. Il ne dit pas *où* le
cas devient difficile, et l'issue #310 est née de là : la table qu'il faut est
« longueur → taux », et elle se fabrique.

`--par-longueur` engendre des fichiers SubRip gradués, les passe au même
détecteur, et rend deux tables. Rien n'est déposé dans le dépôt : le corpus est
jetable, il vit le temps de la mesure. La graine est fixe, donc la table se
rejoue à l'identique.

## Le journal, et sa divergence — `--journal`

Un score relevé une fois vieillit sans que personne l'apprenne : c'est le défaut
de l'issue #311. `--journal` confronte ce qui vient d'être mesuré à ce
qu'affiche `docs/mesures/detection-d-encodage.md`, et **traite les trois
situations séparément** — un score qui baisse échoue, un score qui monte invite
à réenregistrer, un score inchangé se tait. `--record` réécrit le relevé.

**Ce n'est pas un seuil.** Le nombre comparé est celui de la dernière mesure, et
non une barre qu'on aurait posée : une détection est un classifieur, et poser
une barre sur une mesure dont la variance n'est pas connue est ce que ce projet
refuse ailleurs.

## Ce qui n'est pas mesuré ici

Le temps. Une détection qui prend dix millisecondes par fichier n'est pas un
problème dans un éditeur de sous-titres ; le banc est là pour le jour où la
question se posera.
"""

from __future__ import annotations

import argparse
import codecs
import collections
import datetime
import importlib.util
import random
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LABELLED = REPO_ROOT / "src" / "test" / "data" / "encodages"
PRIVATE = REPO_ROOT / "src" / "data"
JOURNAL = REPO_ROOT / "docs" / "mesures" / "detection-d-encodage.md"

# Les deux bornes du bloc que `--record` réécrit ; tout le reste du journal est
# de la prose écrite à la main, et elle survit à un réenregistrement.
BLOCK_OPEN = "<!-- relevé engendré : ne pas modifier à la main -->"
BLOCK_CLOSE = "<!-- fin du relevé -->"

# L'ancre que `--journal` relit dans ce bloc.
RECORDED = re.compile(r"^\s*corpus étiqueté\s*:\s*(\d+)\s*/\s*(\d+)\s*$", re.MULTILINE)

GREEN = "\033[32m"
RED = "\033[31m"
BOLD = "\033[1m"
RESET = "\033[0m"

# Ce que les détecteurs écrivent, et que `codecs` ne sait pas relire tel quel.
ALIASES = {
    "utf-16": "utf-16",
    "utf-16le": "utf-16-le",
    "utf-16be": "utf-16-be",
    "us-ascii": "ascii",
    "unknown-8bit": "?",
    "binary": "?",
    "": "?",
    "none": "?",
}


def canonical(name):
    """Le nom que Python donne à cet encodage, ou `?` s'il n'en connaît aucun.

    Les détecteurs n'écrivent pas les mêmes mots pour le même encodage —
    `WINDOWS-1252`, `cp1252`, `windows-1252`. La comparaison passe donc par le
    nom canonique plutôt que par la chaîne rendue.
    """
    cleaned = name.strip().lower()
    if cleaned in ALIASES:
        return ALIASES[cleaned]
    try:
        return codecs.lookup(cleaned).name
    except LookupError:
        return cleaned or "?"


def labelled_fixtures():
    """Les fixtures et leur encodage, lus de la table qui les fabrique."""
    spec = importlib.util.spec_from_file_location(
        "encoding_fixtures", REPO_ROOT / "src" / "scripts" / "encoding-fixtures.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return [(LABELLED / name, encoding, bom) for name, encoding, bom, *_ in module.FIXTURES]


# La forme d'un nom d'encodage, et rien d'autre. Lettres, chiffres, tirets,
# points et soulignés, sur quelques dizaines de caractères au plus.
ENCODING_NAME = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,31}$")


def detect(command, path):
    """Ce que le détecteur dit de ce fichier, ou `?` s'il ne dit rien.

    **Sans shell, et la réponse est filtrée.** Les deux gardes existent pour la
    même raison, et elles ont été payées : la première version passait la
    commande au shell avec le chemin substitué en clair, et les noms du corpus
    privé portent des espaces et des crochets. `file` a donc reçu des arguments
    éclatés, s'est plaint de chacun — et **ses plaintes, qui citaient les noms,
    ont été lues comme des encodages et imprimées.**

    C'est exactement ce que #290 demandait de rendre impossible : « l'outil ne
    doit pas être capable de nommer un fichier, pas seulement s'en abstenir ».

    Le chemin est donc un élément d'`argv` et non un morceau de ligne de
    commande, et **ce qui ressort est refusé s'il n'a pas la forme d'un nom
    d'encodage** — un chemin n'en a jamais la forme. La sortie d'erreur, elle,
    n'est jamais lue.
    """
    try:
        parts = shlex.split(command)
    except ValueError:
        return "?"

    argv = [str(path) if part == "{}" else part for part in parts]

    try:
        done = subprocess.run(argv, capture_output=True, text=True, timeout=30)
    except (OSError, subprocess.SubprocessError):
        return "?"

    if done.returncode != 0:
        return "?"

    said = done.stdout.strip().splitlines()[0].strip() if done.stdout.strip() else ""
    return canonical(said) if ENCODING_NAME.match(said) else "?"


def agrees(said, expected, has_bom):
    """Le détecteur a-t-il raison ?

    **`utf-16` sans son côté est une bonne réponse quand le fichier a un BOM.**
    C'est le BOM qui porte l'ordre des octets, et un détecteur qui rend `UTF-16`
    ne s'est pas trompé : il a laissé au BOM ce qui lui revient.
    """
    if said == expected:
        return True
    return has_bom and said == "utf-16" and expected in ("utf-16-le", "utf-16-be")


def decodes(path, encoding):
    """Ces octets se décodent-ils entièrement dans cet encodage ?

    C'est la seule chose qu'on puisse vérifier sans étiquette — et elle est
    nécessaire sans être suffisante : un mono-octet décode presque tout, ce qui
    est précisément d'où vient le mojibake.
    """
    if encoding == "?":
        return False
    try:
        path.read_bytes().decode(encoding)
        return True
    except (UnicodeDecodeError, LookupError):
        return False


def score_labelled(command):
    fixtures = labelled_fixtures()
    right = 0
    confusions = []

    print(f"{BOLD}corpus étiqueté — {len(fixtures)} fixtures{RESET}")
    for path, expected, has_bom in fixtures:
        if not path.is_file():
            print(f"  {RED}✗{RESET} {path.name} — absente ;"
                  " ./src/scripts/encoding-fixtures.py --generate", file=sys.stderr)
            continue
        said = detect(command, path)
        wanted = canonical(expected)
        if agrees(said, wanted, has_bom):
            right += 1
            print(f"  {GREEN}✓{RESET} {path.name:<22} {said}")
        else:
            confusions.append((path.name, wanted, said))
            print(f"  {RED}✗{RESET} {path.name:<22} {said}  au lieu de {wanted}")

    total = len(fixtures)
    part = f"{100 * right / total:.0f} %" if total else "—"
    print(f"\n  {right}/{total} — {part}")
    if confusions:
        print("\n  confusions :")
        for name, wanted, said in confusions:
            print(f"    {wanted} lu comme {said}   ({name})")
    return right, total, confusions


def report_private(commands):
    """Ce que le corpus privé peut dire, et il ne peut pas dire un taux."""
    files = sorted(p for p in PRIVATE.rglob("*")
                   if p.is_file() and p.suffix.lower() in (".srt", ".vtt"))
    if not files:
        print(f"\n{BOLD}corpus privé — absent de cette machine{RESET}")
        return

    print(f"\n{BOLD}corpus privé — {len(files)} fichiers, sans étiquettes{RESET}")
    print("  aucun taux : il n'y a pas de vérité à confronter, seulement ce qui")
    print("  se vérifie sans elle.")

    for command in commands:
        answers = collections.Counter()
        readable = 0
        for path in files:
            said = detect(command, path)
            answers[said] += 1
            if decodes(path, said):
                readable += 1
        print(f"\n  {BOLD}{command}{RESET}")
        for name, count in answers.most_common():
            print(f"    {count:3d}  {name}")
        print(f"    {readable}/{len(files)} réponses sous lesquelles le fichier"
              " se décode entièrement")

    if len(commands) > 1:
        disagreements = 0
        for path in files:
            said = {detect(command, path) for command in commands}
            if len(said) > 1:
                disagreements += 1
        print(f"\n  les détecteurs se contredisent sur {disagreements}"
              f" fichier(s) sur {len(files)}")


# ---------------------------------------------------------------------------
# La falaise : où la détection cesse de savoir — issue #310.
# ---------------------------------------------------------------------------

# Les mêmes répliques que les fixtures, et quelques-unes de plus : une table de
# taux demande des tirages, là où une fixture n'est qu'un fichier.
POOLS = {
    "iso-8859-1": [
        "Le port était vide à cette heure-là.",
        "Où êtes-vous passé ?",
        "Ça n'a pas duré très longtemps.",
        "Je n'en sais rien, à vrai dire.",
        "Il fait déjà nuit sur la côte.",
        "Elle a répété la même phrase, très bas.",
        "On m'a dit que c'était réglé.",
        "L'hôtel était fermé depuis février.",
    ],
    "cp1250": [
        "Přišel jsem pozdě.",
        "Gdzie jesteś?",
        "Nem tudom, hová ment.",
        "Nevěděl jsem, co říct.",
        "Czy słyszałeś to wszystko?",
        "Már késő van, menjünk haza.",
        "Ona zawsze wraca późną nocą.",
        "Musíme počkat do rána.",
    ],
    "koi8-r": [
        "Порт был пуст в этот час.",
        "Где вы были?",
        "Это длилось недолго.",
        "Я ничего не знаю об этом.",
        "На побережье уже темно.",
        "Она повторила ту же фразу.",
        "Мне сказали, что всё улажено.",
        "Гостиница закрыта с февраля.",
    ],
}

# Ce qui remplit un fichier autour des répliques accentuées : de l'anglais, qui
# ne porte aucun octet haut. C'est la deuxième table qui en a besoin.
FILLER = [
    "I don't know what you mean.",
    "The harbour was empty at that hour.",
    "Where did he go?",
    "It didn't last very long.",
    "She said the same thing again, quietly.",
    "Do you think they will come back?",
    "They told me it was settled.",
    "The hotel had been closed since February.",
]

# Combien de tirages par ligne de table. Soixante donne un taux au point de
# pourcentage près sans que la mesure dure plus de quelques secondes.
DRAWS = 60

# La graine, écrite plutôt que tirée : une table qu'on ne peut pas rejouer à
# l'identique est un témoignage, pas une mesure.
SEED = 310


def script_of(character):
    """L'écriture d'un caractère, en gros — ce qui distingue un alphabet d'un
    autre pour un lecteur, et non pour Unicode.

    Les codes `0x80–0x9f` ont leur propre famille et ce n'est pas une finesse :
    c'est là que la confusion Latin-1 / CP1252 devient visible. Une apostrophe
    typographique lue en Latin-1 revient en caractère de commande, ce qui est
    une perte réelle là où le reste de cette confusion n'en est pas une.
    """
    code = ord(character)
    if code < 0x80:
        return "ascii"
    if code < 0xA0:
        return "commande"
    if code < 0x250 or 0x1E00 <= code < 0x1F00:
        return "latin"
    if 0x370 <= code < 0x400:
        return "grec"
    if 0x400 <= code < 0x530:
        return "cyrillique"
    if code >= 0x2E80:
        return "idéogrammes"
    return "autre"


def scripts_of(text):
    return [script_of(character) for character in text if ord(character) >= 0x80]


def subrip(lines):
    """Un fichier SubRip, dans la forme que le projet écrit."""
    parts = []
    for index, text in enumerate(lines, start=1):
        start = index * 2
        parts.append(f"{index}\n"
                     f"00:{start // 60:02d}:{start % 60:02d},000"
                     f" --> 00:{start // 60:02d}:{(start + 1) % 60:02d},000\n"
                     f"{text}\n\n")
    return "".join(parts)


def measure(command, directory, encoding, subtitles, accented):
    """Le taux d'un détecteur sur des fichiers de `subtitles` répliques, dont
    `accented` portent du texte accentué et le reste de l'anglais.

    Trois taux, et ils ne disent pas la même chose. **Exact** — la réponse nomme
    l'encodage d'écriture. **Texte** — le fichier relu sous la réponse est le
    fichier écrit, ce qui pardonne une confusion sans conséquence. **Écriture**
    — le texte relu garde l'alphabet du texte écrit, ce qui ne pardonne que le
    mojibake dans la même famille.
    """
    # La graine est une chaîne et non un tuple : `hash` d'une chaîne varie d'un
    # processus à l'autre, et la table cesserait de se rejouer à l'identique.
    draw = random.Random(f"{SEED}-{encoding}-{subtitles}-{accented}")
    pool = POOLS[encoding]
    exact = same_text = same_script = 0
    sizes = []
    highs = []
    answers = collections.Counter()

    for round_number in range(DRAWS):
        lines = [draw.choice(FILLER) for _ in range(subtitles)]
        for where in draw.sample(range(subtitles), accented):
            lines[where] = draw.choice(pool)
        text = subrip(lines)
        data = text.encode(encoding)
        sizes.append(len(data))
        highs.append(sum(1 for byte in data if byte >= 0x80))

        path = directory / f"{encoding}-{subtitles}-{accented}-{round_number}.srt"
        path.write_bytes(data)
        said = detect(command, path)
        path.unlink()
        answers[said] += 1

        if said == canonical(encoding):
            exact += 1
        try:
            read_back = data.decode(said)
        except (UnicodeDecodeError, LookupError):
            continue
        if read_back == text:
            same_text += 1
        if scripts_of(read_back) == scripts_of(text):
            same_script += 1

    return {
        "octets": sum(sizes) // DRAWS,
        "hauts": sum(highs) // DRAWS,
        "exact": exact,
        "texte": same_text,
        "écriture": same_script,
        "réponses": answers,
    }


# Les deux tables, et elles n'attaquent pas le problème par le même bout.
#
# **Par longueur** — tout le fichier est accentué, et il grandit. C'est la table
# que #310 demande, et elle montre la falaise : une réplique isolée ne porte que
# trois octets hauts.
#
# **À texte rare** — le fichier est long et presque tout ASCII, et seules
# quelques répliques portent des accents. C'est l'autre bout, et c'est celui qui
# interdit de poser un seuil sur la longueur : un fichier de quarante kilooctets
# peut ne rien donner à peser.
BY_LENGTH = [1, 2, 3, 5, 8, 13, 21]
WHEN_RARE = [(40, 1), (40, 3), (200, 1), (200, 3), (200, 10), (600, 1), (600, 10)]


def print_table(command, directory, title, rows):
    print(f"{BOLD}{title}{RESET}")
    print(f"  {'écrit en':<12} {'répliques':>9} {'accentuées':>10} {'octets':>7}"
          f" {'≠ASCII':>7} {'exact':>7} {'texte':>7} {'écriture':>9}   réponses")
    for encoding, subtitles, accented in rows:
        found = measure(command, directory, encoding, subtitles, accented)
        top = ", ".join(f"{count}×{name}"
                        for name, count in found["réponses"].most_common(3))
        print(f"  {encoding:<12} {subtitles:>9} {accented:>10} {found['octets']:>7}"
              f" {found['hauts']:>7}"
              f" {100 * found['exact'] / DRAWS:>6.0f}%"
              f" {100 * found['texte'] / DRAWS:>6.0f}%"
              f" {100 * found['écriture'] / DRAWS:>8.0f}%   {top}")
    print()


def report_cliff(command):
    """Les deux tables graduées, sur un corpus jetable."""
    print(f"{BOLD}la falaise — corpus engendré, graine {SEED}, {DRAWS} tirages"
          f" par ligne{RESET}\n")
    with tempfile.TemporaryDirectory(prefix="falaise-") as name:
        directory = Path(name)
        print_table(command, directory, "par longueur — tout le fichier est accentué",
                    [(encoding, count, count)
                     for encoding in POOLS for count in BY_LENGTH])
        print_table(command, directory, "à texte rare — un fichier long, presque tout ASCII",
                    [(encoding, subtitles, accented)
                     for encoding in POOLS for subtitles, accented in WHEN_RARE])


# ---------------------------------------------------------------------------
# Le journal, et sa divergence — issue #311.
# ---------------------------------------------------------------------------


def journal_text():
    if not JOURNAL.is_file():
        print(f"{RED}✗{RESET} journal introuvable : {JOURNAL}", file=sys.stderr)
        sys.exit(1)
    return JOURNAL.read_text(encoding="utf-8")


def recorded_score(text):
    """Le score qu'affiche le journal, ou l'arrêt du script.

    **Une ancre absente ou dupliquée arrête**, plutôt que de laisser passer :
    c'est ce que `check-coverage.sh` a appris à faire de son propre cliquet.
    Un journal réécrit à la main sur lequel plus rien ne se lit rendrait la
    comparaison muette, donc verte.
    """
    found = RECORDED.findall(text)
    if len(found) != 1:
        print(f"{RED}✗{RESET} « corpus étiqueté : N/M » apparaît {len(found)} fois dans"
              f" {JOURNAL.relative_to(REPO_ROOT)}, une seule attendue", file=sys.stderr)
        print("  corriger le fichier à la main avant de relancer.", file=sys.stderr)
        sys.exit(1)
    return int(found[0][0]), int(found[0][1])


def version_of():
    """Le numéro courant, lu là où il fait foi."""
    for line in (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8").splitlines():
        parts = line.split()
        if len(parts) == 2 and parts[0] == "VERSION":
            return parts[1]
    print(f"{RED}✗{RESET} version illisible dans CMakeLists.txt", file=sys.stderr)
    sys.exit(1)


def rewrite_journal(right, total, confusions):
    """Réécrit le seul bloc engendré du journal, et laisse la prose intacte."""
    text = journal_text()
    opened = text.find(BLOCK_OPEN)
    closed = text.find(BLOCK_CLOSE)
    if opened < 0 or closed < opened:
        print(f"{RED}✗{RESET} bornes du relevé introuvables dans"
              f" {JOURNAL.relative_to(REPO_ROOT)}", file=sys.stderr)
        sys.exit(1)

    body = [f"{BLOCK_OPEN}\n\n", f"    corpus étiqueté : {right}/{total}\n\n",
            f"Relevé sur la version {version_of()}, le"
            f" {datetime.date.today().isoformat()}.\n\n"]
    if confusions:
        body.append("| Fixture | Écrite en | Lue comme |\n| :------ | :-------- | :-------- |\n")
        for name, wanted, said in confusions:
            body.append(f"| `{name}` | `{wanted}` | `{said}` |\n")
    else:
        body.append("Aucune confusion.\n")
    body.append("\n")

    written = text[:opened] + "".join(body) + text[closed:]
    if written == text:
        print(f"{GREEN}✓{RESET} relevé déjà à jour : rien à réenregistrer")
        return
    JOURNAL.write_text(written, encoding="utf-8")
    print(f"{GREEN}✓{RESET} relevé enregistré : {right}/{total}")


def compare_to_journal(right, total, confusions, record):
    """Les trois situations, traitées séparément.

    Le nombre comparé est celui de la dernière mesure et non une barre posée —
    ce qui vaut au cliquet de couverture d'être un cliquet et non un seuil. La
    comparaison passe par les produits en croix plutôt que par les comptes :
    ajouter une fixture change le dénominateur, et deux comptes bruts ne se
    comparent plus.
    """
    if record:
        rewrite_journal(right, total, confusions)
        return 0

    kept, over = recorded_score(journal_text())
    # La cible `make` plutôt que la ligne complète : elle construit le détecteur
    # avant de le lancer, ce qu'une invocation nue de ce script ne fait pas.
    command = "make score-record"

    if right * over < kept * total:
        print(f"{RED}✗{RESET} la détection a reculé : {right}/{total} reconnues,"
              f" contre {kept}/{over} au relevé", file=sys.stderr)
        if confusions:
            print("\n  confusions actuelles :", file=sys.stderr)
            for name, wanted, said in confusions:
                print(f"    {wanted} lu comme {said}   ({name})", file=sys.stderr)
        print(f"\n  comparer à {JOURNAL.relative_to(REPO_ROOT)} pour voir ce qui a bougé.",
              file=sys.stderr)
        print(f"  corriger la détection, ou réenregistrer sciemment :\n    {command}",
              file=sys.stderr)
        return 1

    if (right, total) != (kept, over):
        print(f"{GREEN}✓{RESET} la détection progresse : {right}/{total} reconnues,"
              f" contre {kept}/{over} au relevé")
        print(f"  l enregistrer :\n    {command}")
        return 0

    print(f"{GREEN}✓{RESET} {right}/{total} reconnues, comme au relevé")
    return 0


def main():
    parser = argparse.ArgumentParser(add_help=True)
    parser.add_argument("--detector", action="append", required=True,
                        help="commande de détection, « {} » pour le chemin ; répétable")
    parser.add_argument("--prive", action="store_true",
                        help="ajoute le rapport sur le corpus privé, en agrégats")
    parser.add_argument("--par-longueur", action="store_true",
                        help="engendre un corpus gradué et rend la table « longueur → taux »")
    parser.add_argument("--journal", action="store_true",
                        help="confronte le score au relevé de docs/mesures/")
    parser.add_argument("--record", action="store_true",
                        help="réécrit ce relevé depuis la mesure ; implique --journal")
    options = parser.parse_args()

    # **Un seul détecteur quand le journal est en jeu.** Le relevé porte le
    # score de la détection du projet ; en confronter deux à un chiffre unique
    # ne voudrait rien dire.
    if (options.journal or options.record) and len(options.detector) != 1:
        parser.error("--journal et --record demandent un seul --detector")

    right = total = 0
    confusions = []
    for command in options.detector:
        print(f"{BOLD}détecteur : {command}{RESET}\n")
        got, seen, missed = score_labelled(command)
        right, total = right + got, total + seen
        confusions += missed
        print()

    # **Un corpus vide n'est pas un score de zéro, c'est une absence de
    # mesure**, et la rendre verte serait le pire des défauts de ce script.
    if total == 0:
        print(f"{RED}✗{RESET} le corpus étiqueté est vide", file=sys.stderr)
        return 1

    if options.par_longueur:
        for command in options.detector:
            report_cliff(command)

    # Le corpus privé n'est jamais lu par une porte : il est absent de toute
    # machine qui ne l'a pas, et `--prive` reste un geste à la main.
    if options.prive:
        report_private(options.detector)

    if options.journal or options.record:
        return compare_to_journal(right, total, confusions, options.record)

    return 0


if __name__ == "__main__":
    sys.exit(main())
