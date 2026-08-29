# subedit-cli — manuel

Outil en ligne de commande de subedit : manipulation de fichiers de sous-titres
sans interface graphique, en traitement par lot ou depuis un script. Pour
l'autre programme et pour savoir par où commencer, voir
[le manuel](../index.md).

> **État actuel.** Les sept sous-commandes de `subedit-cli` existent :
> inspecter, convertir, décaler, transformer, recaler la cadence, aligner sur
> une grille d'images et retirer les mentions pour malentendants. Ce manuel
> décrit ce qui existe, jamais ce qui est prévu ; ce qui vient ensuite est dans
> la [feuille de route](../../feuille-de-route.md).

## Sections

| Section | Contenu |
| :------ | :------ |
| [Installation](installation.md) | construire et installer l'outil |
| [Invocation](invocation.md) | options globales, sorties, codes de retour |
| [`inspect`](inspect.md) | rapporter ce qu'un fichier contient |
| [`convert`](convert.md) | écrire un fichier dans un autre format ou une autre forme |
| [`shift`](shift.md) | décaler toutes les positions d'une même durée |
| [`transform`](transform.md) | corriger toutes les positions à partir de deux repères |
| [`framerate`](framerate.md) | recaler un fichier d'une cadence d'images vers une autre |
| [`snap`](snap.md) | reposer les horodatages sur les images d'une cadence |
| [`hearing-impaired`](hearing-impaired.md) | retirer les mentions pour malentendants |
