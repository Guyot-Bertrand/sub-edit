# Insérer et supprimer des lignes

Deux entrées du menu `Edit`, sous un séparateur qui les sépare d'`Undo` et de
`Redo` : défaire est ce qu'on fait *à* une édition, insérer et supprimer **sont**
des éditions.

| Entrée | Raccourci | Ce qu'elle fait |
| :----- | :-------- | :-------------- |
| `Insert Subtitles…` | `Inser` | ouvre un dialogue, puis pose des lignes vierges |
| `Remove Subtitles` | `Suppr` | retire la sélection, sans rien demander |

Les deux entrent dans l'historique : `Ctrl+Z` les défait comme le reste.

## Insérer

![Le dialogue d'insertion : le nombre de lignes, et le côté de la sélection où
elles iront.](captures/insertion.png)

| Champ | Ce qu'il vaut | Défaut |
| :---- | :------------ | :----- |
| `How many` | un entier de 1 à 99 999 | `1` |
| `Where` | `Above the selection` ou `Below the selection` | le dernier choix, `Below` au premier lancement |

`OK` pose les lignes, `Annuler` ne pose rien.

### Où elles vont

**Au dernier sélectionné**, et non au premier. Sélectionner les lignes 1 et 3
puis insérer en dessous pose la nouvelle ligne en position 4 — pas en
position 2. C'est ce que Gaupol fait depuis vingt ans, et c'est ce que la main
attend après avoir balayé du haut vers le bas.

`Above the selection` la pose juste avant cette même ligne.

### Combien de temps elles durent

Une ligne vierge n'a pas de texte, mais elle a des positions. Elles se déduisent
de la place disponible :

| Situation | Début | Durée |
| :-------- | :---- | :---- |
| entre deux sous-titres | la fin de celui d'avant | la place jusqu'au suivant, partagée en parts égales |
| après le dernier | la fin du dernier | trois secondes chacune |
| en tête d'un fichier | l'origine | la place jusqu'au premier, partagée |
| dans un fichier vide | l'origine | trois secondes chacune |

Quand le fichier se chevauche à l'endroit choisi, il n'y a **pas de place à
partager** : les lignes reçoivent une durée nulle, ce que la table montre, plutôt
qu'une durée inventée qui passerait par-dessus la voisine.

### Le document vide

**Aucune sélection n'est exigée**, et c'est la seule façon de commencer un
fichier neuf : les lignes vont à l'index zéro. Le choix du côté est alors éteint
dans le dialogue — il n'y a pas de sélection à situer.

Dès que le document porte une ligne, l'entrée `Insert Subtitles…` **s'éteint tant
que rien n'est sélectionné** : sans sélection, l'index serait deviné.

### Après coup

Les lignes posées sont **sélectionnées**, et la table s'y rend. Appuyer sur
`Inser` une seconde fois insère donc à la suite, sans avoir à cliquer entre les
deux.

### Le côté est retenu

`Above` ou `Below` est retenu **d'une insertion à la suivante, et d'une session à
la suivante** : c'est l'option `edit.insert-placement` du
[fichier de préférences](preferences.md#le-fichier). On n'insère pas une fois,
on insère dix lignes de suite, toujours du même côté.

## Supprimer

`Remove Subtitles`, ou `Suppr`, retire les lignes sélectionnées — y compris une
sélection discontinue, chaque ligne revenant à sa place si l'on annule.

**Aucune confirmation n'est demandée**, et ce n'est pas une négligence :
l'opération se défait par `Ctrl+Z`. Une modale devant un geste annulable
coûterait un clic à chaque fois pour épargner un `Ctrl+Z` de temps en temps.

**L'entrée est éteinte quand rien n'est sélectionné.** Ailleurs dans la fenêtre,
« rien de sélectionné » veut dire « tout le fichier » — c'est ce qui rend un
décalage utilisable sans sélectionner quatre mille lignes. Ici, cette lecture
viderait un document d'un `Suppr` malheureux, donc elle n'est pas offerte.

Après le retrait, **la ligne qui a pris la place de la première retirée est
sélectionnée** — ou la dernière restante, si le retrait a emporté la fin du
fichier. Appuyer sur `Suppr` plusieurs fois de suite retire donc les lignes une
à une.

Un fichier entièrement vidé laisse la fenêtre utilisable : `Insert Subtitles…`
se rallume, puisqu'un document vide s'insère sans sélection.

## Ce que l'action d'annulation en dit

| Opération | Ce que `Undo` lit |
| :-------- | :---------------- |
| une insertion | `Undo: inserting` |
| une suppression | `Undo: removing` |

Voir [Annuler et rétablir](annulation.md).

## Les raccourcis pendant qu'une cellule est ouverte

`Inser` et `Suppr` appartiennent au champ de saisie tant qu'un éditeur de cellule
est ouvert : `Suppr` y efface un caractère, et non un sous-titre. Fermer
l'éditeur — `Entrée` ou `Échap` — leur rend leur sens.
