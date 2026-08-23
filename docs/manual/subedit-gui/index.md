# subedit-gui — manuel

La fenêtre de subedit : ouvrir un fichier de sous-titres et le voir dans une
table.

> **État actuel.** La fenêtre **ouvre**, **affiche**, **édite ses cellules**,
> **annule**, **enregistre**, **marque les sous-titres dont les positions ne
> tiennent pas debout**, et porte les quatre opérations de la phase — décaler,
> transformer, convertir la fréquence d'image, retirer les mentions pour
> malentendants. Elle **associe une vidéo au document**, choisie ou devinée,
> sans encore la jouer : le lecteur vient ensuite. L'insertion et la suppression
> de lignes viennent après, avec les préférences persistées. Voir la
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
| [Les opérations](operations.md) | décaler, transformer, convertir, retirer les mentions |
| [La vidéo associée](video.md) | choisir une vidéo, la proposition automatique, la barre d'état |
