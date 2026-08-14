# Phase 3 — CLI

**État :** conception validée, prête à découper
**Milestone :** `3 — CLI : harnais`

## Objectif

Donner au noyau une **surface invocable**, de sorte que ce qu'il sait faire
puisse être exercé de bout en bout sur des fichiers réels, et mesuré.

La ligne de commande n'apparaît pas dans les besoins de l'utilisateur. Elle
n'est pas une fin : c'est le harnais qui permet de dire que le noyau marche
avant qu'une fenêtre existe. Les sous-commandes destinées à un usage réel
relèvent de la phase 13.

## Portée

**Inclus** — cinq sous-commandes : `inspect`, `convert`, `shift`, `transform`,
`framerate`. L'analyse d'arguments, l'aide, les codes de retour, le traitement
d'un lot de fichiers et le comportement en cas d'échec partiel.

**Élargie par rapport à la feuille de route**, qui annonçait *inspection,
conversion, décalage*. La raison est écrite plus bas.

**Exclu, par phase** — suppression des mentions pour malentendants (phase 4) ;
toute interface graphique (phase 5) ; les formats SubViewer, SSA et ASS
(phase 9) ; les encodages autres qu'UTF-8 (phase 9) ; les sous-commandes
d'usage réel et la sortie lisible par une machine (phase 13).

## Décisions applicables

| ADR | Décision |
| :-- | :------- |
| [0006](../adr/0006-positions-en-millisecondes.md) | positions en millisecondes entières, types forts |
| [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) | lecture au mieux, diagnostics remontés |
| [0013](../adr/0013-mise-a-l-echelle-exacte-des-positions.md) | mise à l'échelle par un rationnel exact |
| [0014](../adr/0014-registre-d-exigences.md) | registre d'exigences confronté aux tests |
| [0016](../adr/0016-cli11-pour-l-analyse-d-arguments.md) | CLI11 pour l'analyse d'arguments |

## Pourquoi cinq sous-commandes et non trois

La feuille de route restreignait la phase à l'inspection, à la conversion et au
décalage. Deux constats faits au cadrage ont élargi ce contour.

**Le noyau sait déjà davantage.** `transform` et `convert_frame_rate` sont
livrés depuis la phase 2, testés unitairement et mesurés par les benchmarks.

**Un harnais qui n'expose pas une opération ne la valide pas.** C'est la raison
d'être de cette phase : confronter le noyau à des fichiers entiers plutôt qu'à
des cas construits. Laisser deux opérations hors de la CLI, c'est garder du code
qu'aucun test de bout en bout ne traverse jusqu'à la phase 13 — et découvrir
alors, très tard, ce qu'un fichier réel leur fait.

Le surcoût est faible : les deux opérations existent, il ne reste qu'à les
appeler. Le coût de leur absence, lui, ne se paierait qu'au moment où il serait
le plus cher.

## La langue de la ligne de commande

**La CLI parle anglais** : noms de sous-commandes, noms d'options, texte
d'aide, diagnostics, messages d'erreur, en-têtes de rapport.

Ce n'est pas une exception à la règle de langue du projet, c'est son
application. La frontière est celle du compilateur C++, et ces chaînes sont des
littéraux C++.

Le manuel reste en français et **cite la sortie anglaise telle quelle**. Depuis
que `make manual` régénère les blocs d'exemple, ces citations ne peuvent plus
mentir : elles sont produites par le binaire.

## Grammaire commune

### Les options globales

Acceptées par toutes les sous-commandes, et par l'invocation nue :

| Option | Effet |
| :----- | :---- |
| `-q`, `--quiet` | niveau 0 — plus aucune narration |
| `-v`, `-vv`, `-vvv` | niveaux 1 à 3, le niveau 1 étant le défaut |
| `-h`, `--help` | l'aide, celle de la sous-commande si l'on en nomme une |
| `--version` | `subedit <version>` |

Elles se placent **avant ou après** le nom de la sous-commande, indifféremment :
`subedit-cli -vv shift …` et `subedit-cli shift -vv …` sont la même invocation.
Exiger une position serait un piège sans contrepartie.

### Le temps

Deux formes acceptées partout où une position ou une durée est attendue :

