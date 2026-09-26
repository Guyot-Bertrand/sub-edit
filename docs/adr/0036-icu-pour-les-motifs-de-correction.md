# 0036 — ICU pour appliquer les motifs de correction

**Date :** 2026-09-26
**Statut :** acceptée

Décidée en cadrant la phase 12, issue
[#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493). Rouvre
l'[ADR 0017](0017-analyseur-de-mentions-ecrit-a-la-main.md), comme elle l'avait
prévu elle-même.

## Contexte

Les motifs de correction de Gaupol — 81 enregistrements : 63 erreurs courantes,
3 majuscules, 6 mentions, 9 découpages — sont écrits pour le module `re` de
Python. **25 portent un lookaround** et **38 remplacements une référence
arrière** : RE2, qui n'a ni l'un ni l'autre, est exclu sans mesure.

Restent deux candidats :

- **PCRE2**, que la feuille de route nommait depuis la phase 0 pour sa proximité
  avec la syntaxe de Python ;
- **ICU**, que personne ne nommait alors et qui est pourtant **déjà dans l'arbre
  deux fois** : dépendance du noyau depuis l'[ADR 0027](0027-icu-pour-les-encodages.md),
  et moteur de la recherche de la fenêtre depuis la phase 10
  (`core/edit/search.cpp`, `icu::RegexMatcher`), sans ADR — l'index le signalait
  comme un troisième candidat.

L'ADR 0017 a posé son propre déclencheur : « le troisième motif demandé, quel
qu'il soit, rouvre cette ADR ». La phase 12 en demande quatre de plus pour les
seules mentions.

## Ce qui a été mesuré

**Une mesure jetable**, écrite hors du dépôt pour ce cadrage, comme #492 l'avait
décidé : les deux moteurs appliquent les mêmes motifs, avec la sémantique du
`Finder` de Gaupol (recherche dans le texte déjà modifié, avance d'un caractère
après une correspondance vide, `Repeat` qui reboucle), et leurs sorties sont
confrontées à celles de l'oracle Python de
[#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494). ICU 74.2, PCRE2
10.42, GCC 13 en `-O2`, machine chargée (moyenne de 3,5 à 5) : les durées sont
des ordres de grandeur, pas un relevé de banc.

**Le temps**, les 63 motifs d'erreurs courantes appliqués tous, `Repeat`
compris, à 85 000 sous-titres réels (le corpus privé, répété) :

| Moteur | Durée | Par film de 4 000 sous-titres |
| :----- | ----: | ----------------------------: |
| PCRE2, compilé à la volée (JIT) | 1,09 s | 0,05 s |
| ICU | 2,85 s | 0,13 s |
| PCRE2, interprété | 3,95 s | 0,19 s |

Sur des textes où presque tout est à corriger — les entrées de #494, répétées
jusqu'à 85 000 —, le rapport ne change pas : 0,42 s, 0,99 s, 1,14 s. La
compilation des 81 motifs prend moins de quatre millisecondes dans chacun.

**La fidélité**, sur les 18 563 sous-titres du corpus privé, chaque type de
motif appliqué comme une substitution :

| Type | PCRE2 10.42 contre Python | ICU contre Python | ICU, `\w` et `\b` réécrits |
| :--- | ------------------------: | ----------------: | -------------------------: |
| erreurs courantes | 0 | 0 | 0 |
| majuscules | 0 | **8** | 0 |
| mentions | 0 | 0 | 0 |
| découpages | 0 | **12** | 0 |

Les vingt écarts d'ICU ont une seule cause : **une lettre écrite en forme
décomposée**, `i` suivi d'un accent combinant. Pour Python, `\w` est
`str.isalnum()` plus le souligné, et un accent combinant n'en est pas ; pour
ICU, `\w` inclut les marques, et `\b` les ignore. Réécrire `\w`, `\W`, `\b` et
`\B` au chargement avec la classe de Python — `[\p{L}\p{N}_]` — ramène ICU à
zéro écart sur les quatre types, pour **un coût de 10 %** (0,68 s au lieu de
0,61 s sur le corpus seul).

**Et l'accord de PCRE2 est un accident de version.** Sa version 10.43 a changé
`\w` en mode Unicode pour suivre Perl : il inclut désormais les marques non
espaçantes et la ponctuation de liaison (journal des changements de PCRE2,
10.43, point 32). La 10.42 de cette machine est celle d'Ubuntu 24.04 ;
Fedora livre une 10.44. Le paquet `.rpm` lierait donc un moteur qui ne rend pas
les mêmes textes que le `.deb`. **La réécriture de `\w` est due quel que soit le
moteur** — et l'argument qui faisait préférer PCRE2, « compatible avec la syntaxe
de Python », tombe avec elle.

**Ce qu'aucun des deux ne lit tel quel** : `\Z`, la fin absolue chez Python,
qui s'écrit `\z` chez les deux. PCRE2 refuse en plus `\uXXXX`, qu'il écrit
`\x{XXXX}`, et ICU les groupes nommés à la manière de Python, `(?P<nom>…)` et
`(?P=nom)` — qu'aucun motif livré n'emploie, mais qu'un motif utilisateur peut
écrire. Les remplacements, écrits `\1` et `\040`
pour Python, ne sont compris par aucun des deux : c'est la décision D3 de la
spec, qui les convertit au chargement.

## Décision

**ICU**, par `icu::RegexMatcher`, le moteur que la recherche emploie déjà. Les
motifs sont traduits de la syntaxe de Python à celle d'ICU **au chargement**,
une fois, par une fonction du noyau qui connaît la liste fermée de ce qu'elle
réécrit — `\Z`, `\w`, `\W`, `\b`, `\B`, `(?P<…>)`, `(?P=…)` — et qui refuse,
plutôt que de deviner, ce qu'elle ne sait pas traduire.

## Ce qui décide

**Le temps ne départage pas.** 0,13 s pour corriger un film entier, dans un
assistant qui présente ensuite chaque changement à confirmer : la différence
avec PCRE2 est invisible. Le point chaud de la phase est ailleurs — la mesure
de longueur, appelée à chaque candidat du découpage, et non les motifs, que le
découpage n'applique qu'une fois par texte pour y placer ses pénalités.

**Une seule syntaxe pour l'utilisateur.** Le champ de la recherche lit déjà des
expressions ICU. Un motif écrit par l'utilisateur dans un fichier de motifs, et
l'expression qu'il tape dans `Find and Replace…`, obéiront aux mêmes règles ;
avec PCRE2, deux moteurs répondraient différemment à la même expression dans le
même programme.

**Aucune dépendance de plus.** ICU est au noyau depuis la phase 8, et
`subedit-cli` le tire déjà. PCRE2 serait la troisième dépendance externe du
noyau, et, avec le JIT, une page de mémoire exécutable écrite à l'exécution —
ce que certaines politiques de sécurité refusent.

**Un garde-fou à l'emploi d'un motif mal écrit.** `RegexMatcher::setTimeLimit`
borne le retour arrière catastrophique qu'un motif utilisateur peut déclencher ;
le moteur s'arrête, la correction de ce texte est abandonnée et dite. PCRE2 a
son équivalent — la question ne départage pas, mais elle devait avoir sa
réponse.

## Alternatives écartées

- **PCRE2 avec JIT** — 2,6 fois plus rapide, pour une différence qu'aucun
  utilisateur ne verra sur un film, au prix d'une dépendance, d'un second moteur
  dans le programme et d'un comportement qui change d'une distribution à
  l'autre. Si un jour un usage rend le temps visible — un traitement par lot de
  centaines de fichiers en phase 13 —, la mesure est à refaire, et l'interface
  du moteur (D1 de la spec) est ce qui rend le remplacement local.
- **RE2** — ni lookaround ni référence arrière : 25 motifs sur 81 ne
  s'écriraient pas.
- **`std::regex`** — pas de lookbehind, lenteur connue, et l'ADR 0017 l'a déjà
  écarté pour les mentions.
- **Réécrire les motifs à la main dans un sous-ensemble commun** — les fichiers
  de Gaupol cesseraient d'être ceux de Gaupol, et la décision sur le format
  ([ADR 0037](0037-lire-les-motifs-de-gaupol-tels-quels.md)) repose sur le fait
  qu'on les lit tels quels.

## Conséquences

**Ce que devient l'ADR 0017.** Son déclencheur a joué, et sa décision **tient
pour ses deux motifs** : les crochets et les parenthèses restent au balayage
écrit à la main, pour la raison qu'elle donnait — la couture est locale au site
du retrait. Les quatre autres enregistrements de mentions passent par le moteur.
C'est la décision D7 de la spec ; 0017 n'est pas remplacée.

**Une traduction de syntaxe à tenir.** Elle vit au noyau, elle est éprouvée par
les 184 cas de #494 et par ceux de #495, et elle refuse ce qu'elle ne connaît
pas : un motif qui ne se traduit pas est **désactivé et nommé** — fichier,
enregistrement, raison —, jamais appliqué au hasard. L'[ADR 0008](0008-lecture-au-mieux-avec-diagnostics.md)
dit déjà comment on lit au mieux en rapportant ce qui ne va pas.

**La recherche peut en profiter, sans y être obligée.** Elle a sa propre
expansion des remplacements (`$1`), écrite pour ce qu'un utilisateur tape ; la
phase ne la change pas.

**Ce qui justifierait de rouvrir** : une mesure, dans le banc permanent que la
phase installe avec le moteur, où la correction d'un fichier devient perceptible
— au-delà d'une seconde pour un film —, ou un motif de Gaupol qu'ICU ne sait pas
exprimer.
