# Registre des exigences

Ce que `subedit` promet, ligne par ligne, avec l'état de ce qui le prouve.

Une exigence est **ce que le binaire montre** : sortie standard, sortie
d'erreur, code de retour, fichier produit. Les garanties internes du noyau n'en
sont pas — leurs tests unitaires et la couverture de lignes tiennent ce rôle.
La décision, et les sept options écartées, sont dans
[l'ADR 0014](adr/0014-registre-d-exigences.md).

Ce registre ne remplace pas le [manuel](manual/), qui reste la description
exhaustive de ce que l'utilisateur voit. Il dit autre chose : **quelle promesse
est démontrée par un test**, ce qu'aucun taux de couverture ne sait dire.

## Identifiants

`SURFACE-SUJET-NN`, en capitales, numéro sur deux chiffres — `CLI-VERSION-01`.
`CLI` aujourd'hui, `GUI` le jour où la fenêtre existera.

Cette forme est ce qui permet de reconnaître un identifiant parmi les tags d'un
test sans tenir de liste à jour : un tag ordinaire du projet — `[e2e]`,
`[format]`, `[framerate]` — n'a ni capitales ni segment numérique final.

**Un identifiant n'est jamais réutilisé.** Une exigence qui disparaît laisse sa
ligne, barrée, la raison dans la colonne d'état. Un identifiant retiré sans
trace laisse un trou que personne ne sait interpréter six mois plus tard.

## États

| État | Ce que la ligne affirme | Ce que le contrôle exige |
| :--- | :---------------------- | :----------------------- |
| `prévue` | l'exigence est décidée, le code ne l'honore pas encore | aucun test ne la cite |
| `implémentée` | le code l'honore | au moins un test la cite |
| `abandonnée` | elle ne sera pas honorée ; la raison suit dans la colonne | aucun test ne la cite |
| `remplacée` | une autre exigence prend sa place ; son identifiant suit | aucun test ne la cite |

Un test cite une exigence par un **tag Catch2** :

```cpp
TEST_CASE("invoking with no argument writes the version", "[e2e][CLI-VERSION-01]")
```

`src/scripts/check-requirements.sh` confronte ces affirmations aux tags que le
binaire de test déclare, et `make check-local` l'exécute. Une exigence
`implémentée` sans test ne franchit pas la porte ; un tag qui ne désigne
aucune exigence non plus.

**Le registre s'alimente en début d'issue**, avant le code. Une exigence écrite
après coup décrit ce qui a été fait ; écrite avant, elle décide ce qui sera
fait.

## Registre

**Rien ne suit cette table.** `src/scripts/verify-gates.sh` prouve que la porte
se referme en ajoutant une ligne en fin de fichier ; du texte après la table
lui ferait injecter son défaut au mauvais endroit, et la preuve ne prouverait
plus rien.

| ID | Exigence | Phase | État |
| :- | :------- | :---- | :--- |
| ~~`CLI-VERSION-01`~~ | l'invocation sans argument écrit `subedit <version>` sur la sortie standard | 3 | remplacée par `CLI-USAGE-01` |
| ~~`CLI-VERSION-02`~~ | rien n'est écrit sur la sortie d'erreur | 3 | remplacée par `CLI-VERSION-05` |
| ~~`CLI-VERSION-03`~~ | tout argument est ignoré, code de retour 0 | 3 | remplacée par `CLI-USAGE-02` |
| `CLI-VERSION-04` | `--version` écrit `subedit <version>` sur la sortie standard, code 0 | 3 | implémentée |
| `CLI-VERSION-05` | une invocation qui réussit n'écrit rien sur la sortie d'erreur | 3 | implémentée |
| `CLI-USAGE-01` | sans argument, l'aide est écrite sur la sortie standard, code 0 | 3 | implémentée |
| `CLI-USAGE-02` | une option inconnue ou une valeur invalide donne le code 1 | 3 | implémentée |
| `CLI-USAGE-03` | une erreur d'usage est détectée avant tout traitement | 3 | implémentée |
| `CLI-USAGE-04` | `--quiet` et `-v` ensemble donnent une erreur d'usage | 3 | implémentée |
| `CLI-OUTPUT-01` | seul le résultat va sur la sortie standard, la narration sur la sortie d'erreur | 3 | implémentée |
| `CLI-OUTPUT-02` | `--quiet` supprime toute narration et laisse passer les erreurs | 3 | implémentée |
| `CLI-OUTPUT-03` | chaque niveau de verbosité contient le précédent, ligne pour ligne | 3 | implémentée |
| `CLI-OUTPUT-04` | le niveau par défaut écrit une ligne par fichier traité | 3 | implémentée |
| `CLI-OUTPUT-05` | le bilan n'apparaît qu'à partir de deux fichiers d'entrée | 3 | implémentée |
| `CLI-BATCH-01` | plusieurs fichiers sont traités indépendamment, les échecs nommés | 3 | implémentée |
| `CLI-BATCH-02` | tous en échec donne le code 2, un échec partiel le code 3 | 3 | implémentée |
| `CLI-INSPECT-01` | format, encodage, BOM, fins de ligne, nombre et étendue sont rapportés | 3 | implémentée |
| `CLI-INSPECT-02` | des fins de ligne mélangées sont signalées avec leur ligne | 3 | implémentée |
| `CLI-INSPECT-03` | `--order-report` rend les deux lectures du désordre | 3 | prévue |
| `CLI-CONVERT-01` | `--to` produit le format demandé | 3 | prévue |
| `CLI-CONVERT-02` | fins de ligne et BOM sont conservés par défaut, réglables sinon | 3 | prévue |
| `CLI-CONVERT-03` | sans destination explicite, rien n'est écrit et le code est 1 | 3 | prévue |
| `CLI-SHIFT-01` | `--by` décale toutes les positions, dans les deux sens | 3 | prévue |
| `CLI-SHIFT-02` | un décalage rendant une position négative est refusé en nommant le sous-titre | 3 | prévue |
| `CLI-TRANSFORM-01` | deux repères transforment les positions, les autres suivent | 3 | prévue |
| `CLI-TRANSFORM-02` | deux indices confondus sont refusés | 3 | prévue |
| `CLI-FRAMERATE-01` | `--from`/`--to` convertit les positions | 3 | prévue |
| `CLI-FRAMERATE-02` | une fréquence nulle ou négative est refusée | 3 | prévue |
