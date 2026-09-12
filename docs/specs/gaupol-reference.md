# Gaupol — inventaire des fonctionnalités (référence)

Source : https://github.com/otsaloma/gaupol — clone local dans `reference/gaupol`
(non commité), à la révision `f433bae` (2026-07-28, post-1.16, branche 2.0 en
cours).

## 1. Architecture d'origine

Deux paquets Python nettement séparés :

| Paquet | LOC (hors tests) | Rôle |
| :----- | ---------------: | :--- |
| `aeidon` | ~9 100 | Bibliothèque pure : lecture/écriture/manipulation de sous-titres. Publiée seule sur PyPI, aucune dépendance GUI. |
| `gaupol` | ~13 100 | Interface GTK 4 (PyGObject), lecteur vidéo GStreamer, dialogues. |
| tests | ~6 600 | Suite pytest couvrant les deux. |

Traductions : 20 locales (`po/`).

Cette séparation lib/UI correspond directement au découpage `src/lib` + `src/exe`.

## 2. Formats de fichiers (9)

Chaque format a un lecteur/écrivain (`aeidon/files/`) **et** un convertisseur de
balises (`aeidon/markups/`), plus un en-tête par défaut (`aeidon/data/headers/`).

| Format | Ext. | Mode | En-tête | Détection |
| :----- | :--- | :--- | :------ | :-------- |
| SubRip | `.srt` | temps | non | `00:00:00,000 --> …` |
| Advanced Sub Station Alpha | `.ass` | temps | oui | `ScriptType: v4.00+` |
| Sub Station Alpha | `.ssa` | temps | oui | `ScriptType: v4.00` |
| WebVTT | `.vtt` | temps | oui | `WEBVTT` |
| MicroDVD | `.sub` | **frames** | oui | `{123}{456}` |
| SubViewer 2.0 | `.sub` | temps | oui | `00:00:00.00,00:00:00.00` |
| MPL2 | `.txt` | temps | non | `[123][456]` |
| TMPlayer | `.txt` | temps | non | `0:00:00:` |
| LRC | `.lrc` | temps | oui | `[00:00.00]` |

Détection de format par regex sur le contenu. Conversion inter-format (y compris
temps ↔ frames) et conversion de balises (italique, gras, couleur, police…)
entre les syntaxes de chaque format.

### Encodages et fins de ligne
- Détection automatique de l'encodage (via `charset-normalizer`), encodage de la
  locale, liste d'encodages de repli configurable.
- Fins de ligne Unix / Windows / Mac classique, forçables à l'enregistrement.

## 3. Modèle de données

- **Sous-titre** = position début/fin + texte principal + texte de traduction.
- Positions manipulables en trois modes : **temps**, **frames**, **secondes**,
  avec conversion via framerate (8 valeurs prédéfinies de 23.976 à 60.000 +
  framerates personnalisés).
- **Deux documents par projet** : `MAIN` et `TRANSLATION` — le mode traduction
  est un pilier de l'outil, pas un ajout.
- **Undo/redo complet** (`aeidon/agents/register.py`) : toute action est
  enregistrée comme réversible, avec regroupement d'actions et description
  lisible. Pile bornée.
- Modèle observable (signaux) découplant lib et UI.

## 4. Opérations cœur (API `aeidon`)

Regroupées par « agent » — chaque agent est un lot cohérent de méthodes.

**Ouverture / sauvegarde** — `open_main`, `open_translation`, `save_main`,
`save_translation`, avec alignement du fichier de traduction par numéro de
sous-titre ou par position.

**Édition** — insertion, suppression, fusion (`merge_subtitles`), scission
(`split_subtitle`), remplacement en masse de textes et de positions, effacement.

**Positions** — décalage (`shift_positions`), transformation affine à partir de
deux points de repère (`transform_positions`), conversion de framerate
(`convert_framerate`), ajustement des durées (`adjust_durations` : durée
min/max, vitesse de lecture en caractères/seconde, écart minimal entre
sous-titres, allonger et/ou raccourcir).

**Texte** — casse (minuscules / MAJUSCULES / Phrase / Title Case), italiques,
tirets de dialogue (ajout / retrait / bascule), retour à la ligne automatique,
correction d'erreurs courantes, suppression des mentions pour malentendants,
jonction/scission de mots via correcteur orthographique.

