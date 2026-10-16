#!/usr/bin/env bash
# Pours the measurements of a benchmark run into the journal.
#
# The source is Catch2's XML output, never its console output: Catch2 wraps its
# lines at 70 columns and concatenates the spellings of a same tag, so parsing
# the display would amount to guessing its layout. Its JSON reporter would be
# handier but emits nothing for benchmarks — `benchmarkEnded` has an empty body
# there. XML or nothing.
#
# The values of the XML are in NANOSECONDS, sometimes in scientific notation.
#
# python3 reads the XML: a dependency already acquired, gcovr is written in it.
# We stick to xml.etree from the standard library, and not to defusedxml: the
# file read is produced by our own binary in build/, it never comes in from
# outside, and adding a Python dependency to reread our own output would go
# against the project's dependency rule (ADR 0004). Should this script one day
# read an XML of third-party origin, the question would come up again — and be
# settled the other way.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly RETENTION_DAYS=31

# The journal is a parameter, for the reason that gave `--input` to
# prune-runs.sh: **what this script decides is a function of the journal it
# reads**, so it is demonstrated on a hand-written journal. Without it, the only
# way to test the choice of extremes would be to make it write into the
# repository's tracked file, which no proof can afford.
journal="${REPO_ROOT}/docs/mesures/performances.md"

readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

xml=""
mode=""
load=""
below=""