| Forme | Exemple | Sens |
| :---- | :------ | :--- |
| secondes | `-7.001`, `2.999`, `30` | signe optionnel, décimales optionnelles, à la milliseconde |
| horodatage | `01:39:37.040`, `-00:00:07.001` | `[±]HH:MM:SS.mmm` |

Le séparateur décimal est le **point**, la CLI parlant anglais. La virgule est
refusée avec un message qui le dit, plutôt qu'interprétée au jugé.

La résolution est la milliseconde, comme le modèle (ADR 0006). Une valeur plus
fine est refusée : l'arrondir en silence donnerait un résultat qui ne
correspond à rien de ce qui a été demandé.

### L'indice d'un sous-titre

**Compté à partir de 1**, comme il s'affiche. Un indice hors bornes est refusé
en nommant la borne.

### La destination

Aucune écriture n'a lieu sans destination explicite. Trois façons de la donner,
mutuellement exclusives :

| Option | Condition |
| :----- | :-------- |
| `--output FILE` | exactement un fichier d'entrée |
| `--output-dir DIR` | quel que soit le nombre d'entrées ; le nom de base est conservé |
| `--in-place` | écrase les entrées, par écriture atomique |

Un harnais qui écrase son entrée sans qu'on l'ait demandé est un harnais qu'on
cesse d'utiliser au bout de la deuxième fois. `--output-dir` est **obligatoire**
dès qu'il y a plusieurs entrées : `--output` n'aurait alors aucun sens, et le
refuser vaut mieux que d'écrire le dernier fichier sur les précédents.

`inspect` n'écrit aucun fichier et n'accepte donc aucune de ces options.

### Le format du fichier écrit

`shift`, `transform` et `framerate` **conservent le format du fichier lu**.
Changer de format est le travail de `convert`, et une sous-commande qui ferait
les deux rendrait indistinguables les erreurs de l'une et de l'autre.

## Ce qui s'écrit, et où

Le code de retour dit **si** ça a marché ; il ne dit pas **ce qui s'est passé**.
Un harnais dont toute la sortie tient dans un chiffre oblige à rouvrir les
fichiers pour comprendre — c'est précisément le travail qu'il existe pour
éviter.

### Deux sorties, deux rôles

| Sortie | Ce qu'elle porte |
| :----- | :--------------- |
| standard | **le résultat, et lui seul** — le rapport d'`inspect`. Les sous-commandes qui écrivent des fichiers n'y écrivent rien : leur résultat *est* le fichier. |
| erreur | **tout le reste** — la narration du traitement, les avertissements, les erreurs. |

Ce partage n'est pas une convention décorative. Il rend
`subedit-cli inspect a.srt | grep …` utilisable sans que le récit vienne polluer
le tube, et il permet à `--quiet` de taire la narration **sans jamais taire le
résultat** — ce qu'un `--quiet` qui écrase la sortie standard rendrait
impossible.

### Quatre niveaux

| Niveau | Comment | Ce que la sortie d'erreur porte |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet`, `-q` | rien, **sauf les erreurs** |
| 1 | par défaut, ou `-v` | une ligne par fichier traité, et un bilan dès qu'il y en a plusieurs |
| 2 | `-vv` | et ce qui a été décidé pour chacun : format reconnu, encodage, BOM, fins de ligne conservées ou imposées, nombre de sous-titres touchés, chemin écrit |
| 3 | `-vvv` | et la trace de mise au point : arguments analysés, chemins résolus, diagnostics de lecture au détail |

Chaque niveau **contient le précédent** : monter d'un cran ajoute, ne remplace
jamais. Un utilisateur qui relance en `-vv` pour comprendre un incident retrouve
mot pour mot ce qu'il avait déjà lu, plus le reste.

**Les erreurs ne sont jamais tues, `--quiet` compris.** Une commande qui échoue
en silence ne laisse que son code de retour, et transforme chaque incident en
enquête. « Rien » désigne ce qui raconte, pas ce qui alarme.

Le bilan n'apparaît qu'à partir de deux fichiers : sur une entrée unique,
« 1 fichier traité » répète la ligne qui précède.

`--quiet` et `-v` dans la même invocation sont une **erreur d'usage**. Deux
intentions opposées ne s'arbitrent pas au profit de la dernière écrite : les
refuser dit à l'appelant qu'il s'est trompé, là où un arbitrage silencieux le
laisserait croire qu'il a été compris.

### À quoi cela ressemble

