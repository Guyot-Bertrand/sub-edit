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
> calage fin. L'insertion et la suppression de lignes viennent après, avec les
> préférences persistées. Voir la
> [feuille de route](../../feuille-de-route.md). Ce manuel décrit ce qui existe,
> jamais ce qui est prévu.

> **Ce manuel est de la prose, et rien ne le tient.** Les exemples de
> `subedit-cli` sont engendrés en exécutant la commande, ce qui les empêche de
> mentir. Une fenêtre n'a pas de `--help` : ni capture d'écran engendrée ni
> description engendrée de l'arbre de widgets n'ont été retenues, avec leurs
> raisons. **La justesse de ces pages repose donc sur leur relecture**, et sur
> rien d'autre.

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
