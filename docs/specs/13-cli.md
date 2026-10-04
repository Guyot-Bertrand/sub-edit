# Phase 13 — CLI complète

Cadrage de l'issue [#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542),
après l'initialisation [#541](https://github.com/Guyot-Bertrand/sub-edit/issues/541).

## Ce que la feuille de route promet, et ce qui est déjà là

« Sous-commandes destinées à un usage réel : conversion, décalage, transformation,
alignement sur une grille, ajustement des durées, correction, inspection. Traitement par
lot, sortie lisible par un humain et sortie exploitable par un script. » Plus trois renvois
qui atterrissent ici : l'ajustement des durées, à recouper avec
`measure-duration-constraints.py` (phase 10, [#407](https://github.com/Guyot-Bertrand/sub-edit/issues/407)) ;
la traduction en ligne de commande (phase 11, D9) ; la correction en ligne de commande, et
la question de ce qui remplace sa confirmation (phase 12, D8).

**Gaupol n'a pas d'équivalent** : sa ligne de commande ne fait qu'ouvrir la fenêtre
(`FILE…`, `-e/--encoding`, `--list-encodings`, `-t/--translation-file`, `-a/--align-method`,
`-v/--video-file`, `+NUM`). **C'est une conception neuve** : pas d'oracle, pas de table
d'écarts de cent lignes — ce que la spec confronte, c'est ce que le noyau sait faire et
ce que la ligne de commande en montre.

**La moitié du travail est déjà faite**, et c'est elle qui rend cette phase honnête :

| Ce qui est déjà là | Où |
| :----------------- | :- |
| **sept sous-commandes** : `inspect`, `convert`, `shift`, `transform`, `framerate`, `snap`, `hearing-impaired` — la conversion, le décalage, la transformation, l'alignement sur une grille et l'inspection de la promesse | phases 3, 4 et 16 |
| une grammaire commune — temps, indices, cadence, encodage, destination, verbosité — et **quatre codes de retour** | `src/lib/subedit/cli/`, [spec 03](03-cli.md) |
| `--help` engendré par CLI11, et **un contrôle que le manuel et l'aide disent la même chose** | [#546](https://github.com/Guyot-Bertrand/sub-edit/issues/546) |
| un comparateur de **fichiers attendus versionnés**, qui montre la première ligne divergente | [#543](https://github.com/Guyot-Bertrand/sub-edit/issues/543), `MatchesFile` |
| un **lot engendré** dans le `Scratch` d'un test, et les cas e2e d'un lot qui écrit — **qui fixent le comportement actuel, collision comprise** | [#544](https://github.com/Guyot-Bertrand/sub-edit/issues/544) |
| les phrases partagées de `core/wording/`, **découpées en familles** (`analysis`, `conversion`, `counts`, `editing`, `formats`, `settings`, `translation`, `video`) | [#545](https://github.com/Guyot-Bertrand/sub-edit/issues/545), #549 |
| au noyau, **tout ce que les phases 10 à 12 ont écrit** : l'ajustement des durées, la recherche et le remplacement, la casse, les italiques, les tirets de dialogue, la fusion et la scission, le tri, l'ajout d'un fichier, la scission d'un projet, l'ouverture d'une traduction, la correction (motifs, découpage de lignes, jonction et scission de mots), leurs comptes rendus | phases 10, 11, 12 |

**Ce qui n'y est pas.** Aucune sortie structurée ; aucun parcours de répertoire ; aucune
garde sur le lot qui écrit ; **aucune des opérations de la dernière ligne n'est exposée** — sauf
une partie du retrait des mentions, que `hearing-impaired` fait depuis la phase 4.

## Analyse préalable — l'état réel du code

Lue dans `src/exe/cli/application.cpp`, `src/lib/subedit/cli/`, `core/edit/`,
`core/text/correction_run.hpp`, `core/config/`, `core/wording/`, `docs/manual/subedit-cli/`
et `docs/specs/03-cli.md`. Ce qui suit corrige ou précise ce que le corps de #541 et de
#542 supposaient ; chaque point gouverne une décision.

**La CLI n'a pas besoin de Qt, sauf pour deux choses, et elles se contournent.**
`subedit_cli` ne lie que `subedit::core` ; le noyau ne connaît ni Qt ni aucune interface
(invariant 1 de `check-architecture.sh`) ; CLI11 reste dans l'exécutable. Les deux seules
dépendances à Qt que la phase rencontre sont :

- **la mesure en *ems*** — `EmsLineMeasure`, `QFontMetricsF`, dans `gui/`. La ligne de
  commande s'en tient aux **caractères** (`CharacterLineMeasure`, au noyau), comme la
  feuille de route le dit : ce n'est pas un compromis, c'est la seule mesure qui existe sans
  police.
- **la résolution des emplacements des motifs**, que l'ADR 0037 avait promise « hors de
  `gui` » et qui **n'y est pas** : `installedPatternsPath()` et `resolvedUserPatternsPath()`
  sont dans `gui/patterns_path.cpp` et appellent `QCoreApplication::applicationDirPath()` et
  `qgetenv`. Le noyau en fournit la partie pure (`shippedPatternsPath`, `userPatternsPath`) ;
  **il manque l'appel côté ligne de commande**, sans Qt. C'est une issue de la tranche de la
  correction ([#566](https://github.com/Guyot-Bertrand/sub-edit/issues/566)).

**Tout le reste est au noyau et se branche tel quel** : `proposeCorrections` et
`applyCorrections`, `readPatternCatalogue`, `openSpellChecker` et `EnchantSpellProvider`
(la dépendance à Enchant est **déjà** tirée par `subedit-cli`), `adjustDurations`,
`SearchPattern`, `replaceAll`, `setLetterCase`, `setItalics`, `setDialogueDashes`,
`openTranslation` et `attachTranslation`, `SortCommand`, `appendFile`, `splitProject`.

**Une sous-commande de plus coûte dix à vingt et un fichiers** (conclusion de #541) :
structure d'options, description, exécution, fonction du module, deux tests, page de manuel,
tableau d'`invocation.md`, deux listes de la page `subedit-cli(1)`, exigences `CLI-*`. Et
**tout vit dans un seul fichier**, `application.cpp` : 575 lignes pour sept sous-commandes,
une chaîne de `if (…->parsed())`, pas de table. **À dix-sept, ce fichier ferait de l'ordre de
mille quatre cents lignes** (quatre-vingts par sous-commande) — l'histoire de `MainWindow`, qui avait demandé quatre issues
pour être allégée après coup (ADR 0034, 0035). L'extraction passe avant la première
sous-commande neuve ([#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553)).

**`Operation` rend une phrase, et un compte ne se retrouve pas dans une phrase.** `rewriteAll`
prend un `std::function<std::expected<std::string, std::string>(Session&)>` : « 4000
subtitles shifted by 2.999 s ». Une sortie structurée ne peut pas se dériver de cette
chaîne ; **le résultat d'une opération doit devenir un objet**, dont la phrase est une
lecture. C'est le changement de fond de la tranche 1 — et la garantie que le texte et le
JSON ne divergeront pas.

**Les phrases des phases 10 à 12 ne sont pas encore consommées par `cli/`.**
`noticeOfAdjustment`, `noticeOfCorrection`, `noticeOfReplaceAll`, `noticeOfItalics`,
`noticeOfRecase`, `noticeOfDialogueDashes`, `noticeOf(TranslationOutcome)`,
`noticeOfSplit`, `noticeOfAppend`, `noDictionaryFor`, `reasonOf(PatternError)` existent et
attendent leur sous-commande. Seules `noMentionToRemove`, `noticeOfMentionsRemoved` et
`shiftBeforeTheOrigin` sont lues des deux côtés (#545). **Les phrases propres à la ligne de
commande** — « 4000 subtitles shifted by… », « retimed from… » — restent dans `cli/` : la
règle de `CLAUDE.md` porte sur les mots que les **deux** surfaces disent, non sur ceux d'une
seule.

**Le lot est une boucle, et ses trois comportements implicites sont sur le manuel.** Voir
l'ADR 0039 : collision sans garde, dossier non créé, destination écrasée ; et un quatrième,
trouvé en lisant `Destination::pathFor` : **`--output-dir .` écrase les entrées sans
`--in-place`**. Le libellé d'une écriture qui échoue dit « `cannot be read` », parce que
`reasonOf(FileErrorKind)` est la phrase de la lecture.

**Une traduction est un fichier de sous-titres comme un autre** — c'est ce que l'analyse
apporte de plus déroutant. `subedit-cli correct x.fr.srt` traite ce fichier comme un
principal, sans rien de nouveau. Ce que l'appariement ajoute est donc étroit : le
**rapport d'alignement** (lignes rattachées, sous-titres nés, sans traduction, hors
d'ordre), et le geste qu'**aucun fichier seul ne fait** — écrire la traduction recalée sur
les positions du principal. D10 en tire les conséquences.

**Les comptes de `adjustDurations` et du script ne se comparent pas directement** : voir
D7, où ce que le recoupement exige de la sortie est écrit.

**Le corpus privé**, dit globalement : soixante-dix-neuf fichiers, surtout WebVTT, en une
racine et six répertoires — de quoi faire un lot réaliste, pas de quoi éprouver la
traduction. Aucun test ne le lit, aucune mesure ne lui est attribuée ; **les lots des tests
sont engendrés** (#544).

## D1 — Le périmètre : tout ce que le noyau expose et qui a un sens en ligne de commande

**Critère : une sous-commande se justifie par un usage réel, c'est-à-dire par un script, un
lot ou un fichier qu'on n'ouvre pas.** Un miroir de la fenêtre qui n'aurait de sens
qu'avec une table sous les yeux, un presse-papiers ou une sélection à la souris n'en est
pas un.

| Opération du noyau | Verdict | Sous-commande, ou raison |
| :----------------- | :------ | :----------------------- |
| ajustement des durées | **retenue** | **`adjust`** — D7 |
| correction (motifs, découpage, jonction et scission de mots) | **retenue** | **`correct`** — D8 |
| recherche et remplacement | **retenue** | **`replace`** — D9. Il n'y a pas de `find` : `replace --dry-run` liste les correspondances avec ce qu'elles deviendraient, et c'est ce qu'un `find` rendrait en moins |
| casse (titre, phrase, majuscules, minuscules) | **retenue** | **`case`** — D9 |
| italiques | **retenue** | **`italics`** — D9. Mettre tout un fichier en italique n'est pas l'usage ; **les retirer**, et le faire sur une plage, l'est |
| tirets de dialogue | **retenue** | **`dialogue-dashes`** — D9 |
| tri des sous-titres | **retenue** | **`sort`** — D9. Le noyau ne trie jamais de lui-même (ADR 0012) : `sort` est le geste qu'on fait **exprès**, et qu'un lot de fichiers désordonnés réclame |
| traduction | **retenue, comme options** | `-t/--translation-file`, `--document`, `--align-method` sur `inspect` et les sous-commandes de texte — D10 |
| liste des encodages | **retenue** | `--list-encodings`, iso-fonctionnel avec Gaupol ; ICU les énumère ([#564](https://github.com/Guyot-Bertrand/sub-edit/issues/564)) |
| ajout d'un fichier à la suite d'un autre | **retenue, derrière une porte** | **`append`** — D12. L'usage est réel (un film en deux parties), l'arité ne l'est pas : N entrées, une sortie |
| scission d'un projet | **retenue, derrière une porte** | **`split-file`** — D12 : l'inverse exact de `append` |
| une traduction recalée sur le principal | **retenue, derrière une porte** | **`pair`** — D10, D12 : la seule chose que l'appariement fait et qu'un fichier seul ne fait pas |
| fusion de sous-titres | **écartée** | une plage de numéros qu'on lit dans une table ; la ligne de commande n'a pas de table où la voir, et un script ne sait pas quelle ligne fusionner. Déclencheur : un appelant qui sait quelle ligne |
| scission d'un sous-titre en deux | **écartée** | la seconde moitié naît **vide** : l'opération attend qu'on y tape un texte, et la ligne de commande n'a pas d'« ensuite ». Même déclencheur |
| presse-papiers (copier, couper, coller) | **écartée** | un geste de la fenêtre : il n'y a pas de presse-papiers pour un processus qui se termine |
| insérer, supprimer des sous-titres à la main ; saisir un texte, une position, une durée | **écartée** | édition d'un sous-titre qu'on a sous les yeux ; `replace` et `adjust` couvrent ce qui est automatique |
| vérification orthographique interactive (`Check Spelling…`) | **écartée** | un parcours de mots inconnus qu'on accepte un à un ; **la jonction et la scission automatiques** sont dans `correct`. Une liste des mots inconnus (`misspelledSpellRanges` existe) est un candidat : aucun appelant ne l'a demandée |
| annuler, refaire | **écartée** (phase 3) | un processus qui se termine n'a rien à annuler |
| dépassement de la fin de la vidéo (`BeyondEnd`) | **écartée** | la ligne de commande n'associe aucune vidéo à un fichier ; il faudrait `--video-duration`, et personne ne l'a demandé. Déclencheur : une demande |
| `Preview`, lecteur intégré, vignette | **écartée** | la fenêtre |

**Sept sous-commandes de plus, trois derrière une porte.** Ce qui est écarté l'est avec son
déclencheur : un outil écarté sans trace revient à chaque phase.

**Les sept existantes ne bougent pas.** Les six qui écrivent gagnent les trois fondations —
`--format json`, `--dry-run`, le lot sûr —, et `inspect` gagne `--format json` : **la boucle du
lot, celle de `rewriteAll` comme celle de `convert`, les porte pour elles**, dès que l'opération
rend un objet et non une phrase (tranche 1).

## D2 — Une invocation se suffit : aucun réglage n'est lu

**La ligne de commande ne lit aucun fichier de réglages, n'en écrit aucun, et ne retient
rien d'une invocation à l'autre.** Le résultat d'un lancement ne dépend que de ses
arguments, des fichiers qu'on lui donne, et de ce que l'installation fournit. Le dossier de
configuration d'ADR 0022 reste **celui de la fenêtre** ; `gui::userSettingsPath()` demeure le
seul code qui en résout un, et rien dans `cli/` ne l'appelle.

**Ce qui fait exception, et ne se lit qu'en lecture** :

| Lu | Pourquoi |
| :- | :------- |
| **les motifs livrés**, `share/subedit/patterns`, trouvés à partir de l'exécutable | ils *sont* la correction ; une installation sans eux le dit (diagnostic du catalogue) |
| **les motifs de l'utilisateur**, `$XDG_DATA_HOME/subedit/patterns` | l'iso-fonctionnalité de l'ADR 0037 : on les dépose, ils s'appliquent. Un test ne le résout jamais : le harnais déplace `XDG_DATA_HOME` |
| **le dictionnaire du système et la liste de mots personnelle d'Enchant** | c'est le correcteur (D6 de la spec 12) ; les tests déplacent `ENCHANT_CONFIG_DIR` |

**Ce qui ne se lit pas, et pourquoi.**

- **Les dérogations d'activation** (`correction.activations`) : la ligne de commande part
  des `.conf` livrés et se règle par `--enable` / `--disable` (D8). Lire les dérogations
  de la fenêtre ferait dépendre un script de ce que son auteur y a coché un jour — et un
  test de ce que son auteur y a coché un jour, ce que `check-config-home.sh` existe
  pour empêcher.
- **La liste de remplacements** `spell-check/<langue>.repl` : elle se *remplit* par les
  gestes de la fenêtre. La ligne de commande passe un répertoire de configuration **vide** à
  `spellReplacementFile`, qui rend alors un chemin vide, **qui ne se lit ni ne s'écrit**
  (#530). Ce n'est pas un oubli : c'est la seule différence de résultat entre les deux
  surfaces pour une même correction, et elle est inscrite aux écarts.
- **Les défauts de l'assistant** — tâches cochées, langue, découpage — : **les réglages de
  la fenêtre ne sont pas les défauts de la ligne de commande**, et plusieurs ne pourraient
  pas l'être (D8 : 24 et 3 sont des *ems*).

**Il n'y a pas non plus de langue ambiante.** La fenêtre prend, pour le correcteur, la
langue du système quand rien n'est choisi ; une ligne de commande qui en déduirait
une répondrait autrement sur deux machines. **La langue se nomme.**

## D3 — La sortie lisible par un script : `--format json`

**Tranché par l'[ADR 0038](../adr/0038-sortie-json-lines-versionnee.md)**, et c'est la
décision de l'utilisateur : **`--format json`, opt-in, option globale**. Sur la sortie
standard, **un objet JSON par fichier d'entrée** (JSON Lines) : pour `inspect`, la
description du fichier ; pour chaque sous-commande qui réécrit, **le compte rendu** — le
fichier, `ok`, les comptes, les avertissements, la destination. Le texte reste le défaut ;
la narration (`-q`, `-v`, `-vv`, `-vvv`) reste sur la sortie d'erreur.

**Pourquoi un format structuré et non des lignes stables** : une promesse de stabilité sur
des phrases interdit de corriger une phrase, et `wording/` doit pouvoir le faire. **Pourquoi
des JSON Lines et non un tableau** : un lot s'écoule, un tableau ne se lit qu'à la fin.
L'ADR dit les alternatives et ce qui est promis stable.

**Trois propriétés tiennent la promesse**, et chacune est une exigence :

1. **Un objet par entrée, jamais zéro, jamais deux**, échec compris : le lecteur compte les
   lignes et retrouve ses fichiers.
2. **La verbosité n'agit pas sur la sortie standard.** Les diagnostics de lecture, que le
   texte range au niveau 3, sont des données et sont toujours dans `warnings`.
3. **Aucun nombre à virgule.** Millisecondes entières, comptes entiers, cadences et
   rationnels en chaînes, concentrations en millièmes.

**Ce que la preuve est.** Des **attendus versionnés**, écrits à la main
(`src/test/data/attendus/json/`), comparés par `MatchesFile` (#543) — jamais la sortie du
programme relue par le programme —, plus un script Python indépendant qui vérifie qu'ils
sont du JSON valide, qu'ils portent l'enveloppe et ne contiennent aucun nombre à virgule.
**Un changement de format est un diff de fixture.**

**Ce que les objets portent**, résumé — le manuel porte les tableaux complets, rejoués par
`manual-check` :

| Sous-commande | `counts` |
| :------------ | :------- |
| `inspect` | pas de `counts` : la description — format, encodage et son origine, marque, fins de ligne, nombre de sous-titres, étendue en millisecondes, grille déduite ou cadence, anomalies par numéro |
| `convert` | `subtitles` ; la perte, poste par poste (`ConversionLoss`) |
| `shift`, `transform`, `framerate`, `snap` | `subtitles`, et ce qu'elles disent déjà : le décalage en ms, les positions bougées, le plus grand déplacement en ms |
| `hearing-impaired` | `cleaned`, `removed` |
| `adjust` | `subtitles`, `adjusted`, `sacrificed.{speed,minimum,gap}` — **et les contraintes employées**, D7 |
| `correct` | `corrected`, `removed` ; `failures` en avertissements — D8 |
| `replace` | `replaced` (correspondances remplacées dans les sous-titres dont le texte a changé — le nombre que dit la phrase), `matched` (correspondances trouvées, que le texte ait changé ou non : ce qui distingue « rien trouvé » de « rien à changer ») |
| `case`, `italics`, `dialogue-dashes` | `changed` |
| `sort` | `subtitles`, `moved` (les places qui ont changé de sous-titre) |

## D4 — Le lot : sûr, créé, arborescent

**Tranché par l'[ADR 0039](../adr/0039-le-lot-collisions-dossiers-et-arborescence.md)**, sur
les décisions de l'utilisateur et sur les constats de #544. En une phrase par règle :

- **deux entrées de même destination sont refusées avant tout écrit** — erreur d'usage,
  code `1`, les deux entrées nommées — ; il en va de même d'**une destination qui est une
  entrée** sans `--in-place` ;
- **le dossier de sortie est créé** s'il manque ; **une destination existante est écrasée**,
  par écriture atomique, **et le manuel le dit en tête de la section de la destination** ;
- **`--recursive` / `-r` et des répertoires en entrée**, parcours trié, sans lien
  symbolique, extensions des formats connus (`.txt` exclu du parcours), **arborescence
  conservée sous `--output-dir`**, relative au répertoire donné ;
- **le libellé d'une écriture dit « written »**, et le manuel y est aligné.

**La liste des entrées est établie en entier avant d'écrire** : c'est ce qui rend ces gardes
vérifiables, et ce qui fait de `--format json` « un objet par entrée » sans condition.

**Le lot reste séquentiel.** Sa mesure naît avec le parcours (#541 l'avait écartée tant qu'il
n'y avait rien à parcourir) : N fichiers engendrés contre le même travail en un seul,
journalisé par `make bench`.

## D5 — Écrire directement ; proposer par `--dry-run`

**Tranché par l'[ADR 0040](../adr/0040-correct-ecrit-directement-dry-run-propose.md)**, sur
la décision de l'utilisateur : **`correct` écrit directement**, et **`--dry-run` n'écrit
rien et imprime les changements proposés — le texte avant et après, par sous-titre — sur la
sortie standard**. C'est le « mode qui propose sans appliquer » que la phase 12 laissait à
celle-ci, et il **remplace** la page de confirmation : mêmes changements, même fonction du
noyau, sans l'étape d'accepter ou de refuser.

**Il est uniforme.** Toute sous-commande qui écrit l'accepte ; il n'exige aucune destination,
vérifie celle qu'on donne, et rend le code d'un vrai lancement. Sur les sous-commandes de
texte, la sortie standard est la liste des changements ; sur les autres, ce que leur sortie
dirait. **Pourquoi `replace` en même temps que `correct`** : voir ce qu'on va remplacer avant
de le faire est le même besoin, et une grammaire à part serait un second mécanisme.

## D6 — La sélection : `--range`

La fenêtre agit sur une sélection ; la ligne de commande, par défaut, sur **tout le
fichier**. **`--range N-M`** (compté à partir de 1, **bornes comprises**, `N-` jusqu'à la
fin) restreint l'opération — l'indice a déjà sa grammaire (`index_grammar`), et un indice hors
bornes est refusé **en nommant la borne**, avant tout traitement.

**Pour quelles sous-commandes** : celles dont l'opération du noyau prend une `Selection` et
dont la plage a un sens — `adjust`, `replace`, `case`, `italics`, `dialogue-dashes`,
`correct`. **Les sept existantes ne la prennent pas** dans cette phase : ce n'est pas un oubli,
c'est le périmètre (point ouvert 6).

**C'est aussi l'outil du recoupement de D7** : un désaccord entre deux comptes se localise en
rétrécissant la plage jusqu'à un sous-titre.

**Une plage vaut pour chaque fichier du lot**, ce qui n'a de sens que si les fichiers se
ressemblent ; c'est une limite, dite au manuel, non une erreur — un lot de fichiers
différents n'a rien à faire d'une plage.

## D7 — `adjust`, et ce que le recoupement de #407 exige

```
subedit-cli adjust [--speed CPS|off] [--shorten] [--no-lengthen]
                   [--minimum TIME|off] [--maximum TIME] [--gap TIME|off]
                   [--range N-M] [--dry-run]
                   (--output … | --output-dir … | --in-place) <file>...
```

**Les défauts sont ceux de `DurationConstraints{}`, qui sont ceux de Gaupol et de la
première ouverture de la fenêtre** : vitesse de 15 caractères par seconde qui allonge
seulement, minimum de 1,5 s, **pas de maximum**, écart de 0. `off` éteint une contrainte
(*absente, pas nulle*, comme la spec 10 : un minimum de zéro vaut zéro) ; `--maximum`
l'allume. **Sans aucune contrainte active, l'opération est refusée** (`isAny()`) : erreur
d'usage.

**Le compte rendu dit ce qui a été sacrifié**, par `noticeOfAdjustment` — ADR 0008 : c'est
la seule chose que la phase 10 ajoute à Gaupol, qui viole en silence.

**Le recoupement.** [#407](https://github.com/Guyot-Bertrand/sub-edit/issues/407) a
inscrit ce que la ligne de commande devait permettre : relever, sur les mêmes fichiers et
avec les mêmes réglages, ce que le script prédit et ce que le noyau déclare, et **chercher
l'erreur de l'un des deux** si elles diffèrent. Ce que le cadrage établit, en relisant les
deux :

| Le script (`measure-duration-constraints.py`) | Le noyau (`adjustDurations`) |
| :-------------------------------------------- | :--------------------------- |
| compte **avant** l'ajustement, sur les positions écrites | compte **après**, sur la fin résultante |
| **une place** par sous-titre : `suivant.début − début − écart` | **une fin** par sous-titre, contrainte par contrainte |
| trois paires : **minimum contre écart** (place < minimum), **vitesse contre écart** (place < longueur / vitesse), **vitesse contre maximum** (longueur / vitesse > maximum) ; plus l'union des trois | trois comptes, **un par contrainte encore violée** : `sacrificed.minimum`, `sacrificed.speed`, `sacrificed.gap` ; **pas de compte du maximum**, qui est toujours tenu |
| un **maximum posé à 6 s**, qui est une information même quand personne ne s'en sert | **pas de maximum par défaut** |
| ne lit que `.srt` et `.vtt` | lit les neuf formats |

**Les comptes qui se comparent** — relus dans `adjustDurations` et dans le script par
[#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560), qui a corrigé ce que le cadrage
supposait :

```
place = suivant.début − début − écart               (millisecondes entières)

sacrificed.minimum  ↔  place < minimum   ou   maximum < minimum
sacrificed.speed    ↔  place < besoin    OU   besoin > maximum        (l'union)
sacrificed.gap      ↔  place < 0
besoin = longueur × 1000 / vitesse, arrondi à la milliseconde
```

- `sacrificed.minimum` ↔ « minimum contre écart » — plus le cas d'un minimum supérieur au
  maximum, que le noyau compte comme un minimum : **les deux coïncident quand le maximum est
  éteint ou au moins égal au minimum** ;
- `sacrificed.speed` ↔ **l'union** de « vitesse contre écart » et de « vitesse contre
  maximum » — **pas la somme** : un sous-titre dans les deux compte une fois pour le noyau.
  Le script **l'écrit désormais**, sous « sacrifié (prédit) » ;
- `sacrificed.gap` ↔ les sous-titres dont la place est **négative** (la fin bornée au début).
  **Le cadrage disait que le script les classait « hors sujet » : c'était faux** — son « hors
  sujet » est « fin avant le début », une autre chose, et la place négative n'avait aucune
  colonne. Elle en a une ;
- **le besoin est arrondi à la milliseconde, comme le noyau l'arrondit** (`readingTimeOf`). Le
  script comparait en flottants : « Hi » à 15 caractères par seconde demande 133,33 ms, et une
  place de 133 ms passait pour insuffisante. **C'était le script qui avait tort**, et la
  fixture (`arrondi.srt`) le montre : sans l'arrondi, son contrôle échoue ;
- **la place ne dépend pas de la durée écrite** : un sous-titre dont la fin précède le début
  compte pour ce qu'il est, le noyau le traitant comme les autres ;
- **« suivant » est le suivant dans le fichier**, des deux côtés — le noyau ne trie pas à la
  lecture ;
- **le maximum** : le noyau ne le compte pas, le script oui ; le recoupement le pose des deux
  côtés (`--maximum 6`) pour que les deux calculs portent sur la même question, et ne compare
  que les trois comptes ci-dessus.

**Ce que `adjust` doit exposer en `--format json` pour que le recoupement se fasse** :

- **`counts.subtitles`** (la taille de la cible) et **`counts.adjusted`** ;
- **`counts.sacrificed.speed`, `.minimum`, `.gap`** — les trois comptes, tels que le noyau
  les rend ;
- **`constraints`** : les contraintes **employées**, en toutes lettres — la vitesse en
  caractères par seconde et ses deux sens, le minimum, le maximum **ou `null`**, l'écart, en
  millisecondes. C'est ce qui permet de **vérifier que les deux calculs portent sur les mêmes
  réglages** sans lire la ligne de commande ;
- **`--dry-run`**, pour compter sans écrire.

**Comment la comparaison se prouve.** Ce qui doit devenir une garantie plutôt qu'une
observation a besoin d'une **fixture versionnée** : une petite suite de fichiers, dans
`src/test/data/`, construite pour couvrir chaque paire et leurs recouvrements, avec **les
comptes attendus écrits à la main** à partir des définitions. Les deux outils sont confrontés
à elle — le noyau par un cas e2e en `--format json` et `MatchesFile`, le script par son
propre contrôle. **Rejouer ensuite sur le corpus privé est une observation** : « les deux
comptes s'accordent sur les fichiers `.srt` et `.vtt` » ou « ils divergent sur tant de
fichiers, pour telle raison » ; **aucun fichier n'est nommé et aucun chiffre ne lui est
attribué**. S'ils divergent, l'un des deux a tort, et l'issue dit lequel et le corrige.

**Ce qui a été fait** ([#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560)) : la fixture est `src/test/data/durees/` (neuf fichiers, SubRip et WebVTT, un par paire de contraintes et par recouvrement), ses attendus à la main sont `attendus/json/recoupement-defaut.jsonl` et `recoupement-ecart.jsonl` (deux réglages, dont un écart de 0,5 s qui rend la place négative possible). Le noyau y est confronté par un cas de bout en bout (`CLI-ADJUST-06`), le script par `--check-fixtures`, qui tourne dans `make check-local`. La confrontation au corpus privé est `--crosscheck`.

## D8 — `correct`

```
subedit-cli correct --tasks LIST [--code CODE] [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles]
                    [--document main|translation] [-t FILE] [--range N-M] [--dry-run]
                    (--output … | --output-dir … | --in-place) <file>...
```

**Les tâches se nomment, et aucune n'est cochée d'avance.** `--tasks` est requis ; il prend
une liste parmi **`mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`,
`line-break`** — les quatre tâches du noyau (`CorrectionTask`), la jonction et la scission
de mots étant deux réglages d'une même tâche. **Elles s'appliquent dans l'ordre de
Gaupol**, que `proposeCorrections` tient : mentions, jonction et scission, erreurs courantes,
majuscules, découpage. **L'ordre de la liste n'y change rien** — et le manuel le dit.

**`--code` nomme la cascade de motifs** : `Écriture[-langue[-PAYS]]`, `Zyyy` pour toute
écriture (`Latn`, `Latn-en`, `Latn-en-US`…). **Il est requis dès qu'une tâche lit des
motifs** : sans langue ambiante (D2), un défaut universel appliquerait *moins* que ce que
l'utilisateur croit, sans le dire. **Il vaut pour toutes les tâches de motifs d'un même
lancement** ; une tâche qui voudrait un code à elle est un second lancement.

**`--classes`** filtre les erreurs courantes **pour de bon** (D4 de la spec 12) : décocher
`ocr` retire ses motifs de l'application, et pas seulement de l'affichage.

**`--enable` / `--disable NOM`** — répétables — règlent l'activation d'un motif par son nom
anglais, le nom étant la clé (ADR 0037). **C'est nécessaire, et c'est ce qui écarte les
valeurs par défaut de la fenêtre** : les `.conf` livrés de `Latn` **désactivent les
motifs de mentions**, si bien que `--tasks mentions` n'appliquerait rien. Les deux motifs
« Sound in brackets » et « Sound in parentheses » commandent le balayage de la phase 4 (D7 de
la spec 12) et se règlent par les mêmes options. **Un nom qui ne désigne rien est une erreur
d'usage** — la fenêtre ignore une dérogation périmée parce qu'un réglage peut survivre à un
motif disparu ; une invocation n'a pas de survivant, et une faute de frappe doit être
bruyante. **Un nom ambigu entre deux types** s'écrit `type:nom`. **Une tâche de motifs
(`mentions`, `common-errors`, `capitalization`) dont aucun motif n'est actif** est refusée :
« rien à faire » est une réponse qui se dit, pas un succès silencieux.

**`--language`** nomme le dictionnaire du correcteur (`fr_FR`, `en`…), **requis avec
`join-words` ou `split-words`**. **Sans dictionnaire pour cette langue, le lancement est une
erreur d'usage**, avec la phrase de la fenêtre, `noDictionaryFor` (« no dictionary for fr »),
**avant tout traitement** : la fenêtre grise la fonction et le dit, la ligne de commande
refuse et le dit — dans les deux cas, plutôt que de taire.

**Le découpage de lignes se mesure en caractères.** `CharacterLineMeasure`, au noyau ; la
mesure en *ems* est celle de la fenêtre (D5 de la spec 12). **`--max-length` est requis avec
`line-break`, sans défaut** : le 24 de Gaupol est une valeur en *ems* — une em calibrée à
0,55 par lettre —, et le recopier en caractères couperait presque tout, silencieusement.
`--max-lines` vaut 3, le défaut de Gaupol, que l'unité ne change pas. **Le saut** — un
sous-titre dans les limites est laissé tel quel — reprend, par défaut, **les mêmes bornes**,
comme Gaupol ; `--skip-length off` et `--skip-lines off` l'éteignent.

**Les sous-titres que la correction vide sont retirés**, comme la case par défaut de la
fenêtre ; `--keep-blank-subtitles` les laisse vides. Une traduction vidée reste vide (règle
de #431, tenue par `proposeCorrections`).

**Ce que `correct` dit.** Le compte est celui de la fenêtre, par `noticeOfCorrection` —
« Edited N and removed M subtitles » : **des textes changés, jamais des correspondances**.
**Un motif qui ne se lit pas, ne se traduit pas ou ne termine pas** est nommé — et c'est ici
que la ligne de commande dépasse la fenêtre : la page de confirmation nomme le motif une
seule fois, sans le texte ; **la ligne de commande nomme le sous-titre**, que
`PatternFailure::text` porte au noyau (la relecture de la phase 12 l'avait inscrit). Au
niveau 1 sur la sortie d'erreur, en avertissement `pattern-failed` en JSON. **Un motif
abandonné n'est pas un fichier en échec** : code `0`.

**`--dry-run`** (D5) imprime, par sous-titre changé, le texte d'avant et le texte d'après —
`ProposedCorrection::original` et `proposed`, `proposed` absent pour une suppression.

**Le calcul est déterministe**, et c'est ce qui permet de ne pas appliquer en deux temps
(ADR 0040).

## D9 — Les sous-commandes de texte, et `sort`

Chacune est **une opération du noyau, une grammaire, un compte rendu de `wording/`**.

```
subedit-cli replace PATTERN REPLACEMENT [--regex] [--case-sensitive] [--document …] [-t FILE] [--range N-M] [--dry-run] <destination> <file>...
subedit-cli case --to title|sentence|upper|lower [--document …] [-t FILE] [--range N-M] [--dry-run] <destination> <file>...
subedit-cli italics (--on | --off) [--document …] [-t FILE] [--range N-M] [--dry-run] <destination> <file>...
subedit-cli dialogue-dashes (--add | --remove) [--document …] [-t FILE] [--range N-M] [--dry-run] <destination> <file>...
subedit-cli sort <destination> <file>...
```

- **`replace`** cherche dans **le texte visible** et remplace dans **le texte source** sans
  casser une balise (règle de la phase 10, `recherche.cas`). **Texte brut par défaut,
  `--regex` pour une expression ICU, majuscules ignorées par défaut** (`SearchOptions`, comme
  la fenêtre) ; **`--case-sensitive`** les distingue. L'expression est lue **avant tout
  traitement** : `reasonOf(PatternError)` en erreur d'usage. Le compte est
  `noticeOfReplaceAll`, « not found » quand rien n'a été trouvé, `nothingToChange` quand la
  correspondance se remplace par elle-même.
- **`case`** : les quatre casses, balises intactes ; compte `noticeOfRecase`.
- **`italics`** : dans **les balises du format du fichier** — un format sans style
  (TMPlayer, LRC) **refuse**, fichier par fichier, avec la raison, comme la fenêtre éteint
  l'entrée sans la cacher. Le noyau prend **un sens**, et non une bascule : « une opération,
  deux portes, la décision reste à qui a un utilisateur devant lui » — c'est la phrase du
  noyau, et c'est pourquoi `--on` et `--off` sont deux mots.
- **`dialogue-dashes`** : même forme, `--add` / `--remove`.
- **`sort`** : tri **stable** par début ; compte les sous-titres déplacés, **zéro** quand le
  fichier était déjà en ordre — et **le fichier est alors écrit quand même**, la règle des
  sous-commandes qui écrivent (`hearing-impaired` l'a posée).

**Tous les textes visent le principal par défaut**, et la traduction par `--document
translation` avec `-t` (D10).

## D10 — Les deux documents : `-t`, `--document`, `--align-method`

**Ce que l'analyse a trouvé** : un fichier de traduction **se traite déjà comme un principal**.
Ce que l'appariement ajoute à une invocation est **étroit**, et c'est ce qu'on livre :

- **`inspect -t FILE`** : le rapport d'appariement — lignes rattachées, sous-titres nés,
  sous-titres sans traduction, lignes hors d'ordre —, par `noticeOf(TranslationOutcome)`, et
  en JSON. **Un script sait si une traduction est alignée avant de la corriger**, ce que le
  corpus privé ne permet pas d'éprouver à l'échelle faute de paires.
- **`--document main|translation`** sur les sous-commandes de texte, avec **`-t FILE`**
  (`-t/--translation-file`, comme Gaupol) et **`--align-method position|number`** (`-a` chez
  Gaupol ; **la position est le défaut**, D4 de la spec 11 — les huit cas de
  `src/test/data/paires/` disent pourquoi).
- **Règles** : `--document translation` **exige `-t`**, et `-t` **exige** `--document
  translation` sur une sous-commande de texte — l'un sans l'autre est une erreur d'usage,
  parce qu'un fichier lu qu'aucun geste n'emploie est une omission, pas une préférence.
  **`-t` nomme un fichier et n'a donc de sens que pour une seule entrée** ; avec un lot, il est
  refusé comme l'est `--output` (point ouvert 3).
- **Ce qui s'écrit** : **le document visé, et lui seul**, à **son propre chemin et dans son
  propre format** (ADR 0032). `--output FICHIER` nomme le fichier de la traduction,
  `--output-dir` y met son nom de base, `--in-place` la réécrit. Le principal n'est pas touché.
- **Le texte principal n'est jamais visé par accident** : sans `--document`, rien ne change
  pour qui n'ouvre pas de traduction.
- **Une mention qui viderait une traduction l'écrit vide** et garde le sous-titre : la règle de
  la phase 11 traverse jusqu'ici.

**Ce que ces règles ne font pas** : elles ne décalent pas une traduction avec son principal.
Les sous-commandes de **position** (`shift`, `transform`, `framerate`, `snap`, `adjust`) se
donnent **les deux fichiers**, comme n'importe quel lot : `subedit-cli shift --by 2
principal.srt traduction.srt --output-dir out/`.

**La chose que seul l'appariement fait** est écrire la traduction recalée sur les positions
du principal — **`pair`** (D12, derrière sa porte) : `pair PRINCIPAL -t TRADUCTION --output
FICHIER`. L'alignement a un résultat que rien d'autre ne produit, et c'est précisément
pourquoi il est séparé de `-t`.

## D11 — Les phrases et le manuel

**Toute sous-commande neuve dit ses comptes par le noyau.** C'est la règle que la conclusion
de #541 a inscrite à la place d'un contrôle de parité, dont le critère n'est pas
décidable : **la parité passe par `core/wording/`**. Concrètement :

- les phrases que la fenêtre dit aussi — celles de `counts`, `editing`, `translation`,
  `conversion` — sont **appelées**, jamais recopiées ;
- les phrases propres à la ligne de commande — « N subtitles shifted by… », le bilan d'un lot,
  les refus d'usage — restent dans `cli/` ;
- **le texte et le JSON se lisent au même objet** (ADR 0038), si bien qu'aucune phrase ne porte
  un nombre que le JSON ne porte pas ;
- tout ce que le binaire écrit est **en anglais** ; les intitulés de tests aussi ;
- **le manuel** a **une page par sous-commande**, des exemples rejoués par `manual-check`, les
  codes de retour et les erreurs — et **chaque option y figure**, ce que #546 vérifie contre
  `--help`. La page `subedit-cli(1)` et le tableau d'`invocation.md` suivent. Le manuel
  décrit ce qui existe : `docs/manual/subedit-cli/invocation.md` gagne une section
  **« Sortie lisible par un script »** à la livraison de `--format json`, et la section de
  **la destination** est réécrite par le lot.

**Les exemples du manuel sont engendrés par le binaire** (`make manual`) : ils ne peuvent pas
mentir. **Les attendus des tests, eux, ne le sont jamais** : écrits à la main, par la règle de
#338.

## D12 — Une porte avant la dernière tranche

**Comme #435 pour le multi-projets, parce que le besoin n'est pas confirmé.** Les trois
sous-commandes de structure — `append`, `split-file`, `pair` — ont toutes un usage réel et
**aucune n'a d'appelant connu** : un film en deux parties, une traduction à recaler. Les
construire avant que quelqu'un le dise, c'est ajouter trois pages de manuel et trois
grammaires — dont une, `append`, **brise le modèle d'un lot** (N entrées, une sortie), et
une autre, `split-file`, produit **deux sorties** — à un outil dont la qualité est d'être
prévisible.

**La porte est posée à l'entrée de la tranche 5**, et décidée par l'utilisateur (point
ouvert 2). **Ce qui s'arrête avant elle se tient seul** : les tranches 1 à 4 ne dépendent
d'aucune des trois.

## Comment la phase se prouve

- **Chaque sous-commande a ses cas de bout en bout** — le vrai binaire, les deux sorties
  séparées, les quatre codes de retour — **et ses attendus versionnés** pour ce qu'elle écrit
  (`MatchesFile`, #543). Les attendus sont écrits à la main.
- **Le lot est éprouvé sur un lot engendré** (#544) : collisions, dossier absent, destination
  existante, entrée écrasée, échec au milieu et code `3`, parcours d'un arbre et son
  arborescence conservée.
- **Le JSON a ses attendus et son script de validité** (D3).
- **La correction ne lit ni la configuration de l'utilisateur ni ses motifs ni ses mots** :
  le harnais déplace `XDG_CONFIG_HOME`, `XDG_DATA_HOME` et `ENCHANT_CONFIG_DIR` ;
  `check-config-home.sh` surveille déjà ce que la ligne de commande pourrait toucher
  (`~/.config/subedit`, `~/.config/enchant`) ; lire les motifs de l'utilisateur ne l'écrit pas.
- **Un banc du lot** (D4) et **le banc de `correct`** de la phase 12 (qui existe déjà) :
  la phase ajoute le coût du parcours, pas un second banc des motifs.
- **Le recoupement de #407** est le seul endroit où deux calculs indépendants sont comparés ;
  ses attendus sont une fixture, ses observations sur le corpus privé sont dites
  globalement (D7).

## Ce que la phase ne livre pas

Chacun avec un destinataire, parce qu'un renvoi sans destinataire finit par désigner une
phase déjà passée.

- **La mesure en *ems* en ligne de commande** — **écartée, et pas renvoyée** : elle exige
  `QFontMetricsF`, donc Qt, donc une police. Déclencheur : une mesure sans Qt qui reproduise la
  calibration de Gaupol, et personne ne l'a demandée.
- **La traduction des noms et descriptions des motifs** — phase 15, avec le reste.
- **L'appariement d'une traduction par fichier dans un lot** (`--translation-dir`, un
  suffixe, un motif de noms) — **point ouvert 3**, décidé à la relecture de fin de phase.
- **`--range` sur les sept sous-commandes existantes** — point ouvert 6.
- **Un parcours parallèle**, `--include`, `--no-clobber` — **candidats**, et leur
  déclencheur est écrit dans l'ADR 0039 : un lot qui se voit au banc, un appelant qui le
  demande.
- **Un fichier de réglages pour la ligne de commande** — **écarté** : D2.
- **La vérification orthographique interactive, une liste de mots inconnus, un `find`, la
  fusion et la scission d'un sous-titre, le presse-papiers, la durée de la vidéo** — **écartés**
  avec leur raison et leur déclencheur : D1.
- **Un schéma JSON, un sélecteur de version du schéma** — ADR 0038, déclencheur écrit.
- **Un état entre deux invocations** (un correctif qu'on rejoue, un cache) — ADR 0040.

## Écarts avec Gaupol

**Peu, et c'est normal** : la ligne de commande de Gaupol n'ouvre qu'une fenêtre. Les écarts
sont ceux de la **correction** — l'assistant de Gaupol s'y conforme ou non — et ceux des
options que les deux ont.

| Ce que fait Gaupol | Ce que fait `subedit` | Pourquoi |
| :----------------- | :-------------------- | :------- |
| `-t/--translation-file` **ouvre** la traduction dans la fenêtre | **apparie** pour une sous-commande, sur **un** fichier principal ; écrit le document visé | D10 : une invocation n'a pas de fenêtre |
| l'assistant demande confirmation, chaque texte se coche | **écrit directement** ; `--dry-run` propose | l'ADR 0040 |
| le découpage est réglé en *ems*, **24 de long** | en **caractères**, `--max-length` **requis** | D8 : la mesure en *ems* est celle de la fenêtre, et 24 caractères coupent presque tout |
| l'activation des motifs se lit dans les `.conf` de l'utilisateur ; la fenêtre dans nos réglages | **les `.conf` livrés**, réglés par `--enable` / `--disable` | D2 : une invocation ne lit aucun réglage |
| la liste de remplacements `.repl` se lit et s'écrit | **ni lue ni écrite** | D2 : elle se remplit par les gestes de la fenêtre |
| un motif qui gèle ou qui ne se traduit pas : rien (Gaupol) ; la fenêtre le nomme **une fois** | **le sous-titre est nommé** | D8 : la relecture de la phase 12 l'avait renvoyé ici |
| la langue du correcteur vient de la locale quand rien n'est choisi | **toujours nommée** (`--language`) | D2 : pas de langue ambiante |
| un nom de motif périmé dans une dérogation est **ignoré** | **une erreur d'usage** | D8 : une invocation n'a pas de survivant |
| `-v/--video-file`, `+NUM` | **non repris** | fenêtre |

## Exigences

**Soixante-six**, toutes `prévues` — dont trois derrière la porte de D12 —, **inscrites au
registre avec cette spec** : `check-requirements.sh` confronte la table d'une spec de phase au
registre dans les deux sens, et une spec dont les identifiants manquent au registre ne franchit
pas `check-local`. Chaque issue en **relit** les siennes avant son code — l'état passe à
`implémentée` quand un test les cite, jamais avant. `CLI-*` : la nomenclature
(`SURFACE-SUJET-NN`) est celle de `docs/exigences.md`, et **aucun identifiant ci-dessous
n'était déjà pris** — les sujets neufs sont `JSON`, `DRYRUN`, `RANGE`, `ADJUST`, `CORRECT`,
`REPLACE`, `CASE`, `ITALIC`, `DASH`, `SORT`, `TRANS`, `LISTENC`, `APPEND`, `PSPLIT`, `PAIR` ;
`BATCH` continue de `CLI-BATCH-02`.

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `CLI-JSON-01` | `--format json` écrit sur la sortie standard un objet JSON par ligne ; `text` est le défaut et ne change pas |
| `CLI-JSON-02` | chaque entrée donne **exactement un** objet, échec compris, dans l'ordre des entrées |
| `CLI-JSON-03` | chaque objet porte `schema`, `command`, `file` et `ok` ; un échec porte `error.kind` et `error.message` |
| `CLI-JSON-04` | la verbosité n'agit pas sur la sortie standard en `json` ; la narration reste du texte sur la sortie d'erreur |
| `CLI-JSON-05` | les diagnostics de lecture sont dans `warnings`, à tous les niveaux |
| `CLI-JSON-06` | aucun nombre à virgule : positions en millisecondes entières, cadences en chaînes |
| `CLI-JSON-07` | une erreur d'usage n'écrit rien sur la sortie standard, en `json` comme en `text` |
| `CLI-JSON-08` | `inspect` décrit le fichier : format, encodage, marque, fins de ligne, nombre, étendue, grille ou cadence, anomalies |
| `CLI-JSON-09` | chaque sous-commande qui écrit dit sa destination et ses comptes |
| `CLI-JSON-10` | mêmes entrées et mêmes arguments donnent les octets des attendus versionnés |
| `CLI-DRYRUN-01` | `--dry-run` n'écrit aucun fichier et ne crée aucun dossier, et le code est celui d'un vrai lancement |
| `CLI-DRYRUN-02` | `--dry-run` n'exige aucune destination |
| `CLI-DRYRUN-03` | une destination donnée avec `--dry-run` est vérifiée comme sans lui |
| `CLI-DRYRUN-04` | sur une sous-commande de texte, la sortie standard porte, par sous-titre changé, son numéro, le texte d'avant et le texte d'après |
| `CLI-DRYRUN-05` | en `json`, `changes` porte les mêmes changements, `after` valant `null` pour une suppression |
| `CLI-DRYRUN-06` | la narration d'un `--dry-run` dit que rien n'a été écrit |
| `CLI-BATCH-03` | deux entrées de même destination sont refusées avant tout écrit, code `1`, les deux nommées |
| `CLI-BATCH-04` | une destination qui est une entrée est refusée sans `--in-place` |
| `CLI-BATCH-05` | le dossier de sortie absent est créé |
| `CLI-BATCH-06` | une écriture qui échoue dit « written », jamais « read » |
| `CLI-BATCH-07` | une destination existante est écrasée par écriture atomique |
| `CLI-BATCH-08` | un répertoire en entrée sans `--recursive` est refusé, code `1` |
| `CLI-BATCH-09` | `--recursive` parcourt dans l'ordre des noms, sans suivre de lien, les extensions des formats connus, `.txt` exclu |
| `CLI-BATCH-10` | l'arborescence relative au répertoire donné est conservée sous `--output-dir` |
| `CLI-BATCH-11` | le dossier de sortie compris dans l'arbre parcouru est exclu du parcours |
| `CLI-BATCH-12` | un fichier nommé sur la ligne de commande n'est jamais filtré par son extension |
| `CLI-RANGE-01` | `--range N-M` limite l'opération aux sous-titres N à M, bornes comprises ; `N-` va jusqu'à la fin |
| `CLI-RANGE-02` | une plage hors bornes est refusée en nommant la borne, avant tout traitement |
| `CLI-ADJUST-01` | `adjust` sans option applique les quatre contraintes de Gaupol, dans leur ordre |
| `CLI-ADJUST-02` | chaque contrainte se règle et s'éteint (`off`) ; `--maximum` l'allume |
| `CLI-ADJUST-03` | sans aucune contrainte active, `adjust` est refusé |
| `CLI-ADJUST-04` | le compte rendu dit ce qui a été ajusté et ce qui a été sacrifié, contrainte par contrainte |
| `CLI-ADJUST-05` | en `json`, `counts` porte `adjusted` et `sacrificed.{speed,minimum,gap}`, et `constraints` les contraintes employées |
| `CLI-ADJUST-06` | les comptes sacrifiés s'accordent avec ceux que le script de mesure prédit sur les fixtures versionnées |
| `CLI-CORRECT-01` | `--tasks` est requis et nomme les tâches ; aucune n'est cochée d'avance |
| `CLI-CORRECT-02` | `--code` est requis dès qu'une tâche lit des motifs |
| `CLI-CORRECT-03` | le compte rendu dit les textes changés et supprimés, jamais les correspondances |
| `CLI-CORRECT-04` | `--classes` retire de l'application les motifs de la classe décochée |
| `CLI-CORRECT-05` | `--enable` / `--disable` règlent un motif par son nom ; un nom inconnu ou ambigu est refusé |
| `CLI-CORRECT-06` | une tâche de motifs sans aucun motif actif est refusée |
| `CLI-CORRECT-07` | les motifs de l'utilisateur s'ajoutent aux motifs livrés |
| `CLI-CORRECT-08` | un motif qui ne se lit pas, ne se traduit pas ou ne termine pas est nommé avec le sous-titre, et les autres s'appliquent |
| `CLI-CORRECT-09` | `line-break` mesure en caractères ; `--max-length` est requis ; les bornes du saut suivent, et s'éteignent |
| `CLI-CORRECT-10` | `join-words` et `split-words` joignent et scindent selon le dictionnaire de `--language` |
| `CLI-CORRECT-11` | sans dictionnaire pour `--language`, `correct` est refusé avec la phrase de la fenêtre, avant tout traitement |
| `CLI-CORRECT-12` | les sous-titres vidés sont retirés, sauf `--keep-blank-subtitles` |
| `CLI-CORRECT-13` | `correct` ne lit aucun réglage de l'utilisateur ni sa liste de remplacements |
| `CLI-REPLACE-01` | `replace` cherche dans le texte visible et remplace dans le texte source, sans casser une balise |
| `CLI-REPLACE-02` | `--regex` lit une expression ; une expression illisible est refusée avant tout traitement, avec la raison |
| `CLI-REPLACE-03` | les majuscules sont ignorées par défaut, `--case-sensitive` les distingue |
| `CLI-REPLACE-04` | le compte rendu dit les textes changés, ou que rien n'a été trouvé |
| `CLI-CASE-01` | `case` applique les quatre casses sans toucher aux balises |
| `CLI-ITALIC-01` | `italics --on` / `--off` pose et retire l'italique dans les balises du format du fichier |
| `CLI-ITALIC-02` | un format qui ne porte aucun style refuse, fichier par fichier, avec la raison |
| `CLI-DASH-01` | `dialogue-dashes --add` / `--remove` pose et retire les tirets de dialogue |
| `CLI-SORT-01` | `sort` met les sous-titres dans l'ordre de leur début, de façon stable |
| `CLI-SORT-02` | `sort` dit combien de sous-titres ont bougé, et écrit le fichier même s'il est déjà en ordre |
| `CLI-TRANS-01` | `inspect -t` rapporte lignes rattachées, sous-titres nés, sans traduction et hors d'ordre |
| `CLI-TRANS-02` | `--document translation` exige `-t`, et `-t` exige `--document translation` sur une sous-commande de texte |
| `CLI-TRANS-03` | `-t` n'a de sens que pour une seule entrée ; avec un lot, il est refusé |
| `CLI-TRANS-04` | seul le document visé est écrit, à son propre chemin et dans son propre format |
| `CLI-TRANS-05` | `--align-method` choisit entre position (défaut) et numéro |
| `CLI-LISTENC-01` | `--list-encodings` écrit les encodages qu'ICU sait convertir, un par ligne, et s'arrête |
| `CLI-APPEND-01` | `append` ajoute les fichiers à la suite du premier, décalés de la fin du dernier sous-titre, dans une seule sortie *(derrière la porte, D12)* |
| `CLI-PSPLIT-01` | `split-file --at N` écrit les deux moitiés, la seconde ramenée à l'origine, ou refuse en nommant le sous-titre *(derrière la porte)* |
| `CLI-PAIR-01` | `pair` écrit la traduction recalée sur les positions du principal, et dit l'alignement *(derrière la porte)* |

## Découpage

**Cinq tranches**, les fondations d'abord, puis les sous-commandes **par coût et par
dépendance** — le noyau prêt avant la traduction, la traduction avant la correction qui s'y
branche, la porte à la fin. **Les issues d'outillage de l'initialisation
(#543 à #546) sont livrées et passent avant ; aucune issue ci-dessous ne les attend.**

**Les issues sont ouvertes** : [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) à
[#573](https://github.com/Guyot-Bertrand/sub-edit/issues/573), dans le milestone 13 ; les trois de la tranche 5 ([#570](https://github.com/Guyot-Bertrand/sub-edit/issues/570) à [#572](https://github.com/Guyot-Bertrand/sub-edit/issues/572)) portent `blocked` et `needs-decision` — la porte de D12.

| Tranche | Issue | Ce qu'elle livre | Dépend de | Taille |
| :------ | :---- | :--------------- | :-------- | :----- |
| 1 — les fondations | [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) | **découper `application.cpp`** : un fichier par sous-commande, une table d'enregistrement ; aucun comportement ne change | — | M |
| | [#554](https://github.com/Guyot-Bertrand/sub-edit/issues/554) | **le lot sûr** : collisions et entrées écrasées refusées avant tout écrit, dossier créé, libellé d'écriture, manuel de la destination ; ADR 0039 | — | M |
| | [#555](https://github.com/Guyot-Bertrand/sub-edit/issues/555) | **`--recursive`** : répertoires en entrée, parcours trié, arborescence conservée ; le banc du lot | [#554](https://github.com/Guyot-Bertrand/sub-edit/issues/554) | M |
| | [#556](https://github.com/Guyot-Bertrand/sub-edit/issues/556) | **`--format json`** : l'objet de résultat à la place de la phrase, l'écrivain JSON Lines, **`inspect`**, les attendus et le script de validité ; ADR 0038 | [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) | L |
| | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557) | **`--format json` des sous-commandes qui écrivent, et `--dry-run`** uniforme ; ADR 0040 | [#556](https://github.com/Guyot-Bertrand/sub-edit/issues/556), [#554](https://github.com/Guyot-Bertrand/sub-edit/issues/554) | L |
| | [#558](https://github.com/Guyot-Bertrand/sub-edit/issues/558) | **`--range`** : la grammaire et son passage à l'opération | [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) | S |
| 2 — le noyau est prêt | [#559](https://github.com/Guyot-Bertrand/sub-edit/issues/559) | **`adjust`** | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), [#558](https://github.com/Guyot-Bertrand/sub-edit/issues/558) | M |
| | [#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560) | **le recoupement de #407** : les fixtures, le script qui écrit l'union, la comparaison | [#559](https://github.com/Guyot-Bertrand/sub-edit/issues/559) | M |
| | [#561](https://github.com/Guyot-Bertrand/sub-edit/issues/561) | **`replace`** | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), [#558](https://github.com/Guyot-Bertrand/sub-edit/issues/558) | M |
| | [#562](https://github.com/Guyot-Bertrand/sub-edit/issues/562) | **`case`, `italics`, `dialogue-dashes`**, et `--dry-run` de `hearing-impaired` | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), [#558](https://github.com/Guyot-Bertrand/sub-edit/issues/558) | M |
| | [#563](https://github.com/Guyot-Bertrand/sub-edit/issues/563) | **`sort`** | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557) | S |
| | [#564](https://github.com/Guyot-Bertrand/sub-edit/issues/564) | **`--list-encodings`** | [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) | S |
| 3 — la traduction | [#565](https://github.com/Guyot-Bertrand/sub-edit/issues/565) | **`-t`, `--document`, `--align-method`** : `inspect` et les sous-commandes de texte | [#556](https://github.com/Guyot-Bertrand/sub-edit/issues/556), [#561](https://github.com/Guyot-Bertrand/sub-edit/issues/561), [#562](https://github.com/Guyot-Bertrand/sub-edit/issues/562) | L |
| 4 — la correction | [#566](https://github.com/Guyot-Bertrand/sub-edit/issues/566) | **les motifs et le terrain de la correction, hors de Qt** : l'emplacement des motifs livrés et de l'utilisateur, le harnais qui déplace `XDG_DATA_HOME`, l'arbre de construction qui reproduit l'installation | [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) | M |
| | [#567](https://github.com/Guyot-Bertrand/sub-edit/issues/567) | **`correct` : mentions, erreurs courantes, majuscules** — `--tasks`, `--code`, `--classes`, `--enable`/`--disable`, le compte, les motifs nommés, `--dry-run` | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), [#558](https://github.com/Guyot-Bertrand/sub-edit/issues/558), [#565](https://github.com/Guyot-Bertrand/sub-edit/issues/565), [#566](https://github.com/Guyot-Bertrand/sub-edit/issues/566) | L |
| | [#568](https://github.com/Guyot-Bertrand/sub-edit/issues/568) | **`correct --tasks line-break`**, en caractères | [#567](https://github.com/Guyot-Bertrand/sub-edit/issues/567) | M |
| | [#569](https://github.com/Guyot-Bertrand/sub-edit/issues/569) | **`correct --tasks join-words,split-words`** : `--language`, le dictionnaire, son absence | [#567](https://github.com/Guyot-Bertrand/sub-edit/issues/567) | M |
| 5 — la porte, D12 | [#570](https://github.com/Guyot-Bertrand/sub-edit/issues/570) | **`append`** | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557) | M |
| | [#571](https://github.com/Guyot-Bertrand/sub-edit/issues/571) | **`split-file`** | [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), [#570](https://github.com/Guyot-Bertrand/sub-edit/issues/570) | M |
| | [#572](https://github.com/Guyot-Bertrand/sub-edit/issues/572) | **`pair`** | [#565](https://github.com/Guyot-Bertrand/sub-edit/issues/565) | M |
| | [#573](https://github.com/Guyot-Bertrand/sub-edit/issues/573) | **relecture de fin de phase** | tout | M |

**[#556](https://github.com/Guyot-Bertrand/sub-edit/issues/556) est la pierre d'angle de la phase**, et la plus risquée : c'est elle qui change ce que
`rewriteAll` rend, donc ce que **chaque** sous-commande doit produire, et qui engage le format
que des scripts liront. **[#567](https://github.com/Guyot-Bertrand/sub-edit/issues/567) est la plus grosse de la correction** : c'est elle qui exerce
ensemble le catalogue, les réglages, le moteur et les phrases du noyau. **[#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) est la plus
ingrate et la moins négociable** : la phase doublerait un fichier qui a déjà 575 lignes.

**Ce qui peut avancer en parallèle** : [#554](https://github.com/Guyot-Bertrand/sub-edit/issues/554) et [#555](https://github.com/Guyot-Bertrand/sub-edit/issues/555) avec [#556](https://github.com/Guyot-Bertrand/sub-edit/issues/556) et [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557) (le lot et le format ne se
touchent que dans l'objet « liste des entrées », que [#554](https://github.com/Guyot-Bertrand/sub-edit/issues/554) pose) ; [#564](https://github.com/Guyot-Bertrand/sub-edit/issues/564) à tout moment après [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553) ;
[#566](https://github.com/Guyot-Bertrand/sub-edit/issues/566) dès [#553](https://github.com/Guyot-Bertrand/sub-edit/issues/553), et **tôt**, parce qu'il touche l'empaquetage et le harnais — ce qui s'éprouve tôt.

## Points ouverts

**Quatre sont tranchés par l'utilisateur le 2026-10-01** (1, 5, 7 et 11 : la supposition devient la décision), et **la porte de la tranche 5 est gardée** (point 2) : `append`, `split-file` et `pair` se décident à l'entrée de la tranche 5, la fusion et la scission d'un sous-titre restent écartées avec leur déclencheur. Ce que le cadrage ne peut pas trancher seul, **chacun avec ce qui est supposé en attendant**.

| N° | Point | Supposé | Qui, quand |
| :- | :---- | :------ | :--------- |
| 1 | **la racine des sorties d'un parcours** : relative au répertoire donné (`out/a/x.srt`) ou y compris son nom (`out/films/a/x.srt`) | relative, sans le nom (ADR 0039) | **tranché le 2026-10-01** : la supposition est retenue |
| 2 | **la porte de la tranche 5** : `append`, `split-file`, `pair` sont-ils voulus, et sous quelle grammaire (`split-file` écrit deux fichiers, `append` en lit N pour en écrire un) | non construits tant que personne ne répond ; les tranches 1 à 4 se tiennent seules | l'utilisateur, à l'entrée de la tranche 5 |
| 3 | **l'appariement des traductions dans un lot** : `-t` n'a de sens que pour une entrée ; un `--translation-dir`, un suffixe (`film.srt` ↔ `film.fr.srt`), ou rien | rien : on passe les deux fichiers, ou une invocation par paire | la relecture, sur demande réelle |
| 4 | **la forme texte de `--dry-run`** : des blocs « sous-titre N / avant / après », ou autre chose | des blocs, un préfixe par ligne ; **la forme JSON, elle, est engagée par l'ADR 0038** | **tranché en [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557)** : des blocs — `<chemin>: subtitle N` (suivi de `(translation)` et de `(removed)` quand ils s'appliquent), puis `- ` devant chaque ligne d'avant et `+ ` devant chaque ligne d'après ; une suppression n'a pas de lignes `+`. Un texte de plusieurs lignes tient dans son bloc, ce qu'un diff ligne à ligne ou une forme sur une ligne ne permettait pas |
| 5 | **`--code` requis, ou `Zyyy` par défaut** : le choix de D8 est de refuser un défaut qui appliquerait moins qu'attendu | requis | **tranché le 2026-10-01** : la supposition est retenue |
| 6 | **`--range` sur les sept sous-commandes existantes** (`shift` sur une plage, `hearing-impaired` sur une plage) | non, dans cette phase | la relecture |
| 7 | **le parent de `--output FICHIER`** est créé comme le dossier de `--output-dir` (ADR 0039) | oui, même règle | **tranché le 2026-10-01** : la supposition est retenue |
| 8 | **un chemin qui n'est pas de l'UTF-8** en JSON : U+FFFD et l'avertissement `path-not-utf8` | oui, la fidélité n'est pas promise pour eux | [#556](https://github.com/Guyot-Bertrand/sub-edit/issues/556) |
| 9 | **si les comptes divergent** (D7) et qu'aucune plage ne localise le désaccord : faut-il que le noyau rende **quels** sous-titres sont sacrifiés, non seulement combien ? | non, tant que les comptes s'accordent | [#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560) |
| 10 | **`--include`** : un motif de noms pour le parcours, plus de grammaire pour le cas où l'extension ne suffit pas | non | la relecture, sur demande réelle |
| 11 | **`--list-encodings`** : retenu comme iso-fonctionnel avec Gaupol et pour son coût ; l'est-il vraiment ? | oui, issue [#564](https://github.com/Guyot-Bertrand/sub-edit/issues/564) | **tranché le 2026-10-01** : la supposition est retenue |
