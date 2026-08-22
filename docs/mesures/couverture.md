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

## Relevé

    total : 38

Relevé sur la version 0.5.3, le 2026-08-22.

| Lignes | Fichier |
| -----: | :------ |
| 30 | `src/lib/subedit/gui/qt_prompts.cpp` |
| 3 | `src/lib/subedit/core/process/start_process.cpp` |
| 2 | `src/lib/subedit/core/edit/insert_command.cpp` |
| 2 | `src/lib/subedit/core/io/real_file_system.cpp` |
| 1 | `src/lib/subedit/core/time/ratio.hpp` |
