# Les opérations

Le menu **Tools** porte quatre opérations. Chacune ouvre un dialogue, et chacune
s'annule d'un `Ctrl+Z`.

## Sur quoi elles portent

**Les lignes sélectionnées, ou tout le fichier si rien ne l'est.** C'est la même
règle pour les quatre, et le dialogue le rappelle en toutes lettres :

```
Applies to: 4 subtitles
```

Sélectionner toutes les lignes revient au même que n'en sélectionner aucune.

Les quatre actions sont **inactives sur un fichier vide** : il n'y aurait rien à
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

## `Remove Hearing-Impaired Mentions…`

Retire les mentions destinées aux spectateurs sourds ou malentendants — les
descriptions de sons entre crochets ou entre parenthèses, les noms de locuteurs.

Le dialogue ne demande rien : l'opération n'a pas de réglage, et il ne sert qu'à
confirmer et à rappeler sur quoi elle porte.

**C'est la seule opération qui fasse disparaître des lignes.** Un sous-titre qui
n'était *que* mention n'a plus de texte une fois nettoyé, et un sous-titre sans
texte n'a pas lieu d'être : il est retiré du fichier. Les autres sont réécrits
sans leur mention.

Le compte rendu dit les deux :

```
1 subtitle cleaned, 1 removed
```

**Quand rien ne mord, rien ne se passe** — et le dialogue le dit : « no mention
to remove ». Aucune entrée n'entre dans l'historique, car une opération qui ne
change rien n'est pas une opération à annuler.

**Annuler remet tout en place**, les sous-titres retirés comme les textes
réécrits, chacun avec le texte qu'il avait. Un sous-titre que la règle a vidé
n'est jamais réécrit avant d'être retiré, précisément pour qu'il revienne
entier.

> **La sélection est perdue à cette occasion**, et à celle-là seulement : retirer
> des lignes change la structure de la table, qui se reconstruit. Les trois
> autres opérations la conservent.

Une référence purement numérique — « Voir [1] la note » — n'est pas une mention
et reste telle quelle.
