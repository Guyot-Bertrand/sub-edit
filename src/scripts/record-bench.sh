#!/usr/bin/env bash
# Verse les mesures d'une exécution de benchmarks dans le journal.
#
# La source est la sortie XML de Catch2, jamais sa sortie console : Catch2
# replie ses lignes à 70 colonnes et concatène les orthographes d'un même tag,
# donc analyser l'affichage reviendrait à deviner sa mise en page. Son
# rapporteur JSON serait plus commode mais n'émet rien pour les benchmarks —
# `benchmarkEnded` y a un corps vide. XML ou rien.
#
# Les valeurs du XML sont en NANOSECONDES, parfois en notation scientifique.
#
# python3 sert à lire le XML : dépendance déjà acquise, gcovr en est écrit.
# On s'en tient à xml.etree de la bibliothèque standard, et non à defusedxml :
# le fichier lu est produit par notre propre binaire dans build/, il n'entre
# jamais depuis l'extérieur, et ajouter une dépendance Python pour relire notre
# propre sortie irait contre la règle de dépendances du projet (ADR 0004). Si
# ce script devait un jour lire un XML d'origine tierce, la question se
# reposerait — et se trancherait dans l'autre sens.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly RETENTION_DAYS=31

# Le journal est un paramètre, pour la raison qui a donné `--input` à
# prune-runs.sh : **ce que ce script décide est une fonction du journal qu'il
# lit**, donc cela se démontre sur un journal écrit à la main. Sans cela, la
# seule façon d'éprouver le choix des extrêmes serait de le faire écrire dans
# le fichier versionné du dépôt, ce qu'aucune preuve ne peut se permettre.
journal="${REPO_ROOT}/docs/mesures/performances.md"

readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

xml=""
mode=""
load=""
below=""