Illustratif — **le manuel porte les sorties réelles**, engendrées par
`make manual`, et fait foi.

Au niveau par défaut, un lot dont un fichier échoue :

```
a.vtt: 1356 subtitles shifted by -7.001 s -> out/a.vtt
b.vtt: read failed: line 42: timing line expected
1 of 2 files processed, 1 failed
```

Code de retour `3`. En `-vv`, chaque fichier dit en plus ce qui a été reconnu et
ce qui a été écrit :

```
a.vtt: WebVTT, UTF-8, no BOM, LF line endings, 1356 subtitles
a.vtt: 1356 subtitles shifted by -7.001 s -> out/a.vtt
a.vtt: written as WebVTT, UTF-8, no BOM, LF line endings
```

## Les sous-commandes

Chaque sous-commande précise ci-dessous ce qu'elle porte sur la **sortie
standard** ; sa narration sur la sortie d'erreur suit la table des niveaux.

### `inspect`

```
subedit-cli inspect [--order-report breaks|late] <file>...
```

Ne modifie rien et n'écrit aucun fichier.

**Sortie standard** — le rapport, par fichier : le format reconnu, l'encodage,
la présence d'un BOM, les fins de ligne — et leur mélange, avec la ligne où le
second type apparaît —, le nombre de sous-titres, l'étendue temporelle, et les
ruptures d'ordre. C'est le résultat de la commande : il est écrit à tous les
niveaux, `--quiet` compris.

Les diagnostics de lecture (ADR 0008) sont rapportés tels quels : un fichier qui
s'ouvre en signalant quelque chose n'est pas un fichier qui a échoué. Ils vont
sur la **sortie d'erreur**, avec le reste des avertissements — sans quoi un
rapport passé dans un tube contiendrait deux natures de lignes mêlées.

### `convert`

```
subedit-cli convert --to srt|vtt
                    [--line-endings unix|windows|mac] [--bom | --no-bom]
                    (--output FILE | --output-dir DIR | --in-place) <file>...
```

`--to` est requis. `--line-endings` et le BOM valent, **par défaut, ce que
portait le fichier lu** — le modèle de la phase 1 retient déjà les deux, et les
imposer ferait perdre à chaque conversion une information que le noyau avait
pris soin de garder.

`mac` désigne le retour chariot seul (`\r`), la fin de ligne du Mac OS
classique.

**Sortie standard** — rien : le résultat est le fichier écrit.

### `shift`

```
subedit-cli shift --by <time> (--output … ) <file>...
```

Décale toutes les positions de `<time>`, dans un sens ou dans l'autre. Un
décalage qui rendrait une position négative est refusé, en nommant le
sous-titre en cause.

**Sortie standard** — rien : le résultat est le fichier écrit.

