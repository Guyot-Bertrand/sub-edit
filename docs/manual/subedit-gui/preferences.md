# Les préférences

La fenêtre se souvient de ce qu'on a posé à la souris — **où elle était**, **si
elle était agrandie**, **la largeur de ses colonnes**, **la position de la
poignée** entre le film et la table — et de **là où l'on travaille**, pour que
`Open…` s'ouvre au bon endroit. Rien de tout cela ne se règle par un dialogue :
on déplace, on tire, et c'est retenu.

**Une seule préférence a un dialogue à elle**, `Edit ▸ Preferences…`, et c'est le
critère : une préférence qui a déjà un geste n'a pas besoin d'un champ. Le
thème n'en a aucun.

**Une autre se règle là où elle sert** : le côté où `Insert Subtitles…` pose ses
lignes est un champ de ce dialogue-là, et le dernier choix est retenu. Voir
[Insérer et supprimer des lignes](lignes.md#le-côté-est-retenu).

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
| `window.table-share` | la part de la hauteur donnée à la table, en pour cent, de 1 à 99 | la fenêtre partage comme elle l'a toujours fait |
| `file.directory` | un chemin **absolu** de répertoire | aucun : `Open…` s'ouvre là où le programme a été lancé |
| `general.theme` | `system`, `light` ou `dark` | `system` |
| `edit.insert-placement` | `above` ou `below` | `below` |
| `file.write-encoding` | le nom d'un encodage qu'ICU sait écrire, et dont le convertisseur n'écrit pas sa propre marque | aucun |

**`file.write-encoding` retient le dernier encodage choisi dans `Save As…`, et
ne s'impose jamais à un fichier.** Un document ouvert porte son propre encodage,
et c'est celui-là que la boîte propose : le réécrire dans un autre parce qu'un
réglage vieux de trois semaines le dit serait perdre ce que la lecture a gardé.
Ce dont ce réglage se souvient sert au document qui n'a pas de fichier — celui
qu'on vient de créer.

**Une part et non des hauteurs pour la poignée**, et c'est ce qui la rend
utile : trois hauteurs en pixels ne veulent plus rien dire dès que la fenêtre
s'ouvre à une autre taille, et une fenêtre s'ouvre souvent à une autre taille.
La part se rejoue partout. Elle est **ramenée à ce que la fenêtre accepte** si
elle est trop petite — la bande du film a une hauteur minimale.

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

## Le répertoire retenu

`Open…` s'ouvre sur le répertoire du **dernier fichier ouvert ou enregistré**,
et non sur celui d'une boîte qu'on a annulée : ce qui compte est là où l'on
travaille, pas là où l'on a regardé.

**Le dernier fichier, lui, n'est pas retenu.** S'en souvenir voudrait dire
rouvrir au lancement un document que personne n'a demandé. Retenir un répertoire
*pointe* une boîte de dialogue ; retenir un fichier en ouvrirait un.

## Le thème

`Edit ▸ Preferences…` ouvre la seule préférence qui n'a pas de geste.

| Valeur | Ce qu'elle fait |
| :----- | :-------------- |
| `System` | **rien** — les couleurs restent celles du bureau |
| `Light` | pose une palette claire |
| `Dark` | pose une palette sombre |

Le thème choisi s'applique **immédiatement**, sans redémarrer.

**« System » ne fait rien, et ce n'est pas un manque.** La version de Qt sur
laquelle subedit est bâti n'a aucun moyen de demander au bureau s'il se veut
clair ou sombre, ni d'être prévenue quand il change d'avis. Plutôt que
d'inventer une lecture qu'on ne sait pas faire, l'outil livre les deux thèmes
qu'on peut demander et laisse la boîte à outils décider pour le troisième —
ce que fait Gaupol sous les mêmes conditions.

Sous un bureau dont le thème est déjà sombre, `System` donne donc une fenêtre
sombre ; sous un bureau clair, une fenêtre claire. `Light` et `Dark` passent
outre, quel que soit le bureau.

**Les teintes d'anomalie restent lisibles dans les deux palettes** — voir
[La table](table.md#les-anomalies). Ce n'est pas une supposition : le contraste
du texte sur chacune des teintes est vérifié par un test, sur les deux fonds.

## Ce qui ne se règle pas

**Deux réglages seulement se choisissent dans un dialogue** — le thème, ici, et
le côté d'une insertion, dans le dialogue d'insertion. Il n'y en a pas de
troisième caché ailleurs : rien ne se règle pour le panneau de diagnostics, le
volume du lecteur, ni le format d'enregistrement par défaut — celui-ci vient du
fichier ouvert.

Et **aucun nombre qui appartient à un document** : une durée de décalage, deux
repères de transformation sont vrais d'un fichier et faux du suivant. Les
dialogues les redemandent à chaque fois, délibérément.

Ce manuel décrit ce qui existe : ce qui viendra est dans la
[feuille de route](../../feuille-de-route.md).