usage() {
    cat >&2 <<'USAGE'
usage: record-bench.sh --xml <sortie.xml> --mode <mode> --load <charge> --below <seuil>
                       [--journal <fichier>]

  --xml    la sortie du rapporteur XML de Catch2
  --mode   le mode de compilation à consigner dans l'en-tête de section
           (ex. Release) — c'est à l'appelant de le dire, pas au script de
           le supposer : lui seul sait quel preset CMake a produit le binaire.
  --load   la charge de la machine au moment de mesurer, ou « inconnue »
  --below  le seuil sous lequel une mesure compte comme propre
  --journal  le journal à écrire (défaut : docs/mesures/performances.md) —
             c'est ce qui permet d'éprouver le choix des extrêmes sur un
             journal jetable plutôt que sur celui du dépôt
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --xml)  [[ $# -ge 2 ]] || usage; xml="$2"; shift 2 ;;
        --mode)  [[ $# -ge 2 ]] || usage; mode="$2";  shift 2 ;;
        --load)  [[ $# -ge 2 ]] || usage; load="$2";  shift 2 ;;
        --below) [[ $# -ge 2 ]] || usage; below="$2"; shift 2 ;;
        --journal) [[ $# -ge 2 ]] || usage; journal="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

[[ -n "${xml}" ]] || usage
[[ -n "${mode}" ]] || usage
[[ -n "${load}" ]] || usage
[[ -n "${below}" ]] || usage
[[ -f "${xml}" ]] || { printf 'sortie XML introuvable : %s\n' "${xml}" >&2; exit 1; }
[[ -f "${journal}" ]] || { printf 'journal introuvable : %s\n' "${journal}" >&2; exit 1; }

version="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' \
    "${REPO_ROOT}/CMakeLists.txt" | head -1)"
[[ -n "${version}" ]] || { printf 'version illisible dans CMakeLists.txt\n' >&2; exit 1; }

python3 - "${xml}" "${journal}" "${version}" "$(date +%Y-%m-%d)" "${RETENTION_DAYS}" "${mode}" \
    "${load}" "${below}" <<'PY'
import datetime
import os
import re
import statistics
import sys
import tempfile
import xml.etree.ElementTree as ET

xml_path, journal_path, version, today, retention, mode, load, below = sys.argv[1:9]
retention = int(retention)
today_date = datetime.date.fromisoformat(today)

root = ET.parse(xml_path).getroot()
results = list(root.iter("BenchmarkResults"))
if not results:
    sys.exit("aucune mesure dans le XML : le binaire a-t-il des benchmarks ?")


def humanise(nanoseconds):
    """Rend une durée lisible sans perdre trois chiffres significatifs."""
    for limit, unit, factor in (
        (1e3, "ns", 1.0),
        (1e6, "µs", 1e3),
        (1e9, "ms", 1e6),
        (float("inf"), "s", 1e9),
    ):
        if nanoseconds < limit:
            return f"{nanoseconds / factor:.3g} {unit}"
    return f"{nanoseconds:.3g} ns"


def escape_cell(text):
    """Protège une cellule de table markdown contre un '|' dans le nom."""
    return text.replace("|", "\\|")


UNITS = {"ns": 1.0, "µs": 1e3, "ms": 1e6, "s": 1e9}


def dehumanise(cell):
    """Relit une durée écrite par humanise(), en nanosecondes.

    Trois chiffres significatifs, donc à un pour mille près. Assez pour juger
    si un relevé tourne dix pour cent au-dessus des précédents, et c'est le
    seul usage qui en est fait.
    """
    match = re.match(r"^([0-9.eE+-]+)\s*(ns|µs|ms|s)$", cell.strip())
    if match is None:
        return None
    try:
        return float(match.group(1)) * UNITS[match.group(2)]
    except ValueError:
        return None


def unescape_cell(text):
    return text.replace("\\|", "|")


measures = []
for entry in results:
    mean = entry.find("mean")
    deviation = entry.find("standardDeviation")
    if mean is None or deviation is None:
        # Un benchmark en échec à l'exécution ne porte pas de <mean> — Catch2
        # y met un <failed/> à la place. Sans ce garde-fou, mean.get("value")
        # lève une AttributeError brute : une pile Python plutôt que le ✗
        # attendu du reste de ce script.
        sys.exit(
            f"✗ benchmark en échec à l'exécution : {entry.get('name')!r} — "
            "corriger le benchmark avant de rejouer make bench."
        )
    measures.append(
        {
            "name": entry.get("name"),
            "mean": float(mean.get("value")),
            "sd": float(deviation.get("value")),
        }
    )

with open(journal_path, encoding="utf-8") as handle:
    document = handle.read()

# Les deux ancres qui délimitent les trois zones du journal (préambule, table
# des extrêmes, relevés) doivent apparaître exactement une fois chacune. Une
# ancre absente — titre renommé, ligne effacée par une fusion mal résolue —
# ne fait planter ni partition() ni le regex qui suit : le premier rend une
# moitié vide, le second ne trouve simplement rien à reconduire, et la table
# des extrêmes se reconstruirait alors depuis le seul run courant (min == max
# partout), la section « ## Relevés » se dupliquerait, et tout cela passerait
# pour un ✓. C'est le bug que le commit 0b3ac03 a déjà corrigé une fois, un
# cran plus loin dans le pipeline ; le vérifier ici ferme l'autre bout.
#
# Un marqueur de conflit git non résolu (<<<<<<<, =======, >>>>>>>) est le cas
# concret le plus probable : make bench versionne performances.md à chaque
# PR, donc une fusion qui touche ce fichier est désormais le cas normal, pas
# l'exception. Un conflit dans la table des extrêmes laissé par un `git add`
# hâtif se lirait comme un journal propre et perdrait silencieusement un
# extrême.
anchors = {"<!-- extrêmes -->": document.count("<!-- extrêmes -->"), "<!-- relevés -->": document.count("<!-- relevés -->")}
bad_anchors = {name: count for name, count in anchors.items() if count != 1}
if bad_anchors:
    details = ", ".join(f"{name!r} : {count} fois" for name, count in bad_anchors.items())
    sys.exit(
        f"journal incohérent, une seule occurrence attendue par ancre ({details}) — "
        "corriger le journal à la main avant de rejouer make bench."
    )

conflict_pattern = re.compile(r"^(<{7} |={7}$|>{7} )", re.MULTILINE)
if conflict_pattern.search(document):
    sys.exit(
        "journal incohérent : marqueur de conflit git détecté — "
        "corriger le journal à la main avant de rejouer make bench."
    )

# --- relevés : une section par version, la nouvelle remplaçant l'ancienne ---
#
# Le motif ne contraint que l'ouverture d'une section (« ### » en tête de
# ligne) : une section dont l'en-tête ne respecte pas la forme attendue est
# tout de même capturée, pour ne jamais la perdre. header_pattern décide
# ensuite si elle est comprise ou non.
section_pattern = re.compile(r"^### .*?(?=^### |\Z)", re.MULTILINE | re.DOTALL)
header_pattern = re.compile(r"^### (?P<version>\S+) — (?P<date>\d{4}-\d{2}-\d{2})")

head, _, records = document.partition("<!-- relevés -->")

kept = []
for match in section_pattern.finditer(records):
    block = match.group(0)
    header = header_pattern.match(block)
    if header is None:
        # Section illisible : on ne connaît pas sa date, donc on ne peut pas
        # décider de l'élaguer. La perdre serait pire qu'un journal qui
        # grossit — on la garde telle quelle et on le signale.
        print(
            "avertissement : section de relevé illisible conservée telle "
            f"quelle : {block.splitlines()[0]!r}",
            file=sys.stderr,
        )
        kept.append(block.rstrip())
        continue
    if header.group("version") == version:
        continue  # remplacée par le relevé courant
    age = (today_date - datetime.date.fromisoformat(header.group("date"))).days
    if age <= retention:
        kept.append(block.rstrip())

# --- l'allure d'un relevé : ce qu'il vaut comparé à ceux d'avant ---
#
# Le rapport médian, mesure par mesure, entre ce relevé et la médiane des
# PACE_WINDOW précédents. Médian, parce qu'une seule mesure qui s'emballe ne
# dit rien de la machine, alors que la moitié d'entre elles qui glissent
# ensemble le dit.
#
# **C'est un diagnostic, pas un critère**, et cette distinction a été payée.
# L'allure a été essayée à la place de la charge, pour l'issue #168 : elle
# semblait mesurer la bonne chose — « ce relevé est-il lent ? » plutôt que
# « la machine était-elle occupée il y a une minute ? ». La mesure a tranché
# dans l'autre sens. Sur les sept cent quarante-deux mesures comparables du
# journal, la charge en admet trois cent soixante et une dont quatre pointes
# au-delà de 1,5 fois leurs voisines ; l'allure en admettrait cinq cent
# quatre-vingt-quatorze, dont vingt-cinq pointes.
#
# La raison est nette une fois vue : **une machine occupée ne ralentit pas
# tous les benchmarks, elle en ralentit quelques-uns.** Le relevé 0.4.18
# tournait à l'allure 1,04 sous une charge de 6,57, et portait pourtant trois
# mesures à plus du double de leurs voisines. Un critère qui juge le relevé
# entier ne peut pas voir cela ; la charge, elle, écarte le relevé.
#
# L'allure reste écrite dans l'en-tête parce qu'elle répond à la question
# qu'on se pose vraiment devant un relevé qui pose dix maxima d'un coup — et
# que faute d'elle, cette question a été mal tranchée une fois.
PACE_WINDOW = 3
PACE_MINIMUM_MEASURES = 5

row_pattern = re.compile(r"^\| (?P<name>.+?) \| (?P<mean>[^|]+?) \| (?P<sd>[^|]+?) \|$", re.MULTILINE)


def read_means(block):
    """Relit les moyennes d'un relevé déjà écrit.

    Seules les moyennes : l'en-tête n'a rien à apprendre ici, et le lire
    demanderait un motif de plus à tenir en accord avec celui qui l'écrit.
    """
    means = {}
    for row in row_pattern.finditer(block):
        value = dehumanise(row.group("mean"))
        if value is not None:
            means[unescape_cell(row.group("name").strip())] = value
    return means


def read_spreads(block):
    """La dispersion de chaque mesure d'un relevé : son écart-type sur sa moyenne.

    Sans unité, donc comparable d'une mesure à l'autre — ce qu'un écart-type
    nu n'est pas, une nanoseconde et une milliseconde ne se comparant pas.
    """
    spreads = {}
    for row in row_pattern.finditer(block):
        mean = dehumanise(row.group("mean"))
        sd = dehumanise(row.group("sd"))
        if mean and sd is not None:
            spreads[unescape_cell(row.group("name").strip())] = sd / mean
    return spreads


def pace_of(means, previous):
    """L'allure de `means` face à `previous`, du plus récent au plus ancien.

    Rend None quand l'historique est trop court pour trancher — les premiers
    relevés du journal, et ceux dont les mesures viennent d'être renommées.
    """
    ratios = []
    for name, value in means.items():
        earlier = [record[name] for record in previous[:PACE_WINDOW] if name in record]
        if len(earlier) < 2:
            continue
        base = statistics.median(earlier)
        if base > 0:
            ratios.append(value / base)
    if len(ratios) < PACE_MINIMUM_MEASURES:
        return None
    return statistics.median(ratios)


# Une charge illisible — pas de /proc — ne vaut pas condamnation : la mesure est
# consignée telle quelle, sans qualité affirmée, et elle ne fixe pas d'extrême
# non plus. Affirmer qu'elle est propre serait affirmer ce qu'on n'a pas vu.
try:
    quiet = float(load) < float(below)
except ValueError:
    quiet = False

# `kept` est du plus récent au plus ancien : c'est l'ordre qu'attend pace_of.
history = [means for means in (read_means(block) for block in kept) if means]

current_means = {measure["name"]: measure["mean"] for measure in measures}
pace = pace_of(current_means, history)

pace_cell = "inconnue" if pace is None else f"×{pace:.2f}"
lines = [f"### {version} — {today} — {mode} — charge {load} — allure {pace_cell}", "", "| Mesure | Moyenne | Écart-type |", "| :----- | ------: | ---------: |"]
for measure in measures:
    lines.append(
        f"| {escape_cell(measure['name'])} | {humanise(measure['mean'])} | {humanise(measure['sd'])} |"
    )
current = "\n".join(lines)

new_records = "\n\n".join([current] + kept)

# --- extrêmes : jamais élagués, mis à jour si un record tombe ---
#
# Les colonnes de date exigent la forme « version — AAAA-MM-JJ » : sans cette
# contrainte, l'en-tête (« Relevé le ») et la ligne de séparation (« :--- »)
# du tableau markdown sont elles aussi des lignes « | ... | ... | ... | ... |
# ... | » valides, et se glisseraient dans `known` comme de fausses mesures.
extreme_pattern = re.compile(
    r"^\| (?P<name>.+?) \| (?P<min>[^|]+?) \| "
    r"(?P<minsrc>\S+ — \d{4}-\d{2}-\d{2}) \| "
    r"(?P<max>[^|]+?) \| (?P<maxsrc>\S+ — \d{4}-\d{2}-\d{2}) \|$",
    re.MULTILINE,
)
preamble, _, extremes_block = head.partition("<!-- extrêmes -->")
extremes_block, _, records_head = extremes_block.partition("## Relevés")

known = {}
for match in extreme_pattern.finditer(extremes_block):
    known[unescape_cell(match.group("name"))] = {
        "min": match.group("min").strip(),
        "minsrc": match.group("minsrc").strip(),
        "max": match.group("max").strip(),
        "maxsrc": match.group("maxsrc").strip(),
    }

# Les valeurs affichées sont arrondies ; on garde la valeur brute en commentaire
# de ligne pour pouvoir comparer sans reparser du texte mis en forme.
raw_pattern = re.compile(r"<!-- (?P<name>.+?) min=(?P<min>\S+) max=(?P<max>\S+) -->")
raw = {
    match.group("name"): (float(match.group("min")), float(match.group("max")))
    for match in raw_pattern.finditer(extremes_block)
}

# Le tableau visible et les commentaires bruts sont deux écritures du même
# état ; ils doivent porter exactement les mêmes noms. S'ils divergent — une
# ligne effacée à la main, un merge qui n'a touché que l'un des deux — les
# valeurs brutes ne sont plus dignes de confiance : mieux vaut arrêter que
# réinitialiser en silence des extrêmes historiques.
if set(known) != set(raw):
    only_table = sorted(set(known) - set(raw))
    only_comments = sorted(set(raw) - set(known))
    details = []
    if only_table:
        details.append(f"seulement dans le tableau visible : {only_table}")
    if only_comments:
        details.append(f"seulement dans les commentaires bruts : {only_comments}")
    sys.exit(
        "table des extrêmes incohérente entre le tableau visible et les "
        "commentaires bruts (" + " ; ".join(details) + ") — "
        "corriger le journal à la main avant de rejouer make bench."
    )

stamp = f"{version} — {today}"

current_means = {measure["name"]: measure["mean"] for measure in measures}

# La table des extrêmes n'est jamais élaguée : une mesure absente du relevé
# courant — renommée, filtrée par une étiquette Catch2, retirée — garde son
# minimum, son maximum et leurs dates tels quels plutôt que de disparaître.
# L'ordre déjà présent dans le journal est conservé ; les mesures nouvelles
# sont ajoutées à la suite, dans l'ordre où le XML les rapporte.
#
# **Une mesure neuve n'y entre que sur un relevé propre**, et cette garde a été
# payée. Sans elle, elle posait ses deux extrêmes au premier relevé qui la
# portait, **même sale** : faute d'avoir quelque chose à comparer, la règle qui
# protège l'enveloppe n'avait rien à refuser, et le seuil de charge ne
# s'appliquait donc jamais à ce qui venait de naître. Les quatre mesures vidéo
# de la phase 6 sont nées ainsi sous une charge de 5,73, et la table a dû être
# corrigée à la main. Laissée hors de la table, une mesure neuve y entrera au
# premier relevé calme — le journal montre son chiffre entre-temps.
# ## La dispersion, second critère d'admission — issue #202
#
# **Le seuil de charge lit la machine ; il ne lit pas la mesure.** Un relevé pris
# à charge 1,44 — donc admis — a posé un maximum définitif sur un ticket qui ne
# changeait aucun `.cpp`. Sa moyenne était haute de 10 %, et son écart-type valait
# 26 % de sa moyenne là où cette mesure-là se tient entre 3 et 8 %. La mesure
# disait elle-même qu'elle ne valait rien, et personne ne l'écoutait.
#
# **Le critère est relatif à la mesure, et il le fallait.** La dispersion médiane
# du journal est de 17 %, et elle va de 3,8 % pour la mise à l'échelle d'un
# rationnel à 43,6 % pour un décalage de quatre mille sous-titres. Un seuil unique
# à 10 %, tel que l'issue le proposait, refuserait plus d'une mesure sur deux ;
# pesé sur le journal il fait tomber les maxima posés de 134 à 84 pour n'en
# épargner que huit de mauvais. Ce qui compte n'est pas qu'une mesure soit
# dispersée, c'est qu'elle le soit **plus que d'habitude**.
#
# **Le facteur vaut un quart au-dessus de la médiane**, et c'est là que la courbe
# tourne — mesuré sur les 87 relevés du journal, en comptant les maxima posés et
# ceux que les relevés suivants démentent :
#
#     charge seule                 134 maxima posés, 11 démentis (8,2 %)
#     charge et dispersion x1,0    119 posés,  4 démentis (3,4 %)
#     charge et dispersion x1,25   126 posés,  4 démentis (3,2 %)
#     charge et dispersion x1,5    121 posés,  5 démentis (4,1 %)
#     charge et dispersion x2,0    131 posés,  6 démentis (4,6 %)
#
# `analyse-bench-journal.py` rejoue ce calcul ; le seuil se relève ou se resserre
# sur des chiffres et non sur une opinion.
#
# **Il s'ajoute à la charge, il ne la remplace pas.** Les deux n'attrapent pas le
# même défaut, et la dispersion seule est nettement moins bonne — 24 pointes
# admises contre 5. La charge se lit de plus **avant** de mesurer, ce qui permet
# de renoncer ; la dispersion ne se connaît qu'après.
SPREAD_FACTOR = 1.25

# En deçà de tant de relevés, la médiane d'une mesure ne dit rien et le critère
# ne s'applique pas — la charge reste seule juge. Une mesure neuve est déjà
# tenue par la règle de #189, qui la laisse hors de la table jusqu'au premier
# relevé calme.
SPREAD_MINIMUM = 5

history_spreads = {}
for block in kept:
    for name, spread in read_spreads(block).items():
        history_spreads.setdefault(name, []).append(spread)

current_spreads = {
    measure["name"]: measure["sd"] / measure["mean"]
    for measure in measures
    if measure["mean"]
}


def steady(name):
    """Cette mesure-ci est-elle assez peu dispersée pour toucher à l'enveloppe ?"""
    seen = history_spreads.get(name, [])
    if len(seen) < SPREAD_MINIMUM or name not in current_spreads:
        return True
    return current_spreads[name] <= SPREAD_FACTOR * statistics.median(seen)


def admits(name):
    """Les deux critères, et il en faut deux."""
    return quiet and steady(name)


newcomers = [measure["name"] for measure in measures if measure["name"] not in known]
ordered_names = list(known) + [name for name in newcomers if admits(name)]



def posed_by_replaced(source):
    """Cet extrême vient-il du relevé que celui-ci remplace ?

    La comparaison porte sur la version seule : c'est elle qui décide du
    remplacement d'une section, quelle que soit la date qui l'accompagne.
    """
    return source.split(" — ")[0] == version


rows = []
comments = []
withheld = []
stale = []
for name in ordered_names:
    if name not in current_means:
        # Non mesurée cette fois : recopiée sans y toucher.
        low, high = raw[name]
        minsrc, maxsrc = known[name]["minsrc"], known[name]["maxsrc"]
    else:
        value = current_means[name]
        if name in raw:
            low, high = raw[name]
            minsrc, maxsrc = known[name]["minsrc"], known[name]["maxsrc"]

            # **Un extrême posé par le relevé qu'on remplace n'a plus de
            # source.** Sa section disparaît du fichier, remplacée par
            # celle-ci ; le laisser tel quel ferait citer, pour une version
            # présente au journal, des chiffres que cette version n'y montre
            # plus. Repris du relevé courant s'il est propre — meilleur ou
            # pire, ce qui compte est qu'il soit vérifiable.
            if admits(name) and posed_by_replaced(minsrc):
                low, minsrc = value, stamp
            if admits(name) and posed_by_replaced(maxsrc):
                high, maxsrc = value, stamp
            if not admits(name) and (posed_by_replaced(minsrc) or posed_by_replaced(maxsrc)):
                stale.append(name)
        else:
            low = high = value
            minsrc = maxsrc = stamp
        # Le cœur de la règle : une mesure prise sur une machine occupée entre
        # dans l'historique mais ne touche pas à l'enveloppe. Un maximum posé
        # par du bruit est définitif — la table n'est jamais élaguée — et rend
        # la mesure aveugle à toute régression plus petite que ce bruit.
        if admits(name) and value < low:
            low, minsrc = value, stamp
        if admits(name) and value > high:
            high, maxsrc = value, stamp
        if not admits(name) and (value < raw.get(name, (value, value))[0] or
                                 value > raw.get(name, (value, value))[1]):
            withheld.append(name)
    rows.append(
        f"| {escape_cell(name)} | {humanise(low)} | {minsrc} | {humanise(high)} | {maxsrc} |"
    )
    comments.append(f"<!-- {name} min={low!r} max={high!r} -->")

new_extremes = "\n".join(
    ["", "| Mesure | Minimum | Relevé le | Maximum | Relevé le |",
     "| :----- | ------: | :-------- | ------: | :-------- |"]
    + rows
    + [""]
    + comments
    + [""]
)

# Écriture atomique : un fichier temporaire à côté de la cible, renommé
# ensuite. os.replace() est atomique sur un même système de fichiers, donc un
# processus interrompu en cours d'écriture (Ctrl-C, OOM, disque plein) laisse
# le journal versionné intact plutôt que tronqué.
#
# mkstemp crée son fichier en 0600 : sans correction, le renommage remplacerait
# un fichier suivi en 644 par un fichier illisible du groupe et des autres.
directory = os.path.dirname(journal_path) or "."
original_mode = os.stat(journal_path).st_mode & 0o777
descriptor, tmp_path = tempfile.mkstemp(prefix=".performances-", dir=directory)
try:
    os.chmod(tmp_path, original_mode)
    with os.fdopen(descriptor, "w", encoding="utf-8") as handle:
        handle.write(preamble)
        handle.write("<!-- extrêmes -->")
        handle.write(new_extremes)
        handle.write("\n## Relevés")
        handle.write(records_head)
        handle.write("<!-- relevés -->\n\n")
        handle.write(new_records)
        handle.write("\n")
    os.replace(tmp_path, journal_path)
except BaseException:
    os.unlink(tmp_path)
    raise

unsteady = sorted(name for name in current_spreads if quiet and not steady(name))

# **Deux en-têtes, un seul jeu de détails.** Les deux critères écartent la même
# chose — le droit de toucher à l'enveloppe — et ce qui a été écarté se dit une
# fois, quelle que soit la raison.
if not quiet:
    print(
        f"avertissement : charge {load}, seuil {below} — ce relevé entre au "
        "journal mais ne fixe aucun extrême.",
        file=sys.stderr,
    )
elif unsteady:
    print(
        f"avertissement : {len(unsteady)} mesure(s) plus dispersées que "
        f"d'habitude — au-delà de {SPREAD_FACTOR:g} fois leur médiane, une "
        "mesure ne fixe aucun extrême :",
        file=sys.stderr,
    )
    for name in unsteady:
        usual = statistics.median(history_spreads[name])
        print(
            f"  {current_spreads[name]:.0%} contre {usual:.0%} d'habitude — {name}",
            file=sys.stderr,
        )

if not quiet or unsteady:
    if withheld:
        print(
            "  il en aurait posé pour : " + ", ".join(sorted(withheld)),
            file=sys.stderr,
        )
    if newcomers:
        print(
            "  mesures neuves laissées hors de la table : "
            + ", ".join(sorted(newcomers))
            + " — elles y entreront au premier relevé calme.",
            file=sys.stderr,
        )
    if stale:
        print(
            "  extrêmes laissés sur un relevé que celui-ci remplace : "
            + ", ".join(sorted(stale))
            + " — rejouer make bench au calme pour qu'ils redeviennent "
            "vérifiables.",
            file=sys.stderr,
        )

PY

printf '%s✓%s relevé versé dans %s pour la version %s\n' \
    "${GREEN}" "${RESET}" "${journal#"${REPO_ROOT}"/}" "${version}"
