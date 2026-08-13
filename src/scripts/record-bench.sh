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
readonly JOURNAL="${REPO_ROOT}/docs/mesures/performances.md"
readonly RETENTION_DAYS=31

readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

xml=""
mode=""

usage() {
    cat >&2 <<'USAGE'
usage: record-bench.sh --xml <sortie.xml> --mode <mode>

  --xml   la sortie du rapporteur XML de Catch2
  --mode  le mode de compilation à consigner dans l'en-tête de section
          (ex. Release) — c'est à l'appelant de le dire, pas au script de
          le supposer : lui seul sait quel preset CMake a produit le binaire.
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --xml)  [[ $# -ge 2 ]] || usage; xml="$2"; shift 2 ;;
        --mode) [[ $# -ge 2 ]] || usage; mode="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

[[ -n "${xml}" ]] || usage
[[ -n "${mode}" ]] || usage
[[ -f "${xml}" ]] || { printf 'sortie XML introuvable : %s\n' "${xml}" >&2; exit 1; }
[[ -f "${JOURNAL}" ]] || { printf 'journal introuvable : %s\n' "${JOURNAL}" >&2; exit 1; }

version="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' \
    "${REPO_ROOT}/CMakeLists.txt" | head -1)"
[[ -n "${version}" ]] || { printf 'version illisible dans CMakeLists.txt\n' >&2; exit 1; }

python3 - "${xml}" "${JOURNAL}" "${version}" "$(date +%Y-%m-%d)" "${RETENTION_DAYS}" "${mode}" <<'PY'
import datetime
import os
import re
import sys
import tempfile
import xml.etree.ElementTree as ET

xml_path, journal_path, version, today, retention, mode = sys.argv[1:7]
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

lines = [f"### {version} — {today} — {mode}", "", "| Mesure | Moyenne | Écart-type |", "| :----- | ------: | ---------: |"]
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
ordered_names = list(known)
for measure in measures:
    if measure["name"] not in known:
        ordered_names.append(measure["name"])

rows = []
comments = []
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
        else:
            low = high = value
            minsrc = maxsrc = stamp
        if value < low:
            low, minsrc = value, stamp
        if value > high:
            high, maxsrc = value, stamp
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
PY

printf '%s✓%s relevé versé dans %s pour la version %s\n' \
    "${GREEN}" "${RESET}" "${JOURNAL#"${REPO_ROOT}"/}" "${version}"
