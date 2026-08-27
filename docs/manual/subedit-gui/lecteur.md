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

La séparation entre l'image et la table **se déplace à la souris**. L'image ne
descend pas sous 180 pixels de haut. Agrandir la fenêtre donne la place gagnée
à la table, pas à l'image : ce dont on manque en éditant, ce sont des lignes.

Le son est celui de la vidéo, tel que le système le règle. Il n'y a pas encore
de commande de volume dans la fenêtre.

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
| la lecture avance | la **ligne courante** suit le sous-titre à l'écran |

La ligne courante et la sélection sont deux choses distinctes. La lecture
déplace la première et **ne touche jamais la seconde** : la sélection est ce à
quoi une opération s'applique, et un film qui joue dans un coin n'a pas à
réécrire la cible de l'utilisateur ligne après ligne.

**Une édition en cours n'est pas interrompue.** Tant qu'un éditeur de cellule
est ouvert, la ligne courante reste où elle est ; la réplique dessinée sur
l'image, elle, continue de suivre, puisqu'elle ne dérange personne.

## La réplique dessinée

Ce qui s'affiche sur l'image vient du **document ouvert**, jamais d'un fichier :
**ce qu'on voit sur l'image est ce qu'on vient de taper**, sans passage par le
disque. Le texte apparaît centré, au bas de l'image, et disparaît entre deux
sous-titres.

Deux conséquences à connaître :

- **les balises du format s'affichent telles quelles** — un `<i>` est dessiné
  comme un `<i>`. Le modèle porte le texte tel que le fichier l'écrit, et
  comprendre une balise assez bien pour la retirer viendra avec les formats
  riches ;
- **le fichier de sous-titres voisin n'est pas chargé** par le lecteur, même
  s'il porte le nom du film. Il serait celui qu'on est en train d'éditer, et
  l'image montrerait alors l'état du disque pendant que la table montre autre
  chose.

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

**Le lecteur intégré s'appuie sur l'adoption d'une fenêtre native, qui est un
mécanisme X11.** Sous une session Wayland, la fenêtre que Qt fournit par défaut
n'en est pas une. `subedit-gui` s'en occupe : il demande la plateforme `xcb`
avant de construire son application, ce qui le fait passer par XWayland, et le
film s'affiche dans la fenêtre.

Il ne le fait **que si personne n'a choisi**. Un `QT_QPA_PLATFORM` déjà posé est
respecté tel quel :

```console
$ QT_QPA_PLATFORM=wayland subedit-gui film.fr.srt   # pas d'image, et c'est voulu
```

Restent deux cas où le second message apparaît, et ils sont rares : une session
Wayland **sans serveur X du tout**, où il n'y a rien de mieux à demander ; et une
bibliothèque `libmpv` qui refuse de démarrer, alors qu'elle est requise à la
compilation.

## Ce qui n'y est pas

**Le pilotage se limite à jouer et arrêter.** Tout le reste — et c'est la
majeure partie d'une barre de lecteur — vient avec le calage fin :

| Ce qui manque | Où c'est prévu |
| :------------ | :------------- |
| barre de position, saut avant et arrière | calage fin |
| saut au sous-titre précédent ou suivant, au début ou à la fin de la sélection | calage fin |
| jouer la seule sélection | calage fin |
| réglage du volume | calage fin |
| avance image par image, poser un repère depuis la position courante | calage fin |
| incrustation du timecode, choix de la piste audio | calage fin |
| forme d'onde | rien ne la demande |

Voir la [feuille de route](../../feuille-de-route.md).

## La ligne que le film montre

Pendant la lecture, **la ligne du sous-titre affiché est teintée** dans la
table, et la table défile pour la garder en vue.

**Teintée, et non sélectionnée**, et la distinction compte : la sélection est ce
sur quoi une opération s'applique, et un film qui tourne en fond n'a pas à
réécrire la cible de l'utilisateur ligne après ligne. Choisir une ligne reste un
geste de l'utilisateur ; la teinte n'est qu'un repère.

**Une anomalie l'emporte sur elle.** Une ligne qui porte les deux garde la
couleur de son défaut : un défaut est là pour être réparé, là où une ligne
montrée l'est le temps d'une réplique. Voir [La table](table.md).
