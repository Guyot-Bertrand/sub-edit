# subedit-cli — manuel

Outil en ligne de commande de subedit : manipulation de fichiers de sous-titres
sans interface graphique, en traitement par lot ou depuis un script. Pour
l'autre programme et pour savoir par où commencer, voir
[le manuel](../index.md).

> **État actuel.** Les quatorze sous-commandes de `subedit-cli` existent :
> inspecter, convertir, décaler, transformer, recaler la cadence, aligner sur
> une grille d'images, retirer les mentions pour malentendants, ajuster les
> durées, remplacer un texte, changer la casse, l'italique et les tirets de dialogue, remettre les sous-titres dans l'ordre, et corriger les textes avec les motifs de Gaupol. Ce manuel
> décrit ce qui existe, jamais ce qui est prévu ; ce qui vient ensuite est dans
> la [feuille de route](../../feuille-de-route.md).

## Sections

| Section | Contenu |
| :------ | :------ |
| [Installation](installation.md) | construire et installer l'outil |
| [Invocation](invocation.md) | options globales, sorties, codes de retour |
| [Traiter un arbre](lots.md) | plusieurs fichiers, un répertoire entier, `--recursive` |
| [`inspect`](inspect.md) | rapporter ce qu'un fichier contient |
| [`convert`](convert.md) | écrire un fichier dans un autre format ou une autre forme |
| [`shift`](shift.md) | décaler toutes les positions d'une même durée |
| [`transform`](transform.md) | corriger toutes les positions à partir de deux repères |
| [`framerate`](framerate.md) | recaler un fichier d'une cadence d'images vers une autre |
| [`snap`](snap.md) | reposer les horodatages sur les images d'une cadence |
| [`hearing-impaired`](hearing-impaired.md) | retirer les mentions pour malentendants |
| [`adjust`](adjust.md) | ajuster les durées à une vitesse de lecture et à des bornes |
| [`replace`](replace.md) | remplacer un texte sans casser une balise |
| [`case`](case.md) | mettre les textes en casse de titre, de phrase, en capitales ou en minuscules |
| [`italics`](italics.md) | mettre les textes en italique, ou retirer leur italique |
| [`dialogue-dashes`](dialogue-dashes.md) | poser ou retirer les tirets de dialogue |
| [`sort`](sort.md) | remettre les sous-titres dans l'ordre de leur début |
| [`correct`](correct.md) | corriger les textes avec les motifs de Gaupol : mentions, erreurs courantes, majuscules, découpage de lignes |
