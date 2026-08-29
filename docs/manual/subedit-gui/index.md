# subedit-gui — manuel

La fenêtre de subedit : ouvrir un fichier de sous-titres et le voir dans une
table.

> **État actuel.** La fenêtre **ouvre**, **affiche**, **édite ses cellules**,
> **annule**, **enregistre**, **marque les sous-titres dont les positions ne
> tiennent pas debout**, et porte quatre opérations — décaler, transformer,
> convertir la fréquence d'image, retirer les mentions pour malentendants. Elle
> **associe une vidéo au document**, choisie ou devinée, et la **joue dans la
> fenêtre**, la réplique courante dessinée sur l'image. Du pilotage, elle ne
> donne que jouer et arrêter : le reste d'une barre de lecteur vient avec le
> calage fin. Elle **retient sa géométrie et ses colonnes** d'une session à
> l'autre, et se porte **claire ou sombre** au choix. L'insertion et la
> suppression de lignes viennent après. Voir la
> [feuille de route](../../feuille-de-route.md). Ce manuel décrit ce qui existe,
> jamais ce qui est prévu.

> **Les images de ce manuel sont engendrées, la prose non.** Les exemples de
> `subedit-cli` sont engendrés en exécutant la commande ; les captures d'écran
> le sont en construisant la vraie fenêtre et en la photographiant — `make
> manual` fait les deux. Une image ne peut donc pas décrire une interface qui
> n'existe plus, et une interface qui change se voit dans le diff du dépôt. Ce
> qui reste sans filet est le texte autour, dont la justesse repose sur la
> relecture.

![La fenêtre à l'ouverture : les menus, la barre d'actions, la bande où le film
prendra place, et la table des sous-titres.](captures/fenetre.png)

## Les menus

`File`, `Edit`, `Video`, `Tools`, `Help` — dans l'ordre où l'on s'en sert : le
document, ce qu'on lui fait, ce qui l'accompagne, ce qui l'examine, ce qui
l'explique.

`Edit` porte l'annulation, puis, sous un séparateur, `Preferences…` — défaire
est ce qu'on fait *à* une édition, régler le thème n'est pas une édition.

`Help` porte deux entrées. `About subedit` dit la version et la licence.
`Manual` est **présente et éteinte** : le manuel qu'elle ouvrira, et l'endroit
où il sera installé, viennent avec l'empaquetage.

## Sections

| Section | Contenu |
| :------ | :------ |
| [Invocation](invocation.md) | lancer la fenêtre, arguments, codes de retour |
| [Ouvrir et enregistrer](fichiers.md) | les trois commandes, les diagnostics, l'aller-retour |
| [La table](table.md) | ce que chaque colonne montre |
| [Éditer une cellule](edition.md) | quelles cellules s'éditent, et comment |
| [Annuler et rétablir](annulation.md) | l'historique, les deux actions, la marque de modification |
| [La grille d'images](grille.md) | la cadence déduite des positions, et l'analyse |
| [Les opérations](operations.md) | décaler, transformer, convertir, retirer les mentions, et ce qui dépasse la fin du film |
| [La vidéo associée](video.md) | choisir une vidéo, la proposition automatique, la barre d'état, `ffmpeg` |
| [Le lecteur](lecteur.md) | la vue vidéo, jouer, la ligne qui suit, la réplique dessinée |
| [Les préférences](preferences.md) | le thème, le fichier de préférences, ses options, et ce qu'il advient d'une valeur illisible |
