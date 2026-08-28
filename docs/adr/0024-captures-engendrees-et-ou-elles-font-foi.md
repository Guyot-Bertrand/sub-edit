# 0024 — Des captures engendrées, et l'environnement où elles font foi

Statut : acceptée — 2026-08-28
Décidée en écrivant l'issue #199.

## Contexte

Le manuel de `subedit-cli` porte des blocs `console` que `generate-manual.sh`
réécrit **en exécutant le binaire**. C'est ce qui les empêche de mentir, et
c'est pour cela qu'aucun exemple d'appel n'est recopié à la main.

Le manuel d'interface n'avait pas d'équivalent. #116 puis #161 l'ont écrit
chaque fois, avec la même raison : une fenêtre ne s'écrit pas dans un bloc
`console`. #161 avait ajouté l'avertissement — « c'est la deuxième section sans
filet, et il n'y en a pas de troisième gratuite ». Il y en a eu neuf.

Ce qui a changé depuis, et qui rend la question posable : depuis #191, **les
tests montrent les fenêtres qu'ils pilotent**. Une fenêtre montrée a une vraie
géométrie et une disposition qui a réellement tourné — donc quelque chose à
photographier. Avant, il n'y avait rien.

Une capture soulève alors une question que le bloc `console` ne soulevait pas.
La sortie de `subedit-cli --help` est la même partout ; l'image d'une fenêtre
ne l'est pas. Deux machines n'ont pas les mêmes polices, pas le même style de
widgets, pas le même thème. **Une référence engendrée ici se ferait promouvoir
là-bas, et réciproquement** : la référence oscillerait au gré de qui a lancé la
commande, et le diff du dépôt cesserait de vouloir dire quelque chose.

## Décision

**Les captures sont engendrées par le même geste que les blocs `console` —
`make manual` — et ne font foi que sous les réglages que le programme de
capture pose lui-même.**

Ces réglages sont quatre, et le programme les pose tous :

| Réglage | Valeur | Pourquoi il ne peut pas être laissé à la machine |
| :------ | :----- | :----------------------------------------------- |
| plateforme Qt | `offscreen` | il n'y a pas d'écran en intégration continue, et une session graphique apporterait son gestionnaire de fenêtres |
| style | `Fusion` | le style par défaut suit le bureau installé ; Fusion est le même partout |
| police | `DejaVu Sans`, corps 10 | la seule chose de cette liste qu'une machine peut ne pas avoir |
| dimensions | fixées, en pixels | `sizeHint()` dépend de la police, donc de la machine |

**La police est la seule qui puisse manquer, donc c'est la seule qui est
vérifiée : le programme refuse de photographier avec une remplaçante.** Qt
substitue en silence une police absente, et une capture faite sous une
substituée aurait exactement l'apparence d'une capture légitime — jusqu'au
moment où elle se ferait promouvoir chez le suivant.

C'est ce qui fait de « où les captures font foi » une **condition
vérifiable** plutôt qu'un nom de machine. Elles font foi partout où le
programme accepte de tourner ; là où il refuse, il dit pourquoi et quoi
installer.

## Le mécanisme, et pourquoi le dépôt ne gonfle pas

Une capture versionnée telle qu'elle est engendrée produirait un binaire modifié
à chaque exécution. Trois pièces l'évitent :

1. le programme de capture n'écrit **jamais** la référence : il écrit
   `<nom>.new.png` ;
2. `compare-screenshots.py` confronte le `.new` à la référence de même nom ;
   au-delà du seuil, le `.new` **remplace** la référence — git voit une
   modification, et elle veut dire quelque chose ; en deçà, le `.new` est
   **effacé**, et git ne voit rien ;
3. `check-screenshots.py` attrape ce qu'aucun des deux ne peut voir : une image
   que le manuel montre et que rien n'engendre, une image engendrée que
   personne ne montre, une capture qui écrirait directement la référence.

**Le seuil a deux composantes, et c'est ce qui le rend utile.** La *sévérité*
dit de combien un canal doit différer pour qu'un pixel compte ; la *surface*
dit combien de pixels doivent ainsi différer pour que l'image ait changé.
Un seuil unique ne distingue pas un antialiasing qui a bougé partout d'un mot
qui a changé quelque part — le premier est du bruit, le second est le seul
changement qui compte.

## Alternatives écartées

**Versionner la capture directement, sans comparateur.** C'est la forme la plus
simple, et elle donne un dépôt dont chaque `make manual` modifie six binaires.
Un vrai changement d'interface s'y noierait.

**Comparer les octets du fichier plutôt que les pixels.** Tentant, puisque les
captures se sont révélées reproductibles à l'octet près sous les réglages
ci-dessus. Écarté : la reproductibilité tient à un encodeur PNG donné, et un
Qt de version différente réécrirait les mêmes pixels autrement. Le comparateur
lit donc les pixels — mais il compare **d'abord** les octets, ce qui n'est pas
une contradiction : deux fichiers identiques portent forcément les mêmes
pixels, et l'écrasante majorité des comparaisons s'arrête là, en une
milliseconde.

**Ajouter une bibliothèque d'images à la chaîne d'outils.** `numpy` ou
`Pillow` pour compter des pixels différents. Écarté : le décodage du
sous-ensemble PNG que Qt écrit — huit bits par canal, sans entrelacement —
tient en soixante lignes, et une dépendance de plus se paie à chaque
installation de la chaîne d'outils, sur chaque machine, pour toujours.

**Faire de la CI l'autorité.** C'est la réponse d'un projet à plusieurs mains,
et elle a un défaut ici : elle rend impossible de régénérer une capture sans
pousser. Les quatre réglages ci-dessus donnent la même garantie sans le
détour — et si la CI les honore, elle est une machine comme une autre.

## Conséquences

**`make manual` a changé de nature.** Elle ne construisait que `subedit-cli`,
délibérément ; elle construit désormais l'interface, donc Qt. Le prix est celui
d'une compilation incrémentale que la porte de qualité fait déjà de toute façon.

**Le panneau vidéo ne se photographie pas.** libmpv dessine dans une fenêtre
native adoptée, hors du dessin de Qt : un `grab()` en rendrait un rectangle
vide. C'est le sujet de #190 — avec l'API de rendu, mpv dessinerait dans un
widget Qt et l'image deviendrait capturable. La section du lecteur reste donc
sans image, et c'est une absence nommée plutôt qu'un oubli.

**Le doublement des captures par thème attend le thème.** La D4 du cadrage de
la phase 7 promet deux captures par écran, clair et sombre ; le thème est #241
et il n'existe pas encore. Le manuel décrit ce qui existe, donc il y a une
capture par écran aujourd'hui. Le mécanisme les prendra sans rien changer :
une capture de plus est un appel de plus dans le programme, un nom de plus dans
le répertoire, une image de plus dans la section.

**Un changement d'interface se voit dans le diff du dépôt.** C'est le bénéfice
principal, et il vaut d'être dit à l'endroit : une capture promue est une
relecture demandée à celui qui pousse, pas seulement une image mise à jour.
