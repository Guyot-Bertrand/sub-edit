# 0018 — Séparer le vocabulaire des formats de leurs opérations

**Date :** 2026-08-17
**Statut :** acceptée

## Contexte

La fenêtre de la phase 5 ouvre un fichier, l'édite, et l'enregistre. Pour le
réécrire, elle doit se souvenir de son format. Or `SourceFile` — dont le
commentaire annonce « ce à quoi ressemblait le fichier dont vient un projet » —
retient le chemin, les fins de ligne, le BOM et l'en-tête, **et rien d'autre**.
Le format vit dans `ReadResult`, que la CLI se passe de fonction en fonction et
qu'un `Project` ne conserve pas.

Ajouter `SubtitleFormat` à `SourceFile` crée la première arête `model/ →
format/`. Le graphe du noyau est aujourd'hui acyclique — `time/` et `text/` ne
dépendent de rien, `model/ → time/`, `command/ → model/`, `edit/ → command,
model, time, text`, `format/ → model, time` — et cette arête serait son premier
cycle entre répertoires.

**Sauf que le cycle existe déjà, et que le nom des fichiers le cache.**
`model/format_extras.hpp` définit `SubRipExtras` et `WebVttExtras` ;
`model/subtitle.hpp` les inclut, donc un `Subtitle` porte des données nommées
d'après deux formats. `model/source_file.hpp` porte un champ `header` documenté
comme « l'en-tête WebVTT, vide pour les formats qui n'en ont pas ». Le modèle
connaît déjà les formats : il évite seulement d'en nommer l'énumération. La
frontière est propre parce que les types propres aux formats ont été rangés du
côté `model/`, et non parce que la dépendance serait absente.

Symétriquement, `format/` héberge des choses qui ignorent jusqu'au mot
« sous-titre » : une abstraction de système de fichiers et son écriture
atomique, un découpage de texte en lignes, une validation UTF-8, un repérage de
BOM et de fins de ligne.

Le critère qui départage n'est donc pas « est-ce lié à la lecture ? » mais
**« est-ce que ça connaît un format de sous-titres ? »**

## Décision

`format/` ne contient que les **opérations** qui traduisent des octets en modèle
et réciproquement — lecteurs, écrivains, détection — et les types que ces
opérations produisent.

Le **vocabulaire** qui décrit ce qu'un document *est* vit dans `model/` : son
format, ses données propres au format, sa forme de fichier.

Ce qui ne connaît aucun sous-titre sort des deux : `io/` pour l'accès au
disque, `text/` pour les opérations sur des chaînes.

| Ce qui bouge | Vers | Parce que |
| :----------- | :--- | :-------- |
| `SubtitleFormat`, l'énumération seule | `model/` | un document *a* un format comme il a des fins de ligne |
| `detectFormat` | reste dans `format/` | c'est une opération, pas une propriété |
| `lines` — `splitLines`, `trimmedBlanks`, `isBlank` | `text/` | découper du texte n'est pas lire un sous-titre |
| `encoding` — `isValidUtf8`, `hasUtf8Bom`, `scanNewlines` | `text/` | valider des octets non plus |
| `file_system`, `real_file_system`, `in_memory_file_system`, `atomic_write` | `io/` | ouvrir et écrire un fichier, quel qu'il soit |

`SourceFile` gagne alors son format sans qu'aucune arête nouvelle apparaisse.

### Le cas difficile : les diagnostics

`DiagnosticKind` mélange dix catégories dont trois — `EndBeforeStart`,
`OverlappingSubtitles`, `OutOfOrder` — ne décrivent pas ce qu'une lecture a
rencontré mais **ce qu'un document est**. `Project::outOfOrder()` recalcule
d'ailleurs la troisième.

Elles se séparent, et le repère dit pourquoi :

| | Repère | Vit tant que |
| :--- | :--- | :--- |
| `format/diagnostic.hpp` — ce qu'une lecture a rencontré | un **numéro de ligne** | le fichier n'a pas été édité |
| `analysis/anomaly.hpp` — ce qu'un document est | un **`SubtitleIndex`** | le document existe |

Une ligne n'existe qu'au moment de la lecture ; un indice survit à l'édition.
Restent donc du côté de la lecture : `IgnoredLine`, `MalformedTimestamp`,
`MissingNumbering`, `InconsistentNumbering`, `TextBeforeAnyTimestamp`,
`UnknownBlock`, `MixedNewlines`.

## Alternatives écartées

- **Laisser `SubtitleFormat` dans `format/` et faire porter le format par la
  fenêtre.** Elle tiendrait à côté de sa `Session` un champ dont `SourceFile`
  est précisément fait, et la question reviendrait au deuxième consommateur —
  la prévisualisation de la phase 6, le multi-projets de la phase 11. Un
  contournement qui se répète est une décision qu'on n'a pas prise.

- **Déplacer `FormatExtras` vers `format/`,** ce que la règle « les classes qui
  caractérisent les formats vont dans `format/` » suggère à première vue.
  Impossible sans inverser le graphe : `model/subtitle.hpp` inclut
  `FormatExtras`, donc `model/ → format/`. Le cycle réapparaît par l'autre bout.
  L'un des deux sens doit céder, et c'est celui-ci : **un lecteur a besoin du
  modèle, le modèle n'a jamais besoin d'un lecteur.**

- **Scinder `Diagnostic` en gardant le numéro de ligne des deux côtés.** Un
  document édité n'a plus de lignes ; la valeur serait juste à la lecture et
  fausse ensuite, ce qui est pire qu'absente.

- **Ne rien réorganiser.** L'accès au disque, la validation UTF-8 et le
  découpage en lignes resteraient sous un nom qui promet des formats de
  sous-titres, et `readSubtitles` se chercherait dans un répertoire qui contient
  aussi une écriture atomique.

## Conséquences

**Rendu facile.** `SourceFile` porte son format, donc enregistrer est une
opération complète sans état tenu à côté. Les phases à venir trouvent des
répertoires qui portent leur nom : les encodages de la phase 8 arrivent dans
`text/`, les formats complémentaires de la phase 9 dans `format/`. Les
anomalies d'un document deviennent calculables **après une édition**, ce que
Gaupol ne sait pas faire — et c'est exactement ce dont une table qui surligne
des rangs a besoin.

**Rendu difficile.** Rien, structurellement. Le déplacement touche 38 fichiers
et faisait retomber `tidy-scope.sh` sur l'analyse complète : une porte à plein
tarif, payée une fois. (Ce script n'existe plus depuis #269 ; l'analyse suit
maintenant les dépendances réelles.)

**Ce que ça coûte de défaire.** Un renommage, mécanique. La scission des
diagnostics, elle, ne se défait pas gratuitement : elle change ce que `inspect`
imprime — un numéro de sous-titre là où il donnait un numéro de ligne — et donc
le manuel, les exigences et les tests de bout en bout. C'est un repère de moins
bonne qualité pour qui ouvre le fichier dans un éditeur de texte, accepté parce
que l'indice est le seul qui survive à une édition.

**Déclencheur de réexamen.** Un format dont la *lecture* réclamerait un type de
modèle que rien d'autre n'utilise — les styles nommés d'ASS, en phase 9. Si le
modèle doit accueillir un type dont un lecteur est le seul client, la frontière
tracée ici est au mauvais endroit.
