# Phase 12 — Moteur de correction complet

Cadrage de l'issue [#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493),
après l'initialisation [#492](https://github.com/Guyot-Bertrand/sub-edit/issues/492).

## Ce que la feuille de route promet, et ce qui est déjà là

Motifs déclaratifs par écriture, langue et pays — erreurs courantes classées
Humain et OCR, remise en majuscule, mentions pour malentendants restantes —,
découpage de lignes, et correcteur orthographique avec la jonction et la
scission de mots qu'il permet. Plus quatre renvois : les trois motifs de
mentions et le choix du moteur, venus de la phase 4 ; PCRE2 ou RE2 et hunspell,
de la phase 0 ; la jonction, la scission et les erreurs courantes, de la
phase 10 ; et le parseur conscient des balises, que la phase 10 a écrit pour
qu'on l'emprunte ici.

**C'est la première phase dont le cadrage trouve ses attendus déjà écrits.**
L'initialisation a produit deux outils, et ils changent la nature des décisions :
on ne choisit plus un moteur sur sa réputation, on le confronte à ce que Gaupol
fait.

| Ce qui est déjà là | Où |
| :----------------- | :- |
| les **24 fichiers de motifs de Gaupol**, versionnés tels quels, avec leur attribution | [#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494), `src/test/data/motifs/gaupol/` |
| **184 cas** écrits à la main aux bords des 72 enregistrements de correction, et **ce que Gaupol en fait**, écrit par un oracle Python qui reproduit son `Finder` | #494, `entrees/`, `attendus/`, `src/scripts/pattern-oracle.py` |
| **26 découpages** attendus, le `Liner` de Gaupol porté dans le même oracle, en unité caractère | [#495](https://github.com/Guyot-Bertrand/sub-edit/issues/495), `line-break.cas` |
| un **moteur d'expressions en service**, ICU, celui de la recherche | phase 10, `core/edit/search.cpp` |
| le **parseur conscient des balises**, `replace` et `transform` | phase 10, `core/text/markup_parser` |
| le **retrait des mentions** entre crochets et parenthèses, par un balayage écrit à la main | phase 4, [ADR 0017](../adr/0017-analyseur-de-mentions-ecrit-a-la-main.md) |
| un **compte des textes changés**, et non des correspondances | `rewrittenCount`, phase 10 |
| le **multi-projets**, qui donne son sens à la cible « tous les projets ouverts » | phase 11 |

**Ce qui n'y est pas.** Aucun lecteur de fichiers de motifs, aucune traduction
de la syntaxe de Python, aucune expansion des remplacements à la manière de
Python, aucune mesure de longueur autre que `size()`, aucun correcteur, aucun
assistant.

## Analyse préalable — ce que Gaupol fait

Lue dans `aeidon/patternman.py`, `pattern.py`, `agents/text.py`, `parser.py`,
`finder.py`, `liner.py`, `spell.py`, `gaupol/ruler.py`, `gaupol/assistants.py`,
`gaupol/dialogs/spell_check.py` et `gaupol/config.py` ; **exécutée** par #494 et
#495, dont les oracles ont été confrontés au vrai `aeidon` (360 cas puis 26, sans
désaccord). Le corps de #493 en donne le tableau ; ce qui suit est ce qui
gouverne une décision.

**Le chargement.** `Zyyy`, puis l'écriture, la langue, le pays, concaténés dans
cet ordre ; un nom répété se range après le précédent, sauf `Policy=Replace` qui
le remplace ; `SkipIn` exclut un enregistrement d'un code. Les fichiers livrés
sont lus, puis ceux de l'utilisateur (`~/.local/share/gaupol/patterns`) ; les
`.conf` livrés, puis ceux de l'utilisateur. **Le `.conf` de `Latn` pour les
mentions nomme deux motifs qui n'existent pas** — « Song lyrics between
asterisks » et sa variante d'une ligne : l'activation d'un nom inconnu est
ignorée sans un mot.

**L'application.** Les motifs dans l'ordre, chacun par un `Finder` qui cherche
dans le texte déjà modifié. `Repeat=True` reboucle **tant qu'une passe a trouvé
quelque chose**, et non tant qu'elle a changé quelque chose : un motif dont le
remplacement correspond encore à lui-même ne s'arrête jamais. Aucun motif livré
ne le fait ; un motif de l'utilisateur le peut.

**Le filtre Humain/OCR n'est qu'un filtre d'affichage.** `CommonErrorPage._filter_patterns`
(`gaupol/assistants.py`, lignes 445-450) masque les motifs de la classe
décochée ; `correct_texts` (ligne 430) les applique **tous**, masqués compris.
Vérifié par #492 en lisant, et non plus sur la foi d'une lecture déléguée.

**La majuscule a un état qui traverse les sous-titres** : un `.` en fin de
sous-titre met en majuscule le début du suivant, et le premier sous-titre du
document l'est toujours. La traversée s'arrête aux trous de la sélection.

**Le retrait des mentions est suivi de sept nettoyages globaux**
(`_remove_leftover_hi`), et d'une suppression des sous-titres vidés.

**Le découpage** mesure chaque candidat par `length_func`, et **les motifs n'y
sont appliqués qu'une fois par texte**, pour placer leurs pénalités. Le point
chaud est la mesure : en *ems*, c'est la largeur naturelle d'un `Gtk.Label` de
la police par défaut, rapportée à une em calibrée pour que l'alphabet
minuscule vaille 0,55 em par lettre.

**Le correcteur** passe par libspelling, donc par Enchant, qui choisit le
dictionnaire et tient la liste de mots personnelle. Par-dessus : une liste de
remplacements `mot|remplacement` par langue, dans la configuration de Gaupol ;
des heuristiques **pour l'anglais seulement** (`-in'` pour `-ing`, suffixes
`'s 'll 're 've 'd`, ordinaux) ; deux suggestions ajoutées en tête pour l'OCR
(`I` → `l`, chiffre collé à une unité). Un découpage en mots qui s'avoue
« unlikely to work well for all languages ».

**L'assistant** enchaîne ses pages — tâches et cible, mentions, jonction et
scission (absente sans correcteur), erreurs courantes, majuscules, découpage et
ses réglages, progression, **confirmation** —, calcule tout sur une copie de
chaque projet, présente chaque texte changé avec son original, se coche, se
décoche, se modifie, et n'applique que ce qui est accepté : **une entrée
d'historique par projet**, puis « Edited N and removed M subtitles ».

## D1 — Le moteur : ICU

**Tranché par l'[ADR 0036](../adr/0036-icu-pour-les-motifs-de-correction.md)**,
sur une mesure jetable dont elle porte les chiffres : PCRE2 compilé à la volée
est 2,6 fois plus rapide, pour une différence de 0,13 s à 0,05 s par film
qu'aucun utilisateur ne verra ; ICU est déjà au noyau, déjà le moteur de la
recherche, et ne change pas de comportement d'une distribution à l'autre.

**La fidélité ne départage pas non plus, et c'est la découverte du cadrage.**
Ni ICU ni PCRE2 ne lisent `\w` comme Python dès qu'une lettre est écrite en
forme décomposée — ICU jamais, PCRE2 à partir de sa version 10.43. Réécrire
`\w`, `\W`, `\b` et `\B` au chargement ramène ICU à zéro écart avec Python, sur
les quatre types de motifs et les 18 563 sous-titres du corpus privé.

**Le moteur se cache derrière une interface du noyau**, `PatternEngine` ou son
équivalent, avec une seule implémentation. C'est l'exception que les principes
de conception admettent : la variation est identifiée — l'ADR dit dans quel cas
PCRE2 reviendrait —, et c'est l'interface qui rendrait ce retour local.

**Un motif ne tourne jamais sans borne.** `RegexMatcher::setTimeLimit` arrête un
retour arrière catastrophique ; la correction de ce texte par ce motif est
abandonnée, et dite.

## D2 — Le format : celui de Gaupol, lu tel quel

**Tranché par l'[ADR 0037](../adr/0037-lire-les-motifs-de-gaupol-tels-quels.md).**
Les fichiers livrés sont ceux de Gaupol, sans retouche, et passent de
`src/test/data/motifs/gaupol/` à **`packaging/patterns/`** : une seule copie, que
le paquet installe, que l'oracle lit, et que les tests lisent. Ils s'installent
dans `share/subedit/patterns`, trouvé **à partir de l'exécutable** comme le
manuel. Les motifs de l'utilisateur se lisent dans
`$XDG_DATA_HOME/subedit/patterns`, après les livrés, comme chez Gaupol.

**L'activation se garde dans nos réglages**, par type, code et nom ; les `.conf`
livrés en sont les valeurs par défaut. Un réglage qui nomme un motif disparu
est ignoré, comme Gaupol ignore un `.conf` périmé — et comme l'ADR 0022 le fait
option par option.

**Le modèle est typé.** Un enregistrement est un `CorrectionPattern` : un type
énuméré (`CommonError`, `Capitalization`, `HearingImpaired`, `LineBreak`), des
classes énumérées, des drapeaux énumérés, et les champs propres à chaque type
— `Repeat` pour les erreurs courantes, `Capitalize` pour les majuscules,
`Group` et `Penalty` pour le découpage. Pas de dictionnaire de chaînes : c'est
le contre-exemple que les principes de conception citent, et c'est précisément
celui de Gaupol.

## D3 — Les remplacements : convertis au chargement, appliqués à la manière de Gaupol

**Le gabarit de Python est converti une fois**, au chargement, en une suite de
morceaux — texte littéral ou numéro de groupe. Tout ce que Python y lit s'y
traduit : `\1` à `\99`, `\g<n>`, `\g<nom>`, les octaux (`\040` est une espace,
`\060` un zéro), les échappements `\n` et `\t` ; le `\0` isolé a déjà disparu à
la lecture, comme chez Gaupol. Le gabarit converti ne dépend d'aucun moteur :
ce n'est ni le `$1` d'ICU ni le `\1` de Python, c'est une structure.

**L'application est celle du `Finder`, et pas un « remplacer tout ».** #494 l'a
mesuré : la différence ne se voit que dans le nettoyage qui suit le retrait des
mentions, et deux cas la montrent — mais un moteur qui l'ignorerait les
échouerait. La boucle est écrite au noyau, une fois, sur l'interface de D1 :
chercher à partir de la fin du dernier remplacement dans le texte modifié,
avancer d'un caractère après une correspondance vide répétée.

**`Repeat` s'arrête quand le texte ne change plus**, et non quand une passe ne
trouve plus rien. **Partout où Gaupol termine, le résultat est identique** : une
passe qui trouve sans rien changer rendrait la suivante identique, et Gaupol ne
terminerait pas. Là où Gaupol boucle sans fin, `subedit` s'arrête. Une borne
tient en plus le cas d'un motif qui oscille entre deux textes : **cent passes**,
au-delà desquelles le motif est abandonné pour ce texte, et dit.

## D4 — Humain et OCR : un vrai filtre

**Décocher une classe retire ses motifs de l'application**, et pas seulement de
la liste. Le comportement de Gaupol — des motifs masqués qui s'appliquent quand
même — n'est défendable par aucune lecture de son interface : une case
« OCR » décochée dit « ne corrige pas les erreurs d'OCR ». C'est un écart
voulu, inscrit plus bas.

Un enregistrement qui porte les deux classes s'applique si l'une des deux est
cochée — la règle de Gaupol pour l'affichage, qui devient celle de
l'application.

## D5 — La mesure : injectée, et en cache

**Une interface du noyau, `LineMeasure`, deux implémentations.**

| Implémentation | Où | Pour qui |
| :------------- | :- | :------- |
| en caractères : points de code, balises retirées, saut de ligne compté comme une espace | noyau | les tests, la ligne de commande de la phase 13, et l'unité « caractères » de la fenêtre |
| en *ems* : la largeur de la police de l'application, par `QFontMetricsF`, rapportée à une em calibrée comme Gaupol — l'alphabet minuscule vaut 0,55 em par lettre | `gui` | l'unité « ems » de la fenêtre |

**Le calibrage de Gaupol se reprend tel quel**, avec sa bizarrerie : l'em n'est
pas celle de la typographie, c'est une unité où une lettre moyenne vaut 0,55.
Les défauts de Gaupol — **24 de long, 3 lignes, en ems** — n'ont de sens que
dans cette unité-là, et les garder demande de garder l'unité.

**Ce qui se met en cache : la longueur d'une chaîne, le temps d'un découpage.**
Le découpeur mesure chaque ligne de chaque candidat, et les mêmes lignes
reviennent d'un candidat à l'autre. Une table chaîne → longueur, vivant le temps
d'une opération sur un document, suffit ; elle ne survit pas à l'opération, pour
qu'un changement de police ne rende jamais une longueur périmée. Gaupol
mémorise par identité d'objet — #495 l'a appris à ses dépens —, ce qu'une table
par valeur n'a pas à reproduire.

**Les tests de la mesure en ems** ne font foi que sous les réglages de
l'[ADR 0024](../adr/0024-captures-engendrees-et-ou-elles-font-foi.md) :
plateforme sans écran, `DejaVu Sans`. Ailleurs, ils éprouvent le découpeur avec
une mesure de test à largeurs fixes, qui dit ce que le calcul fait d'une
longueur sans dépendre d'une police.

## D6 — Le correcteur : Enchant, et une fonction qui s'éteint sans dictionnaire

**Enchant 2**, la bibliothèque que Gaupol emploie à travers libspelling. Trois
raisons, dont la première tranche :

- **la liste de mots personnelle est celle de Gaupol.** Enchant la tient dans
  son propre répertoire, commun à toutes les applications qui s'en servent :
  les mots qu'un utilisateur a ajoutés dans Gaupol sont reconnus par `subedit`
  dès le premier jour, sans importation. C'est l'iso-fonctionnalité au sens le
  plus littéral — et c'est un utilisateur de Gaupol qui a demandé ce programme.
  **Lu dans le code, pas encore observé** : libspelling ajoute un mot par le
  dictionnaire d'Enchant, et l'issue du correcteur le vérifie sur une machine
  où Gaupol a ajouté un mot, avant que la décision ne soit bâtie dessus ;
- **les dictionnaires sont ceux du système**, quel qu'en soit le moteur —
  hunspell, nuspell, aspell, voikko pour le finnois —, sans qu'on écrive leur
  découverte ;
- une API C stable, empaquetée partout, Windows compris.

**hunspell directement**, que la phase 0 nommait, est écarté : il faudrait
écrire la recherche des dictionnaires, tenir une liste personnelle à nous — que
les mots de Gaupol n'atteindraient pas — et renoncer aux langues qu'il sert mal.
**nuspell** : même dictionnaires, même manques, empaquetage plus rare. **Un
service système** : il n'y en a pas de portable.

**Derrière une interface du noyau, `SpellChecker`**, et un double pour les tests,
dont #492 avait renvoyé la forme ici : un dictionnaire écrit dans le test, une
liste de mots et leurs suggestions. Aucun test ne dépend des dictionnaires
installés sur la machine qui le lance, par la règle qui interdit déjà de lire
`reference/`.

**Sans dictionnaire, le programme fonctionne et la fonction s'éteint**, comme on
a fait sans `ffprobe` : l'entrée `Check Spelling…` et la page de jonction et de
scission restent visibles, **grisées**, et disent pourquoi — « no dictionary for
fr » —, là où Gaupol retire la page sans un mot. Enchant absent au lancement ne
se produit pas : c'est une dépendance du paquet.

**Ce qu'on reprend du correcteur de Gaupol, à l'identique** : les heuristiques
anglaises, les deux suggestions d'OCR, la liste de remplacements par langue —
dans notre répertoire de configuration, `spell-check/<langue>.repl`, sans lire
celle de Gaupol —, et son découpage en mots. Celui-ci s'avoue imparfait ; il
reste celui de Gaupol tant qu'un cas ne le prend pas en défaut, parce que la
jonction et la scission en dépendent directement.

**La dépendance est la seule de la phase**, et elle se paie trois fois :
`find_package` au CMake, `Depends` du `.deb` et `Requires` du `.rpm`, et
l'installation dans la CI et dans `fedora.yml`. `subedit-cli` la tire, puisque le
correcteur est au noyau et que la phase 13 joindra et scindera des mots en
ligne de commande.

## D7 — Les mentions de la phase 4 : le balayage reste, le moteur prend le reste

**Le balayage écrit à la main garde les crochets et les parenthèses.** L'ADR
0017 avait posé son déclencheur, et il a joué ; mais sa raison tient toujours :
la couture est **locale au site du retrait**, et une passe d'expression la
rendrait globale. Les trente-neuf cas de `mentions.cas` décrivent une règle
meilleure que les sept nettoyages de Gaupol, et le programme la tient depuis la
phase 4.

**Les quatre autres enregistrements passent par le moteur** — paroles entre
dièses, sur plusieurs lignes et sur une, nom du locuteur avant deux-points, en
capitales ou non —, **suivis des nettoyages de Gaupol**, parce que c'est ce que
les attendus de #494 décrivent et que ces motifs-là n'ont pas de règle à nous.

**Dans l'assistant, la page des mentions montre les six noms de Gaupol.**
« Sound in brackets » et « Sound in parentheses » y commandent le balayage ; les
quatre autres, le moteur. Un motif de mentions déposé par l'utilisateur passe
par le moteur. Les six sont **décochés par défaut**, comme chez Gaupol.

**L'entrée `Remove Hearing-Impaired Mentions…` de la phase 4 ne change pas**, ni
la sous-commande `hearing-impaired` : elles retirent crochets et parenthèses,
comme avant. L'assistant est l'endroit des six motifs ; l'entrée directe est le
geste court qu'elle était.

**Ce que la phase 4 a déjà dit et qui reste vrai** : la règle vit derrière une
fonction libre, `withoutHearingImpaired`, qu'aucune de ces décisions ne touche.

## D8 — L'assistant, et ce que la ligne de commande en attend

**Un assistant comme celui de Gaupol**, `Tools ▸ Correct Texts…`, dans le même
ordre de pages, et d'abord **la page de confirmation** : c'est elle qui fait de
la correction automatique une proposition plutôt qu'un fait accompli. Chaque
texte changé s'y montre avec son original, la différence marquée ; il
s'accepte, se refuse ou se retouche ; `Mark All` et `Unmark All` cochent tout
ou rien ; « Remove all blank subtitles » est une case, cochée par défaut.
`Preview`, qui lance chez Gaupol un lecteur externe au sous-titre choisi, place
ici le lecteur intégré sur ce sous-titre quand le projet a une vidéo — la
réponse que la phase 11 a déjà donnée à la barre d'outils.

**Le calcul et l'application sont au noyau**, la fenêtre ne fait que montrer :
une fonction prend les tâches, leurs réglages, les projets et la cible, et rend
la liste des changements — projet, sous-titre, texte d'origine, texte proposé —
**sans toucher à aucun projet** ; une autre applique une liste acceptée, **une
commande composée par projet**, donc une entrée d'historique par projet. C'est
ce qui rend l'assistant testable sans fenêtre, et la phase 13 écrivable sans
lui. Gaupol copie chaque projet pour y faire ses calculs ; on n'a pas à le
faire, puisque les corrections sont des fonctions de textes.

**La cible est celle de Gaupol** — la sélection, le projet courant, tous les
projets ouverts — **et le document se choisit** sur la première page, texte ou
traduction, la traduction n'étant offerte que si un projet en porte une. Le
choix de la phase 11 — le document de la colonne courante — ne s'applique pas
ici : un assistant de plusieurs pages ne se lance pas « depuis une colonne ».

**Le compte rendu compte des textes changés**, jamais des correspondances : #492
l'a montré, `'[Il][Il]\b` → `'ll` correspond à un texte juste et le rend tel
quel. La phrase vit dans `core/wording.hpp`, puisque la phase 13 dira la même.

**Ce que l'assistant retient** : les tâches cochées, la cible, le document,
l'écriture, la langue et le pays de chaque type, les classes, les réglages du
découpage, la case des sous-titres vides — dans les réglages de l'ADR 0022.

**La ligne de commande attend la phase 13**, comme la traduction l'attendait à
la phase 11. Rien de ce que la phase écrit n'est propre à la fenêtre, sauf la
mesure en ems et les pages elles-mêmes : la phase 13 trouvera le calcul, les
motifs, le correcteur et les phrases au noyau, et n'aura qu'une grammaire à
écrire — `correct`, avec ses tâches, sa langue, ses réglages de découpage en
caractères. La confirmation, elle, n'a pas d'équivalent en ligne de commande ;
la phase 13 dira si un mode qui écrit les changements proposés sans les
appliquer la remplace.

## D9 — Les balises : le parseur de la phase 10, par `transform`

La question du cadrage — le parseur tient-il les décalages que les motifs
demandent ? — a une réponse en deux temps.

**Oui pour le mécanisme.** Une correction passe par `MarkupParser::transform`,
et non par `replace` : `replace` est la moitié faite pour ce qu'un utilisateur
tape, qui prend le style de tout ce qu'il touche ; une correction est une
transformation, qui ne crée ni n'étend aucun style. `transform` sait déjà
remplacer un morceau par un autre de longueur différente et décaler ce qui suit.

**Non pour la fidélité, tant qu'un cas ne l'a pas montré.** Le parseur de Gaupol
place autrement une balise **à l'intérieur** d'une correspondance : il la
ramène au début du morceau retiré, et devine si une balise au bord est ouvrante
ou fermante à l'espace qui la suit. Le nôtre la garde « aussi loin que le
nouveau texte va », et supprime un style que la correction a vidé, là où Gaupol
laisse les balises vides. Aucun des 184 cas de #494 ne porte de balise :
**l'écart n'est pas mesuré**.

**Donc : une issue de la première tranche porte l'oracle aux balises** — le
parseur de Gaupol porté dans `pattern-oracle.py` pour les balises de SubRip,
des cas balisés écrits à la main là où une correction touche une balise — et
**décide cas par cas** : un désaccord est corrigé dans `transform`, ou inscrit
comme écart avec sa raison. Le seul écart déjà voulu est celui d'une balise
vidée : un `<i></i>` laissé dans un fichier n'est un progrès pour personne.

## D10 — Pas de porte

L'issue demandait une porte, **comme #435 pour le multi-projets**, si une partie
de la phase se révélait trop lourde. Aucune ne l'est au point d'en demander une.
#435 portait une réserve que la feuille de route avait écrite — « le besoin
n'est pas confirmé » ; ici, chaque morceau est une fonction de Gaupol que
l'utilisateur de départ emploie, sans réserve écrite.

**Ce qui aurait pu en justifier une, et pourquoi non** : le correcteur, pour sa
dépendance et son dialogue. La dépendance est un paquet présent sur toute
distribution qui porte Gaupol ; le dialogue est le plus petit de la phase. Il
vient **en dernier**, ce qui suffit : si la phase devait s'arrêter plus tôt, ce
qui serait livré tiendrait seul.

## Ce que la phase ne livre pas

Chacun avec une phase ou une issue, parce qu'un renvoi sans destinataire finit
par désigner une phase déjà passée.

- **La ligne de commande de la correction** — `correct`, la jonction et la
  scission en ligne de commande : phase 13, D8.
- **La traduction des noms et descriptions des motifs** — gettext chez Gaupol :
  phase 15, avec le reste de l'interface.
- **La vérification orthographique au fil de la frappe** — Gaupol l'offre dans
  sa cellule d'édition (`spell_check.inline`). Elle demande un surlignage dans
  le délégué de la table, que rien n'a encore : **renvoyée à la relecture de fin
  de phase**, qui dira si elle devient une issue.
- **Un éditeur de motifs** — Gaupol n'en a pas ; on dépose un fichier.
- **La longueur des lignes affichée dans les cellules** — la règle de Gaupol
  (`gaupol/ruler.py`, sa marge) ; elle emprunterait la mesure de D5, mais c'est
  une fonction d'édition, pas de correction : **renvoyée à la relecture**, comme
  la précédente.

## Écarts avec Gaupol

| Ce que fait Gaupol | Ce que fait `subedit` | Pourquoi |
| :----------------- | :-------------------- | :------- |
| `\w` et `\b` à la manière de Python | **à l'identique**, par réécriture | sans elle, ni ICU ni un PCRE2 récent ne rendent les mêmes textes — D1 |
| `Repeat` boucle tant qu'une passe trouve quelque chose, **sans fin possible** | tant que le texte change, **cent passes au plus**, et dit | identique partout où Gaupol termine — D3 |
| un motif qui retourne en arrière sans fin **gèle le programme** | **arrêté**, abandonné pour ce texte, et dit | D1 |
| une classe décochée **s'applique quand même** | décochée, elle **ne s'applique pas** | la case dit ce qu'elle dit — D4 |
| l'activation s'écrit en `.conf` XML | dans **nos réglages** | l'ADR 0022, [0037](../adr/0037-lire-les-motifs-de-gaupol-tels-quels.md) |
| crochets et parenthèses retirés par motif, puis **sept nettoyages globaux** | par le **balayage de la phase 4**, à couture locale | l'ADR 0017, maintenue — D7 |
| la page de jonction et de scission **disparaît** sans correcteur | **grisée**, et dit pourquoi | dire plutôt que taire, l'ADR 0008 — D6 |
| la liste de remplacements dans la configuration **de Gaupol** | dans **la nôtre** | un programme n'écrit pas chez un autre — D6 |
| une balise vidée par une correction **reste** | **retirée** | D9 ; les autres placements se décident cas par cas |

## Exigences

**Douze, toutes `prévues`** — le registre s'alimente en début d'issue.

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `GUI-CORRECT-01` | l'assistant applique les tâches cochées à la cible choisie — sélection, projet, tous les projets — et au document choisi |
| `GUI-CORRECT-02` | chaque texte changé se montre avec son original avant d'être appliqué, et s'accepte, se refuse ou se retouche |
| `GUI-CORRECT-03` | appliquer fait une entrée d'historique par projet, et la barre d'état dit combien de sous-titres ont été modifiés et supprimés |
| `GUI-CORRECT-04` | les motifs se cochent par nom, pour une écriture, une langue et un pays, et le choix est retenu |
| `GUI-CORRECT-05` | décocher Humain ou OCR retire ces motifs de l'application |
| `GUI-CORRECT-06` | un motif qui ne se lit pas, ne se traduit pas ou ne termine pas est nommé, et les autres s'appliquent |
| `GUI-CORRECT-07` | les motifs déposés par l'utilisateur s'ajoutent aux motifs livrés |
| `GUI-HEARING-03` | l'assistant retire les paroles entre dièses et le nom du locuteur, en plus des crochets et des parenthèses |
| `GUI-BREAK-01` | le découpage tient la longueur et le nombre de lignes demandés, en caractères ou en ems, et saute les sous-titres qui les tiennent déjà |
| `GUI-SPELL-01` | la vérification parcourt les mots inconnus : ignorer, tout ignorer, ajouter au dictionnaire, remplacer, tout remplacer, joindre au précédent ou au suivant |
| `GUI-SPELL-02` | sans dictionnaire pour la langue choisie, les fonctions du correcteur sont grisées et disent pourquoi |
| `GUI-SPELL-03` | l'assistant joint et scinde des mots selon le correcteur |

**Le noyau n'a pas d'exigence**, par la règle du registre : les cas de #494 et
#495 le tiennent, et ce sont eux que ses tests liront.

## Découpage

**Quatre tranches**, du noyau vers la fenêtre, et le correcteur en dernier —
D10.

| Tranche | Issue | Ce qu'elle fait | Ce dont elle dépend |
| :------ | :---- | :-------------- | :------------------ |
| 1 — le moteur | [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) | les motifs livrés passent dans `packaging/patterns/` et s'installent ; leur lecteur, la cascade des codes, l'activation par défaut, les motifs de l'utilisateur | — |
| | [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) | la traduction de syntaxe, le gabarit, la boucle du `Finder`, les erreurs courantes ; **les 184 cas passent** ; le banc permanent | [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) |
| | [#500](https://github.com/Guyot-Bertrand/sub-edit/issues/500) | la majuscule qui traverse les sous-titres, les quatre motifs de mentions et leurs nettoyages | [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) |
| | [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501) | les textes balisés : l'oracle porte le parseur de Gaupol, des cas balisés, et `transform` décide cas par cas | [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) |
| 2 — le découpage | [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502) | la mesure injectée, en caractères ; le découpeur de Gaupol porté ; **les 26 découpages passent** | [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) |
| | [#503](https://github.com/Guyot-Bertrand/sub-edit/issues/503) | la mesure en ems, par la police de l'application, et son cache | [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502) |
| 3 — l'assistant | [#504](https://github.com/Guyot-Bertrand/sub-edit/issues/504) | au noyau : calculer les changements de plusieurs tâches sur plusieurs projets sans les toucher, appliquer une liste acceptée, le compte rendu ; les réglages | [#500](https://github.com/Guyot-Bertrand/sub-edit/issues/500), [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502) |
| | [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505) | la fenêtre : `Correct Texts…`, ses pages de tâches, de mentions, d'erreurs courantes, de majuscules, et la confirmation | [#504](https://github.com/Guyot-Bertrand/sub-edit/issues/504), [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501) |
| | [#506](https://github.com/Guyot-Bertrand/sub-edit/issues/506) | les pages du découpage et de ses réglages | [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505), [#503](https://github.com/Guyot-Bertrand/sub-edit/issues/503) |
| 4 — le correcteur | [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507) | Enchant au noyau, derrière `SpellChecker`, et son double ; les heuristiques et la liste de remplacements ; l'absence de dictionnaire ; la dépendance empaquetée | — |
| | [#508](https://github.com/Guyot-Bertrand/sub-edit/issues/508) | joindre et scinder des mots, au noyau et dans l'assistant | [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507), [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505) |
| | [#509](https://github.com/Guyot-Bertrand/sub-edit/issues/509) | `Check Spelling…`, le dialogue | [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507) |

La phase se clôt sur la relecture de fin, ouverte après la dernière issue.

**[#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) est la pierre d'angle**, et la plus grosse de la phase : c'est elle qui
confronte le moteur aux attendus, et un désaccord qu'elle trouverait remettrait
en cause l'ADR 0036 avant que rien ne soit bâti dessus. **[#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507) ne dépend de
rien** et peut se faire à tout moment ; elle est placée en tête de sa tranche
parce que la dépendance qu'elle ajoute touche l'empaquetage, qu'il vaut mieux
éprouver tôt que tard. **[#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505) est la plus grosse de la fenêtre** : un assistant
de plusieurs pages, dont la confirmation est une table éditable à deux textes.