**Recherche** — recherche et remplacement, texte brut ou regex, sensible ou non
à la casse, ciblant le texte principal, la traduction ou les deux, sur la
sélection / le projet courant / tous les projets ouverts.

`aeidon/parser.py` est la pièce que l'[ADR 0009](../adr/0009-texte-en-chaine-brute.md)
appelle le parseur conscient des balises : il retire les balises, retient leurs
positions, remplace dans le texte nu, puis les remet. **Il n'a pas de règle pour
une correspondance qui traverse une frontière de balise, seulement une
arithmétique de décalage**, et la frontière retombe à une place qui ne veut rien
dire dans le texte neuf :

| Texte | Cherché → mis à la place | Ce que Gaupol écrit |
| :---- | :----------------------- | :------------------ |
| `<i>Bon</i>jour` | `Bonjour` → `Salut` | `<i>S</i>alut` |
| `<i>Bon</i>jour` | `Bonjour` → `Bonjour tout le monde` | `<i>Bonjour tout le m</i>onde` |
| `Bon<i>jour</i>` | `Bonjour` → `Salut` | `S<i>alut</i>` |

Mesuré en exécutant le parseur, non déduit de sa lecture. C'est le seul point où
la phase 10 s'écarte de lui délibérément : `src/test/data/textes/recherche.cas`
porte la règle retenue — une frontière qui coupe un mot l'englobe — et les cas
qui la fixent.

**Presse-papiers** — copier / couper / coller **des textes seuls**.
`aeidon/clipboard.py` est une liste de chaînes, une par sous-titre, que
`get_string` recolle par une ligne vide : ni positions, ni format, ni sous-titre
entier. Coller ajoute des lignes s'il en manque pour recevoir les textes.
*(Corrigé après lecture, en initialisant la phase 10 — cet inventaire disait
« sous-titres entiers ».)*

**Prévisualisation** — génération d'un fichier temporaire et lancement d'un
lecteur externe (MPlayer, mpv, VLC, ou commande personnalisée) positionné au
sous-titre courant.

### `adjust_durations`, dans l'ordre

Lu dans `aeidon/agents/position.py`, parce que l'ordre de résolution est ce que
la phase 10 doit spécifier et qu'il n'est écrit nulle part chez Gaupol : il est
l'ordre des lignes.

**Un seul parcours, et une seule borne déplacée — la fin.** Le début n'est
jamais touché. Les quatre contraintes s'appliquent l'une après l'autre :

| Rang | Contrainte | Ce qu'elle pose |
| ---: | :--------- | :-------------- |
| 1 | vitesse de lecture | `fin = début + longueur / vitesse`, si `lengthen` et trop court, ou si `shorten` et trop long |
| 2 | durée minimale | `fin = début + minimum` |
| 3 | durée maximale | `fin = début + maximum` |
| 4 | écart au suivant | `fin = max(début, suivant.début − écart)` |

**La dernière appliquée gagne, et elle défait la précédente.** L'écart peut
ramener la durée sous le minimum qu'on venait de poser, et jusqu'à zéro. Gaupol
ne le signale pas.

**La longueur du texte se compte hors balises** — `get_text_length` retire ce
que l'expression de balises du format reconnaît, puis prend `len`, saut de ligne
compris.

Trois pièges, à ne pas reproduire par inadvertance :

- **un minimum de zéro n'est pas un minimum de zéro, c'est pas de minimum.**
  `domin = minimum and …` : en Python, `0` est faux. Le maximum a le même
  défaut ; l'écart, testé par `gap is not None`, ne l'a pas ;
- **le dernier sous-titre n'a pas de suivant**, et reçoit une borne en dur à
  360 000 s, soit cent heures ;
- les valeurs par défaut de `gaupol/config.py` sont minimum 1,5 s **actif**,
  maximum 6 s **inactif**, écart 0 s actif, vitesse 15 car/s, allonger seulement.

**Les quatre sont contradictoires sur un corpus réel**, et cela se mesure sans
écrire un ajustement : `src/scripts/measure-duration-constraints.py` compte les
sous-titres pour lesquels aucune fin ne satisfait deux d'entre elles. Issue #371.

## 5. Moteur de correction de texte

