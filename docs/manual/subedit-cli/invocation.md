# Invocation

```
subedit-cli
```

Affiche la version de la bibliothèque avec laquelle l'outil a été construit,
puis se termine.

```console
$ subedit-cli
subedit 0.2.2
```

## Sortie

Une ligne sur la **sortie standard**, terminée par un retour à la ligne :
le mot `subedit`, une espace, puis la version au format `majeur.mineur.patch`.
Rien n'est écrit sur la sortie d'erreur.

## Arguments

**L'outil n'en accepte aucun, et n'en refuse aucun non plus.** Tout ce qui suit
le nom du programme est ignoré sans un mot, y compris `--help` et `--version` :

```console
$ subedit-cli --help fichier.srt
subedit 0.2.2
$ echo $?
0
```

C'est le comportement d'un programme qui n'a pas encore d'analyse d'arguments,
pas celui d'un programme qui les valide. Un script ne doit donc pas déduire
d'un code de retour nul que ce qu'il a passé a été compris.

## Code de retour

| Code | Signification |
| :--- | :------------ |
| `0` | succès — le seul cas possible aujourd'hui |

## À venir

L'analyse d'arguments, les sous-commandes et les codes de retour signifiants
relèvent de la phase 3. Leur périmètre — conversion, décalage, transformation,
ajustement des durées, correction, inspection — sera arbitré au cadrage de cette
phase, sachant que Gaupol n'offre pas d'équivalent : c'est une conception neuve.
