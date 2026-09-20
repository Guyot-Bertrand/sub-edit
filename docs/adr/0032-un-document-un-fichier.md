# 0032 — Retenir un fichier par document

Statut : acceptée — 2026-09-20
Décidée en cadrant la phase 11, issue #426.

## Contexte

Un sous-titre porte depuis la phase 1 **les deux textes et une seule paire de positions** : une
traduction n'a pas de temps à elle. Le modèle est prêt à accueillir un second document, et
l'historique le sait déjà — il tient ses modifications par document, et `Session` sait dire si l'un
ou l'autre est modifié, ou vient d'être enregistré.

**Ce qui manque est ce qui *entre*.** `Project` ne retient qu'**un** `SourceFile` : un chemin, un
format, des fins de ligne, un encodage, un en-tête, ce que le fichier déclare de lui-même
([ADR 0030](0030-ce-qu-un-document-retient-de-son-fichier.md)). Or une traduction est un fichier
comme l'autre, et **rien n'oblige les deux à se ressembler** : un principal en SubRip et une
traduction en SSA, l'un en UTF-8 et l'autre en Windows-1252. Gaupol le sait : il tient un
`main_file` et un `tran_file`, et lit les balises **du format du document** quand il met en italique.

`sourceFile()` est lu à vingt-neuf endroits. **Douze sont des opérations du noyau** qui y cherchent le
dialecte de balises — `italics_command` (2), `dialogue_dashes_command` (2), `clipboard` (2),
`search` (3), et `letter_case_command`, `hearing_impaired_removal` et `duration_adjustment` (1 chacune) —
et lisent aujourd'hui toutes le principal. C'est vrai tant qu'il n'y a qu'un document, et faux le jour où
une traduction est en SSA : **deux d'entre elles reçoivent déjà un `Document`** et lisent pourtant le
format du principal.

## Décision

**`Project` retient un second `SourceFile`, celui de la traduction, optionnel.** Un accesseur prend
un `Document` : `sourceFile(Document)`, et son pendant pour écrire.

**Sans fichier de traduction, la traduction suit le format du principal** : une traduction tapée à la
main dans une colonne vide s'écrira comme lui, jusqu'à ce qu'on lui en choisisse un.

**Onze des douze sites du noyau lisent le format du document qu'on leur donne**, non plus celui du
principal. **Le douzième, l'ajustement des durées, reste au principal, à dessein** : sa vitesse de lecture
se calcule sur `mainText`. Les autres — la fenêtre, l'ouverture, l'enregistrement, la ligne de commande —
restent tels quels tant qu'aucune issue ne les fait toucher au second document.

## Alternatives écartées

- **Un fichier de traduction que le projet ignore, tenu par la fenêtre.** C'est ce que
  l'[ADR 0018](0018-vocabulaire-des-formats-dans-le-modele.md) a déjà refusé pour le format : la
  fenêtre tiendrait à côté de sa `Session` un champ dont `SourceFile` est précisément fait, et
  la question reviendrait au deuxième consommateur — la ligne de commande de la phase 13, qui n'a
  pas de fenêtre.
- **Une traduction lue, puis oubliée comme source.** Ce serait plus simple, et on ne saurait plus
  l'enregistrer à son chemin, ni dans son format, ni dans son encodage : la promesse d'un
  aller-retour fidèle, seule promesse forte du noyau, ne vaudrait que pour le principal.
- **Un second `Project`, lié au premier.** Une traduction n'a pas de positions à elle, et un second
  projet en aurait : il faudrait les garder d'accord, et **c'est exactement le défaut que « les deux
  textes, une paire de positions » a été écrit pour ne pas avoir**.
- **Un tableau de fichiers indexé par `Document`.** La forme la plus générale, et elle n'a qu'un
  cas d'emploi de plus que la forme choisie : il n'y a que deux documents. Le déclencheur est plus
  bas.

## Conséquences

**Facile** : enregistrer la traduction à son chemin, dans son format ; mettre en italique une traduction
SSA avec `{\i1}` ; savoir, par document, s'il est modifié — l'historique le fait déjà.

**Difficile, et à surveiller** : `sourceFile()` reste lu à dix-sept endroits qui n'en font pas partie,
et **tous supposent le principal**. Ce n'est pas un défaut tant qu'ils sont des sites du principal — la
fenêtre, l'ouverture d'un projet — ; c'en est un le jour où l'un d'eux devient une opération de texte
sans que personne l'ait relu. La relecture de fin de phase le vérifie.

**Ce que la décision coûte.** Les réglages **par sous-titre** d'une traduction — un style ASS, la position
d'un sous-titre WebVTT — **ne sont pas conservés** : un sous-titre porte un seul jeu de `FormatExtras`,
celui du principal, et l'alignement d'une ligne de traduction ne recopie que son texte. Ce qui se perd est
celui d'un fichier qu'on ne relit presque jamais pour sa mise en forme, mais c'est une perte, et elle est
nommée ici plutôt que découverte.

**Ce qu'il coûterait de la défaire** : peu, tant que l'accesseur prend un `Document` — le jour où le
second fichier devient un tableau, seul son corps change.

**Le déclencheur** qui rouvrirait la question est un **troisième document** — un texte de référence, une
seconde langue. Deux documents nommés ne sont plus une forme, à trois ils sont une liste.
