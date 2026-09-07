# 0030 — Ce qu'un document retient de son fichier, la fréquence comprise

Statut : acceptée — 2026-09-07
Décidée en cadrant la phase 9, issue #337.

## Contexte

Un document `subedit` retient déjà de son fichier ce qu'aucun sous-titre ne
porte : son **format**, son **encodage**, sa **marque d'ordre des octets**, ses
**fins de ligne** et son **en-tête**, une chaîne libre. C'est ce qui rend
l'aller-retour d'octets possible — l'écriture repose ce que la lecture a trouvé.

Les sept formats de la phase 9 ajoutent trois choses à cette liste, et la
troisième est la plus lourde.

**L'ordre des champs d'événement SSA.** Un fichier SSA déclare ses colonnes :
`Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text`.
Gaupol le garde dans `event_fields` parce que l'ordre varie d'un producteur à
l'autre. L'oublier ferait réécrire tout le fichier avec d'autres colonnes.

**Un réglage de forme, `two_digit_hour`.** TMPlayer s'écrit `0:00:12:` ou
`00:00:12:` selon le fichier, et Gaupol retient lequel.

**La fréquence d'image de MicroDVD.** Elle est d'une autre nature, et c'est le
cœur de cette décision. MicroDVD ne compte pas en temps : ses positions **sont**
des numéros d'image — `{1194}{1285}`. Le modèle, lui, compte en millisecondes
entières depuis l'[ADR 0006](0006-positions-en-millisecondes.md). Lire un
MicroDVD demande donc une fréquence, et **aucun fichier MicroDVD n'en déclare.**

La déduction de la phase 16 ne peut rien : `deduceFrameRate` cherche une grille
dans des millisecondes, et des numéros d'image consécutifs tombent sur toutes
les grilles à la fois.

## Décision

**Un `variant` par format au niveau du fichier**, jumeau de `FormatExtras` au
niveau du sous-titre, porte ce qu'un fichier déclare de lui-même et qu'aucun
sous-titre ne porte. L'en-tête libre reste la chaîne qu'il est.

**La fréquence d'un fichier en images y entre**, et l'écriture la reprend : c'est
ce qui rend l'aller-retour exact quelle que soit sa valeur.

**Elle a trois sources, dans cet ordre**, et la lecture dit toujours laquelle a
servi :

1. ce que l'utilisateur dit — `--frame-rate`, ou le dialogue de la fenêtre ;
2. ce que la vidéo associée déclare, quand il y en a une ;
3. **un défaut déclaré, 23,976**, annoncé par un diagnostic.

## Alternatives écartées

### Garder les images dans le modèle

Ce serait le `modes.FRAME` de Gaupol, et cela contredit frontalement
l'[ADR 0006](0006-positions-en-millisecondes.md), dont tout le noyau dépend :
chaque opération, chaque commande d'annulation, chaque anomalie et la table
entière raisonnent en millisecondes. Un second mode de positions doublerait la
surface du modèle pour un format sur neuf.

L'[ADR 0011](0011-numero-d-image-en-type-fort.md) a déjà tranché la forme que
prend une image chez nous : un type fort `Frame`, qui traverse une conversion et
ne s'installe pas dans le modèle.

### Refuser d'ouvrir faute de fréquence

C'est la voie la plus honnête sur le papier — nous n'inventons rien — et elle a
été écartée sur l'usage. Un fichier qu'on vient de double-cliquer doit s'ouvrir.
Une valeur annoncée qu'on peut corriger en une action vaut mieux qu'un refus qui
laisse la fenêtre vide et n'apprend rien.

Elle contredirait aussi la règle de la maison : la lecture ouvre au mieux et
signale ([0008](0008-lecture-au-mieux-avec-diagnostics.md)). Refuser est réservé
à ce qui n'est pas lisible du tout.

### Deviner la fréquence dans le contenu

Il n'y a rien à deviner. Un `{1194}{1285}` est compatible avec 23,976 comme avec
60 ; ce qu'on lirait n'est pas une fréquence mais une préférence.

### Un défaut à 25

25 est la fréquence PAL, celle de l'Europe, et probablement la plus fréquente
sur les fichiers que ce projet rencontrera. **23,976 est celle de Gaupol**, et
l'iso-fonctionnalité est la seule raison qui départage deux chiffres également
arbitraires. Un fichier ouvert par les deux outils doit donner les mêmes
horodatages, sans quoi la comparaison qui fonde tout le projet ne veut plus rien
dire.

### Tout mettre dans l'en-tête

`header` est une chaîne, et elle suffit à ce qu'un aller-retour doit rendre
puisque rien n'a besoin de la comprendre. Elle ne suffit pas à `event_fields` :
l'écriture doit **relire** l'ordre des colonnes pour composer chaque ligne
`Dialogue:`, donc le reconstruire par analyse de la chaîne à chaque écriture. Ce
serait ranger une donnée structurée dans du texte libre pour la réanalyser
aussitôt.

## Conséquences

**`FormatExtras` ne devient pas un variant à neuf branches.** Cinq des neuf
formats ne portent rien par sous-titre — SubViewer 2, MicroDVD, MPL2, TMPlayer,
LRC restent sur `std::monostate` — et SSA et ASS partagent une branche. Il passe
de trois à quatre. La garde du compilateur que son commentaire revendique tient
sans changer de forme.

**Un document en images a une fréquence, un document en temps n'en a pas.** La
distinction est portée par le variant, donc par le compilateur : demander sa
fréquence à un document SubRip ne compile pas.

**La conversion vers un format en images demande une fréquence qui n'existe pas
dans la source.** Elle vient alors de `--frame-rate`, ou de la grille déduite —
et c'est le seul endroit du projet où la phase 16 sert à autre chose qu'à
informer : la grille d'un fichier temporel *est* la fréquence à laquelle il a
été calé. Sans l'une ni l'autre, la conversion refuse plutôt que d'inventer un
chiffre qui déplacerait chaque réplique.
