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

## Ce qui n'est pas là

Avancer ou reculer **image par image**, et décaler un bord d'une image, ne sont pas encore dans la
fenêtre.
