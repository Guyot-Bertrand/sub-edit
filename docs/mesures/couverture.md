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

## Relevé

    total : 8

Relevé sur la version 0.2.2, le 2026-08-11.

| Lignes | Fichier |
| -----: | :------ |
| 2 | `src/lib/subedit/core/edit/insert_command.cpp` |
| 2 | `src/lib/subedit/core/format/real_file_system.cpp` |
| 2 | `src/lib/subedit/core/format/subtitle_writer.hpp` |
| 1 | `src/lib/subedit/core/command/composite_command.hpp` |
| 1 | `src/lib/subedit/core/time/ratio.hpp` |
