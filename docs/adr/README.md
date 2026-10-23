# Architecture decisions

One decision per file, numbered. The format is in
[0000-modele.md](0000-modele.md).

**An ADR is never modified.** When a decision changes, a new one is written
that supersedes the old one, and the old one takes the status "superseded".
The reasoning thus stays readable, including the reasoning that turned out to
be wrong.

What these files provide that the git history does not: the history says
*what* was done; the ADR says *why the other options were dismissed*. That is
the information that is missing six months later.

| No. | Decision | Status |
| :-- | :------- | :----- |
| [0001](0001-cpp20-et-qt6.md) | Write subedit in C++20 with Qt 6 | accepted |
| [0002](0002-licence-gpl3.md) | Publish subedit under GPL-3.0-or-later | accepted |
| [0003](0003-linux-d-abord.md) | Target Linux first, without closing the door to other systems | accepted |
| [0004](0004-gestion-des-dependances.md) | Resolve dependencies through system packages | accepted |
| [0005](0005-catch2-pour-les-tests.md) | Use Catch2 v3 for tests and benchmarks | accepted |
| [0006](0006-positions-en-millisecondes.md) | Represent positions as whole milliseconds, with strong types | accepted |
| [0007](0007-cpp23-et-std-expected.md) | Move the project to C++23 and report errors through `std::expected` | accepted |
| [0008](0008-lecture-au-mieux-avec-diagnostics.md) | Open files as best we can, reporting diagnostics | accepted |
| [0009](0009-texte-en-chaine-brute.md) | Store text as a raw string carrying the format's markup | accepted |
| [0010](0010-annulation-par-commandes.md) | Undo through commands that carry their own inverse | accepted |
| [0011](0011-numero-d-image-en-type-fort.md) | Type the frame number rather than leave it an integer | accepted |
| [0012](0012-ordre-des-sous-titres-par-composition.md) | Never sort unprompted, and make strict mode a composition | accepted |
| [0013](0013-mise-a-l-echelle-exacte-des-positions.md) | Scale positions by an exact rational, rounded once | accepted |
| [0014](0014-registre-d-exigences.md) | Name requirements in a flat register, cite them by a test tag | accepted |
| [0015](0015-memoire-des-mesures.md) | Count uncovered lines, and keep a memory of performance | accepted |
| [0016](0016-cli11-pour-l-analyse-d-arguments.md) | Use CLI11 for argument parsing | accepted |
| [0017](0017-analyseur-de-mentions-ecrit-a-la-main.md) | Write the mention parser by hand, without a regular expression engine | accepted |
| [0018](0018-vocabulaire-des-formats-dans-le-modele.md) | Separate the vocabulary of formats from their operations | accepted |
| [0019](0019-table-en-adaptateur-mince.md) | Read through the core's model, which reports its changes | accepted |
| [0020](0020-libmpv-pour-le-lecteur-integre.md) | libmpv for the built-in player | accepted |
| [0021](0021-analyse-du-document-a-l-ouverture.md) | A document analysis, computed on opening | accepted |
| [0022](0022-configuration-au-noyau-et-tolerance-par-option.md) | A configuration in the core, tolerant option by option | accepted |
| [0023](0023-deb-et-rpm-pour-la-premiere-livraison.md) | Two native packages, `.deb` and `.rpm`, for the first release | accepted |
| [0024](0024-captures-engendrees-et-ou-elles-font-foi.md) | Generated screenshots, and the environment in which they are authoritative | accepted |
| [0025](0025-une-recette-d-ouverture-au-noyau.md) | A single opening recipe, in the core, and the error that comes out of it | accepted |
| [0026](0026-le-graphe-d-inclusions-reste-tel-quel.md) | Leave the include graph as it is, and why | accepted |
| [0027](0027-icu-pour-les-encodages.md) | ICU to read and write encodings | accepted |
| [0028](0028-peser-les-lignes-qui-discriminent.md) | Detection weighs only the lines that discriminate | accepted |
| [0029](0029-fins-deduites-et-annoncees.md) | Deduce the ends the file does not carry, and announce it | accepted |
| [0030](0030-ce-qu-un-document-retient-de-son-fichier.md) | What a document keeps from its file, frame rate included | accepted |
| [0031](0031-pivot-de-balises-a-la-conversion.md) | A markup pivot, for conversion only | accepted |
| [0032](0032-un-document-un-fichier.md) | Keep one file per document | accepted |
| [0033](0033-un-projet-est-une-page.md) | Make a project a page of the window | accepted |
| [0034](0034-trois-collaborateurs-de-la-fenetre.md) | Three collaborators of the window, which receive the page they target | accepted |
| [0035](0035-les-operations-sortent-de-la-fenetre.md) | The `Tools` operations leave the window | accepted |
| [0036](0036-icu-pour-les-motifs-de-correction.md) | ICU to apply the correction patterns | accepted |
| [0037](0037-lire-les-motifs-de-gaupol-tels-quels.md) | Read Gaupol's pattern files as they are | accepted |
| [0038](0038-sortie-json-lines-versionnee.md) | Offer output a script can read: JSON Lines, versioned | accepted |
| [0039](0039-le-lot-collisions-dossiers-et-arborescence.md) | A safe batch: collisions refused, directory created, tree preserved | accepted |
| [0040](0040-correct-ecrit-directement-dry-run-propose.md) | Write directly, and propose through `--dry-run` | accepted |
| [0041](0041-afficher-la-video-par-le-rendu-logiciel.md) | Display the video through libmpv's software rendering | accepted |
| [0042](0042-gettext-catalogues-read-by-our-own-reader.md) | Keep messages in gettext catalogues, read by our own reader | accepted |
| [0043](0043-english-is-the-language-of-the-repository.md) | Write the whole repository in English; French is a locale | accepted |
| [0044](0044-one-manual-feeds-the-help-and-the-man-pages.md) | One manual feeds the window's help and the man pages | accepted |
| [0045](0045-one-settings-file-two-parts.md) | One settings file, an operation part and a display part | accepted |
| [0046](0046-a-port-for-transcription-engines.md) | A port for transcription engines, with capabilities per engine and model | accepted |

