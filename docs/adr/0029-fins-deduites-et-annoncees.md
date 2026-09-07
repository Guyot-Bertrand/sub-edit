# 0029 — Déduire les fins que le fichier ne porte pas, et l'annoncer

Statut : acceptée — 2026-09-07
Décidée en cadrant la phase 9, issue #337.

## Contexte

**Deux des neuf formats ne portent qu'une position par ligne.** LRC écrit
`[00:12.34]texte`, TMPlayer écrit `00:00:12:texte` : le début d'une réplique, et
rien d'autre. Aucun des deux n'a de champ pour la fin.

Gaupol lit ces fichiers en posant la fin d'une réplique au début de la suivante,
et en donnant cinq secondes à la dernière — `subtitles[-1].duration_seconds = 5`,
la même ligne dans `lrc.py` et dans `tmplayer.py`.

Notre `Subtitle` porte `start` **et** `end`, tous deux non optionnels, depuis la
phase 1. La question est de savoir si un fichier qui ne dit pas la fin a le
droit de faire porter au modèle une valeur qu'il n'a pas lue.

Elle n'est pas rhétorique : le modèle a jusqu'ici refusé d'inventer. Il tolère
un `end` antérieur au `start` plutôt que de le corriger, il refuse de trier de
lui-même ([0012](0012-ordre-des-sous-titres-par-composition.md)), et sa règle
d'ouverture est de signaler plutôt que de réparer
([0008](0008-lecture-au-mieux-avec-diagnostics.md)).

## Décision

**Les fins sont déduites à la lecture** — la fin d'une réplique est le début de
la suivante, la dernière reçoit cinq secondes — **et la lecture émet un
diagnostic qui le dit.**

Le modèle ne change pas. `Subtitle::end` reste un `Timestamp`.

## Alternatives écartées

### Rendre `end` optionnel

C'est la voie qui dit la vérité, et c'est pourquoi elle a été instruite avant
d'être écartée.

Le compte n'est pas ce qui décide, mais il donne l'échelle : **vingt-huit
endroits** de `src/lib` et `src/exe` lisent ou écrivent `end` directement, six
appellent `duration()`, et `position(Boundary::End)` rend une référence qui ne
peut plus en être une.

**Ce qui décide est qu'il n'y a rien à décider vingt-huit fois.** Que fait un
décalage sur un sous-titre sans fin ? Rien. Qu'affiche la colonne Durée ? Rien.
Que signifie un chevauchement quand l'un des deux n'a pas de fin ? La question
ne se pose pas. Chacune de ces réponses est un vide qu'il faudrait écrire,
tester et maintenir — et pas une n'apprend quoi que ce soit à l'utilisateur.

**Et l'état ne survit à rien.** Il naît de deux formats sur neuf, et disparaît
dès que le document est enregistré ailleurs : SubRip, WebVTT, SubViewer 2, SSA,
ASS, MicroDVD et MPL2 exigent tous une fin. Faire porter à tout le noyau un cas
que sept formats sur neuf ne peuvent pas représenter est un mauvais échange.

### Refuser les deux formats

Contredit l'iso-fonctionnalité, qui est l'objet du projet. Écartée sans plus.

### Déduire en silence, comme Gaupol

C'est la voie de Gaupol, et c'est **la seule chose que cette décision lui
ajoute** : le silence. Un utilisateur qui ouvre un LRC voit une colonne Fin
remplie de valeurs plausibles, et rien ne lui dit qu'aucune ne vient de son
fichier. S'il en corrige une, il corrige une invention.

L'[ADR 0008](0008-lecture-au-mieux-avec-diagnostics.md) tranche déjà ce genre de
cas dans l'autre sens, et le vocabulaire existe depuis la phase 1 : un
`Diagnostic` porté par la lecture, montré par la fenêtre et par
`--verbose` sur les trois sous-commandes.

## Conséquences

**L'aller-retour d'octets reste exact, et il ne prouve rien.** L'écriture ne rend
que des débuts, donc tout ce qui a été déduit est redéduit à la lecture suivante
et le fichier revient identique — **quelle que soit la valeur inventée.** Un
test qui ne comparerait que les octets resterait vert si la déduction posait les
fins n'importe où.

C'est ce que la table de promesses de #338 appelle **l'octet à vide**, et
pourquoi ces deux formats y portent leurs quatre fins attendues écrites à la
main. La propriété qui les couvre n'est pas la comparaison d'octets, c'est cette
attente-là.

**Les cinq secondes de la dernière réplique sont arbitraires, et le restent.**
Le choix est celui de Gaupol, retenu par iso-fonctionnalité. Une règle plus
savante — la durée médiane du fichier, une vitesse de lecture — serait moins
prévisible sans être plus vraie, et la phase 10 apporte de toute façon
l'ajustement des durées à qui veut mieux.

**La conversion, elle, perd vraiment.** Convertir un SubRip en LRC jette les
fins, et cette perte-là n'est pas déduite mais subie : c'est la politique de
dégradation de la phase, et `docs/mesures/conversion.md` la déclare.