### `transform`

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output … ) <file>...
```

Deux points de repère : le sous-titre `<index>` doit désormais commencer à
`<time>`. Toutes les autres positions suivent, par la mise à l'échelle exacte de
l'ADR 0013.

Les deux indices doivent différer — deux repères confondus ne définissent aucune
transformation, et le refuser vaut mieux que de rendre une division par zéro
déguisée en résultat.

**Sortie standard** — rien : le résultat est le fichier écrit.

### `framerate`

```
subedit-cli framerate --from <fps> --to <fps> (--output … ) <file>...
```

Les fréquences s'écrivent en images par seconde, décimales comprises : `25`,
`24`, `23.976`. Une fréquence nulle ou négative est refusée.

**Sortie standard** — rien : le résultat est le fichier écrit.

## Codes de retour

| Code | Signification |
| :--- | :------------ |
| `0` | tout a réussi |
| `1` | erreur d'usage — option inconnue, valeur invalide, combinaison interdite |
| `2` | aucun fichier n'a pu être traité |
| `3` | certains fichiers ont été traités, d'autres non |

Distinguer `2` de `3` est ce qui permet à un script d'agir sans relire la
sortie : « rien n'a marché » et « il en manque un » n'appellent pas la même
réaction.

Une erreur d'usage est détectée **avant tout traitement** : elle ne laisse
jamais un lot à moitié écrit.

## Un lot, et ce qu'on en fait

Toutes les sous-commandes acceptent plusieurs fichiers. Chacun est traité
indépendamment ; l'échec de l'un n'interrompt pas les autres, et les échecs sont
rapportés en nommant le fichier et la raison.

S'arrêter au premier échec obligerait à relancer autant de fois qu'il y a de
fichiers fautifs pour en connaître la liste. Le harnais existe pour donner cette
liste en une fois.

## Le désordre : deux lectures, et une option pour les départager

`Project::outOfOrder()` compare aujourd'hui chaque sous-titre à son
**prédécesseur immédiat**. L'autre lecture compare au **plus grand début
rencontré**. Sur les départs `0, 4000, 2000, 3000`, la première rend `{2}`, la
seconde `{2, 3}`.

Les deux s'accordent toujours sur l'existence d'un désordre et ne diffèrent que
sur la liste. Jusqu'ici, aucun appelant ne consommait cette liste : la question
pouvait rester ouverte.

`inspect` est le premier à devoir la trancher. **Il ne la tranche pas : il
expose les deux.**

| Valeur | Ce qu'elle rend |
| :----- | :-------------- |
| `breaks` (défaut) | les lignes qui rompent l'ordre par rapport à la précédente |
| `late` | toutes les lignes qui démarrent avant quelque chose de déjà vu |

C'est un usage assumé de ce qu'est cette phase. Un harnais sert à découvrir, et
comparer les deux sorties sur des fichiers réels tranchera mieux qu'un
raisonnement mené sans données. **La phase 5 hérite de ce choix**, et l'option
disparaîtra alors au profit de la lecture retenue — c'est écrit dans les points
ouverts pour que la disparition ait un déclencheur, et non une intention.

La seconde lecture demande une addition au noyau de la phase 2 : la règle
devient un paramètre nommé plutôt qu'un comportement figé.

## Ce que l'invocation nue devient

Trois exigences de la phase 1 décrivent un binaire qui n'analyse rien :
l'invocation sans argument écrit la version, et **tout argument est ignoré**.
Cette phase leur retire leur objet.

| Avant | Après |
| :---- | :---- |
| sans argument → la version | sans argument → l'aide, code `0` |
| tout argument ignoré, code `0` | argument inconnu → erreur d'usage, code `1` |
| — | `--version` → `subedit <version>` |

**Les trois passeront à l'état `remplacée`**, chacune avec l'identifiant qui
prend sa place — `CLI-VERSION-02` comprise. Sa formulation, « rien n'est écrit
sur la sortie d'erreur », décrivait un binaire qui n'a aucune erreur à signaler ;
elle devient fausse dès qu'il en a. La garder en la relisant comme « rien sur la
sortie d'erreur *quand tout va bien* » serait réécrire une exigence sans le
dire, ce que l'ADR 0014 interdit précisément.

**Pas au cadrage : dans l'issue d'ossature**, en même temps que les tests qui les
citent. L'état `remplacée` exige qu'aucun test ne désigne l'identifiant ; les
basculer avant de réécrire ces tests ferait échouer `check-requirements.sh` sur
une contradiction que le registre existe justement pour empêcher. Elles restent
donc `implémentée` dans le diff de ce cadrage, ce qui est exact : le binaire les
honore encore.

La section « Arguments » du manuel, qui explique aujourd'hui que rien n'est
analysé, est réécrite en conséquence.

## Ce qui est écarté, avec sa raison

| Écarté | Raison |
| :----- | :----- |
| sortie lisible par une machine (`--format json`) | rien ne la consommerait aujourd'hui ; sa forme doit être dictée par un appelant réel, donc par la phase 13 |
| lecture de SubViewer, SSA, ASS | le noyau ne les lit pas ; c'est la phase 9 |
| encodages autres qu'UTF-8 | le noyau refuse tout le reste, délibérément (phase 9) |
| une sous-commande qui convertit *et* transforme | rendrait indistinguables les erreurs de l'une et de l'autre |
| annuler depuis la CLI | l'historique existe, mais un processus qui se termine n'a rien à annuler |
| la couleur dans la sortie | il faudrait `--color`, la détection d'un terminal, et une sortie qui cesse d'être stable octet pour octet — or les tests de bout en bout la comparent, et `make manual` la recopie dans le manuel |
| une barre de progression | même raison, et les fichiers se traitent en quelques dizaines de millisecondes |

## Tests

**De bout en bout, par le binaire réel**, sur le harnais de la phase 3 : chaque
sous-commande, chaque code de retour, chaque refus d'usage.

**Les deux sorties se vérifient séparément.** Le harnais les sépare déjà —
`CliRun::output` et `CliRun::errors` — donc rien n'est à construire ; ce qui est
exigé ici, c'est que les tests s'en servent. Les confondre rendrait invisible la
faute la plus probable de cette phase : une ligne de narration qui se glisse sur
la sortie standard, indétectable tant qu'on lit un terminal où les deux
arrivent mêlées.

**L'emboîtement des niveaux se prouve par comparaison**, et non en réécrivant
l'attendu à chaque cran : **chaque ligne du niveau *n* se retrouve, identique,
dans le niveau *n+1***, les lignes ajoutées pouvant s'intercaler. Écrire trois
fois le même attendu à la main laisserait les trois diverger sans que rien le
signale.

**Les fixtures sont fabriquées**, et couvrent délibérément ce qui casse : un
fichier par fin de ligne, avec et sans BOM, un fichier aux fins de ligne
mélangées, un fichier vide, un fichier dont l'ordre est rompu de chacune des
deux façons, des décalages à la milliseconde, une conversion `25 → 23.976` dont
le facteur n'est pas exact en décimal.

**Aucun test ne compare deux implémentations.** Ce qui est attendu est écrit
dans le test, pas produit par un autre programme.

## Exigences

Nouvelles entrées au registre, à l'état `prévue` jusqu'à ce qu'un test les cite :

| ID | Exigence |
| :- | :------- |
| `CLI-VERSION-04` | `--version` écrit `subedit <version>` sur la sortie standard, code `0` |
| `CLI-VERSION-05` | une invocation qui réussit n'écrit rien sur la sortie d'erreur |
| `CLI-USAGE-01` | sans argument, l'aide est écrite sur la sortie standard, code `0` |
| `CLI-USAGE-02` | une option inconnue ou une valeur invalide donne le code `1`, sans rien écrire sur la sortie standard |
| `CLI-USAGE-03` | une erreur d'usage est détectée avant tout traitement |
| `CLI-USAGE-04` | `--quiet` et `-v` ensemble donnent une erreur d'usage |
| `CLI-OUTPUT-01` | seul le résultat va sur la sortie standard, la narration sur la sortie d'erreur |
| `CLI-OUTPUT-02` | `--quiet` supprime toute narration, et laisse passer les erreurs |
| `CLI-OUTPUT-03` | chaque niveau contient le précédent, mot pour mot |
| `CLI-OUTPUT-04` | le niveau par défaut écrit une ligne par fichier traité |
| `CLI-OUTPUT-05` | le bilan n'apparaît qu'à partir de deux fichiers d'entrée |
| `CLI-BATCH-01` | plusieurs fichiers sont traités indépendamment, les échecs nommés |
| `CLI-BATCH-02` | tous en échec donne le code `2`, un échec partiel le code `3` |
| `CLI-INSPECT-01` | format, encodage, BOM, fins de ligne, nombre et étendue sont rapportés |
| `CLI-INSPECT-02` | des fins de ligne mélangées sont signalées avec leur ligne |
| `CLI-INSPECT-03` | `--order-report` rend les deux lectures du désordre |
| `CLI-CONVERT-01` | `--to` produit le format demandé |
| `CLI-CONVERT-02` | fins de ligne et BOM sont conservés par défaut, réglables sinon |
| `CLI-CONVERT-03` | sans destination explicite, rien n'est écrit et le code est `1` |
| `CLI-SHIFT-01` | `--by` décale toutes les positions, dans les deux sens |
| `CLI-SHIFT-02` | un décalage rendant une position négative est refusé en nommant le sous-titre |
| `CLI-TRANSFORM-01` | deux repères transforment les positions, les autres suivent |
| `CLI-TRANSFORM-02` | deux indices confondus sont refusés |
| `CLI-FRAMERATE-01` | `--from`/`--to` convertit les positions |
| `CLI-FRAMERATE-02` | une fréquence nulle ou négative est refusée |

## Découpage en issues

L'ADR 0016 n'y figure pas : elle est écrite **au cadrage**, avec cette spec. Une
décision de dépendance prise en même temps que le code qui la consomme n'est
plus une décision, c'est une justification après coup.

| Issue | Sujet | Dépend de |
| :---- | :---- | :-------- |
| Issue | Sujet | Dépend de |
| :---- | :---- | :-------- |
| [#61](https://github.com/Guyot-Bertrand/sub-edit/issues/61) | ossature — aiguillage, `--version`, `--help`, codes de retour, lot, verbosité, partage des deux sorties — **et `inspect`** | — |
| [#62](https://github.com/Guyot-Bertrand/sub-edit/issues/62) | la seconde lecture du désordre dans le noyau | — |
| [#63](https://github.com/Guyot-Bertrand/sub-edit/issues/63) | `--order-report` sur `inspect` | #61, #62 |
| [#64](https://github.com/Guyot-Bertrand/sub-edit/issues/64) | `convert`, fins de ligne et BOM, **et la destination** | #61 |
| [#65](https://github.com/Guyot-Bertrand/sub-edit/issues/65) | `shift`, **et la grammaire du temps** | #61 |
| [#66](https://github.com/Guyot-Bertrand/sub-edit/issues/66) | `transform`, **et la grammaire des indices** | #61 |
| [#67](https://github.com/Guyot-Bertrand/sub-edit/issues/67) | `framerate` | #61 |
| [#68](https://github.com/Guyot-Bertrand/sub-edit/issues/68) | relecture de fin de phase | tout |

**`inspect` est dans l'ossature, et la grammaire commune n'y est pas.** Ce
n'était pas le découpage initial ; il a été corrigé en l'attaquant, pour une
raison qui vaut d'être écrite.

Une ossature sans sous-commande n'a **rien d'atteignable** : pas de liste de
fichiers, donc pas de lot ; pas de traitement, donc rien à raconter aux niveaux
1 à 3 ; et les codes `2` et `3` ne peuvent pas se produire. La première issue
aurait promis quatre codes de retour et quatre niveaux en n'en prouvant que deux
et un. `inspect` est le plus simple des exerciseurs — il n'écrit aucun fichier —
et il rend le reste atteignable.

Symétriquement, la grammaire du temps, des indices et de la destination
descend dans les issues qui en ont l'usage. L'écrire dans l'ossature la
laisserait sans appelant, donc sans preuve — le même défaut, dans l'autre sens.

`#62` ne touche pas à la CLI et peut se faire en parallèle de `#61`. Les quatre
sous-commandes qui écrivent — `#64` à `#67` — sont indépendantes entre elles une
fois l'ossature posée.

