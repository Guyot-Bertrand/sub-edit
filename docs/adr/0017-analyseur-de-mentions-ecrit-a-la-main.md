# 0017 — Écrire l'analyseur de mentions à la main, sans moteur d'expressions rationnelles

**Date :** 2026-08-15
**Statut :** acceptée

## Contexte

La phase 4 retire d'un sous-titrage les mentions pour malentendants : les bruits
décrits entre crochets ou entre parenthèses. Gaupol le fait avec deux
expressions rationnelles, `\[.*?\]` et `\(.*?\)`, en `DOTALL`.

Le projet vise l'iso-fonctionnalité, et la pente naturelle serait donc de
reprendre ces deux motifs. La feuille de route l'avait d'ailleurs anticipé en
notant que ces motifs n'ont **aucune référence arrière** — l'arbitrage entre
PCRE2 et RE2, qui se posera à la phase 12, ne bloque pas celle-ci.

Reste à décider avec quoi cette phase-ci reconnaît les mentions.

## Décision

**Un balayage écrit à la main**, dans `src/lib/subedit/core/text/`. Aucun moteur
d'expressions rationnelles : ni PCRE2, ni RE2, ni `std::regex`.

## Ce qui décide : la couture est locale, une passe est globale

Les deux motifs de Gaupol ne suffisent pas, et Gaupol le sait : il les fait
suivre de **sept passes de rattrapage** (`_remove_leftover_hi`) qui recollent ce
que le retrait a laissé — bords de ligne, espaces consécutifs, lignes vides,
tirets de dialogue orphelins.

Notre règle, écrite cas par cas dans `src/test/data/textes/mentions.cas` avant
tout code, dit autre chose :

> Une mention retirée laisse **exactement un espace entre ce qui l'entourait**,
> et **rien en bord de ligne**. Ce n'est pas « on recolle les espaces qui
> traînent » : `Bonjour[il tousse]Marie` n'en avait aucun et en gagne un.

**C'est une règle sur le site du retrait.** Une passe d'expression rationnelle,
elle, ne sait pas où était la mention : `" {2,}" → " "` réécrit tous les doubles
espaces du texte, y compris ceux que le fichier portait déjà et auxquels
personne n'a demandé de toucher. La règle globale n'est pas une approximation de
la règle locale, c'en est une autre — et sur du texte réel, les deux ne rendent
pas le même résultat.

Pour rester local, il faut savoir où était la mention : donc parcourir les
correspondances une à une et assembler la sortie soi-même. À ce moment-là,
l'analyseur est écrit, et le moteur ne fournit plus que « trouve le prochain
`[…]` » — une boucle de quinze lignes. Le même raisonnement vaut pour la mention
coupée par le saut de ligne : le balayage sait trivialement si le morceau retiré
contenait un `\n`, quand il faudrait le redemander à une expression sous forme
d'une capture de plus.

Ce que le moteur apporterait — deux motifs déclaratifs et relisibles — n'est
donc pas ce qui est difficile ici.

## Alternatives écartées

- **`std::regex`** — aucune dépendance à installer, puisque c'est la
  bibliothèque standard, et elle sait faire davantage qu'il n'y paraît : garder
  les références numériques tient dans une alternance qui capture ce qu'on
  garde, `(\[[ \t]*[0-9]+[ \t]*\])|\[[\s\S]*?\]|\([\s\S]*?\)` remplacé par
  `$1`. Écartée sur la couture, argument ci-dessus, et non sur ses défauts
  connus — l'absence de drapeau `DOTALL`, qu'on contourne par `[\s\S]`, et sa
  lenteur, qui pèserait sur le chemin que la fixture de #89 vient précisément de
  rendre mesurable. Ce serait de surcroît un troisième moteur, adopté pour être
  remplacé à la phase 12.
- **Trancher PCRE2 ou RE2 dès maintenant** — pour ne le faire qu'une fois. Mais
  ce serait choisir un moteur pour des besoins qu'on ne connaît pas : les
  références arrière, les motifs par langue et le correcteur orthographique
  arrivent à la phase 12, et c'est là que les critères de comparaison
  apparaîtront. Le seul usage qu'on saurait mesurer aujourd'hui — deux
  délimiteurs littéraux — ne départage pas deux moteurs.
- **Reprendre les données de motifs de Gaupol** — écartée à #88, pour ses
  propres raisons : la copie serait licite, les deux projets sont sous GPL-3,
  mais elle engagerait sur un format de fichiers qu'on n'a pas choisi, dont un
  `.conf` XML et des sections INI homonymes qu'aucun lecteur standard n'accepte.

## Conséquences

**Aucune dépendance nouvelle**, ce qui laisse la question du moteur entière et
libre pour la phase 12 — elle la tranchera avec ses vrais critères sous les yeux.

**Une fonction pure**, `withoutHearingImpaired`, sans état ni contexte : les
cas de `mentions.cas` l'éprouvent directement, sans construire de
projet ni de commande. Un moteur aurait imposé le sien — compilation des motifs,
durée de vie, gestion d'erreur de compilation — entre les cas et la règle.

**Ce qui rend la décision peu coûteuse à défaire :** la règle vit derrière une
fonction libre d'une seule signature. La phase 12, si son moteur permet
d'exprimer la couture locale, réécrira son corps sans toucher ni à la commande,
ni à la CLI, ni aux cas qui l'éprouvent — et ce sont eux qui diront si la
réécriture est fidèle.

**Ce qu'il faudra surveiller** : la tentation d'ajouter au balayage les motifs
de la phase 12 un par un, jusqu'à écrire un moteur d'expressions rationnelles
sans l'avoir décidé. Le déclencheur est écrit : le troisième motif demandé, quel
qu'il soit, rouvre cette ADR.
