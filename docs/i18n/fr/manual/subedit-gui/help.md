# Le manuel dans la fenêtre

`Aide ▸ Manuel`, ou `F1`, ouvre **ce manuel-ci** dans une fenêtre à lui.

![Le manuel ouvert dans sa fenêtre, palette claire : la barre de navigation, le
titre, et les tableaux rendus.](captures/manuel.png)

![Le même manuel sous la palette sombre.](captures/manuel-sombre.png)

## Ce qu'il ouvre, et pourquoi pas une adresse

**Le manuel installé à côté du programme**, et non une page sur Internet.

Une adresse décrirait la version en cours de développement, pas celle qu'on a
sous la main. Un utilisateur qui lit le manuel d'une version qu'il n'a pas est
moins bien servi qu'un utilisateur sans manuel — c'est pourquoi la fenêtre lit
les fichiers déposés par l'installation, sous
`<prefix>/share/subedit/manual`. Voir [Installation](../subedit-cli/installation.md).

Il s'ouvre sur la [page d'accueil](../index.md), celle qui dit par où commencer.

## Dans quelle langue

**Dans la langue de la fenêtre.** Le manuel s'ouvre dans la langue que parle
l'interface : celle choisie dans `Édition ▸ Préférences…`, ou sinon celle de
l'environnement — voir
[La langue de l'interface](preferences.md#la-langue-de-linterface).

| La fenêtre parle | Le manuel s'ouvre en |
| :--------------- | :------------------- |
| anglais | anglais |
| une langue dont le manuel est installé — le français | cette langue |
| une langue dont le manuel n'est pas installé | anglais |

**Page par page, et non tout ou rien** : une page que la traduction n'a pas
s'ouvre en anglais, et toutes les autres restent dans la langue de la fenêtre.
Un lien mène de l'une à l'autre de la même façon.

Les traductions sont installées à côté du manuel anglais, chacune dans un
répertoire au nom de sa langue : `<prefix>/share/subedit/manual/fr` pour le
français. La langue est celle dans laquelle la fenêtre a démarré : une langue
choisie dans les préférences vaut donc aussi pour le manuel, au lancement
suivant.

## Naviguer

| Ce qu'on fait | Ce qui se passe |
| :------------ | :-------------- |
| cliquer un lien du manuel | la page s'ouvre dans la même fenêtre |
| cliquer un lien qui vise une section | la page s'ouvre, et la fenêtre y descend |
| cliquer un lien qui ne vise qu'une section | la fenêtre y descend, sans changer de page |
| `Back` | revient à la page précédente ; éteint sur la première |
| `Contents` | revient à la page d'accueil ; éteint quand on y est |

La fenêtre **n'est pas modale** : elle reste ouverte pendant qu'on travaille,
ce qui est tout l'intérêt d'avoir le manuel sous la main. Rouvrir l'entrée ne
crée pas une seconde fenêtre, elle ramène celle-ci devant.

## Ce qu'il ne peut pas ouvrir, et il le dit

Le manuel renvoie parfois à la **feuille de route** ou à une **décision
d'architecture**. Ce sont des documents du dépôt, pas du paquet : ils ne sont
pas installés, et la fenêtre l'écrit en toutes lettres plutôt que de laisser le
clic sans effet.

```
../../feuille-de-route.md is not part of the installed manual;
it lives in the project's repository.
```

Ils se lisent sur le dépôt, dont l'adresse est dans `Aide ▸ À propos de subedit`.

## Quand l'entrée est éteinte

**`Aide ▸ Manuel` est éteinte quand il n'y a pas de manuel à côté du
programme.** Deux cas, et aucun n'est une panne :

| Cas | Pourquoi |
| :-- | :------- |
| le binaire est lancé depuis son arbre de construction | rien n'y a été installé, donc rien n'y est à lire |
| l'installation est partielle | le manuel a été retiré, ou n'a jamais été copié |

Survoler l'entrée dit lequel des deux : « Aucun manuel n'est installé à côté de ce
programme ».

Une page manquante alors que le manuel est là — un fichier effacé à la main —
ne referme pas la fenêtre : elle nomme le fichier qu'elle n'a pas pu lire et
garde à l'écran ce qu'elle montrait.

## Où le manuel est cherché

**À côté de l'exécutable**, dans `../share/subedit/manual` — la disposition que
toute installation produit, quel que soit le préfixe. C'est ce qui fait qu'une
installation sous `~/.local` trouve son manuel aussi bien qu'un paquet installé
sous `/usr`.
