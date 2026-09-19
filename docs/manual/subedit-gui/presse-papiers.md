# Couper, copier et coller des textes

Trois entrées du menu `Edit`, sous `Undo` et `Redo`, et au-dessus de
`Find and Replace…`.

| Entrée | Raccourci | Ce qu'elle fait |
| :----- | :-------- | :-------------- |
| `Cut Texts` | `Ctrl+X` | copie les textes de la sélection, puis les vide |
| `Copy Texts` | `Ctrl+C` | copie les textes de la sélection |
| `Paste Texts` | `Ctrl+V` | écrit les textes copiés à partir de la première ligne sélectionnée |

**Ce sont des textes qui voyagent, et rien d'autre** : ni début, ni fin, ni ligne
entière. Coller ne déplace donc aucune position — le calage de la ligne qui
reçoit un texte reste le sien. C'est ce que fait Gaupol.

## Sur quoi elles portent

**La sélection, et jamais tout le fichier.** Les trois entrées sont éteintes tant
que rien n'est sélectionné. Ailleurs dans la fenêtre, « rien de sélectionné » veut
dire « tout le fichier » ; ici, un `Ctrl+X` malheureux viderait tous les textes
du document, et un collage n'aurait pas de ligne où commencer.

Pendant qu'une cellule est ouverte, `Ctrl+X`, `Ctrl+C` et `Ctrl+V` appartiennent
au champ de saisie : ils coupent, copient et collent des caractères, comme `Del`
y efface un caractère et non un sous-titre. Lancées par le menu, elles valident
d'abord la saisie : `Copy Texts` copie alors le texte tel qu'il vient d'être
tapé. Voir [Éditer une cellule](edition.md).

## Copier

`Copy Texts` copie le texte de chaque ligne sélectionnée, de la première à la
dernière. **Une sélection discontinue garde sa forme** : une ligne laissée hors
de la sélection devient un trou, et le collage ne l'écrasera pas.

**Une ligne sélectionnée dont le texte est vide devient un trou, elle aussi** :
collée, elle laisse la ligne cible telle quelle. Copier un seul texte vide ne
colle donc rien.

Le presse-papiers du système reçoit ces textes **en texte brut, séparés par une
ligne vide** — un trou s'y écrit comme un texte vide, et c'est pourquoi un texte
vide ne peut pas être autre chose qu'un trou : le collage donne le même résultat
que la copie vienne de cette fenêtre ou d'un autre programme. Ce qu'on copie ici se
colle donc dans n'importe quel programme, et ce qu'on copie ailleurs se colle ici.

Copier ne modifie pas le document, et n'entre pas dans l'historique.

## Couper

`Cut Texts` copie, puis **vide les textes** des lignes sélectionnées. Les lignes
restent, avec leurs positions : couper n'est pas supprimer — pour retirer des
lignes, c'est [`Remove Subtitles`](lignes.md#supprimer).

Couper des textes déjà tous vides ne fait rien, et n'entre pas dans
l'historique.

## Coller

`Paste Texts` écrit les textes du presse-papiers **à partir de la première ligne
sélectionnée**, une ligne par texte, vers le bas.

| Ce que contient le presse-papiers | Ce qui se passe |
| :-------------------------------- | :-------------- |
| un texte | il remplace celui de la ligne |
| un trou, ou un texte vide copié | la ligne garde son texte |
| plus de textes qu'il ne reste de lignes | des lignes sont ajoutées à la fin pour les recevoir |

Les lignes ajoutées naissent comme des lignes insérées après la dernière : trois
secondes chacune, à la suite. **La fenêtre le dit**, dans une boîte d'information :

```text
inserted 2 subtitles to fit the clipboard
```

Après le collage, **les lignes écrites sont sélectionnées**.

Un collage qui ne change rien — les mêmes textes aux mêmes lignes — n'entre pas
dans l'historique.

### Les balises d'un autre format sont traduites

Une copie faite dans cette fenêtre **se souvient du format de son document**, et
ce souvenir survit à l'ouverture d'un autre fichier. Coller dans un document d'un
autre format traduit donc les balises, comme `Save As…` le fait :
`{\i1}Bonjour{\i0}` copié d'un Advanced SSA se colle `<i>Bonjour</i>` dans un
SubRip.

**Ce que le format d'arrivée ne sait pas écrire est perdu, et c'est dit**, dans
les mots de `Save As…` :

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Contrairement à `Save As…`, rien n'est demandé avant : le collage s'annule par
`Ctrl+Z`, qui rend les textes tels qu'ils étaient.

**Un texte copié hors de `subedit` n'a pas de format**, et se colle tel quel :
il n'y a rien à traduire depuis nulle part. C'est aussi le cas d'une copie de
cette fenêtre que le presse-papiers du système a remplacée depuis — c'est le
contenu du presse-papiers du système qui fait foi.

Quand les deux messages sont dus, ils tiennent dans une seule boîte, séparés par
un point-virgule.

## Ce que l'action d'annulation en dit

Chaque geste est **une** entrée de l'historique, quel que soit le nombre de
lignes qu'il écrit ou ajoute.

| Opération | Ce que `Undo` lit |
| :-------- | :---------------- |
| couper | `Undo: cutting texts` |
| coller | `Undo: pasting texts` |

Voir [Annuler et rétablir](annulation.md).
