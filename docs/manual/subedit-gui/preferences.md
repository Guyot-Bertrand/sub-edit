# Ce que la fenêtre retient

La fenêtre se souvient de trois choses d'une session à l'autre : **où elle
était**, **si elle était agrandie**, et **la largeur de ses colonnes**. Rien de
tout cela ne se règle par un dialogue : on déplace la fenêtre, on tire une
bordure de colonne, et c'est retenu.

**Ce qui n'est pas retenu :** le dernier fichier ouvert. S'en souvenir voudrait
dire rouvrir au lancement un document que personne n'a demandé.

## Le fichier

```
~/.config/subedit/settings.conf
```

Plus exactement `$XDG_CONFIG_HOME/subedit/settings.conf` lorsque cette variable
est posée. Il est écrit **à la fermeture de la fenêtre**, et lu à son ouverture.
Un programme interrompu — tué, ou arrêté par une panne — n'écrit rien, et la
session suivante retrouve ce que la précédente avait laissé.

Le fichier est du texte, et il se modifie à la main. C'est une propriété et non
un effet de bord : c'est ce qui rend un réglage corrigeable quand l'interface
qui le pose est justement celle qui ne va pas.

| Option | Ce qu'elle vaut | Défaut |
| :----- | :-------------- | :----- |
| `window.geometry` | `x,y,largeur,hauteur`, en pixels ; largeur et hauteur non nulles | la fenêtre se dimensionne elle-même |
| `window.maximised` | `true` ou `false` | `false` |
| `table.columns` | quatre largeurs en pixels, séparées par des virgules | la table se dimensionne elle-même |

**Quatre largeurs pour cinq colonnes**, et ce n'est pas un oubli : la dernière,
`Text`, prend ce que les quatre autres laissent. Lui donner une largeur ne ferait
rien.

## Ce qui arrive quand le fichier ne dit pas ce qu'on attend

Une configuration est un confort. **Sa défaillance coûte le confort et rien
d'autre** : la fenêtre s'ouvre dans tous les cas.

| Cas | Ce qui se passe |
| :-- | :-------------- |
| le fichier n'existe pas | tous les défauts, et rien n'est dit — c'est le premier lancement |
| le fichier existe et ne se lit pas | tous les défauts, et la raison est écrite sur la sortie d'erreur |
| une option est absente | son défaut |
| une clé est inconnue | ignorée, sans un mot, et absente du fichier réécrit |
| une valeur ne se lit pas | son défaut est gardé, et l'option est nommée sur la sortie d'erreur |

```console
$ subedit-gui film.srt
subedit-gui: /home/vous/.config/subedit/settings.conf: window.geometry: "plus tard" cannot be read, keeping the default
```

**Le message va sur la sortie d'erreur et nulle part ailleurs.** Une fenêtre
modale au démarrage pour une préférence illisible arrêterait l'utilisateur pour
un défaut qui ne l'empêche de rien, et avant qu'il ait rien demandé.

## Les options commentées

Le fichier réécrit contient **toutes** les options connues, et celles restées à
leur défaut sont écrites en commentaire :

```
#window.geometry = 0,0,1280,800
#window.maximised = false
#table.columns = 60,110,110,110
```

Ce n'est pas une coquetterie. Sans cela, une option jamais touchée resterait
figée à la valeur du jour où elle a été écrite, et **un défaut qu'on améliore
n'atteindrait jamais personne**. Avec cela, changer un défaut change le
comportement de tous ceux qui ne l'ont pas surchargé — ce qui est très
exactement ce qu'un défaut doit faire.

Pour surcharger une option : retirer le `#` et changer la valeur. Pour revenir
au défaut : effacer la ligne, ou la remettre en commentaire.

Les deux premières lignes ci-dessus montrent une **forme** et non une valeur en
vigueur : le défaut de la géométrie et celui des colonnes sont « ce que la
fenêtre choisit », qui ne s'écrit pas en nombres. L'en-tête du fichier le dit
aussi.

## Ce qui viendra

Le thème clair et sombre, et une fréquence d'image par défaut, avec l'entrée
`Preferences…`. Voir la [feuille de route](../../feuille-de-route.md) ; ce
manuel décrit ce qui existe.