usage() {
    cat >&2 <<'USAGE'
usage: record-bench.sh --xml <output.xml> --mode <mode> --load <load> --below <threshold>
                       [--journal <file>]

  --xml    the output of Catch2's XML reporter
  --mode   the build mode to record in the section header (e.g. Release) —
           the caller says it, the script does not assume it: only the caller
           knows which CMake preset produced the binary.
  --load   the machine's load at the time of measuring, or "unknown"
  --below  the threshold under which a measurement counts as clean
  --journal  the journal to write (default: docs/mesures/performances.md) —
             this is what lets the choice of extremes be tested on a
             disposable journal rather than on the repository's
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
        *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

[[ -n "${xml}" ]] || usage
[[ -n "${mode}" ]] || usage
[[ -n "${load}" ]] || usage
[[ -n "${below}" ]] || usage
[[ -f "${xml}" ]] || { printf 'XML output not found: %s\n' "${xml}" >&2; exit 1; }
[[ -f "${journal}" ]] || { printf 'journal not found: %s\n' "${journal}" >&2; exit 1; }

version="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' \
    "${REPO_ROOT}/CMakeLists.txt" | head -1)"
[[ -n "${version}" ]] || { printf 'unreadable version in CMakeLists.txt\n' >&2; exit 1; }

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
    sys.exit("no measurement in the XML: does the binary have benchmarks?")


def humanise(nanoseconds):
    """Makes a duration readable without losing three significant digits."""
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
    """Protects a markdown table cell against a '|' in the name."""
    return text.replace("|", "\\|")


UNITS = {"ns": 1.0, "µs": 1e3, "ms": 1e6, "s": 1e9}


def dehumanise(cell):
    """Rereads a duration written by humanise(), in nanoseconds.

    Three significant digits, so to within one per thousand. Enough to judge
    whether a record runs ten per cent above the previous ones, and that is the
    only use made of it.
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
        # A benchmark failing at run time carries no <mean> — Catch2 puts a
        # <failed/> there instead. Without this guard, mean.get("value") raises
        # a raw AttributeError: a Python stack rather than the ✗ expected from
        # the rest of this script.
        sys.exit(
            f"✗ benchmark failed at run time: {entry.get('name')!r} — "
            "fix the benchmark before replaying make bench."
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

# The two anchors that delimit the three zones of the journal (preamble,
# table of extremes, records) must appear exactly once each. A missing anchor —
# renamed title, line erased by a badly resolved merge — crashes neither
# partition() nor the regex that follows: the first returns an empty half, the
# second simply finds nothing to carry over, and the table of extremes would
# then be rebuilt from the current run alone (min == max everywhere), the
# "## Records" section would be duplicated, and all of it would pass for a ✓.
# It is the bug commit 0b3ac03 already fixed once, one step further down the
# pipeline; checking it here closes the other end.
#
# An unresolved git conflict marker (<<<<<<<, =======, >>>>>>>) is the most
# likely concrete case: make bench commits performances.md with every PR, so a
# merge that touches this file is now the normal case, not the exception. A
# conflict in the table of extremes left by a hasty `git add` would read as a
# clean journal and silently lose an extreme.
anchors = {"<!-- extremes -->": document.count("<!-- extremes -->"), "<!-- records -->": document.count("<!-- records -->")}
bad_anchors = {name: count for name, count in anchors.items() if count != 1}
if bad_anchors:
    details = ", ".join(f"{name!r}: {count} times" for name, count in bad_anchors.items())
    sys.exit(
        f"inconsistent journal, a single occurrence expected per anchor ({details}) — "
        "fix the journal by hand before replaying make bench."
    )

conflict_pattern = re.compile(r"^(<{7} |={7}$|>{7} )", re.MULTILINE)
if conflict_pattern.search(document):
    sys.exit(
        "inconsistent journal: git conflict marker detected — "
        "fix the journal by hand before replaying make bench."
    )

# --- records: one section per version, the new one replacing the old one ---
#
# The pattern constrains only the opening of a section ("###" at the start of a
# line): a section whose header does not follow the expected form is captured
# all the same, so that it is never lost. header_pattern then decides whether
# it is understood or not.
section_pattern = re.compile(r"^### .*?(?=^### |\Z)", re.MULTILINE | re.DOTALL)
header_pattern = re.compile(r"^### (?P<version>\S+) — (?P<date>\d{4}-\d{2}-\d{2})")

head, _, records = document.partition("<!-- records -->")

kept = []
for match in section_pattern.finditer(records):
    block = match.group(0)
    header = header_pattern.match(block)
    if header is None:
        # Unreadable section: its date is unknown, so pruning it cannot be
        # decided. Losing it would be worse than a journal that grows — it is
        # kept as is and reported.
        print(
            "warning: unreadable record section kept as is: "
            f"{block.splitlines()[0]!r}",
            file=sys.stderr,
        )
        kept.append(block.rstrip())
        continue
    if header.group("version") == version:
        continue  # replaced by the current record
    age = (today_date - datetime.date.fromisoformat(header.group("date"))).days
    if age <= retention:
        kept.append(block.rstrip())

# --- the pace of a record: what it is worth compared with the previous ones ---
#
# The median ratio, measure by measure, between this record and the median of
# the PACE_WINDOW previous ones. Median, because a single measure running away
# says nothing about the machine, whereas half of them drifting together does.
#
# **It is a diagnostic, not a criterion**, and that distinction was paid for.
# The pace was tried instead of the load, for issue #168: it seemed to measure
# the right thing — "is this record slow?" rather than "was the machine busy a
# minute ago?". The measurement settled it the other way. Out of the seven
# hundred and forty-two comparable measures of the journal, the load admits
# three hundred and sixty-one, with four spikes beyond 1.5 times their
# neighbours; the pace would admit five hundred and ninety-four, with
# twenty-five spikes.
#
# The reason is clear once seen: **a busy machine does not slow down every
# benchmark, it slows down a few.** Record 0.4.18 ran at pace 1.04 under a load
# of 6.57, and yet carried three measures at more than twice their neighbours.
# A criterion that judges the whole record cannot see that; the load, for its
# part, discards the record.
#
# The pace stays written in the header because it answers the question one
# really asks in front of a record that sets ten maxima at once — and for lack
# of it, that question was once settled wrongly.
PACE_WINDOW = 3
PACE_MINIMUM_MEASURES = 5

row_pattern = re.compile(r"^\| (?P<name>.+?) \| (?P<mean>[^|]+?) \| (?P<sd>[^|]+?) \|$", re.MULTILINE)


def read_means(block):
    """Rereads the means of a record already written.

    Only the means: the header has nothing to teach here, and reading it would
    need one more pattern to keep in agreement with the one that writes it.
    """
    means = {}
    for row in row_pattern.finditer(block):
        value = dehumanise(row.group("mean"))
        if value is not None:
            means[unescape_cell(row.group("name").strip())] = value
    return means


def read_spreads(block):
    """The dispersion of each measure of a record: its standard deviation over its mean.

    Unitless, so comparable from one measure to another — which a bare standard
    deviation is not, a nanosecond and a millisecond not being comparable.
    """
    spreads = {}
    for row in row_pattern.finditer(block):
        mean = dehumanise(row.group("mean"))
        sd = dehumanise(row.group("sd"))
        if mean and sd is not None:
            spreads[unescape_cell(row.group("name").strip())] = sd / mean
    return spreads


def pace_of(means, previous):
    """The pace of `means` against `previous`, from the most recent to the oldest.

    Returns None when the history is too short to decide — the first records of
    the journal, and those whose measures have just been renamed.
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


# An unreadable load — no /proc — is no condemnation: the measurement is
# recorded as is, with no quality claimed, and it sets no extreme either.
# Claiming it clean would be claiming what was not seen.
try:
    quiet = float(load) < float(below)
except ValueError:
    quiet = False

# `kept` runs from the most recent to the oldest: the order pace_of expects.
history = [means for means in (read_means(block) for block in kept) if means]

current_means = {measure["name"]: measure["mean"] for measure in measures}
pace = pace_of(current_means, history)

pace_cell = "unknown" if pace is None else f"×{pace:.2f}"
lines = [f"### {version} — {today} — {mode} — load {load} — pace {pace_cell}", "", "| Measure | Mean | Standard deviation |", "| :----- | ------: | ---------: |"]
for measure in measures:
    lines.append(
        f"| {escape_cell(measure['name'])} | {humanise(measure['mean'])} | {humanise(measure['sd'])} |"
    )
current = "\n".join(lines)

new_records = "\n\n".join([current] + kept)

# --- extremes: never pruned, updated when a record falls ---
#
# The date columns require the form "version — YYYY-MM-DD": without that
# constraint, the header ("Recorded on") and the separator line (":---") of the
# markdown table are valid "| ... | ... | ... | ... | ... |" lines too, and would
# slip into `known` as false measures.
extreme_pattern = re.compile(
    r"^\| (?P<name>.+?) \| (?P<min>[^|]+?) \| "
    r"(?P<minsrc>\S+ — \d{4}-\d{2}-\d{2}) \| "
    r"(?P<max>[^|]+?) \| (?P<maxsrc>\S+ — \d{4}-\d{2}-\d{2}) \|$",
    re.MULTILINE,
)
preamble, _, extremes_block = head.partition("<!-- extremes -->")
extremes_block, _, records_head = extremes_block.partition("## Records")

known = {}
for match in extreme_pattern.finditer(extremes_block):
    known[unescape_cell(match.group("name"))] = {
        "min": match.group("min").strip(),
        "minsrc": match.group("minsrc").strip(),
        "max": match.group("max").strip(),
        "maxsrc": match.group("maxsrc").strip(),
    }

# The displayed values are rounded; the raw value is kept in a line comment so
# that comparing needs no reparsing of formatted text.
raw_pattern = re.compile(r"<!-- (?P<name>.+?) min=(?P<min>\S+) max=(?P<max>\S+) -->")
raw = {
    match.group("name"): (float(match.group("min")), float(match.group("max")))
    for match in raw_pattern.finditer(extremes_block)
}

# The visible table and the raw comments are two writings of the same state;
# they must carry exactly the same names. If they diverge — a line erased by
# hand, a merge that touched only one of the two — the raw values can no longer
# be trusted: better to stop than to silently reset historical extremes.
if set(known) != set(raw):
    only_table = sorted(set(known) - set(raw))
    only_comments = sorted(set(raw) - set(known))
    details = []
    if only_table:
        details.append(f"only in the visible table: {only_table}")
    if only_comments:
        details.append(f"only in the raw comments: {only_comments}")
    sys.exit(
        "table of extremes inconsistent between the visible table and the "
        "raw comments (" + "; ".join(details) + ") — "
        "fix the journal by hand before replaying make bench."
    )

stamp = f"{version} — {today}"

current_means = {measure["name"]: measure["mean"] for measure in measures}

# The table of extremes is never pruned: a measure absent from the current
# record — renamed, filtered by a Catch2 tag, removed — keeps its minimum, its
# maximum and their dates as they are rather than disappearing. The order
# already present in the journal is kept; new measures are appended after it,
# in the order the XML reports them.
#
# **A new measure enters it only on a clean record**, and that guard was paid
# for. Without it, a new measure set its two extremes at the first record that
# carried it, **even a dirty one**: having nothing to compare against, the rule
# that protects the envelope had nothing to refuse, and the load threshold thus
# never applied to what had just been born. The four video measures of phase 6
# were born that way under a load of 5.73, and the table had to be corrected by
# hand. Kept out of the table, a new measure will enter it at the first quiet
# record — the journal shows its figure meanwhile.
# ## Dispersion, the second admission criterion — issue #202
#
# **The load threshold reads the machine; it does not read the measure.** A
# record taken at load 1.44 — hence admitted — set a final maximum on a ticket
# that changed no `.cpp`. Its mean was 10 % high, and its standard deviation was
# worth 26 % of its mean where that measure stays between 3 and 8 %. The measure
# itself said it was worthless, and nobody listened.
#
# **The criterion is relative to the measure, and it had to be.** The median
# dispersion of the journal is 17 %, and it ranges from 3.8 % for scaling a
# rational to 43.6 % for shifting four thousand subtitles. A single threshold at
# 10 %, as the issue proposed, would refuse more than one measure in two;
# weighed on the journal it brings the maxima set down from 134 to 84 while
# sparing only eight bad ones. What matters is not that a measure is dispersed,
# it is that it is **more so than usual**.
#
# **The factor is a quarter above the median**, and that is where the curve
# turns — measured on the 87 records of the journal, counting the maxima set and
# those the following records contradict:
#
#     load alone                   134 maxima set, 11 contradicted (8.2 %)
#     load and dispersion x1.0     119 set,  4 contradicted (3.4 %)
#     load and dispersion x1.25    126 set,  4 contradicted (3.2 %)
#     load and dispersion x1.5     121 set,  5 contradicted (4.1 %)
#     load and dispersion x2.0     131 set,  6 contradicted (4.6 %)
#
# `analyse-bench-journal.py` replays that computation; the threshold is raised
# or tightened on figures and not on an opinion.
#
# **It adds to the load, it does not replace it.** The two do not catch the
# same defect, and dispersion alone is clearly worse — 24 spikes admitted
# against 5. The load, moreover, is read **before** measuring, which allows
# giving up; the dispersion is known only afterwards.
SPREAD_FACTOR = 1.25

# Below that many records, the median of a measure says nothing and the
# criterion does not apply — the load remains the only judge. A new measure is
# already held by the rule of #189, which keeps it out of the table until the
# first quiet record.
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
    """Is this measure dispersed little enough to touch the envelope?"""
    seen = history_spreads.get(name, [])
    if len(seen) < SPREAD_MINIMUM or name not in current_spreads:
        return True
    return current_spreads[name] <= SPREAD_FACTOR * statistics.median(seen)


def admits(name):
    """Both criteria, and it takes both."""
    return quiet and steady(name)


newcomers = [measure["name"] for measure in measures if measure["name"] not in known]
ordered_names = list(known) + [name for name in newcomers if admits(name)]



def posed_by_replaced(source):
    """Does this extreme come from the record this one replaces?

    The comparison bears on the version alone: it is what decides the
    replacement of a section, whatever the date that comes with it.
    """
    return source.split(" — ")[0] == version


rows = []
comments = []
withheld = []
stale = []
for name in ordered_names:
    if name not in current_means:
        # Not measured this time: copied without touching it.
        low, high = raw[name]
        minsrc, maxsrc = known[name]["minsrc"], known[name]["maxsrc"]
    else:
        value = current_means[name]
        if name in raw:
            low, high = raw[name]
            minsrc, maxsrc = known[name]["minsrc"], known[name]["maxsrc"]

            # **An extreme set by the record being replaced no longer has a
            # source.** Its section disappears from the file, replaced by this
            # one; leaving it as is would quote, for a version present in the
            # journal, figures that this version no longer shows there. Taken
            # from the current record if it is clean — better or worse, what
            # matters is that it can be checked.
            if admits(name) and posed_by_replaced(minsrc):
                low, minsrc = value, stamp
            if admits(name) and posed_by_replaced(maxsrc):
                high, maxsrc = value, stamp
            if not admits(name) and (posed_by_replaced(minsrc) or posed_by_replaced(maxsrc)):
                stale.append(name)
        else:
            low = high = value
            minsrc = maxsrc = stamp
        # The heart of the rule: a measurement taken on a busy machine enters
        # the history but does not touch the envelope. A maximum set by noise
        # is final — the table is never pruned — and makes the measure blind to
        # any regression smaller than that noise.
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
    ["", "| Measure | Minimum | Recorded on | Maximum | Recorded on |",
     "| :----- | ------: | :-------- | ------: | :-------- |"]
    + rows
    + [""]
    + comments
    + [""]
)

