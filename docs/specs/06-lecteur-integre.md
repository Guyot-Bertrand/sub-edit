# Phase 6 — Le lecteur intégré

**Issue de cadrage : #165.** Ce document décide ce que la phase construit, et
le découpe. Il n'est pas un compte rendu : ce qu'il écrit au futur est ce qui
reste à faire.

## Objectif

Après un décalage, une transformation ou une conversion de fréquence,
**vérifier le résultat sur le film**. C'est le geste qui manque : le noyau sait
déplacer quatre mille sous-titres depuis la phase 2, la fenêtre le montre depuis
la phase 5, et rien ne dit encore si le résultat tombe juste.

La vidéo se regarde **dans la fenêtre**, avec la réplique courante dessinée
par-dessus.

## Ce qui a changé, et pourquoi c'est écrit ici

La feuille de route cadrait cette phase autour d'une **prévisualisation par
lecteur externe** — écrire un fichier temporaire, lancer mpv, MPlayer ou VLC —
et rangeait le lecteur intégré en phase 14, « la partie la plus coûteuse du
projet ».

Ce cadrage a été repris. Et l'iso-fonctionnalité le demandait : **Gaupol a les
deux.** `gaupol/player.py` est un lecteur GStreamer embarqué, distinct de
`aeidon/agents/preview.py`. La feuille de route avait retenu le second et oublié
le premier.

Le lecteur externe **disparaît** de la phase. Il n'était qu'un substitut posé
quand l'intégré semblait trop cher ; le garder en plus coûterait deux surfaces,
deux sections de manuel et deux jeux de tests pour un geste que l'intégré rend
mieux.

Ce que cela fait de l'outillage déjà livré, sans détour :

| Issue | Ce qu'elle a livré | Ce qu'il en reste |
| :---- | :----------------- | :---------------- |
| #162 | `ffprobe`, et `findExecutable` | **utilisé** — `ffprobe` reste la seule source exacte de la fréquence |
| #163 | deux fixtures vidéo | **utilisées** — par `ffprobe` et par le lecteur |
| #164 | `startProcess`, `outcomeOf`, l'attente bornée | **utilisés** — c'est ainsi que `ffprobe` est lancé |
| #164 | le faux lecteur vidéo | **détourné** — il ne double plus mpv, il double `ffprobe` : écrire sur sa sortie et rendre un code est exactement ce qu'on attend de lui |

Rien n'est perdu, et le faux lecteur mérite d'être renommé quand une issue le
touchera.

## Portée

**Dedans.**

| Ce que la phase livre | Pourquoi ici |
| :-------------------- | :----------- |
| associer un fichier vidéo à un projet | rien ne peut être regardé sans lui |
| un lecteur dans la fenêtre : ouvrir, chercher, jouer, s'arrêter | l'objet de la phase |
| la réplique courante dessinée sur l'image | c'est ce qui rend le résultat vérifiable |
| lire la fréquence d'image avec `ffprobe` | l'outillage est en place, et c'est la seule source exacte |
| avertir quand une opération pousse des sous-titres au-delà de la fin du film | la durée vient du lecteur, qui l'a déjà |

**Dehors, et pourquoi.**

| Ce qui n'y est pas | Où, et pourquoi |
| :----------------- | :-------------- |
| avance image par image, pose d'un repère depuis la position courante | phase 14 — c'est tout ce qui lui reste |
| prévisualisation par lecteur externe | supprimée, voir plus haut |
| choix de piste audio, forme d'onde | phase 14 ou jamais ; rien ne les demande |
| déduire la fréquence des positions, sans vidéo | phase 16 |
| refuser une opération hors des bornes du film | jamais — la phase avertit et n'empêche rien |

## Décisions applicables

