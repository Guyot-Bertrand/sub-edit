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
