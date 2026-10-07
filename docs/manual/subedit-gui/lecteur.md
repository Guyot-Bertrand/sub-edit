# Le lecteur

La vidéo associée se regarde **dans la fenêtre**, avec la réplique courante
dessinée par-dessus. C'est ce qui permet de vérifier sur le film le résultat
d'un décalage, d'une transformation ou d'une conversion de fréquence.

Voir [La vidéo associée](video.md) pour la façon d'associer un film ; cette
page décrit ce qui se passe une fois qu'il l'est.

## La vue vidéo

| Situation | Ce que montre la fenêtre |
| :-------- | :----------------------- |
| une vidéo est associée et s'ouvre | l'image occupe le haut, la table dessous |
| aucune vidéo associée | pas d'image, la table occupe toute la fenêtre |
| la vidéo ne s'ouvre pas | pas d'image, un message, et la fenêtre reste utilisable |

![La fenêtre avec une vidéo associée : l'image occupe le haut, la réplique
courante est dessinée dessus, la table est dessous.](captures/lecteur.png)

![La même fenêtre sous la palette sombre.](captures/lecteur-sombre.png)

La séparation entre l'image et la table **se déplace à la souris**. L'image ne
descend pas sous 180 pixels de haut. Agrandir la fenêtre donne la place gagnée
à la table, pas à l'image : ce dont on manque en éditant, ce sont des lignes.

Le son est celui de la vidéo, tel que le système le règle. La fenêtre n'a pas de
commande de volume.

## Jouer et arrêter

| Commande | Raccourci | Ce qu'elle fait |
| :------- | :-------- | :-------------- |
| `Video ▸ Play / Pause` | `Ctrl+P` | joue si la lecture est arrêtée, l'arrête sinon |

La commande est **inactive tant qu'aucune vidéo n'est ouverte**. Une vidéo qui
vient d'être associée est ouverte **arrêtée** : ouvrir un film n'est pas le
regarder.

`Ctrl+P` là où Gaupol a un simple `P` : un raccourci d'une seule lettre à
l'échelle de la fenêtre est pris avant que le widget qui a le focus le voie,
donc un `P` serait avalé sur le chemin d'un éditeur de cellule. Rien ne
s'imprime ici, la combinaison est libre.

## Sélection et lecture

Les deux vont dans les deux sens.

| Ce qu'on fait | Ce qui se passe |
| :------------ | :-------------- |
| sélectionner une ligne | la lecture se place **au début de ce sous-titre** |
| étendre la sélection vers le bas | rien de plus : c'est la première ligne de la sélection qui compte |
| la lecture avance | la **ligne courante** suit le sous-titre à l'écran, **teintée**, et la table défile pour la garder en vue |

La ligne courante et la sélection sont deux choses distinctes. La lecture
déplace la première et **ne touche jamais la seconde** : la sélection est ce à
quoi une opération s'applique, et un film qui joue dans un coin n'a pas à
réécrire la cible de l'utilisateur ligne après ligne.

La teinte suit cette même règle : c'est un repère, pas une sélection. **Une
anomalie l'emporte sur elle** — une ligne qui porte les deux garde la couleur de
son défaut, un défaut étant là pour être réparé quand une ligne montrée l'est le
temps d'une réplique. Voir [La table](table.md).

**Une édition en cours n'est pas interrompue.** Tant qu'un éditeur de cellule
est ouvert, la ligne courante reste où elle est ; la réplique dessinée sur
l'image, elle, continue de suivre, puisqu'elle ne dérange personne.

## La réplique dessinée

Ce qui s'affiche sur l'image vient du **document ouvert**, jamais d'un fichier :
**ce qu'on voit sur l'image est ce qu'on vient de taper**, sans passage par le
disque. Le texte apparaît centré, au bas de l'image, et disparaît entre deux
sous-titres.

