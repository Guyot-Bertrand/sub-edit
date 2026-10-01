# 0038 — Offrir une sortie lisible par un script : JSON Lines, versionnée

**Date :** 2026-10-01
**Statut :** acceptée

Décidée en cadrant la phase 13, issue
[#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542). Lève le point ouvert que
la [spec de la phase 3](../specs/03-cli.md) avait écrit : « forme d'une sortie lisible
par une machine — phase 13, quand un appelant réel en aura besoin ».

## Contexte

La ligne de commande écrit deux choses sur deux sorties : le **résultat** sur la sortie
standard — aujourd'hui, le seul rapport d'`inspect`, en blocs de texte —, et **tout le
reste** sur la sortie d'erreur, aux quatre niveaux de narration. Les sous-commandes qui
réécrivent n'écrivent rien sur la sortie standard : leur résultat *est* le fichier.

**Aucune sortie structurée n'existe**, et la phase 3 l'avait écartée avec une raison
qu'il faut garder : « rien ne la consommerait aujourd'hui ; sa forme doit être dictée par
un appelant réel ». L'appelant est venu, et il est précis : un script qui traite des
centaines de fichiers et doit savoir, **par fichier**, s'il a réussi, ce qu'il a compté
(sous-titres changés, contraintes sacrifiées, motifs abandonnés), ce que la lecture a dû
décider, et où le résultat est allé. Le code de retour ne dit que `0`, `2` ou `3` pour
tout le lot ; relire une phrase anglaise pour en extraire un nombre est exactement ce
qu'une sortie « exploitable » doit éviter, et ce que chaque correction de libellé
casserait en silence.

Le dépôt a déjà l'outil qui rend la promesse **prouvable** : `MatchesFile`
([#543](https://github.com/Guyot-Bertrand/sub-edit/issues/543)), un comparateur d'octets
contre un fichier attendu versionné, qui montre la première ligne divergente.

## Décision

**`--format json`, option globale, opt-in.** `text` reste le défaut et ne change pas.
Avec `json`, la sortie standard porte **un objet JSON par fichier d'entrée, en JSON Lines**
(un objet compact par ligne, UTF-8 sans marque, fin de ligne `\n`), dans l'ordre des
entrées. **Les sous-commandes qui réécrivent y écrivent aussi** — c'est ce qui change
leur contrat de la phase 3 — ; leur fichier reste leur résultat, et l'objet est son
compte rendu. La narration (`-q`, `-v`, `-vv`, `-vvv`) reste sur la sortie d'erreur,
**en texte, inchangée**.

### La forme

Un objet par entrée, **jamais zéro, jamais deux**, que l'entrée ait réussi ou non :

```json
{"schema":1,"command":"shift","file":"a.vtt","ok":true,"dry_run":false,"destination":"out/a.vtt","counts":{"subtitles":1356,"shifted_by_ms":-7001},"warnings":[]}
{"schema":1,"command":"shift","file":"b.vtt","ok":false,"error":{"kind":"unknown-format","message":"b.vtt: is in no format this tool knows"}}
```

| Clé | Contenu | Présente |
| :-- | :------ | :------- |
| `schema` | entier, la version de cette forme | toujours |
| `command` | le nom de la sous-commande | toujours |
| `file` | le chemin **tel que donné** sur la ligne de commande — ou tel que le parcours d'un répertoire l'a composé —, jamais normalisé | toujours |
| `ok` | booléen | toujours |
| `error` | `{"kind", "message"}` | si `ok` est faux |
| `destination` | le chemin écrit, ou `null` en `--dry-run` | si `ok` est vrai, sur une sous-commande qui écrit |
| `dry_run` | booléen | si `ok` est vrai, sur une sous-commande qui écrit |
| `counts` | un objet de **entiers**, propre à la sous-commande | si `ok` est vrai |
| `warnings` | un tableau d'objets `{"kind", "line"?, "detail"?, "settled"?}` | si `ok` est vrai |

**Les clés propres à chaque sous-commande** — celles de `counts`, et pour `inspect` la
description du fichier — sont **écrites dans le manuel**, sous forme d'un tableau par
sous-commande et d'un exemple rejoué par `manual-check`. La spec n'en copie pas la liste :
une seconde liste est une seconde chose à tenir. Les trois choix qui engagent, eux, sont
ici :

- **Aucun nombre à virgule.** Une position ou une durée est un **entier de millisecondes**
  (`…_ms`), comme le modèle (ADR 0006) ; un compte est un entier ; une cadence est une
  **chaîne** (`"23.976"`) parce qu'elle est exacte, un rationnel (ADR 0013) que la virgule
  flottante ne tient pas ; une concentration de grille est un entier en **millièmes**.
  Comparer deux sorties octet pour octet ne dépend alors d'aucun arrondi.
- **Les diagnostics de lecture vont toujours dans `warnings`**, à tous les niveaux de
  narration. Le texte les met au niveau 3 parce qu'il est fait pour être lu ; un script
  les veut toujours, et un objet dont le contenu changerait avec `-q` ne serait pas
  exploitable. **La verbosité n'agit pas sur la sortie standard en `json`.**
- **Une erreur d'usage n'écrit aucun objet** : elle est détectée avant tout traitement
  (`CLI-USAGE-03`), il n'y a donc pas d'entrée à laquelle l'attacher. Elle va sur la
  sortie d'erreur, en texte, avec le code `1`, et la sortie standard reste vide.

### Le versionnage

`schema` vaut `1`. Il est **sur chaque objet** : un flux se coupe, se concatène, se filtre,
et chaque ligne doit rester lisible seule.

**Ce qui est promis stable tant que `schema` vaut `1`** :

- les noms des clés, leur type et leur unité ;
- le sens de `ok`, de `file`, de `destination` ;
- **un objet par entrée, dans l'ordre des entrées** ; l'ordre des éléments d'un tableau
  quand il a un sens (les avertissements dans l'ordre de lecture, les anomalies par numéro
  de sous-titre) ;
- les **identifiants** `kind` — des erreurs et des avertissements — qui existent ;
- le déterminisme : mêmes entrées, mêmes arguments, mêmes octets.

**Ce qui n'est pas promis** :

- **le texte des messages** (`message`, `detail`) : c'est de l'anglais pour un humain, il
  vit dans `core/wording/` et se corrige. Un script qui le compare est un script cassé
  d'avance ;
- l'**ordre des clés** dans un objet — il est fixe, parce que le déterminisme l'exige, mais
  JSON ne le dit pas et un lecteur ne doit pas s'y fier ;
- la présence de **clés qu'on n'a pas encore ajoutées**.

**Une évolution compatible ne change pas `schema`** : ajouter une clé, ajouter un
identifiant `kind`. **Un lecteur doit donc ignorer une clé inconnue et traiter un `kind`
inconnu comme « autre »** ; c'est écrit au manuel, en tête de la section. **Une évolution
incompatible incrémente `schema`** : retirer ou renommer une clé, changer un type ou une
unité, retirer un identifiant, changer ce que dit une valeur. L'incrément est une
rupture, annoncée au CHANGELOG et au manuel ; `--format json` écrit toujours la version
courante, sans sélecteur (voir « Ce qui justifierait de rouvrir »).

### La preuve

**Les attendus sont versionnés et écrits à la main** — `src/test/data/attendus/json/`, un
fichier `.jsonl` par cas —, **lus par un test de bout en bout qui les compare par
`MatchesFile`**. Jamais la sortie du programme relue par le programme : c'est la leçon de
#338, et c'est la raison d'être de #543. Un changement de format se voit alors comme un
**diff de fixture**, jamais comme un test qui s'adapte seul.

**Un script Python** (bibliothèque standard, `json`) vérifie que **chaque ligne de chaque
attendu est du JSON valide, porte l'enveloppe, et ne contient aucun nombre à virgule** ;
`check-architecture.sh` l'appelle. Le C++ n'a pas de lecteur JSON — il n'en a pas besoin,
il n'écrit que — et le script est le second regard, indépendant de l'écrivain.

## Alternatives écartées

- **Un tableau JSON unique** — `[ {…}, {…} ]`. Il ne se lit qu'à la fin : un processus
  interrompu au milliersième fichier ne laisse rien de lisible, et le lot entier s'accumule
  en mémoire avant le premier octet. Les JSON Lines se lisent, se coupent et se filtrent
  (`grep`, `head`, `jq -c`) pendant que le lot tourne.
- **Un objet final de bilan** (`{"processed":…,"failed":…}`). Une ligne d'une autre forme
  au milieu d'un flux homogène force chaque lecteur à la distinguer, et le bilan existe
  déjà : le code de retour (`0`, `2`, `3`), et le texte du niveau 1 sur la sortie
  d'erreur. **« Un objet par entrée, jamais zéro, jamais deux »** est une propriété qu'un
  test sait énoncer ; un bilan l'aurait rendue fausse.
- **TSV ou CSV.** Les avertissements et les comptes par contrainte sont imbriqués, et les
  textes de `correct --dry-run` portent des sauts de ligne, des tabulations et des guillemets
  qu'il faudrait échapper de toute façon, dans un format que chaque outil échappe
  autrement.
- **Stabiliser les lignes de texte** (`clé: valeur`) — c'est ce qu'`inspect` écrit déjà.
  Cela engagerait chaque phrase à ne plus jamais changer, et interdirait précisément ce que
  `wording/` a besoin de faire : corriger un libellé. Le texte reste fait pour un humain,
  et on ne lui promet rien.
- **`--json`, un drapeau.** `--format` se lit mieux (`text`, `json`) et laisse la place d'un
  autre format sans nouvelle option. **Aucun autre n'est promis.**
- **Écrire un fichier de schéma JSON Schema.** Il serait une seconde description à tenir,
  à côté des tableaux du manuel et des attendus ; il se justifierait le jour où un
  consommateur outillé le demande.
- **Une bibliothèque JSON** (nlohmann, RapidJSON) — une dépendance de plus
  ([ADR 0004](0004-gestion-des-dependances.md), payée dans le `.deb`, le `.rpm` et la CI)
  pour écrire une centaine de lignes. Un écrivain minimal, conforme à la RFC 8259, suffit
  puisqu'on n'a **rien à lire**.
- **Une version du schéma dans la valeur de l'option** (`--format json:1`). Prématuré :
  il n'y a pas de seconde version. Le sélecteur s'ajoute le jour où un incrément
  l'exige, sans rien casser.

## Conséquences

**Un résultat structuré remplace la phrase.** `cli::Operation` rend aujourd'hui une chaîne
(« 4000 subtitles shifted by 2.999 s »), et un compte ne se retrouve pas dans une phrase.
Chaque opération rend désormais **ses comptes**, et la phrase du niveau 1 s'écrit **depuis
ces mêmes comptes**, par `core/wording/` quand la fenêtre dit la même chose : le texte et
le JSON ne peuvent plus diverger, parce qu'ils viennent du même objet. C'est la discipline
que #541 avait inscrite pour toute sous-commande neuve, tenue ici par construction.

**Un écrivain JSON dans `subedit_cli`**, sans dépendance, avec ses propres tests :
échappement des guillemets, des barres obliques inverses et des caractères de contrôle ;
les autres caractères — les accents, les caractères hors du plan de base — sortent tels
quels, en UTF-8.

**Les chemins qui ne sont pas de l'UTF-8** — possibles sous Linux — ne se représentent pas
en JSON. Ils sont écrits avec U+FFFD à la place des octets invalides, et l'objet porte un
avertissement `path-not-utf8` : la fidélité n'est pas promise pour eux, et c'est dit.

**Coût de défaire** : élevé, et c'est pourquoi cette ADR existe. Un script écrit contre
`schema` 1 est dans les mains de quelqu'un que le dépôt ne voit pas. Ajouter des clés ne
coûte rien ; retirer l'une d'elles coûte un incrément et la patience de qui l'avait lue.

**Ce qui justifierait de rouvrir** : un consommateur qui a besoin de lire l'ancienne forme
après un incrément (le sélecteur de version) ; un format de plus que demanderait un
appelant réel ; ou la preuve que l'ordre des objets ne suffit pas à un lot parallèle, si
le lot le devient (ADR 0039).
