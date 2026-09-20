# subedit-gui — manuel

La fenêtre de subedit : ouvrir un fichier de sous-titres et le voir dans une
table. Pour l'autre programme et pour savoir par où commencer, voir
[le manuel](../index.md).

> **État actuel.** La fenêtre **ouvre un fichier quel que soit son encodage**,
> **affiche**, **édite ses cellules**, **insère et supprime des lignes**,
> **annule**, **enregistre — en choisissant l'encodage, les fins de ligne et la
> marque d'ordre des octets** —, et **marque les sous-titres dont les positions
> ne tiennent pas debout**. Elle **fusionne et scinde** des lignes, **coupe,
> copie et colle** des textes, et **cherche et remplace** dans le texte visible
> sans casser les balises. Le menu `Tools` porte treize opérations — décaler,
> transformer, convertir la fréquence d'image, ajuster les durées, mettre en
> italique, poser les tirets de dialogue, changer la casse de quatre façons,
> retirer les mentions pour malentendants, aligner sur une cadence, ramener sur
> la grille — et l'analyse de grille, qui ne modifie rien. Elle **associe une vidéo
> au document**, choisie ou devinée, et la **joue dans la fenêtre**, la réplique
> courante dessinée sur l'image ; du pilotage, elle ne donne que jouer et
> arrêter. Elle **retient sa géométrie, ses colonnes et ses réglages** d'une
> session à l'autre, et se porte **claire ou sombre** au choix. `Help ▸ Manual`
> ouvre **ce manuel** dans une fenêtre. Ce manuel décrit ce qui existe, jamais
> ce qui est prévu ; ce qui vient ensuite est dans la
> [feuille de route](../../feuille-de-route.md).

> **Les images de ce manuel sont engendrées, la prose non.** Les exemples de
> `subedit-cli` sont engendrés en exécutant la commande ; les captures d'écran
> le sont en construisant la vraie fenêtre et en la photographiant — `make
> manual` fait les deux. Une image ne peut donc pas décrire une interface qui
> n'existe plus, et une interface qui change se voit dans le diff du dépôt. Ce
> qui reste sans filet est le texte autour, dont la justesse repose sur la
> relecture.

**Chaque écran est montré deux fois, sous les deux palettes que l'application
pose** — `Light` et `Dark`, celles que
[les préférences](preferences.md#le-thème) proposent. Sous un bureau dont le
thème gouverne, `System` donne l'une ou l'autre selon ce que le bureau a choisi.

![La fenêtre à l'ouverture, palette claire : les menus, la barre d'actions, la
bande où le film prendra place, et la table des
sous-titres.](captures/fenetre.png)

![La même fenêtre sous la palette sombre.](captures/fenetre-sombre.png)

## Les menus

`File`, `Edit`, `View`, `Video`, `Tools`, `Help` — dans l'ordre où l'on s'en
sert : le document, ce qu'on lui fait, comment on le regarde, ce qui
l'accompagne, ce qui l'examine, ce qui l'explique.

`View` ne porte pour l'instant qu'une entrée, `Translation`, qui montre ou retire
la colonne du même nom — voir [La table](table.md#la-colonne-de-traduction).

`Edit` porte cinq blocs, chacun sous un séparateur : l'annulation ; le
presse-papiers — couper, copier, coller des textes ; `Find and Replace…` ;
l'insertion, la suppression, la fusion et la scission de lignes ; enfin
`Preferences…`. Défaire est ce qu'on fait *à* une édition, déplacer un texte
n'ajoute ni ne retire de ligne, les quatre du bloc suivant changent le nombre de
lignes, et régler le thème n'est pas une édition du tout.

**Les touches sont nommées comme le menu les affiche**, en anglais comme le reste
de l'interface : `Ins`, `Del`, `Ctrl+Shift+Z`. Une touche qu'on presse et
qu'aucun menu ne nomme garde son nom français : `Entrée`, `Échap`, `Maj`.

`Help` porte deux entrées, dans l'ordre où le menu les montre. `Manual`, ou
`F1`, ouvre ce manuel dans une fenêtre — voir
[Le manuel dans la fenêtre](aide.md). `About subedit` dit la version et la
licence.

## Sections

| Section | Contenu |
| :------ | :------ |
| [Installation](../subedit-cli/installation.md) | construire et installer — la page vaut pour les deux programmes |
| [Invocation](invocation.md) | lancer la fenêtre, arguments, codes de retour |
| [Ouvrir et enregistrer](fichiers.md) | les commandes, la traduction, les diagnostics, l'encodage dans la barre d'état, l'aller-retour, la fermeture |
| [La table](table.md) | ce que chaque colonne montre |
| [Éditer une cellule](edition.md) | quelles cellules s'éditent, et comment |
| [Couper, copier et coller des textes](presse-papiers.md) | les trois entrées, ce qui voyage, les lignes ajoutées, les balises traduites |
| [Rechercher et remplacer](recherche.md) | le dialogue, les quatre gestes, les deux options, sur quoi porte la recherche |
| [Insérer, supprimer, fusionner et scinder des lignes](lignes.md) | les quatre entrées, leurs raccourcis, où vont les lignes neuves |
| [Annuler et rétablir](annulation.md) | l'historique, les deux actions, la marque de modification |
| [La grille d'images](grille.md) | la cadence déduite des positions, et l'analyse |
| [Les opérations](operations.md) | décaler, transformer, convertir, ajuster les durées, mettre en italique, la casse, les tirets, retirer les mentions, aligner, ramener sur la grille, et ce qui dépasse la fin du film |
| [La vidéo associée](video.md) | choisir une vidéo, la proposition automatique, la barre d'état, `ffmpeg` |
| [Le lecteur](lecteur.md) | la vue vidéo, jouer, la ligne qui suit, la réplique dessinée |
| [Les préférences](preferences.md) | le thème, le fichier de préférences, ses options, et ce qu'il advient d'une valeur illisible |
| [Le manuel dans la fenêtre](aide.md) | `Help ▸ Manual`, ce qu'il ouvre et comment y naviguer |
