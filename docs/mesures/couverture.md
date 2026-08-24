# Cliquet de couverture

Le nombre de lignes de `src/lib/` que les tests n'exercent pas. **La porte
échoue s'il augmente.**

Pourquoi un compte de lignes et non un pourcentage : un pourcentage monte dès
qu'on ajoute du code bien testé, donc le cliquet monte, donc le travail suivant
se trouve contraint plus dur sans que personne l'ait décidé. Un compte de
lignes non couvertes ne réagit qu'à ce qu'on veut réellement surveiller. La
décision et les alternatives écartées sont dans
[l'ADR 0015](../adr/0015-memoire-des-mesures.md).

## Ce que fait la porte

| Situation | Réaction |
| :-------- | :------- |
| plus de lignes non couvertes qu'ici | échec, en nommant les fichiers qui en ont gagné |
| autant | passe, en silence |
| moins | passe, et invite à lancer `make ratchet` |

`make ratchet` réécrit ce fichier depuis la dernière mesure. **Il n'écrit jamais
de lui-même :** un fichier versionné ne bouge que si quelqu'un le demande.

## Ce que le cliquet a laissé passer, et pourquoi

Le relever est une décision, pas un ajustement. Chaque relèvement se justifie
ici, sans quoi le cliquet ne mesure plus rien.

**Deux lignes du modèle de table, en phase 5 — et le relèvement était de trop.**
Elles avaient été portées au compte des `std::unreachable()` placés après les
`switch` exhaustifs, qu'aucun test ne peut atteindre par construction. C'était
faux : `gcovr` ne compte pas ces lignes-là. Les deux non couvertes étaient les
gardes de rang et de colonne de `data()`, parfaitement atteignables — mais les
tests censés les éprouver fabriquaient leur index avec `index()`, qui refuse de
sortir de la table et rend un index invalide. Ils butaient donc sur la première
garde et n'atteignaient jamais les deux autres.

Corrigé à l'issue #129, en fabriquant ces index avec `createIndex` — ce pour
quoi l'en-tête l'expose. Le modèle est couvert en entier, et le cliquet
redescend de 7 à 5.

La leçon vaut d'être écrite : **un relèvement justifié par une lecture du code
plutôt que par une mesure est un relèvement à vérifier.** Le raisonnement était
plausible, il tenait une phase entière, et il désignait les mauvaises lignes.

**Trente et une lignes de `QtPrompts`, en phase 5.** C'est le relèvement le plus
important du projet, et il tient en une phrase : ces lignes ouvrent une boîte
modale de Qt, qui fait tourner sa propre boucle d'événements jusqu'à ce qu'un
humain clique. Un test qui en atteint une ne rend jamais la main.

**Mesurées, et nommées** — les quatre méthodes de `qt_prompts.cpp`, plus `run`
et le constructeur en en-tête. Il n'y en a pas d'autres, et chacune est soit l'appel à `QFileDialog`
ou `QMessageBox`, soit la garde « l'utilisateur n'a rien choisi », soit la
construction du retour à partir de ce que la boîte a rendu.

**Ce que ce relèvement achète.** L'interface `Prompts` concentre là tout ce qui
n'est pas atteignable, au lieu de le répandre dans la fenêtre. En échange, les
chemins où vivent les fautes — l'utilisateur annule, il choisit d'abandonner, il
choisit d'enregistrer d'abord, l'écriture échoue — sont parcourus par des tests,
ce qu'aucune autre disposition ne permettait. `MainWindow` et
`DiagnosticsPanel` sont couverts en entier.

**Ce qui a été sorti de là plutôt que compté.** Deux décisions vivaient dans ces
méthodes et n'avaient rien à y faire : quel format un filtre désigne, et ce que
vaut un bouton de la boîte de confirmation. Exposées, testées, elles ne sont
plus dans ce compte — c'est ce qui l'a fait tomber de trente-six à
vingt-cinq.

**Deux lignes de plus en phase 5, et pour la même raison.** L'issue #132 ajoute
`QtPrompts::run` — « montre ce dialogue, dis s'il a été accepté ». C'est la
couture des dialogues que le projet écrit lui-même, et elle est plus étroite que
les précédentes : les trois dialogues d'opération sont des widgets ordinaires
qu'un test construit, remplit et interroge, donc seule la boucle modale sort du
compte. Un `exec()`, une ligne, plutôt qu'une méthode par dialogue.

La première mesure de cette issue en donnait six de plus dans `MainWindow` :
deux dialogues annulés et trois gardes qu'aucun utilisateur n'atteint, le bouton
de validation suivant l'état du dialogue. Testées plutôt que justifiées — une
garde qu'aucun test ne traverse est une promesse que personne ne vérifie, et le
faux `Prompts` valide sans regarder le bouton, ce qui est exactement la
situation dont elles protègent.

**Trois lignes de plus en phase 5, et toujours la même raison.** L'issue #133
ajoute `reportOutcome` — la notice qui dit ce qu'une opération a fait, distincte
de l'avertissement qui dit ce qui a échoué : deux icônes, deux sens, et un
utilisateur qui lit « 1 subtitle cleaned, 1 removed » n'a été averti de rien.

Sa confirmation, elle, n'a rien coûté : le dialogue de retrait est un
`OperationDialog` sans champ, donc il passe par le `run` déjà en place. Une
opération sans réglage n'avait pas besoin d'une question de plus.

**Une ligne rendue, à la relecture de la phase.** Le constructeur de `QtPrompts`
était compté : aucun test n'en construisait un, faute d'avoir quoi que ce soit à
lui demander. La relecture lui en a donné une — les boîtes se posaient sur
`nullptr`, donc sur aucune fenêtre — et la fenêtre le lui dit désormais
elle-même. Le test qui l'éprouve construit la classe, ce qui couvre la ligne par
surcroît. **Trente-cinq, contre trente-six.**

Le reste de `qt_prompts.cpp` ne bouge pas, et ne bougera pas : c'est le
`QFileDialog` et le `QMessageBox`, et leur boucle d'événements.

**L'alternative, pesée et écartée.** On sait piloter une boîte modale depuis un
test, en programmant sa fermeture avant d'entrer dans la boucle. Cela aurait
donné un chiffre vert au prix d'un test fragile qui éprouve le dialogue de Qt
plutôt que nos quatre lignes de raccord. Le cliquet compte et justifie ; il
n'exige pas zéro.

**Trois lignes de `startProcess`, en phase 6 — et elles ne parlent qu'à un
manque de mémoire.** L'issue #164 lance un programme extérieur, ce que le projet
n'avait jamais fait. Les trois lignes sont les deux gardes du préparatif de
`posix_spawn` et l'enregistrement de leur code d'erreur :
`posix_spawn_file_actions_init` et `…_addopen` ne peuvent échouer que sur
`ENOMEM` — ou sur `EBADF` pour un descripteur invalide, et les trois passés ici
sont `0`, `1` et `2`.

**Mesuré plutôt que lu**, parce que ce fichier dit lui-même qu'un relèvement
justifié par une lecture du code est un relèvement à vérifier. La lecture
initiale était fausse : on croyait qu'un fichier de sortie impossible à écrire
ferait échouer `addopen`, donc qu'un test l'atteindrait. Il n'en est rien —
`addopen` ne fait qu'enregistrer l'intention, et c'est `posix_spawn` qui rend
`ENOENT` au moment d'ouvrir. Le test existe, il passe, et il ne traverse aucune
des trois lignes.

**Ce qui a été couvert plutôt que justifié.** La première mesure en donnait
quatre : la quatrième était `LaunchErrorKind::Failed`, le cas ni « absent » ni
« pas à nous ». Un fichier exécutable qui n'est pas un programme le produit —
le système répond `ENOEXEC` — et le test qui l'éprouve est le seul des trois
refus qui démontre que la table en est une, et non deux cas particuliers.

**Une ligne de `RealFileSystem::filesIn`, en phase 6.** Le listage d'un
répertoire lit ses entrées une à une, et chaque avance peut échouer : le
répertoire peut disparaître entre deux lectures. La ligne non couverte est le
refus qui en résulte.

**Ce qu'on achète en la gardant.** Sans elle, l'itérateur devient « fin » et la
fonction rend la liste partielle qu'elle avait — silencieusement. Un appelant
lirait « voici les fichiers » là où il faudrait lire « je n'ai pas pu finir »,
et c'est exactement le genre de mensonge qu'une couture au système ne doit pas
raconter. Le prix est d'une ligne, et il est payé une fois.

**Pourquoi aucun test ne l'atteint.** Il faudrait faire échouer une lecture
entre deux entrées, depuis l'extérieur, sur un flux de répertoire déjà ouvert :
retirer les droits ne touche pas un flux ouvert, et effacer le répertoire pendant
qu'on le lit ne fait pas échouer `readdir` sur Linux. C'est la même famille que
les trois lignes de `start_process.cpp` — des refus du système qu'un test ne
sait pas provoquer.

**Une ligne de plus dans `start_process.cpp`, en phase 6.** #172 lui ajoute
`runAndCapture`, qui pose une question à un programme et attend sa réponse par
un tube. La ligne non couverte est le refus rendu quand `pipe` échoue — ce qui
demande d'avoir épuisé les descripteurs du processus, état qu'un test ne peut
pas fabriquer sans emporter avec lui tout le binaire de tests.

C'est la quatrième de ce fichier et la même famille que les trois autres : des
refus du système, atteignables seulement en le mettant à genoux. Le reste de la
fonction est parcouru, y compris ses deux sorties d'erreur utiles — un
programme absent, et un fichier exécutable qui n'est pas un programme.

**Une ligne de `MpvPlayer::create`, en phase 6.** Bâtir un lecteur peut échouer
de trois façons — pas de mémoire pour un handle, une option que cette version de
libmpv ne connaît pas, une initialisation qu'elle refuse — et la ligne non
couverte est le refus qui en sort. Les trois sont écrites comme une seule
réponse courante plutôt que comme trois contrôles, précisément pour qu'il n'y en
ait qu'une : à qui demande un lecteur, ce sont le même événement.

**Ce qui a été restructuré plutôt que compté.** La première mesure en donnait
cinq. Quatre étaient la même garde défensive — « la propriété n'a pas répondu
alors qu'une vidéo est ouverte » — répétée dans la durée, la position et l'état
de lecture. Rien ne fait oublier à un fichier ouvert combien il dure, donc aucun
test ne pouvait les parcourir. Écrites comme une expression unique, où l'absence
de réponse est un résultat du même rang que le nombre, elles disparaissent du
compte sans que rien ne soit perdu — et le code y gagne.

**Huit lignes de plus dans `QtPrompts`, en phase 6.** #175 lui ajoute
`videoToOpen`, le sélecteur de vidéo : un appel à `QFileDialog`, la garde « rien
n'a été choisi », et le chemin rendu. C'est la troisième méthode de cette classe
et la même raison que les précédentes — une boîte modale fait tourner sa propre
boucle d'événements jusqu'à ce qu'un humain clique.

**Ce qui a été sorti de là plutôt que compté**, comme les fois précédentes : le
filtre. `videoFilters` assemble la liste d'extensions que le noyau reconnaît, et
c'est une chose que cette classe peut avoir fausse toute seule — un sélecteur qui
offrirait un fichier que le reste du programme refuse d'appeler une vidéo serait
un piège. Exposé, il est éprouvé par un test ordinaire, qui confronte le filtre à
`isVideoFile` extension par extension.

**Huit lignes de plus en phase 6, et toutes disent la même chose : il n'y a
pas d'écran ici.** #176 met la vidéo dans la fenêtre, et adopter une fenêtre
native est un mécanisme X11 — les tests, eux, tournent sur la plateforme Qt
`offscreen`, qui n'en fournit aucune.

| Lignes | Où | Ce qu'elles font |
| -----: | :- | :--------------- |
| 3 | `mpv_player.cpp` | poser `wid` et nommer le contexte X11, quand une fenêtre est donnée |
| 4 | `player_factory.cpp` | construire un vrai `MpvPlayer` pour cette fenêtre |
| 1 | `main_window.cpp` | la garde de `Play / Pause` quand aucun film n'est ouvert |

Les sept premières demandent une session X11 pour être parcourues. Ce n'est pas
une gêne de test mais la propriété même qu'on mesure : un lecteur sans écran est
ce que #178 a réglé pour tout le reste, et ces sept lignes sont exactement
l'endroit où l'écran redevient nécessaire. Elles ont été **vérifiées à la main**,
sous `QT_QPA_PLATFORM=xcb` — et cette vérification a rapporté **deux** défauts que
le code portait alors : le contexte laissé au choix de mpv ouvrait une fenêtre à
côté de la nôtre plutôt que dans elle, et la fenêtre adoptée avant d'être à
l'écran n'était jamais mappée, si bien que le panneau restait vide. Sept lignes
qu'aucun test ne parcourt sont sept lignes où deux défauts tenaient.

La huitième est une garde défensive derrière une action désactivée. Qt ne
déclenche pas une action désactivée, donc rien ne peut la parcourir ; la retirer
ferait dépendre l'absence de plantage de l'état d'un widget.

**Ce qui a été couvert plutôt que compté.** La composition de la réplique — le
passage d'un texte de sous-titre à l'événement ASS que l'incrustation dessine —
est sortie de `showSubtitle` en fonction libre, `assEventOf`, pour la raison qui
avait sorti le filtre du sélecteur en #175 : c'est une chose que ce fichier peut
avoir fausse tout seul, et rien de ce qui part vers libmpv ne se relit.
`showSubtitle`, lui, est parcouru par deux cas — un lecteur qui a un film, un
qui n'en a pas.

## Relevé

    total : 57

Relevé sur la version 0.5.13, le 2026-08-24.

| Lignes | Fichier |
| -----: | :------ |
| 38 | `src/lib/subedit/gui/qt_prompts.cpp` |
| 4 | `src/lib/subedit/core/process/start_process.cpp` |
| 4 | `src/lib/subedit/gui/mpv_player.cpp` |
| 4 | `src/lib/subedit/gui/player_factory.cpp` |
| 3 | `src/lib/subedit/core/io/real_file_system.cpp` |
| 2 | `src/lib/subedit/core/edit/insert_command.cpp` |
| 1 | `src/lib/subedit/core/time/ratio.hpp` |
| 1 | `src/lib/subedit/gui/main_window.cpp` |
