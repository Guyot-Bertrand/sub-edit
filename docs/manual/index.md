# subedit — manuel

subedit lit, inspecte et recale des fichiers de sous-titres. Il vient en deux
programmes qui partagent le même noyau, donc les mêmes règles et les mêmes mots.

| Programme | Pour quoi faire | Son manuel |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | ouvrir un fichier, le voir dans une table, l'éditer, le regarder contre le film | [manuel de la fenêtre](subedit-gui/index.md) |
| `subedit-cli` | traiter des fichiers en lot, depuis un script, sans interface | [manuel de la ligne de commande](subedit-cli/index.md) |

Les deux lisent et écrivent **SubRip** (`.srt`) et **WebVTT** (`.vtt`), et
reconnaissent le format au contenu plutôt qu'à l'extension. **L'encodage aussi
est reconnu au contenu** : un fichier en Latin-1, en CP1252 ou en UTF-16
s'ouvre, et ce qu'il a été lu comme est dit. Ce qui est écrit, en revanche, est
de l'UTF-8.

## Par où commencer

| Ce que vous voulez | Où aller |
| :----------------- | :------- |
| installer l'outil | [Installation](subedit-cli/installation.md) — elle vaut pour les deux programmes |
| ouvrir un fichier et regarder ce qu'il contient | [La table](subedit-gui/table.md) |
| recaler des sous-titres décalés d'une durée constante | [Les opérations](subedit-gui/operations.md#shift-positions), ou [`shift`](subedit-cli/shift.md) |
| recaler un fichier écrit pour une autre cadence d'images | [`framerate`](subedit-cli/framerate.md), et [`snap`](subedit-cli/snap.md) si le minutage est déjà juste |
| savoir contre quelle cadence un fichier a été écrit | [La grille d'images](subedit-gui/grille.md), ou [`inspect`](subedit-cli/inspect.md) |
| retirer les mentions pour malentendants | [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| traiter cent fichiers d'un coup | [Invocation](subedit-cli/invocation.md) |

**Entre `framerate` et `snap`, on se trompe sans que rien ne le signale.** Si
vous hésitez, [`snap`](subedit-cli/snap.md#snap-nest-pas-framerate-et-sen-tromper-ne-se-voit-pas)
ouvre sur la comparaison des deux.

## Ce que ce manuel promet

**Il décrit ce qui existe, jamais ce qui est prévu.** Ce qui viendra, et dans
quel ordre, est dans la [feuille de route](../feuille-de-route.md).

**Le manuel est en français, l'outil est en anglais.** Noms de sous-commandes,
options, intitulés de menu, messages : tout ce que le programme écrit est en
anglais, et ce manuel le cite tel quel plutôt que de le traduire. La traduction
est une étape à elle seule ; d'ici là, une seule langue à l'écran vaut mieux que
deux qui divergent.

**Les exemples et les images sont engendrés, la prose non.** Les blocs
`console` sont produits en exécutant réellement la commande ; les captures
d'écran, en construisant la vraie fenêtre et en la photographiant. Aucun des
deux ne peut donc décrire un outil qui n'existe plus. Ce qui reste sans filet
est le texte autour, dont la justesse repose sur la relecture.

**Les captures montrent les deux palettes**, `Light` et `Dark` — celles que
[les préférences](subedit-gui/preferences.md#le-thème) proposent.

## Les deux surfaces disent les mêmes mots

Une raison de lire l'un des deux manuels quand on se sert de l'autre : les
messages d'erreur, les noms de format, les verdicts de grille et les libellés
d'annulation sont écrits **une fois**, dans le noyau, et les deux programmes les
citent. Un fichier refusé donne la même phrase des deux côtés.

Ce qui diffère est ce que chaque surface peut faire :

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| plusieurs fichiers d'un coup | non — une fenêtre, un document | oui |
| annuler | oui, mille entrées | sans objet — rien n'est modifié en place sans le demander |
| éditer un texte ou une position à la main | oui | non |
| insérer et supprimer des lignes | oui | non |
| regarder le film pendant qu'on cale | oui | non |
| écrire par-dessus l'entrée | `Save` | `--in-place` |