## Points ouverts

| Point | Déclencheur |
| :---- | :---------- |
| laquelle des deux lectures du désordre garder | la phase 5, qui affichera le même ensemble ; l'option `--order-report` disparaît alors |
| forme d'une sortie lisible par une machine | phase 13, quand un appelant réel en aura besoin |
| valider les opérations contre la durée de la vidéo | la phase 6, qui associe une vidéo au projet — le détail est écrit là-bas, dans la feuille de route |

Sur ce dernier point, ce que cette phase peut et ne peut pas faire mérite d'être
dit ici, puisque c'est en l'écrivant qu'on l'a vu : **on ne sait vérifier qu'une
seule borne, celle de zéro.** Un décalage négatif trop grand rend une position
négative, et le refus est possible. La borne haute — ne pas pousser des
sous-titres après la fin du film — n'existe pas pour le noyau, qui ignore où le
film s'arrête.

**Corrigé à la relecture :** cette phrase ne parlait que de `shift`. `transform`
est concernée de la même façon — un sous-titre placé avant le premier repère
peut être repoussé sous zéro — et porte le même refus, avec le même message.
`framerate` ne l'est pas : le facteur est strictement positif, donc une position
positive le reste. Les deux autres bornes, elles, restent hors d'atteinte.

## Critères de fin

- [ ] Les cinq sous-commandes existent et sont documentées au manuel
- [ ] Chaque exigence du tableau ci-dessus est `implémentée` et citée par un test
- [ ] Les quatre codes de retour sont prouvés
- [ ] Les quatre niveaux de verbosité sont prouvés, y compris l'emboîtement
- [ ] Aucune narration ne se glisse sur la sortie standard
- [ ] Les exigences remplacées portent leur nouvel identifiant
- [ ] L'ADR CLI11 est écrite
- [ ] Les benchmarks sont rejoués et les mesures relevées
- [ ] La relecture de fin de phase est faite