C'est la partie la plus « métier » de l'outil, pilotée par des fichiers de
motifs déclaratifs (`aeidon/data/patterns/*.conf`), classés par script/langue/pays
(ISO 15924 / 639 / 3166) : `Latn`, `Latn-en`, `Latn-fr`, `Latn-fi`, `Latn-en-US`,
`Zyyy`.

Quatre familles de motifs :
- **common-error** — erreurs courantes, classées `Human` / `OCR` (espaces autour
  de la ponctuation, guillemets, confusions OCR `l`/`I`, etc.).
- **capitalization** — remise en majuscule en début de phrase.
- **hearing-impaired** — suppression des `[BRUIT]`, noms de locuteurs, etc.
- **line-break** — pénalités de coupure pour l'algorithme de mise en lignes.

Format de motif : nom, description, classes, regex + flags, remplacement,
politique, répétition — traduisible.

L'**algorithme de retour à la ligne** (`aeidon/liner.py`) est une variante de
Knuth–Plass (boîtes / pénalités / démérites) adaptée au sous-titrage : longueur
max de ligne, nombre max de lignes, unité de longueur (caractères ou **em**),
seuils de non-intervention.

Le tout est exposé dans l'UI par un assistant multi-pages (`gaupol/assistants.py`)
qui enchaîne les corrections choisies avec un écran de confirmation
avant/après.

## 6. Interface graphique

**Multi-projets** : onglets, sauvegarde groupée, fermeture groupée, scission d'un
projet en deux, ajout d'un fichier à la suite d'un autre.

**Vue tabulaire** : colonnes n°, début, fin, durée, texte principal, traduction —
affichables/masquables et réordonnables. Rendus spécialisés par type de colonne
(temps, entier, flottant, multiligne). Affichage de la longueur des lignes,
zébrures, police personnalisable, coloration des différences (insertion /
suppression / modification) lors des corrections.

**Lecteur vidéo intégré** (GStreamer, `gtk4paintablesink`) : lecture/pause,
saut avant/arrière, saut au sous-titre précédent/suivant, lecture de la seule
sélection, incrustation des sous-titres et du timecode (police, couleur,
alpha, position configurables), sélection de piste audio, volume.

**Synchronisation vidéo ↔ sous-titres** : définir le début / la fin depuis la
position vidéo, insérer un sous-titre à la position vidéo, sélectionner le
sous-titre suivant/précédent depuis la position vidéo, avancer/reculer début ou
fin par petits incréments. C'est le cœur du travail de *timing*.

**Correcteur orthographique** (libspelling) : dialogue dédié éditable,
soulignement à la volée, dictionnaire personnel, liste de remplacements
mémorisée.

**Dialogues** : ouverture/sauvegarde, ajout de fichier, décalage de positions,
transformation de positions, conversion de framerate, ajustement des durées,
insertion, scission, recherche/remplacement, correction orthographique,
préférences, encodage, langue, débogage.

**Préférences persistées** (~30 sections) : encodages, éditeur, lecteur vidéo,
prévisualisation, recherche, correction, thème clair/sombre.

## 7. Ligne de commande

`gaupol [FICHIERS…]` avec `-e/--encoding`, `-t/--translation-file`,
`-a/--align-method`, `-v/--video-file`, `--list-encodings`, `--version`.
Pas de mode batch sans interface — la manipulation par script passe par la
bibliothèque `aeidon` en Python.

## 8. Points de tension côté performance

Observations à confirmer par mesure, pas des conclusions :

- Les corrections de texte appliquent des dizaines de regex sur chaque
  sous-titre, en Python, avec réenregistrement undo par action.
- L'algorithme de mise en lignes est combinatoire et tourne sur tout le fichier.
- La vue tabulaire GTK matérialise les données à travers PyGObject à chaque
  rendu de cellule.
- L'ouverture/détection d'encodage lit et décode le fichier plusieurs fois.
- Recherche/remplacement globale : parcours Python de tous les projets ouverts.

## 9. Licence

GPL-3.0-or-later. Une réécriture indépendante ne reprend pas le code ; en
revanche les **fichiers de motifs** et les **fichiers d'en-tête** sont des
données sous la même licence — leur réutilisation telle quelle imposerait la
GPL au projet. À trancher.