**Avec une traduction, la réplique est celle du texte visé** : la traduction quand
la cellule courante est dans la colonne `Translation`, le texte principal
partout ailleurs. C'est la règle de toutes les opérations de texte — voir [Le
texte que les opérations visent](table.md#le-texte-que-les-opérations-visent) —
et **il n'y a pas de réglage** : passer d'une colonne à l'autre change ce que
l'image montre, au plus une dixième de seconde plus tard. Sans traduction, ou
avec la colonne retirée, rien ne change : c'est le texte principal.

Un sous-titre que la traduction n'a pas encore rejoint ne montre **rien** tant
que la colonne courante est la traduction : ce n'est pas le texte principal qui
prend sa place.

**Les balises du format sont comprises, jamais dessinées telles quelles.** Le modèle porte le
texte comme le fichier l'écrit, et c'est le pivot de balises — celui de la conversion de
format — qui le lit avant de le donner à l'image :

| Dans le fichier | Sur l'image |
| :-------------- | :---------- |
| gras, italique, souligné | appliqués : `<i>non</i>` s'affiche *non*, sans ses balises |
| couleur | appliquée |
| police, taille | retirées, le texte reste |
| position, alignement, locuteur, tout ce qui n'a pas d'équivalent à l'écran | retirés, le texte reste |
| `{` et `}` dans le texte | dessinés : « {rires} » s'affiche « {rires} » |

Un fichier Advanced SSA est lu de la même façon : ses `{\i1}` s'appliquent, et son
`{\an8}` est retiré — la réplique reste au bas de l'image. Chaque texte est lu **dans le format
de son propre fichier**, la traduction comme le texte principal.

**Le fichier de sous-titres voisin n'est pas chargé** par le lecteur, même s'il porte le nom du
film. Il serait celui qu'on est en train d'éditer, et l'image montrerait alors l'état du disque
pendant que la table montre autre chose.

## Prévisualiser un changement

Il n'y a **rien de particulier à faire** : on applique l'opération, on regarde
le film, et on choisit `Undo: shifting` — ou l'opération concernée — si le
résultat ne convient pas. Voir [Annuler et rétablir](annulation.md).

Aucun mode de prévisualisation séparé n'existe, et c'est délibéré : ce qu'on
regarde est le document tel qu'il est, pas un état provisoire invisible.

## Quand l'image n'apparaît pas

Deux messages possibles, tous deux préfixés du chemin du fichier :

| Message | Ce qui s'est passé |
| :------ | :----------------- |
| `<fichier>: <raison>` | le lecteur a refusé le fichier — format inconnu, fichier absent, répertoire |
| `<fichier>: no video player is available` | aucun lecteur n'a pu être construit |

Dans les deux cas, **la vidéo reste associée** — il faut bien voir de quel
fichier il s'agit pour en choisir un autre — et **la fenêtre reste utilisable** :
le document est ouvert, les opérations fonctionnent, l'enregistrement aussi.

**L'image est dessinée dans la fenêtre elle-même** : `libmpv` remplit un tampon à la
taille de la vue, en pixels réels, et la fenêtre le peint. Rien n'en dépend de la
plateforme graphique — la même image sous X11, sous Wayland et sans écran — et elle
suit le redimensionnement : le rapport d'aspect du film est conservé, le reste de la
vue est noir.

Le second message n'apparaît donc que si la bibliothèque `libmpv` refuse de démarrer,
alors qu'elle est requise à la compilation.

Le rendu passe par le processeur : le décodage matériel sans copie n'est pas
disponible. Sur un éditeur de sous-titres, une image coûte quelques millisecondes, et
la lecture tient le temps réel.

## Ce qui n'y est pas

**Le pilotage se limite à jouer et arrêter.** Ce qui n'existe pas, et qu'il est
inutile de chercher :

- barre de position, saut avant et arrière ;
- saut au sous-titre précédent ou suivant, au début ou à la fin de la
  sélection ;
- jouer la seule sélection ;
- réglage du volume ;
- avance image par image, poser un repère depuis la position courante ;
- incrustation du timecode, choix de la piste audio ;
- forme d'onde.

Ce manuel décrit ce qui existe : ce qui viendra, et dans quel ordre, est dans la
[feuille de route](../../feuille-de-route.md).
