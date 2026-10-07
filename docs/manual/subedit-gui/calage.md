# Caler depuis la vidéo

On regarde le film, on s'arrête sur l'image où une réplique doit commencer, et **cette position devient
le début du sous-titre**. Cinq gestes, au menu **Video**, sous les noms que Gaupol leur donne. Ils
reposent sur ce qui existe déjà : poser une position est **la même commande que saisir la valeur dans la
cellule**, et insérer est **la même que `Edit ▸ Insert Subtitles…`**.

| Commande | Raccourci | Ce qu'elle fait |
| :------- | :-------- | :-------------- |
| `Video ▸ Set Start from Video Position` | `Alt+U` | pose le **début** du sous-titre sélectionné à la position du film |
| `Video ▸ Set End from Video Position` | `Alt+K` | pose sa **fin** à la position du film |
| `Video ▸ Insert Subtitle at Video Position` | `Alt+J` | insère un sous-titre qui **commence** à la position, et le sélectionne |
| `Video ▸ Select Previous from Video Position` | `Alt+Up` | sélectionne le dernier sous-titre qui **commence avant** la position |
| `Video ▸ Select Next from Video Position` | `Alt+Down` | sélectionne le premier qui **commence après** la position |

**Tous sont éteints sans vidéo ouverte**, comme les gestes du lecteur. `Set Start…` et `Set End…`
le sont aussi tant que rien n'est sélectionné : ils agissent sur la **première ligne** de la
sélection.

**Les raccourcis sont ceux de Gaupol, avec `Alt`** : chez lui `U`, `K` et `J` — des lettres seules, que
la fenêtre prendrait avant la cellule en cours d'édition, si bien que taper un `k` dans un texte
poserait une fin. `Select Previous…` et `Select Next…` sont `Ctrl+Y` et `Ctrl+U` chez lui ; `Ctrl+Y`
est un *rétablir* sur certains systèmes, et les flèches avec `Alt` disent *avant* et *après* mieux que
deux lettres. Une cellule en cours d'édition est **validée** avant `Set Start…`, `Set End…` ou
`Insert Subtitle…`, comme avant toute modification.

## Poser un bord

- **Une entrée d'historique par geste** : `Edit ▸ Undo` rend la position d'avant, et rien d'autre.
- **Les mêmes règles que la cellule.** Un début posé après la fin du sous-titre, ou une fin avant son
  début, **n'est pas refusé** : il est posé, et la table marque le sous-titre comme elle marque celui
  dont on a saisi la valeur à la main — voir [Les anomalies](table.md#les-anomalies). Rien n'est corrigé en silence.
- **Une position qui est déjà celle du bord ne fait rien**, et n'ajoute rien à l'historique.
- **La position est toujours le début d'une image** : le lecteur ne dit jamais une position à
  l'intérieur d'une image. Poser un bord, puis y revenir — avec `Seek Selection Start`, par exemple —
  **montre la même image**.

## Insérer à la position

Le sous-titre neuf **commence à la position**, et dure **trois secondes**, ou jusqu'au début du suivant
quand celui-ci vient plus tôt. Son texte est vide. Il prend sa place dans l'ordre du fichier : après
tous ceux qui commencent à cette position ou avant elle. L'insertion est **une entrée d'historique**,
et le sous-titre inséré est sélectionné : on peut le taper tout de suite.

## Sélectionner depuis la position

Les deux suivent l'ordre du fichier, pas la sélection en cours. **Sans voisin de ce côté**, la
sélection va au dernier sous-titre du fichier pour `Select Next…`, au premier pour
`Select Previous…` — le comportement de Gaupol. Ces deux gestes ne déplacent pas le film.

## Image par image

Avancer et reculer **d'une image** — ou de N, voir plus bas —, pour poser un bord **à l'image près**.
Deux gestes, et deux boutons dans la barre de lecture, de part et d'autre de `Play` :

| Commande | Raccourci | Ce qu'elle fait |
| :------- | :-------- | :-------------- |
| `Video ▸ Step Backward` | `Alt+Left` | recule le film du pas d'image |
| `Video ▸ Step Forward` | `Alt+Right` | l'avance du pas d'image |

- **Le pas est un nombre d'images, une au moins** : c'est le réglage `Frame step` des
  [préférences](preferences.md#le-lecteur). Un pas de N images montre l'image **N plus loin**, à 25 images par
  seconde comme à 23,976 ; il ne s'exprime **jamais en millisecondes**.
- **Le film s'arrête à ses bornes** : reculer depuis la première image la laisse, avancer depuis la dernière
  aussi. La lecture est arrêtée après un pas.
- **Les boutons de la barre** — `|◀` et `▶|` — **déclenchent les mêmes actions que le menu** : même pas, éteints
  sans vidéo comme les entrées du menu, et la même infobulle, qui dit le raccourci. **Un doigt qui reste sur un
  bouton, ou sur la touche, enchaîne les pas.**
- **Un pas à la fois.** Une touche tenue répète plus vite qu'un pas ne se fait : la première répétition part tout
  de suite, celles qui arrivent pendant que le pas se fait **ne s'empilent pas** — elles se partagent une place, la
  dernière, qui part ensuite. Lâcher la touche arrête donc l'image là où l'on est, et ne laisse pas le film
  continuer sur sa lancée.

## Décaler un bord d'une image

Pour régler un bord au clavier : **le début ou la fin du sous-titre sélectionné**, d'un pas, plus tôt ou plus tard.

| Commande | Raccourci | Ce qu'elle fait |
| :------- | :-------- | :-------------- |
| `Video ▸ Nudge Start Earlier` | `Alt+Q` | le sous-titre **commence un pas plus tôt** |
| `Video ▸ Nudge Start Later` | `Alt+W` | il **commence un pas plus tard** |
| `Video ▸ Nudge End Earlier` | `Alt+Shift+Q` | il **finit un pas plus tôt** |
| `Video ▸ Nudge End Later` | `Alt+Shift+W` | il **finit un pas plus tard** |

`Q` et `W` sont ceux de Gaupol pour *plus tôt* et *plus tard*, avec `Alt` pour la raison donnée plus haut ; la fin
prend `Alt+Shift`, parce que le `E` de Gaupol est ici la lettre de `Edit`.

- **Une entrée d'historique par décalage**, annulable. Le sous-titre voisin et le chevauchement se disent
  **comme à la saisie** : le bord est posé, et la table marque ce qui ne tient plus — voir
  [Les anomalies](table.md#les-anomalies).
- **Le même pas que le lecteur**, donc une image par défaut. **Ce que vaut une image** est lu, dans cet ordre,
  dans **la fréquence que la vidéo déclare**, celle du **document quand il est compté en images** (MicroDVD), puis
  **la grille que les positions dessinent**. **Sans aucune des trois, le geste refuse et le dit** : choisir une
  fréquence au hasard décalerait tout bord de ce qui n'est pas une image.
- **Il n'exige pas de vidéo**, seulement une sélection : on s'en sert aussi sur un fichier compté en images, sans
  film. Quand un film est ouvert, **la lecture se place sur la nouvelle position**, pour voir l'image que le bord
  vient de prendre.
- Un bord ne passe pas avant l'origine : reculer d'un pas depuis moins d'une image le met à zéro.