[0011](0011-numero-d-image-en-type-fort.md) complements
[0006](0006-positions-en-millisecondes.md): it gives a type to the "frame
view" that 0006 names without typing it. No point of 0006 is called into
question.

[0007](0007-cpp23-et-std-expected.md) partly supersedes
[0001](0001-cpp20-et-qt6.md) on the point of the standard: C++23 rather than
C++20. The rest of 0001 — Qt 6, a core with no dependency on the interface —
remains in force.

[0021](0021-analyse-du-document-a-l-ouverture.md) gives a place to what
[0006](0006-positions-en-millisecondes.md) and
[0011](0011-numero-d-image-en-type-fort.md) made possible without naming it: an
inference about a document, distinct from what it is and from what changes it.
It made moving `anomaly.hpp` to `core/analysis/` conditional; #227 did it the
day the condition was met.

[0028](0028-peser-les-lignes-qui-discriminent.md) complements
[0027](0027-icu-pour-les-encodages.md) without superseding it: ICU remains the
detector and the converter, and what changes is what is submitted to it. Its
test bench only carried monolingual files, and it was by measuring where
detection stops knowing — #310 — that a bilingual file turned out to lose its
second alphabet entirely.

[0031](0031-pivot-de-balises-a-la-conversion.md) **honours**
[0009](0009-texte-en-chaine-brute.md) without superseding it: the pivot it
promised "only during a conversion" is built in phase 9, because nine formats
make five markup vocabularies where two used to share one. The raw string
remains what a text is on opening and on saving, and the way out through opaque
fragments that 0009 describes remains open.

**0031 calls the markup-aware parser "still not written"; it has been written
since phase 10**, issue #378, in `core/text/markup_parser`. These are indeed
the two pieces 0031 distinguished: the pivot translates one vocabulary into
another and removes what it cannot carry, the parser carries the markup around
a text being transformed and removes none of it. Case, dashes, search and
duration adjustment go through it.

**And a single reader splits the markup for both**, since #403 —
`core/text/markup_reader`. The pivot, the parser, italics and the mention
parser each read their own, and contradicted one another: `<i >` was italics
for one and nothing for another. What a tag says remains each piece's business;
where it begins, where it ends and what it names is decided in one place.

[0029](0029-fins-deduites-et-annoncees.md) and
[0030](0030-ce-qu-un-document-retient-de-son-fichier.md) hold the model in
place where two formats pulled it out: `Subtitle::end` does not become optional
for LRC and TMPlayer, and positions do not become frames for MicroDVD. What
moves is what a **document** keeps from its file, not what a **subtitle**
carries.

[0036](0036-icu-pour-les-motifs-de-correction.md) **reopens** then **upholds**
[0017](0017-analyseur-de-mentions-ecrit-a-la-main.md): the trigger 0017 had set
itself — the third pattern requested — fired in phase 12, and yet the
hand-written scan keeps square brackets and parentheses, for the reason it
gave. The four other hearing-impaired patterns go through the engine; 0017 is
not superseded. [0037](0037-lire-les-motifs-de-gaupol-tels-quels.md), for its
part, settles what that engine reads.

[0038](0038-sortie-json-lines-versionnee.md) **settles** the open point of the
[phase 3 spec](../specs/03-cli.md) — "shape of a machine-readable output" — and
complements [0008](0008-lecture-au-mieux-avec-diagnostics.md): reading
diagnostics, which the text puts at level 3, are data and always go into
`warnings`. [0039](0039-le-lot-collisions-dossiers-et-arborescence.md) holds
for the batch what phase 3 held only for a file — a usage error never leaves a
half-written batch —, and [0040](0040-correct-ecrit-directement-dry-run-propose.md)
answers the confirmation question that
[0036](0036-icu-pour-les-motifs-de-correction.md) and phase 12 had left to the
command line.
