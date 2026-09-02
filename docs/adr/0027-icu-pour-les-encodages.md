# 0027 — ICU pour lire et écrire les encodages

Statut : acceptée — 2026-09-02
Décidée en cadrant la phase 8, issue #288.

## Contexte

La phase 8 doit lire des fichiers qui ne sont pas de l'UTF-8. Deux besoins
distincts en découlent, et on les confond facilement :

**Convertir** — transformer des octets dans un encodage donné en texte, et
l'inverse à l'écriture. Sans cela, rien n'est possible.

**Détecter** — proposer un encodage quand personne ne l'a dit. Un fichier de
sous-titres ne porte aucune déclaration d'encodage : ni en-tête, ni convention
de nom. La seule marque explicite est le BOM, et il n'existe que pour les
formats Unicode.

La feuille de route posait la question ouverte : « Gaupol s'appuie sur
`charset-normalizer`, qui n'a pas d'équivalent direct en C++. Candidats : ICU,
`uchardet`, `compact_enc_det`. »

**L'état de départ compte, et il est particulier.** `subedit-cli` n'a **aucune
dépendance externe** — `ldd` ne montre ni Qt ni ICU. Sept phases l'ont laissé
ainsi. `subedit-gui`, lui, tire déjà `libicui18n`, `libicuuc` et `libicudata`
par Qt.

## Ce qui a été mesuré

Les trois voies ont été passées **au même corpus**, avant de choisir — c'est ce
que l'initialisation de la phase (#287) avait exigé, et ce à quoi
`score-encoding-detection.py` sert. Le corpus étiqueté est celui de #289 : neuf
fixtures dont la table dit l'encodage.

| Voie | Corpus étiqueté | Ce qu'elle sait faire |
| :--- | --------------: | :-------------------- |
| **ICU**, `ucsdet` et `ucnv` | **9/9** | détecter **et** convertir |
| `uchardet` | **9/9** | détecter seulement |
| des tables écrites, prototype | 6/9 | ce qu'on écrit |
| `file -b --mime-encoding`, témoin | 6/9 | — |

Le prototype de tables — BOM, UTF-8 strict, puis un vote sur la part d'octets
hauts qui tombent sur une lettre — se trompe exactement là où c'est difficile :
il lit du Latin-1 comme du CP1252, du CP1252 comme du Latin-2, du CP1250 comme
du CP1252.

Sur le corpus privé, sans étiquettes, les trois proposent un encodage sous
lequel le fichier se décode entièrement dans 71 cas sur 71 — ce qui ne prouve
rien de plus que la nécessité : un mono-octet décode presque tout, et c'est
précisément d'où vient le mojibake. Ils se contredisent sur 19 fichiers sur 71,
tous dans la famille latine.

## Décision

**ICU, pour la conversion comme pour la détection, en dépendance du noyau.**

Trois raisons, dans l'ordre où elles pèsent.

**La conversion décide, pas la détection.** Une détection sans conversion ne
sert à rien, et ICU est le seul des trois candidats qui convertisse. Choisir
`uchardet` obligerait à écrire la conversion à côté — donc à choisir malgré tout
entre ICU et des tables, mais après avoir ajouté une dépendance de plus.

**L'iso-fonctionnalité fixe le jeu d'encodages, et il ne s'écrit pas à la
main.** Gaupol en porte quatre-vingt-dix-sept. Écrire quatre-vingt-dix-sept
tables — dont Big5, Shift-JIS et GB18030, qui ne sont pas des tables mais des
automates — est un travail sans fin et sans intérêt, pour un résultat moins bon
que ce qu'une bibliothèque de la distribution fait depuis vingt ans.

**Le coût réel est plus petit qu'il n'en a l'air pour la fenêtre, et entier pour
la ligne de commande.** `subedit-gui` charge déjà ICU par Qt : sa taille au
lancement ne bouge pas. `subedit-cli` y gagne sa première dépendance, et c'est
le vrai prix de cette décision.

## Ce que la décision coûte, écrit plutôt que tu

**`subedit-cli` cesse d'être autonome.** C'était une propriété réelle et elle
n'était écrite nulle part — donc elle n'a jamais été une promesse, seulement un
fait. Elle est perdue, et ce paragraphe est là pour qu'on sache qu'elle l'a été
délibérément.

**Deux noms de paquet de plus, un par famille.** `libicu74` chez Debian et
Ubuntu, `libicu` chez Fedora — et l'ADR 0023 dit déjà que c'est là que le paquet
peut être faux sans que rien ne le montre à la construction. #266 l'a payé une
fois. Ce qui l'attrape existe désormais : `check-rpm.sh` installe le `.rpm` sur
une vraie Fedora, chaque semaine.

**Le poids.** `libicudata` fait une trentaine de mégaoctets. Il est déjà sur
toute machine qui fait tourner Qt, KDE, Firefox ou LibreOffice ; il ne l'est pas
forcément sur un serveur nu où quelqu'un ne voudrait que `subedit-cli`.

## Alternatives écartées

**`uchardet` pour détecter, des tables écrites pour convertir.** C'est la voie
la moins chère en dépendances — `libuchardet` fait une centaine de kilooctets —
et elle détecte aussi bien qu'ICU sur le corpus. Écartée pour la conversion :
elle plafonne le jeu d'encodages à ce qu'on écrit, ce qui contredit
l'iso-fonctionnalité. Elle reste la bonne réponse le jour où le poids d'ICU
deviendrait un problème réel plutôt qu'une gêne théorique.

**`compact_enc_det`.** Absente des dépôts Ubuntu et Fedora, donc hors de la
règle de l'ADR 0004 — les dépendances viennent de la distribution.

**Écrire la détection à la main, ICU pour la seule conversion.** Le prototype
donne 6/9 sur neuf fixtures fabriquées, c'est-à-dire sur le cas facile. Ajouter
ICU puis ne pas s'en servir pour ce qu'elle fait le mieux serait payer sans
prendre.

**Qt pour la conversion, dans la fenêtre seulement.** `QStringDecoder` existe et
la fenêtre a déjà Qt — mais le noyau n'a pas le droit d'en dépendre, c'est un
invariant d'architecture que `check-architecture.sh` tient. Deux chemins de
conversion, l'un pour la fenêtre et l'autre pour la ligne de commande, seraient
deux façons de lire le même fichier : exactement ce que le partage du noyau
existe pour empêcher.

## Conséquences

**La détection est une proposition, jamais une certitude.** Latin-1 et CP1252
sont indistinguables sur un texte qui n'use pas de la plage `0x80–0x9f` : les
octets ne portent pas la réponse. Un détecteur qui tranche devine, et c'est
`latin1.srt` du corpus étiqueté qui le montre — les trois voies y répondent, et
aucune ne peut le prouver. Ce que la lecture au mieux avec diagnostics
(ADR 0008) impose ici : l'encodage retenu s'écrit dans les diagnostics, et
l'utilisateur peut le contredire.

**Le score se relèvera.** `score-encoding-detection.py` prend le détecteur en
argument : le jour où l'on voudra remplacer ICU, la comparaison se refera sur le
même corpus, dans les mêmes termes, sans rien réécrire.
