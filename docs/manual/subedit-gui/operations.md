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

Les deux listes offrent les huit fréquences normalisées.

> **Se tromper de fréquence d'entrée décale tout le fichier sans rien
> signaler.** C'est le pire mode d'échec de cette opération, et c'est pourquoi
> le dialogue dit d'où viennent les valeurs qu'il propose plutôt que de les
> poser en silence.

### Les deux sources, et ce qu'elles répondent

Le dialogue porte jusqu'à deux lignes de plus, chacune apparaissant quand elle a
quelque chose à dire :

| Ligne | Ce qu'elle dit | Ce qu'elle pré-remplit |
| :---- | :------------- | :--------------------- |
| `The positions say` | la [grille d'images](grille.md) déduite des positions du fichier | `Timed against` |
| `The video declares` | la fréquence que le conteneur annonce, par exemple `24000/1001` | `Should play at` |

**Chaque proposition va au champ auquel elle répond.** La grille dit sur quoi le
fichier a été *écrit* — c'est la liste du haut. Ce que le conteneur annonce est
la cadence à laquelle le film *tourne*, donc celle à laquelle les sous-titres
doivent arriver — c'est la liste du bas.

**Seule une grille nette pré-remplit l'entrée.** Une grille partielle est un
indice que la déduction elle-même qualifie de partiel, et ce champ décide d'une
opération sur le fichier entier. La [barre d'état](grille.md) et l'analyse
portent ce cas-là ; celui-ci non.

### Un désaccord n'est pas une contradiction

Les deux lignes peuvent s'afficher ensemble et annoncer des nombres différents.
**Ce n'est pas une erreur à arbitrer** : le film tourne à une cadence, le
fichier a été écrit sur une autre grille. Un fichier à 24 pour un film à 25 est
exactement le cas que l'alignement — `snap` en ligne de commande — existe pour
traiter.

Rien n'est donc choisi à la place de l'utilisateur : les deux listes restent
modifiables, et c'est délibéré. Une source qui s'impose interdit de croiser.

Les lignes n'apparaissent pas quand il n'y a rien à dire : aucune vidéo
associée, pas de `ffprobe`, ou aucune grille nette. Le dialogue est alors celui
décrit plus haut, et **aucune opération ne se refuse pour autant** — la
fréquence vient de l'utilisateur. Voir
[La vidéo associée](video.md#ffmpeg-nest-pas-requis).

Une fréquence que le film annonce mais qui n'est pas l'une des huit normalisées
est **affichée sans être choisie** : savoir que le film tourne à une cadence
inhabituelle est l'information, et il n'y a rien à quoi la convertir ici.

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

## Au-delà de la fin du film

Quand une vidéo est ouverte, une opération qui **pousse des sous-titres après la
fin du film** le signale, une fois faite :

```
shifting leaves 3 subtitles past the end of the video, by 4.200 s at most
```

La phrase nomme l'opération — `shifting`, `transforming`,
`converting the frame rate` —, combien de sous-titres dépassent, et de combien
va le plus lointain.

| Ce qui est compté | Ce qui ne l'est pas |
| :---------------- | :------------------ |
| ce que l'opération vient de toucher, c'est-à-dire la cible | un sous-titre que personne n'a déplacé |
| un sous-titre qui finit **après** la fin | un sous-titre qui finit exactement avec elle |

**C'est un avertissement, et rien d'autre.** Rien n'est refusé, rien n'est
corrigé : un sous-titre qui tombe après le générique de fin est peut-être
exactement ce qu'on voulait, et un refus qui se trompe coûte plus cher qu'un
avertissement qu'on ignore. L'opération est dans l'historique, et `Undo` la
défait comme n'importe quelle autre.

Le message n'apparaît pas si aucune vidéo n'est ouverte : la durée vient du
lecteur, et sans film il n'y a pas de fin à dépasser. Les trois opérations qui
déplacent des positions sont concernées ; le retrait des mentions, qui n'en
déplace aucune, ne l'est pas.
