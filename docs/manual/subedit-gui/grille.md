# La grille d'images

Un fichier de sous-titres **ne déclare pas** la fréquence d'image contre
laquelle ses positions ont été calculées : SubRip n'a pas d'en-tête, celui de
WebVTT est du texte libre. La fenêtre la **déduit** des positions elles-mêmes,
dès l'ouverture, et la montre à deux endroits qui ne répondent pas à la même
question.

| Surface | Ce qu'elle donne |
| :------ | :--------------- |
| la barre d'état | **la réponse**, en permanence et sans qu'on la demande |
| `Tools ▸ Frame Rate Analysis…` | **le raisonnement**, quand on veut le voir |

## Ce que la barre d'état montre

À droite, à côté de ce qu'elle dit déjà de la [vidéo associée](video.md#ce-que-la-barre-détat-montre) :

| Situation | Ce qui est écrit |
| :-------- | :--------------- |
| les positions sont sur une grille | `Grid: 24 fps` |
| une partie d'entre elles seulement | `Grid: 29.97 fps (partial)` |
| aucune grille connue ne convient | `No grid` |

**`No grid` ne nomme aucune fréquence, et c'est voulu.** L'ensemble des
candidates est clos — les huit que le dialogue de conversion propose — et
rapporter la moins fausse d'entre elles serait donner une mauvaise réponse là où
« je ne sais pas » est la bonne.

**`No grid` a deux causes, et ce ne sont pas la même réponse.** Ou bien aucune
grille ne convient, ou bien le fichier compte trop peu de sous-titres pour qu'on
puisse en dire quoi que ce soit — deux débuts paraissent toujours parfaitement
alignés, et ça ne prouve rien. La barre d'état dit la même chose des deux ;
**l'analyse les distingue**, et donne dans le premier cas de combien la
meilleure candidate échoue.

La ligne **se remet à jour après chaque opération**. Aligner un fichier sur une
autre cadence change la grille, et la barre d'état ne doit pas continuer à
annoncer l'ancienne.

## Ce que l'analyse montre

`Tools ▸ Frame Rate Analysis…` **ne modifie rien.** Elle ouvre sur un résumé et
le classement des huit candidates avec leur score.

Le résumé dit la réponse, puis ce qui la nuance — chaque phrase n'apparaissant
que si elle a quelque chose à dire :

| Ce qui peut s'y lire | Quand |
| :------------------- | :---- |
| sur combien de débuts, et sur quelle étendue | toujours |
| de combien le fichier est décalé de sa grille | ses positions y sont à une constante près |
| qu'une autre cadence convient tout aussi bien | elle est un multiple entier de celle retenue |
| que l'étendue ne permet pas de départager deux cadences | le fichier est trop court pour les séparer |
| combien de débuts quittent la grille, **et en combien de suites** | le fichier est partiel |

**Le nombre de suites est ce qui distingue deux histoires** que le seul
pourcentage confond. Beaucoup de suites d'un seul début, ce sont des positions
corrigées à la main, une par une. Quelques longues suites, c'est une section
recalée, ou un fichier assemblé à partir de deux autres.

**Le classement complet est montré plutôt qu'une seule réponse.** Un fichier qui
convient à 25 à cent pour cent et à 50 à cent pour cent dit quelque chose qu'un
verdict seul cacherait.

## Ce que la table ne montre pas

**Aucune ligne de la table n'est marquée à cause de la grille**, jamais.

Dès qu'on corrige une position à la main, elle cesse d'être alignée. Un
marquage automatique signalerait donc le travail de l'utilisateur comme une
anomalie, sur un fichier qu'il est justement en train de corriger. La déduction
parle du **document**, et l'analyse est la seule exception — parce qu'on l'a
ouverte pour ça.

Les marques de la table, elles, portent sur ce qui ne tient pas debout : une fin
avant son début, un chevauchement, un ordre rompu. Voir [La table](table.md).
