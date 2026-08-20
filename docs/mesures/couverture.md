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

## Relevé

    total : 5

Relevé sur la version 0.4.12, le 2026-08-20.

| Lignes | Fichier |
| -----: | :------ |
| 2 | `src/lib/subedit/core/edit/insert_command.cpp` |
| 2 | `src/lib/subedit/core/io/real_file_system.cpp` |
| 1 | `src/lib/subedit/core/time/ratio.hpp` |
