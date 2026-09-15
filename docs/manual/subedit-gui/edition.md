# Éditer une cellule

Quatre colonnes s'éditent en place : `Start`, `End`, `Duration` et `Text`. `#`
ne s'édite pas — le numéro est le rang de la ligne, pas une donnée du fichier.

**Ouvrir l'éditeur d'une cellule :** double-cliquer dessus, ou la sélectionner
et appuyer sur `F2`.

![Le texte du troisième sous-titre ouvert en édition, palette claire : le champ
est multiligne et montre les deux lignes telles qu'elles seront
écrites.](captures/edition.png)

![La même cellule ouverte sous la palette sombre.](captures/edition-sombre.png)

## Le texte

L'éditeur est un champ **multiligne**, et il s'ouvre à la hauteur du sous-titre :
un sous-titre de deux lignes s'y saisit tel qu'il sera écrit, ses deux lignes
lisibles et sans ascenseur. Un `Maj+Entrée` qui en ajoute une agrandit
l'éditeur ; la ligne de la table s'ajuste à la validation.

| Touche | Effet |
| :----- | :---- |
| `Entrée` | valide et referme l'éditeur |
| `Maj+Entrée` | insère un saut de ligne dans le texte |
| `Échap` | referme l'éditeur ; la cellule garde ce qu'elle avait |
| `Tab` | valide et passe à la cellule suivante |

Cliquer ailleurs valide aussi : la perte du focus vaut `Entrée`.

### Les balises sont celles du fichier

Le texte montré et saisi est **celui du fichier, balises comprises** : un
`<i>…</i>` d'un SubRip s'écrit ainsi, et un `{\i1}…{\i0}` d'un Advanced SSA
aussi. Rien n'est traduit à l'affichage — le tableau de
[`Italic`](operations.md#italic) dit ce que chaque format écrit.

Pour l'italique, il n'y a donc rien à connaître : `Ctrl+I` pose la bonne balise
pour le document ouvert, et un format qui n'en porte aucune — TMPlayer, LRC —
éteint l'entrée plutôt que de laisser chercher. Le gras et le souligné se
tapent, comme dans Gaupol.

**Et les balises ne suivent pas les opérations de texte** : mettre un sous-titre
en minuscules ne transforme pas son `<I>` en `<i>`, et un tiret de dialogue se
pose devant le texte et non devant une balise ouvrante. Voir
[`Case` et `Dialogue`](operations.md#case-et-dialogue).

## Le début et la fin

L'éditeur est un champ d'une ligne, **contraint à la forme d'un horodatage** :
il refuse à la frappe ce qui ne pourrait jamais en être un.

Les formes acceptées sont celles que la lecture d'un fichier accepte déjà :

| Élément | Ce qui passe |
| :------ | :----------- |
| champs | `MM:SS` ou `HH:MM:SS`, un ou deux chiffres chacun |
| décimales | une à trois, après une virgule ou un point, ou aucune |
| signe | un `-` en tête, pour une position avant le début de la vidéo |
| bornes | minutes et secondes inférieures à 60 |

`00:01:02,500`, `1:02.5` et `-0:01,000` sont donc trois saisies valides.

**Une saisie que la lecture refuse laisse la cellule inchangée.** C'est le cas
de `00:70:00,000`, dont la forme est bonne mais dont les minutes sortent des
bornes : l'éditeur se referme et rien n'a bougé. Aucune position n'est inventée
— se tromper de position est silencieux, rien à l'écran ne distingue un
sous-titre mal calé d'un sous-titre bien calé.

Modifier un début ou une fin met la `Duration` à jour aussitôt.

## La durée

**Saisir une durée déplace la fin, et jamais le début** : la fin devient
`Start + Duration`. Déplacer le début déplacerait le sous-titre entier, ce qui
est le travail de [`Shift Positions…`](operations.md), et la colonne `Start` se
saisit déjà.

La durée se tape **sous la forme d'un horodatage**, celle que la colonne
affiche, et les formes de la table ci-dessus valent — à une différence près :

| Élément | Début, fin | Durée |
| :------ | :--------- | :---- |
| forme | celle d'un horodatage | la même |
| signe `-` en tête | accepté | **refusé à la frappe** |

`0:02.5` et `00:00:02,500` sont donc deux saisies valides d'une durée de deux
secondes et demie ; `-0:01,000` ne se tape pas. Une durée nulle est acceptée :
le sous-titre disparaît alors à l'instant où il apparaît.

**Une fin qui passe par-dessus le sous-titre suivant est permise.** Le
chevauchement est teinté dans la table, avec son infobulle — voir
[La table](table.md#les-anomalies) — un avis, pas un refus : c'est peut-être ce
qu'on voulait. Le panneau de diagnostics, lui, ne parle que de la lecture du
fichier et ne bouge pas.

**Une fin saisie au-delà de la fin du film n'est pas signalée**, à la différence
d'une opération du menu `Tools` qui l'y pousserait — voir
[Les opérations](operations.md#au-delà-de-la-fin-du-film). Une cellule montre
déjà la valeur qu'on vient de taper.

Une saisie illisible laisse la cellule inchangée, comme pour une position, et
une durée identique à celle affichée n'entre pas dans l'historique.

## Une validation qui ne change rien ne fait rien

Ouvrir une cellule, ne rien taper, appuyer sur `Entrée` : il ne se passe rien.
Aucune modification n'est enregistrée, et le fichier reste réputé identique à
celui du disque.

La comparaison porte sur la **position**, pas sur la chaîne : saisir `0:01.0`
dans une cellule qui affiche `00:00:01,000` désigne le même instant, et ne
change donc rien non plus.

## Ce qu'une cellule éditée devient

Elle **s'annule** — une cellule, une entrée. Voir
[Annuler et rétablir](annulation.md).

Elle **s'enregistre**, et fermer la fenêtre sans l'avoir fait demande
confirmation. Voir [Ouvrir et enregistrer](fichiers.md).
