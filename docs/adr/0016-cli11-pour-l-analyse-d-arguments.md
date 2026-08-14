# 0016 — Utiliser CLI11 pour l'analyse d'arguments

**Date :** 2026-08-14
**Statut :** acceptée

## Contexte

La phase 3 donne au binaire cinq sous-commandes — `inspect`, `convert`,
`shift`, `transform`, `framerate` — chacune avec ses options, ses valeurs par
défaut, ses combinaisons interdites et ses refus. Jusqu'ici le binaire
n'analysait rien : il écrivait sa version et ignorait tout ce qu'on lui passait.

Le manuel du projet exige que chaque option soit décrite avec sa forme longue et
courte, son caractère requis ou non, sa valeur par défaut, l'ensemble fermé de
ses valeurs acceptées, et le message d'erreur correspondant à chaque refus.
C'est une contrainte lourde, et elle porte précisément sur ce qu'un analyseur
d'arguments produit.

## Décision

CLI11, résolue par `find_package` sur le paquet de la distribution
(`libcli11-dev`), comme le veut l'[ADR 0004](0004-gestion-des-dependances.md).

## Alternatives écartées

- **Une implémentation propre** — zéro dépendance, ce qui est la pente
  naturelle du projet. Écartée sur le rapport entre ce qu'elle coûte et ce
  qu'elle évite : cinq sous-commandes, leurs options, leur aide et leurs
  messages d'erreur sont exactement la matière où un analyseur écrit à la main
  dérive — non pas en cassant, mais en devenant peu à peu incohérent d'une
  sous-commande à l'autre. Le manuel rendrait cette incohérence visible sans
  la corriger.
- **cxxopts** — également empaquetée et plus légère. Écartée parce qu'elle n'a
  pas de notion de sous-commande : l'aiguillage, l'aide par sous-commande et la
  validation des combinaisons resteraient à écrire, c'est-à-dire le gros de ce
  qu'on cherchait à ne pas écrire.

## Conséquences

L'aide est **engendrée** à partir de la déclaration des options, et non
maintenue à côté d'elle. Conjuguée à `make manual`, livré par l'issue #52, cela
ferme la boucle : la déclaration produit l'aide, l'aide produit le bloc du
manuel, et rien n'est recopié à aucune étape. Une option ajoutée sans un mot au
manuel devient impossible plutôt qu'improbable.

Une dépendance de plus à installer, ajoutée à `setup-toolchain.sh`. Elle ne
concerne que `src/exe/` : le noyau reste sans dépendance, et rien dans
`src/lib/` ne connaît CLI11 — une CLI n'est qu'un appelant parmi d'autres, et
la fenêtre de la phase 5 en sera un second.

**Ce qui rend la décision peu coûteuse à défaire :** l'analyse d'arguments vit
dans un seul fichier de `src/exe/`, qui traduit une ligne de commande en appels
au noyau. En changer revient à réécrire ce fichier, sans toucher ni au noyau ni
aux tests de bout en bout, qui passent par le binaire et ignorent ce qu'il y a
dedans.
