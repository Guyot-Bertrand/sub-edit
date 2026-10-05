# Les fichiers attendus

Ce que telle sous-commande doit écrire pour telle entrée, octet pour octet. Un
test de bout en bout les compare à la sortie du programme avec `MatchesFile`
(`src/test/e2e/cli/cli_run.hpp`), qui dit la première ligne qui diffère. Issue
[#543](https://github.com/Guyot-Bertrand/sub-edit/issues/543).

**Un attendu ne s'engendre jamais par le programme testé.** Un fichier que l'outil
a écrit puis qu'un test relit ne prouve que l'accord de l'outil avec lui-même
(leçon de [#338](https://github.com/Guyot-Bertrand/sub-edit/issues/338)). Chaque
fichier ci-dessous dit d'où il vient.

| Fichier | Entrée | Provenance |
| :------ | :----- | :--------- |
| `mentions.hearing-impaired.srt` | `valides/mentions.srt`, sous-commande `hearing-impaired` | **écrit à la main**, d'après l'entrée et la règle du retrait : les mentions entre crochets et parenthèses partent, un sous-titre qu'elles vident disparaît et les suivants sont renumérotés, une référence numérique `[1]` reste, le tiret de dialogue resté seul s'en va, la mention coupée par le retour à la ligne emporte l'espace qui l'entourait et laisse deux lignes. La sortie réelle n'a servi qu'à vérifier cet attendu, jamais à le copier. |

## `json/` — la sortie de `--format json`

Un fichier `.jsonl` par cas, **écrit à la main**, d'après ce que l'ADR 0038 dit de la
forme et ce que l'entrée contient : un objet par entrée, dans l'ordre, l'enveloppe, les
comptes que chaque sous-commande déclare. Le test de bout en bout (`json_test.cpp`) les
compare à la sortie standard par `MatchesFile`, **après avoir remplacé le dossier
temporaire du test par `<scratch>` et celui du corpus par `<corpus>`** — ils changent d'une
exécution et d'une machine à l'autre, et un attendu qui les contiendrait ne serait vrai
nulle part. `src/scripts/check-json-fixtures.py` les relit avec un parseur qui n'est pas le
nôtre : JSON valide, enveloppe, aucun nombre à virgule.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `json/inspect-batch.jsonl` | `inspect` sur un fichier lisible, un absent et un illisible | écrit à la main ; la sortie réelle n'a servi qu'à vérifier |
| `json/inspect-grid.jsonl` | `inspect` sur `grilles/grille-24-courte.srt` et sur un fichier compté en images | à la main, d'après la table de `grilles/LISEZMOI.md` (24 images/s, étendue de 10 s, deux cadences non séparées) ; `concentration_permille` lu sur la mesure, 99,9 % |
| `json/shift-batch.jsonl` | `shift` sur trois entrées dont la deuxième est illisible | à la main : « jamais zéro, jamais deux » |
| `json/writers.jsonl` | `transform`, `framerate`, `snap`, `hearing-impaired`, `convert` sur un même fichier | à la main : les comptes que chacune déclare, tous nuls sauf les sous-titres |
| `json/warnings.jsonl` | un fichier SubRip sans numérotation, à chaque niveau de narration | à la main : un diagnostic `missing-numbering` par bloc, `settled` puisque la lecture a numéroté |
| `json/adjust.jsonl` | `adjust` sur un fichier de quatre sous-titres, treize réglages (défauts, `off`, `--shorten`, `--maximum`, `--gap`, `--range`…) | **à la main**, d'après l'ordre de Gaupol (vitesse, minimum, maximum, écart — l'écart gagne) : chaque fin et chaque compte a été calculé sur le papier avant d'être comparé ; `constraints` dit les réglages employés. Le fichier d'entrée et les fins attendues sont dans `src/test/e2e/cli/adjust_test.cpp` |
| `json/recoupement-defaut.jsonl`, `json/recoupement-ecart.jsonl` | `adjust --dry-run --maximum 6` (et `--gap 0.5`) sur les neuf fichiers de `durees/` | **à la main**, à partir des définitions de la place et du besoin — voir `durees/LISEZMOI.md` ; le script de mesure est confronté aux mêmes lignes |
| `json/dry-run-writers.jsonl` | les six sous-commandes qui écrivent, en `--dry-run` et sans destination | à la main : `dry_run` vrai, `destination` nul, les comptes de `writers.jsonl` ; `changes` vide pour `hearing-impaired`, qui liste ses changements |
| `json/dry-run-hearing-impaired.jsonl` | `hearing-impaired --dry-run` sur `valides/mentions.srt` | à la main, d'après l'entrée et la règle (voir `mentions.hearing-impaired.srt`) : les sous-titres 1 (supprimé, `after` nul), 2, 4 et 5 changent, numérotés comme dans le fichier lu ; le 3, une référence, reste |

## `replace/` — ce que `replace` écrit

Écrits à la main, **d'après les règles de `textes/recherche.cas`** — la balise qui coupe le mot cherché
l'englobe, ce que la correspondance ne touche pas ne bouge pas — et jamais lus dans la sortie du programme.
L'entrée est écrite par `src/test/e2e/cli/replace_test.cpp`, dont c'est le commentaire de tête qui dit
pourquoi chaque sous-titre est là.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `replace/bonjour-salut.srt` | le fichier écrit par `replace Bonjour Salut` : une balise coupait le mot dans le sous-titre 2, un style suit la correspondance dans le 3 | à la main, règles 2 et 4 de `recherche.cas` |
| `replace/dry-run.txt` | la sortie standard de `replace --dry-run Bonjour Salut` : les sous-titres 2 et 3, avant et après | à la main : un bloc par sous-titre changé |
| `json/replace-dry-run.jsonl` | les mêmes changements en JSON, avec `counts.replaced` et `counts.matched` | à la main |

## `texte/` — ce que `case`, `italics` et `dialogue-dashes` écrivent

Écrits à la main, **d'après les corpus de règles de `textes/`** (`casse-titre.cas`, `tirets.cas`…) et la
documentation de `italics` : la balise garde sa casse et sa place, un tiret de tête n'est pas touché, un
italique déjà là n'est pas doublé. Les entrées sont écrites par `src/test/e2e/cli/text_rewriting_test.cpp`.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `texte/casse-titre.srt` | `case --to title` sur six textes : un ordinaire, un crié, une balise en capitales, une balise qui coupe un mot, deux lignes, un texte déjà conforme | à la main, règles de `casse-titre.cas` |
| `texte/italiques-on.srt` | `italics --on` sur quatre textes, dont un italique coupé et un texte déjà en italique | à la main |
| `texte/tirets-add.srt` | `dialogue-dashes --add` sur cinq textes, dont un déjà tiré et une balise ouvrante | à la main, règles de `tirets.cas` |
| `json/texte-dry-run.jsonl` | les trois sous-commandes en `--dry-run --format json` : `counts.changed` et `changes` | à la main |

## `sort/` — ce que `sort` écrit

Écrits à la main : trier les débuts, garder les égalités dans l'ordre du fichier. Les entrées sont écrites
par `src/test/e2e/cli/sort_test.cpp`.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `sort/desordre.srt` | trois sous-titres dont le premier du fichier commence le dernier | à la main : un, trois, cinq, renumérotés |
| `sort/egalite.srt` | deux sous-titres qui commencent ensemble : le tri est stable | à la main : `a`, puis `b-first` avant `b-second`, dans l'ordre du fichier |
| `json/sort.jsonl` | un lot de trois : désordre (3 places changées), égalité (2), déjà en ordre (0) | à la main |

## `dry-run/` — la sortie de `--dry-run`

Comparée par `dry_run_test.cpp`, **après avoir remplacé le dossier temporaire du test par
`<scratch>` et celui du corpus par `<corpus>`**, comme les attendus JSON. Écrits à la main.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `dry-run/narration.txt` | la sortie d'erreur des six sous-commandes qui écrivent, en `--dry-run` | à la main : la phrase de chaque opération, puis `(dry run, nothing written)` à la place de `-> <destination>` |
| `dry-run/hearing-impaired.txt` | la sortie standard de `hearing-impaired --dry-run` sur `valides/mentions.srt` | à la main : un bloc par sous-titre changé, `- ` devant le texte d'avant, `+ ` devant celui d'après, `(removed)` pour le sous-titre vidé ; le tiret de dialogue de l'avant donne `- - ` |

## `translation/` — ce qu'un fichier de traduction écrit

Écrit à la main, **d'après la règle du retrait des mentions** (voir `mentions.hearing-impaired.srt`) appliquée
au texte de la traduction, et jamais lu dans la sortie du programme. L'entrée est écrite par
`src/test/e2e/cli/translation_test.cpp`, posée sur les quatre sous-titres de `paires/temoin/principal.srt`.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `translation/mentions.hearing-impaired.traduction.srt` | le fichier écrit par `hearing-impaired --document translation -t` : la mention du premier sous-titre est tout son texte, qui **s'écrit vide et garde le sous-titre** ; celle du deuxième emporte l'espace qui l'entoure ; la référence `[1]` du troisième reste | à la main : quatre blocs en entrée, quatre en sortie, **le seul écrivain de la traduction étant celui du principal** — un bloc sans texte se lit et revient à l'octet, comme `paires/LISEZMOI.md` le dit |

## `append/` — ce que `append` écrit

Écrits à la main : la fin du dernier sous-titre de ce qui précède est le décalage de ce qui suit
(quatre secondes, puis cinq et demie), et les sous-titres sont renumérotés à partir de 1. Les entrées
sont écrites par `src/test/e2e/cli/append_test.cpp`.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `append/deux-fichiers.srt` | deux sous-titres, puis un qui commence à 0,5 s : il commence à 4,5 s | à la main : 0,5 + 4,0, la balise d'italique gardée |
| `append/trois-fichiers.srt` | les mêmes, puis un fichier WebVTT : il suit la fin du troisième (5,5 s) | à la main : 1,0 + 5,5 = 6,5 s |
| `json/append.jsonl` | trois fichiers, le dernier en Advanced SSA : l'objet dit ce que chacun a coûté | à la main : une balise `{\pos}` et l'en-tête perdus |

## `split/` — ce que `split-file` écrit

Écrits à la main : la queue est ramenée à l'origine de la fin du dernier sous-titre resté (4 s), et
les deux moitiés sont renumérotées à partir de 1. L'entrée est écrite par
`src/test/e2e/cli/split_file_test.cpp` : quatre sous-titres, terminant à 2, 4, 7 et 9 s, coupés au troisième.

| Fichier | Cas | Provenance |
| :------ | :-- | :--------- |
| `split/coupe-tete.srt` | les deux premiers sous-titres, tels quels | à la main |
| `split/coupe-queue.srt` | les deux derniers : 6→2 s, 7→3 s, 8→4 s, 9→5 s | à la main : moins 4 s |
| `json/split-file.jsonl` | la coupe au troisième : `destination` est la tête, `tail` la queue | à la main |
