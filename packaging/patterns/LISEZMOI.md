# Les motifs de correction de Gaupol

Les douze fichiers de motifs de Gaupol et leurs douze `.conf` d'activation, que
`subedit` installe dans `share/subedit/patterns` et lit tels quels —
[ADR 0037](../../docs/adr/0037-lire-les-motifs-de-gaupol-tels-quels.md).

Copiés du dépôt de Gaupol à la révision `f433bae3d23ef3572411fa9adca3fbab5cb1b3f9`
(2026-07-28), depuis le répertoire `aeidon/data/patterns` de celui-ci. **Gaupol est
sous GPL-3, comme ce projet** : la copie est permise, et ce fichier en est
l'attribution — les motifs sont l'œuvre des auteurs de Gaupol, pas de ce dépôt.

**Ne jamais les retoucher à la main.** Ce sont eux que le moteur devra lire, et un
motif « réparé » ici cesserait d'être celui de Gaupol. Une mise à jour se fait en
recopiant l'ensemble depuis une révision plus récente de Gaupol, en changeant la
révision ci-dessus, puis en relançant l'oracle : le diff des attendus dit alors ce
que la mise à jour change.

**Ce qui les lit** : le lecteur du noyau (`core/text/pattern_catalogue`), et
`src/scripts/pattern-oracle.py`, qui écrit ce que Gaupol en fait dans
`src/test/data/motifs/attendus/`. Il n'y a qu'une copie, et c'est celle-ci.

**Ce fichier s'installe avec eux** : le lecteur ne regarde que les fichiers dont le
nom finit par un type de motif ou par `.conf`, et celui-ci n'en est pas un.