# Atomic write: a temporary file next to the target, renamed afterwards.
# os.replace() is atomic on a same file system, so a process interrupted while
# writing (Ctrl-C, OOM, full disk) leaves the tracked journal intact rather than
# truncated.
#
# mkstemp creates its file as 0600: without correction, the rename would
# replace a file tracked as 644 by a file unreadable to the group and others.
directory = os.path.dirname(journal_path) or "."
original_mode = os.stat(journal_path).st_mode & 0o777
descriptor, tmp_path = tempfile.mkstemp(prefix=".performances-", dir=directory)
try:
    os.chmod(tmp_path, original_mode)
    with os.fdopen(descriptor, "w", encoding="utf-8") as handle:
        handle.write(preamble)
        handle.write("<!-- extremes -->")
        handle.write(new_extremes)
        handle.write("\n## Records")
        handle.write(records_head)
        handle.write("<!-- records -->\n\n")
        handle.write(new_records)
        handle.write("\n")
    os.replace(tmp_path, journal_path)
except BaseException:
    os.unlink(tmp_path)
    raise

unsteady = sorted(name for name in current_spreads if quiet and not steady(name))

# **Two headers, a single set of details.** Both criteria withhold the same
# thing — the right to touch the envelope — and what was withheld is said once,
# whatever the reason.
if not quiet:
    print(
        f"warning: load {load}, threshold {below} — this record enters the "
        "journal but sets no extreme.",
        file=sys.stderr,
    )
elif unsteady:
    print(
        f"warning: {len(unsteady)} measure(s) more dispersed than usual — "
        f"beyond {SPREAD_FACTOR:g} times its median, a measure sets no "
        "extreme:",
        file=sys.stderr,
    )
    for name in unsteady:
        usual = statistics.median(history_spreads[name])
        print(
            f"  {current_spreads[name]:.0%} against {usual:.0%} usually — {name}",
            file=sys.stderr,
        )

if not quiet or unsteady:
    if withheld:
        print(
            "  it would have set some for: " + ", ".join(sorted(withheld)),
            file=sys.stderr,
        )
    if newcomers:
        print(
            "  new measures kept out of the table: "
            + ", ".join(sorted(newcomers))
            + " — they will enter it at the first quiet record.",
            file=sys.stderr,
        )
    if stale:
        print(
            "  extremes left on a record this one replaces: "
            + ", ".join(sorted(stale))
            + " — replay make bench when quiet so that they can be checked "
            "again.",
            file=sys.stderr,
        )

PY

printf '%s✓%s record added to %s for version %s\n' \
    "${GREEN}" "${RESET}" "${journal#"${REPO_ROOT}"/}" "${version}"