**D1 — libmpv.** Voir [l'ADR 0020](../adr/0020-libmpv-pour-le-lecteur-integre.md).
Le critère qui tranche n'est pas cette phase, que Qt Multimedia saurait servir,
mais le calage fin qui reste à la phase 14 — et qui n'a pas d'API chez lui.

**D2 — la réplique est dessinée depuis le modèle, jamais chargée d'un fichier.**
libmpv sait charger un fichier de sous-titres externe ; pour un éditeur ce
serait le mauvais choix, puisqu'il faudrait le réécrire à chaque frappe. Ce qui
s'affiche vient du même `Project` que la table de la phase 5, donc ce qu'on voit
sur l'image est ce qu'on vient de taper.

**D3 — prévisualiser un changement, c'est l'appliquer puis l'annuler.** Gaupol
copie les sous-titres, applique l'opération hors historique, prévisualise, et
restaure. Il l'a écrit faute d'une annulation fiable ; `subedit` en a une depuis
la phase 2, que la fenêtre expose depuis la phase 5 — « Annuler : décalage ». Le
noyau ne gagne donc aucune façon d'exécuter une commande sans l'enregistrer, et
l'utilisateur voit ce qui lui arrive plutôt qu'un état provisoire invisible.

**D4 — la borne haute avertit, elle ne refuse pas.** Une opération qui pousse
trois sous-titres après le générique de fin est peut-être exactement ce que
l'utilisateur veut. Un refus qui se trompe coûte plus cher qu'un avertissement
qu'on ignore.

**D5 — la convention de nom propose, le choix explicite décide.** À l'ouverture
d'un fichier de sous-titres, si aucune vidéo n'est associée, on cherche dans le
même répertoire un fichier vidéo dont le nom sans extension est un préfixe du
nôtre — `film.mkv` pour `film.fr.srt`, la règle de `find_video`. Ce que la
convention trouve est **une proposition** : le choix explicite l'écrase et n'est
jamais réécrasé par elle.

Deux points que la règle de Gaupol laissait ouverts, tranchés par #171 :

- **le préfixe se lit segment par segment**, la frontière étant le point.
  Gaupol compare des chaînes brutes, où `fil.mkv` répond pour `film.fr.srt` —
  un film proposé pour un sous-titre qui ne le concerne pas, et l'utilisateur
  ne s'en aperçoit qu'en regardant la mauvaise image ;
- **le nom le plus long l'emporte, et une égalité ne propose rien.**
  `film.fr.mkv` bat `film.mkv` pour `film.fr.srt`, étant le plus proche des
  deux. Mais `film.mkv` et `film.mp4` ne se départagent pas : ni l'ordre du
  système de fichiers — qui n'est pas stable — ni l'ordre de notre propre liste
  d'extensions — que personne ne peut lire — n'est une réponse. Le silence en
  est une, et D5 fait le reste, puisque le choix de l'utilisateur n'est jamais
  réécrasé.

**D6 — la fréquence lue dans le conteneur est proposée, jamais imposée.** C'est
la seconde source de la même donnée, et la phase 16 en apportera une troisième,
déduite des positions. Une source qui s'impose interdit de croiser ; une source
qui propose laisse la comparaison possible.

**D7 — la fréquence vient de `ffprobe`, la durée du lecteur.** `container-fps`
de libmpv est un flottant ; `FrameRate` est un rationnel exact depuis la phase 1,
et l'approcher serait perdre ce que ce choix protégeait. La durée, elle, n'a pas
ce problème et le lecteur la connaît déjà : deux sources pour une même donnée
sans nécessité en feraient une de trop.

## La tolérance à l'absence de `ffprobe`

**Elle n'est pas un mode dégradé, c'est le comportement normal.** Quand
`ffprobe` manque, `subedit` se passe de ce qu'il apporte — la fréquence d'image
déclarée. Si une opération en a besoin, **c'est à l'utilisateur de la fournir**,
exactement comme aujourd'hui.

Aucune opération ne se refuse au motif que `ffprobe` manque, et **le lecteur,
lui, fonctionne sans lui** : libmpv est une dépendance de compilation, `ffprobe`
un exécutable qu'on cherche.

L'ordre d'écriture en découle : **la donnée vient de l'utilisateur, et `ffprobe`
la propose quand il est là.** Pas l'inverse. Un code écrit dans l'autre sens
traite le chemin normal comme un rattrapage, et c'est celui-là qu'on cesse
d'éprouver.

## Le noyau

### La vidéo associée

Le projet gagne la notion de vidéo associée à un document. Trois choses la
composent, et elles ne se confondent pas :

- **le chemin**, choisi ou deviné ;
- **d'où il vient** — deviné par convention, ou choisi — parce que D5 en dépend ;
- **ce que le conteneur déclare**, c'est-à-dire la fréquence, ou rien.

La reconnaissance d'un fichier vidéo se fait par extension, liste fermée, comme
`aeidon.util.is_video_file`. Elle vit avec le reste du vocabulaire du noyau : la
convention de nom en a besoin, et elle est au noyau.

### La lecture de la fréquence

Un appel à `ffprobe`, qui rend `r_frame_rate` sous forme de rationnel exact.
`findExecutable` (#162) le trouve. L'appel attend : il prend quelques
millisecondes, et aucune interface n'a de raison de rendre la main pendant ce
temps.

**Ce n'est pas `startProcess` qui le lance**, contrairement à ce que cette
section a d'abord annoncé. #172 lui a écrit un frère, `runAndCapture` : il
attend, et rend ce que le programme a écrit. Celui de #164 est fait pour un
lecteur vidéo, qui survit à l'appel qui l'a lancé — il rend la main aussitôt et
écrit la sortie dans un fichier, qu'il aurait fallu nommer, poser sur un
support inscriptible, relire et effacer, le tout pour douze octets, et sonder
`outcomeOf` en boucle en attendant. Poser la question par un tube et attendre
la réponse dit la même chose en moins.

Trois issues possibles, et une seule est une erreur :

| Ce qui arrive | Ce que le noyau rend |
| :------------ | :------------------- |
| `ffprobe` est absent du `PATH` | rien, sans erreur |
| il répond, mais le fichier n'est pas une vidéo | rien, sans erreur |
| il répond une fréquence | le rationnel exact |

### Le lecteur

Une couture au sens du principe 3 — « lecteur vidéo » y est nommé comme point de
variation depuis les fondations, et c'est ce cas-là. Derrière elle, libmpv.

Ce qu'elle expose, et rien de plus tant que la phase 14 n'est pas là :

| Opération | Ce qu'elle fait |
| :-------- | :-------------- |
| ouvrir | charge un fichier, ou dit pourquoi il ne s'ouvre pas |
| chercher | se place à une position, **exactement** |
| jouer, s'arrêter | ce que les mots disent |
| position | où en est la lecture, pour suivre la ligne courante |
| durée | ce que le conteneur déclare, une fois ouvert |

**L'interface vit au noyau, l'implémentation dans `subedit_gui`.** La distinction
est celle du principe 3, et elle a été reprise pendant #173 : `core::VideoPlayer`
ne nomme ni Qt ni libmpv, et c'est ce qui permet au reste du noyau de raisonner
sur un lecteur sans en connaître un. `MpvPlayer`, lui, est un élément
d'interface — il n'existe que pour la fenêtre, il en reçoit la sienne, et rien
d'autre ne le construira. Faire entrer libmpv dans `subedit_core` aurait donné au
domaine une dépendance d'infrastructure pour la seule commodité d'un harnais de
test plus léger, ce qui n'est pas une raison d'architecture.

Le déplacement a d'ailleurs révélé un défaut que la place au noyau masquait :
**libmpv refuse de démarrer si `LC_NUMERIC` n'est pas « C »**, et `QApplication`
le règle sur celle de l'utilisateur. Au noyau, les tests tournaient sans
`QApplication`, donc en locale « C » ; le défaut n'apparaissait pas, et toute
vraie fenêtre l'aurait rencontré.

**Le rendu, tranché par #173 : la fenêtre native que libmpv adopte**, par la
propriété `wid`. C'est **un nombre**, et c'est précisément ce qui permet au
noyau de désigner une fenêtre sans rien savoir d'une fenêtre. La voie est la
plus courte des deux, et elle laisse la réplique à l'`osd-overlay` de libmpv —
que D2 autorise, puisque le texte y vient du modèle et non d'un fichier. #176 le
vérifiera ; c'est là que la question se pose vraiment.

**Ce que #173 n'a pas écrit, et pourquoi.** Le lecteur ne montre rien : il n'y a
pas encore de fenêtre à lui donner, et poser `wid` sans qu'aucun test ne puisse
parcourir cette ligne serait écrire du code pour plus tard. #176 a la fenêtre,
et l'ajoutera avec la preuve qui va avec.

### La borne haute

Le noyau ne connaît aujourd'hui qu'une borne, celle de zéro : un décalage
négatif trop grand rend une position négative, et c'est tout ce qu'il sait dire.
La durée que le lecteur annonce lui donne l'autre bout.

**Le décalage est le cas le plus net, et il n'est pas le seul.** Une
transformation dont le second repère tombe après la fin, une conversion de
fréquence qui étire l'ensemble au-delà : les trois dépassent, et l'avertissement
doit être branché sur les trois.

Ce que l'avertissement dit : combien de sous-titres dépassent, et de combien le
plus lointain. Ce qu'il ne fait pas : empêcher quoi que ce soit (D4).

**Ce que #174 a tranché : un calcul, trois formulations.** Les trois opérations
dépassent de trois façons, mais elles arrivent toutes à la même conclusion — des
sous-titres qui finissent trop tard — et `beyondEnd` la lit **après l'opération,
sur l'état qu'elle a produit**, ce que la fenêtre montre de toute façon. Rejouer
l'arithmétique de chaque commande pour l'annoncer d'avance aurait été trois
copies de ce que les commandes calculent déjà.

Ce qui reste propre à chacune est la **phrase**, qui nomme l'opération :
`noticeOf` la compose depuis `CommandKind`, et un avertissement qui ne dirait pas
laquelle des trois vient de tourner laisserait l'utilisateur deviner.

Trois lectures que le calcul assume, et qu'un test fixe :

- **le périmètre est la sélection**, c'est-à-dire ce que l'opération a touché. Un
  sous-titre que personne n'a déplacé, déjà au-delà parce que la vidéo associée
  est la mauvaise, n'est pas le fait de cette opération ;
- **finir exactement avec la vidéo, c'est finir dedans** — la même lecture que la
  borne de zéro, qui admet la position zéro ;
- **le dépassement est celui du plus lointain**, et non du dernier dans l'ordre
  du fichier : un document en désordre est ordinaire, et ce que l'utilisateur a
  besoin de savoir est jusqu'où il va.

## La fenêtre

| Élément | Comportement |
| :------ | :----------- |
| **Video ▸ Select Video…** | un sélecteur filtré sur les extensions vidéo, positionné sur le répertoire du fichier de sous-titres |
| ouverture d'un fichier | déclenche la recherche par convention si aucune vidéo n'est choisie (D5) |
| la vue vidéo | occupe le haut de la fenêtre, la table dessous, la séparation déplaçable |
| aucune vidéo associée | la vue est absente, la table occupe tout, et rien ne clignote |
| **Jouer / Pause** | au clavier et au menu |
| sélectionner une ligne | place la lecture au début de ce sous-titre |
| la lecture avance | la ligne courante suit, sans voler le focus à qui édite |
| la réplique | dessinée sur l'image, depuis le modèle (D2) |
| la vidéo ne s'ouvre pas | le dit, nomme le fichier, et la fenêtre reste utilisable |
| fréquence lue | proposée dans le dialogue de conversion de fréquence, avec sa provenance |
| opération hors bornes | l'avertissement déjà en place pour les autres notices, après l'opération |

**Tout ce que l'utilisateur lit est en anglais**, et les mots vivent dans
`core/wording.hpp`.

## Tests

| Quoi | Comment |
| :--- | :------ |
| la lecture de la fréquence | les fixtures de #163, dont celle à `24000/1001` |
| `ffprobe` absent | le chemin de recherche injecté de #162 |
| la convention de nom | des fichiers vides aux bonnes extensions, dans `src/test/data/` |
| la borne haute | unitaire, sur des durées données à la main — aucune vidéo nécessaire |
| ouvrir, chercher, la durée | la couture, sur les fixtures de #163 — deux secondes de vidéo suffisent |
| la fenêtre | le harnais Catch2 de #119, hors écran |

**Un lecteur sans écran était le point à régler avant tout le reste**, et #178
l'a réglé. C'était exactement le problème que #117 avait résolu pour la fenêtre
— `QT_QPA_PLATFORM=offscreen` — et il se reposait pour la vidéo.

La réponse est `vo=null`, et elle est mesurée plutôt que promise : avec `vo=auto`
et sans écran, mpv n'ouvre même pas le fichier — il répond `end-file`, et la
durée revient « property unavailable ». Ce qu'un test peut alors attendre d'un
lecteur sans sortie : la durée, la position après une recherche exacte, la
géométrie de l'image, une lecture qui avance en temps réel. **Pas une image
décodée** — il n'y a pas de sortie d'où la prendre — et c'est donc dans la
fenêtre que la réplique dessinée se prouvera, jamais ici.

`src/test/gui/mpv_player_test.cpp` porte cette preuve. C'était
d'abord un harnais séparé, écrit par #178 avant que la couture existe ; #173 l'a
absorbé, parce que deux fichiers qui ouvrent les mêmes fixtures pour la même
raison finissent par diverger, et que celui qui compte est celui qui passe par
le code employé.

## Mesures

Deux mesures nouvelles, et pas une de plus :

- **ouvrir une vidéo et chercher une position** — ce qui se passe à chaque
  changement de ligne, donc le seul chemin que l'utilisateur sent ;
- **dessiner la réplique** sur une image, si elle se distingue du bruit.

Le décodage lui-même ne se mesure pas : ce qu'on chronométrerait est libmpv, qui
n'appartient pas au projet.

## Exigences

Ajoutées au registre en début d'issue, à l'état `prévue`.

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `GUI-VIDEO-01` | choisir une vidéo l'associe au document, et la fenêtre la nomme |
| `GUI-VIDEO-02` | ouvrir un fichier de sous-titres propose la vidéo voisine de même nom |
| `GUI-PLAYER-01` | la vidéo s'ouvre dans la fenêtre, et se joue |
| `GUI-PLAYER-02` | sélectionner un sous-titre place la lecture à son début |
| `GUI-PLAYER-03` | une vidéo qui ne s'ouvre pas le dit, et laisse la fenêtre utilisable |
| `GUI-FRAMERATE-02` | la fréquence lue dans la vidéo est proposée, et sa provenance est dite |
| `GUI-BOUNDS-01` | une opération qui dépasse la fin du film le signale sans l'empêcher |

## Manuel

Une section de `docs/manual/subedit-gui/`, **écrite à la main**. C'est la
décision de #116 : `make manual` engendre ses blocs en exécutant le binaire, et
une fenêtre n'a pas de `--help`. Sa justesse repose sur la relecture de fin de
phase.

Ce qu'elle doit dire, au-delà des gestes : que `ffmpeg` n'est pas requis, ce
qu'on perd sans lui, et qu'aucune opération n'en dépend.

## Découpage en issues

| Issue | Ce qu'elle porte |
| :---- | :--------------- |
| #178 — libmpv dans la chaîne d'outils, et un lecteur sans écran | le paquet, le cache de la CI, et un lecteur qu'un test construit sans sortie visible |
| #171 — Noyau : la vidéo associée à un document | le chemin, sa provenance, la reconnaissance par extension, la convention de nom (D5) |
| #172 — Noyau : lire la fréquence avec `ffprobe` | l'appel, le rationnel exact, et les trois issues dont une seule est une erreur |
| #173 — Noyau : la couture du lecteur, et libmpv derrière | ouvrir, chercher, jouer, s'arrêter, la position, la durée |
| #174 — Noyau : avertir au-delà de la fin du film | les trois formulations — décalage, transformation, conversion (D4) |
| #175 — Interface : choisir une vidéo, et la voir | le sélecteur, la proposition à l'ouverture, la barre d'état |
| #176 — Interface : le lecteur dans la fenêtre | la vue, la réplique dessinée (D2), la ligne qui suit, le clavier |
| #177 — Interface : la fréquence lue et la borne du film | la proposition dans le dialogue de conversion (D6), l'avertissement de dépassement |

#178 passe avant toutes les autres. #173 dépend d'elle et de #171 ; #174 dépend de #173 pour la durée ; les trois issues d'interface
dépendent de tout ce qui précède. **#176 est celle qui rend le geste possible :
les cinq qui la précèdent la servent.**

## Renvois

| Ce qui est renvoyé | Où |
| :----------------- | :- |
| avance image par image, pose d'un repère depuis la position | phase 14, qui n'a plus que cela |
| croiser la fréquence lue et la fréquence déduite, et présenter un désaccord | phase 16, qui apporte la seconde |
| le lecteur pour un document de traduction | phase 11, qui apporte le second document |
| rendu dans une fenêtre native ou dans un contexte OpenGL | **tranché par #173** : la fenêtre native, propriété `wid`. Posée par #176, qui a une fenêtre à donner |

## Critères de fin

- [ ] Les huit issues sont fermées
- [ ] Les exigences du registre sont `implémentées` et citées par un test
- [ ] La section de manuel existe et décrit ce qui existe
- [ ] Les benchmarks sont rejoués et les mesures relevées
- [ ] La relecture de fin de phase est faite
