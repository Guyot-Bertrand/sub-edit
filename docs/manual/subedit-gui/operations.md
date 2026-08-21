# Décaler, transformer, convertir

Le menu **Tools** porte trois opérations. Chacune ouvre un dialogue, et chacune
s'annule d'un `Ctrl+Z`.

## Sur quoi elles portent

**Les lignes sélectionnées, ou tout le fichier si rien ne l'est.** C'est la même
règle pour les trois, et le dialogue le rappelle en toutes lettres :

```
Applies to: 4 subtitles
```

Sélectionner toutes les lignes revient au même que n'en sélectionner aucune.

Les trois actions sont **inactives sur un fichier vide** : il n'y aurait rien à
décaler.

## `Shift Positions…`

Déplace la cible d'une durée, vers l'avant ou vers l'arrière. Les deux bornes de
chaque sous-titre bougent d'autant : aucun ne reste à l'écran plus ou moins
longtemps qu'avant.

La durée s'écrit **comme un horodatage, signe compris** — `00:00:02,500` avance,
`-0:01,250` recule. Les formes acceptées sont celles d'une cellule de position.

| Refus | Pourquoi |
| :---- | :------- |
| une durée illisible | le bouton reste inactif ; rien n'est décalé au hasard |
| un décalage de zéro | il n'y a pas d'opération à enregistrer |
| un sous-titre passerait avant le début de la vidéo | **aucun fichier ne peut porter une telle position** |

Le troisième cas nomme le fautif — « subtitle 1 would start before the origin » —
et c'est celui qu'il faut regarder pour voir de combien on s'est trompé. Rien
n'est appliqué.

## `Transform Positions…`

La correction d'un fichier calé sur un autre montage. On dit où **deux**
sous-titres commencent réellement, et tout le reste suit, proportionnellement.

Le dialogue demande deux fois la même chose : un numéro de sous-titre, et la
position où son début appartient. Le second est pré-rempli sur le dernier
sous-titre du fichier — deux repères éloignés donnent une correction plus sûre
que deux repères voisins.

**Les deux repères atterrissent exactement où on les a demandés.** Ce n'est pas
une approximation : le calcul est fait pour que ce soit vrai.

| Refus | Pourquoi |
| :---- | :------- |
| deux repères sur le même sous-titre | ils ne définissent aucune correction |
| une position illisible | même raison que pour le décalage |

Un numéro hors du fichier ne peut pas être saisi : le champ est borné par le
nombre de sous-titres.

## `Convert Frame Rate…`

Re-cale la cible quand les positions ont été calculées contre une fréquence
d'images et doivent l'être contre une autre.

Les deux listes offrent les huit fréquences normalisées. **Celle d'entrée
s'ouvre sur la fréquence du projet.**

> **C'est un point de départ, pas une réponse.** Un fichier de sous-titres ne
> porte pas sa fréquence — SubRip n'a pas d'en-tête, celui de WebVTT est du
> texte libre — et personne ne peut la deviner à sa place. Se tromper décale
> tout le fichier **sans rien signaler** : c'est le pire mode d'échec de cette
> opération, et la seule protection est de savoir d'où vient le fichier.

Convertir une fréquence en elle-même ne change rien : le bouton reste inactif.

## Ce que les opérations ne font pas encore

Le retrait des mentions pour malentendants, quatrième opération de cette phase,
vient ensuite — voir la [feuille de route](../../feuille-de-route.md).
