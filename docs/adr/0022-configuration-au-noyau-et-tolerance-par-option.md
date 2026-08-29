# 0022 — Une configuration au noyau, tolérante option par option

Statut : acceptée — 2026-08-28
Décidée au cadrage de la phase 7 (#237).

## Contexte

La phase 7 apporte des préférences persistées : géométrie de la fenêtre,
largeur des colonnes, thème, fréquence d'image par défaut. Deux questions se
posent ensemble, et il vaut mieux les trancher ensemble parce que la seconde
dépend de la première : **où vit la configuration**, et **ce qui arrive quand
le fichier ne dit pas ce qu'on attend**.

La seconde n'est pas théorique. Un fichier de configuration survit aux versions
du logiciel qui l'écrit : il est écrit par la version d'aujourd'hui et relu par
celle de dans deux ans, qui a ajouté des options, en a retiré, et en a changé
les défauts.

### Ce que fait Gaupol, lu plutôt que supposé

`gaupol/config.py` a été lu au cadrage de la phase 5 et relu ici. **Il n'y a
aucune migration, et ce n'est pas un oubli : c'est un dispositif complet.**

- Le fichier est de l'INI à sections imbriquées — `[a::b]` — dont les valeurs
  sont du JSON, plus un codec pour les énumérations.
- Une clé inconnue est **acceptée** à la lecture, puis **effacée à l'écriture**
  si elle n'est plus dans les défauts.
- Une valeur illisible imprime sur la sortie d'erreur et **laisse le défaut en
  place**. Une valeur nulle est écartée de même.
- Toute option restée à sa valeur par défaut est réécrite **commentée**, si
  bien qu'un changement de défaut prend effet chez qui ne l'a jamais surchargée.
- `general.version` est écrit et **n'est jamais relu** : une trace, pas un
  déclencheur.
- Lecture et écriture sont enveloppées : un échec imprime et n'arrête rien.

Vingt ans d'usage sans migration versionnée. Ce n'est pas une preuve, mais
c'est une donnée.

## Décision

**La configuration est un type du noyau, qui lit et écrit par
`core::FileSystem`, et qui reçoit son emplacement plutôt que de le chercher.**

**Elle est tolérante option par option, sans version ni migration** — le
dispositif de Gaupol, y compris la réécriture commentée des valeurs par défaut.

Concrètement :

| Cas | Ce qui se passe |
| :-- | :-------------- |
| clé inconnue | ignorée à la lecture, absente à la réécriture |
| valeur illisible | le défaut est gardé, un diagnostic est produit |
| option absente | le défaut, sans que ce soit un cas particulier |
| option à son défaut | réécrite **commentée** |
| fichier absent | tous les défauts, et ce n'est pas une erreur |
| fichier illisible | tous les défauts, et un diagnostic |

## Pourquoi au noyau plutôt que dans `QSettings`

`QSettings` donnerait l'emplacement, l'analyse et l'écriture atomique sans
qu'on écrive une ligne. Trois raisons de ne pas le prendre, et la troisième est
la décisive.

**Nos valeurs sont typées.** Une fréquence d'image par défaut n'est pas un
flottant : c'est une `FrameRate`, dont l'invariant est qu'elle est l'une des
huit. `QSettings` rend des `QVariant`, donc la frontière entre le fichier et le
type fort existerait de toute façon — et elle est précisément l'endroit où le
comportement ci-dessus doit être écrit. `QSettings` ne supprime pas le travail,
il le déplace.

**Le noyau est libre de Qt, et `check-architecture.sh` le vérifie.** Une
configuration dans `gui/` couperait la ligne de commande de toute préférence,
définitivement. On n'en veut aucune aujourd'hui, mais fermer la porte par un
choix de bibliothèque plutôt que par une décision serait la fermer sans l'avoir
décidé.

**La couture existe déjà et elle est éprouvée.** `core::FileSystem` est
l'abstraction par laquelle les formats se testent sans disque, et
`InMemoryFileSystem` en est l'implantation de test. Une configuration qui passe
par elle est éprouvable sans écrire nulle part — ce qui est exactement ce que
l'issue #238 cherche, et elle l'obtient sans réglage global de Qt ni mode test.

**Ce que `QStandardPaths` devient alors :** une ligne de câblage dans
`src/exe/gui/main.cpp`, qui donne l'emplacement au type. C'est la définition du
câblage, et c'est le seul chemin que les tests ne couvriront pas — il est nommé
ici pour qu'on le sache.

> **Corrigé en le posant, #238.** Ce chemin est couvert, et il n'a pas coûté
> cher de le rendre tel : `gui::userSettingsPath()` est une fonction de
> bibliothèque — `main.cpp` l'appellera d'une ligne quand #240 lui donnera un
> type à servir — et un test lit ce qu'elle répond sans qu'aucun processus
> n'ait à démarrer. Ce que l'ADR anticipait — une ligne hors de portée —
> était le prix d'un `QStandardPaths` appelé sur place ; il suffisait de ne
> pas l'y appeler. Reste vraie la phrase qui compte : c'est le
> **seul** endroit où un emplacement standard est résolu.

## Pourquoi la tolérance plutôt qu'une migration versionnée

**Elles ne coûtent pas au même moment.** La tolérance coûte à chaque lecture, un
peu ; la migration coûte à chaque changement de format, beaucoup, et le coût
tombe sur celui qui change le format — c'est-à-dire au pire moment, quand on a
la tête à autre chose.

**Une migration ne se teste que contre des fichiers qu'on a fabriqués.** Elle
n'attrape que les cas qu'on a imaginés, et son mode d'échec est de refuser un
fichier légitime. La tolérance a le mode d'échec inverse : elle accepte trop, et
ce qu'elle laisse passer retombe sur un défaut, qui est écrit.

**La réécriture commentée est ce qui rend la tolérance suffisante**, et c'est le
détail de Gaupol qu'il faut retenir : sans elle, une option jamais touchée par
l'utilisateur serait figée à la valeur du jour où elle a été écrite. Un défaut
qu'on améliore n'atteindrait jamais personne. Avec elle, changer un défaut
change le comportement de tous ceux qui ne l'ont pas surchargé — ce qui est très
exactement ce qu'un défaut doit faire.

## Alternatives écartées

**`QSettings`.** Voir plus haut. Écarté sur l'argument du noyau libre de Qt,
pas sur le confort — il en offre.

**Une migration versionnée, déclenchée par un numéro écrit dans le fichier.**
Écartée pour son coût et son mode d'échec. Notons que Gaupol *écrit* ce numéro
et ne le relit jamais : la trace est utile pour lire un fichier à la main, le
déclencheur ne l'est pas. Nous n'écrivons pas de numéro du tout — un champ qui
n'est jamais relu est un champ dont personne ne saura s'il est juste.

**Refuser un fichier qu'on ne comprend pas.** C'est le comportement le plus
strict, et le pire : il transforme une préférence perdue en logiciel qui ne
démarre pas. Une configuration est un confort ; sa défaillance doit coûter le
confort et rien d'autre.

## Conséquences

**Le diagnostic a besoin d'une sortie, et la fenêtre n'en a pas.** Gaupol
imprime sur la sortie d'erreur, y compris depuis son interface graphique. C'est
la voie la moins chère et elle est acceptable pour un défaut qui n'empêche rien ;
l'issue d'implémentation le tranche, et une fenêtre modale au démarrage pour une
préférence illisible serait un mauvais échange.

> **Tranché en le posant, #240 : la sortie d'erreur.** Une ligne par option
> illisible, plus une pour le fichier entier quand il existe et se refuse ;
> chacune nomme le fichier, l'option, et cite la valeur fautive — sans elle,
> « cannot be read » laisse l'utilisateur chercher dans son fichier ce qui
> n'allait pas. La mise en forme vit dans `gui::readUserSettings`, et non dans
> `main.cpp` : un point d'entrée câble, il ne compose pas des messages.

**Le fichier reste lisible et modifiable à la main**, ce qui est une propriété
et non un effet de bord : c'est ce qui rend une préférence corrigeable quand
l'interface qui la pose est justement celle qui ne va pas.

**Un défaut changé change le comportement des utilisateurs existants.** C'est
voulu, et c'est le revers de la réécriture commentée : un changement de défaut
n'est pas anodin et se décide comme un changement de comportement.
