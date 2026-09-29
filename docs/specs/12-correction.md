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

> **Précisé par [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499).** Deux bornes
> s'ajoutent aux cent passes, parce qu'un motif dont le remplacement double ce qu'il trouve, sous
> `Repeat`, remplirait la mémoire avant la centième : **un texte ne dépasse pas 16 384 octets**
> pendant qu'un motif y travaille, et **chaque recherche est bornée par le temps du moteur**
> (`kSearchLimit`, quelques dixièmes de seconde sur un motif catastrophique). Un motif qui
> dépasse l'une des trois laisse le texte **tel qu'il était avant lui** — un remplacement à
> moitié fait vaut moins que pas de remplacement —, et le dit, en nommant le texte. Le moteur
> lit en outre `\n` seul comme fin de ligne (`UREGEX_UNIX_LINES`), ce que `.`, `^` et `$` savent
> de la ligne dans `re`.

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

> **Précisé par [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502).**
> `LineMeasure` (le noyau) et `CharacterLineMeasure` (`get_char_length` :
> points de code, `\n` compté comme n'importe quel caractère puisqu'un compte
> de points de code ne voit pas la différence). `CachedLineMeasure` enveloppe
> n'importe quel `LineMeasure` d'une table chaîne → longueur — **injectée**,
> l'appelant décide de l'envelopper ou non, comme il choisit le moteur des
> motifs. Le découpeur (`Liner` de Gaupol, porté ligne à ligne depuis l'oracle
> déjà confronté au vrai Gaupol par #495) ne connaît que l'interface : les 26
> cas de `line-break.cas` passent sans écart, `essai` compris. **Le cache ne
> se voit pas au banc pour la mesure en caractères** — compter des points de
> code est déjà trop bon marché pour qu'une table de hachage rembourse sa
> propre lecture ; les deux chiffres sont dans la PR, et
> `line_breaking_bench.cpp` explique pourquoi ils se ressemblent. C'est la
> mesure en *ems* de #503, derrière `QFontMetricsF`, qui a une vraie raison
> d'être mise en cache — la même interface la portera sans rien changer au
> découpeur.

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

> **Précisé par [#503](https://github.com/Guyot-Bertrand/sub-edit/issues/503).**
> `EmsLineMeasure`, dans `gui`, reçoit sa police au constructeur — jamais
> `QApplication::font()` d'elle-même, la même règle que `CachedLineMeasure` :
> l'appelant passe la police de l'application quand c'est ce qu'il veut dire,
> un test passe `DejaVu Sans`. Le calibrage mesure l'alphabet minuscule une
> fois, `QFontMetricsF::horizontalAdvance`, rapporté à 0,55 × 26. Les tests
> refusent de conclure sous une police de remplacement, comme
> `subedit_screenshots` refuse de photographier — `iiii` et `MMMM` diffèrent
> en ems, un `breakLines` sur les deux le montre.
>
> **Le banc a exigé une `QApplication` que `subedit_bench` n'avait pas** :
> `QFontMetricsF` ne répond pas sans base de polices. Le binaire porte
> désormais son propre `main`, sous `offscreen`, exactement le motif déjà
> écrit pour `subedit_gui_test` — un seul binaire, comme le veut
> `record-bench.sh`, et les deux bancs `gui/` déjà là, qui s'en passaient, y
> gagnent sans rien perdre. Le cache s'y voit, cette fois : c'est ce que #502
> avait annoncé.

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

> **Précisé par [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507).**
> **Observé, non plus seulement lu** : sur cette machine, le `SpellChecker` de
> Gaupol (1.11, par Gspell) écrit un mot ajouté dans `fr_FR.dic`, dans la
> configuration d'Enchant, et `enchant-2` le tient aussitôt pour correct — pas
> un seul répertoire de Gaupol, pas d'importation. Le dépôt amont passe par
> libspelling, qui aboutit à `enchant_dict_add` de la même façon ; les deux
> moteurs de Gaupol atteignent la même liste. L'observation a eu lieu dans un
> répertoire de configuration temporaire, jamais dans celui de l'utilisateur.
>
> **Deux interfaces, et `SpellChecker` n'en est pas une.** `SpellDictionary`
> (vérifier, suggérer, ajouter à la liste personnelle) et `SpellProvider`
> (les langues, ouvrir un dictionnaire) sont abstraites ; `SpellChecker` est
> une classe, ce que Gaupol pose par-dessus un dictionnaire — les heuristiques
> anglaises, les deux suggestions d'OCR, la liste de remplacements, les mots
> ignorés le temps de la session. Ce dessus est identique pour tout
> dictionnaire et c'est lui que les tests visent : le double
> (`WordListSpellProvider`, au noyau comme `InMemoryFileSystem`) est un
> dictionnaire écrit dans le test, et `EnchantSpellProvider` l'implémentation
> réelle, qui seule inclut `enchant.h`.
>
> **La réponse « pas de dictionnaire » est un `std::unexpected`** —
> `NoDictionary{langue}` — et sa phrase, `noDictionaryFor`, vit dans
> `core/wording.hpp` : « no dictionary for fr », que la fenêtre montrera.
>
> **Écarts de Gaupol, et ils sont dits** : une ligne d'un `.repl` sans barre
> verticale est ignorée, là où Gaupol la lit comme un couple à un seul
> élément qui échoue à la première suggestion ; les blancs d'une ligne sont
> retirés à l'ASCII, non à Unicode. **Conservé tel quel, défaut compris** : la
> classe `[0,4-9]` des ordinaux anglais contient une virgule que Gaupol n'a
> sans doute pas voulue — le découpage en mots n'en livre jamais une, elle ne
> compte que pour un appel direct. Le découpage en mots suit `\w` de Python par
> `u_isalnum` et le tiret bas d'ICU ; les positions rendues sont des octets
> UTF-8, non des points de code.
>
> **Le répertoire de configuration est un argument**, jamais résolu au noyau :
> `spellReplacementFile(répertoire, langue)` rend
> `spell-check/<langue>.repl`, et c'est l'appelant — la fenêtre, une issue plus
> tard — qui sait où il est. Le seul test qui touche le vrai Enchant déplace
> `ENCHANT_CONFIG_DIR` dans un répertoire à lui, et `check-config-home.sh`
> surveille désormais `~/.config/enchant` avec `~/.config/subedit`.
>
> **La dépendance** : `pkg_check_modules(enchant)` au CMake, `libenchant-2-2`
> au `.deb`, `enchant2` au `.rpm`, `libenchant-2-dev` dans la liste de
> `setup-toolchain.sh` — que la CI et `fedora.yml` lisent, sans seconde liste —
> et `check-installation.sh` vérifie que chacun des deux paquets la nomme.

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

> **Précisé par [#500](https://github.com/Guyot-Bertrand/sub-edit/issues/500).**
> L'ordre retenu au noyau, `correctHearingImpaired` : le balayage d'abord, puis
> les quatre motifs du moteur dans l'ordre de la cascade, chacun une seule fois
> — `HearingImpairedFields` ne porte pas de `Repeat` —, puis les sept
> nettoyages de Gaupol **si et seulement si un motif du moteur a changé le
> texte** ; le balayage n'en a pas besoin, sa propre couture étant déjà locale.
> **Deux cas de `hearing-impaired.cas` restent hors accord, sciemment** : une
> ligne réduite à un guillemet seul, à côté d'une mention retirée par le
> balayage. Les sept nettoyages de Gaupol la videraient (`^\W*$` puis la ligne
> vide) ; le balayage, fidèle à sa couture locale, la laisse. C'est l'écart que
> l'ADR 0017 avait annoncé, mesuré ici pour la première fois — `mentions.cas`
> ne le montrait pas, faute d'un cas où la ligne touchée ne portait qu'une
> ponctuation déjà sans rapport avec la mention.

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

> **Précisé par [#504](https://github.com/Guyot-Bertrand/sub-edit/issues/504).**
> Quatre tâches, pas cinq : `CorrectionTask` — mentions, erreurs courantes,
> majuscules, découpage — sans jonction ni scission, absentes du noyau tant
> qu'aucun correcteur n'existe pour les nourrir (D6, #508). L'ordre est celui
> de Gaupol, et un sous-titre qu'une mention vide ne joue aucun rôle dans les
> tâches qui suivent — les tâches qui suivent ne le voient jamais.
>
> **`proposeCorrections` ne résout pas la cible.** Elle prend une liste de
> `CorrectionTarget{project, selection, document}`, déjà décidée — la
> sélection, le projet courant ou tous les projets ouverts devenus une liste
> concrète est le travail de qui appelle, une fenêtre plus tard (#505). Une
> deuxième fonction, `applyCorrections`, prend un sous-ensemble accepté et
> rend un `CompositeCommand` par projet, appliqué mais pas encore inscrit à
> un historique — inscrire l'entrée reste au même appelant, qui tient les
> historiques.
>
> **La règle de #431 sur les traductions traverse jusqu'ici** : une mention
> qui viderait une traduction l'écrit vide plutôt que de retirer le
> sous-titre — `removeHearingImpaired` (phase 4) le faisait déjà pour son
> geste direct, `correctHearingImpaired` (l'assistant) ne le savait pas
> encore, puisqu'elle ignore tout de `Document`. C'est `proposeCorrections`
> qui porte cette règle désormais, une fois pour les deux commandes.
>
> **D4, mesuré et non supposé** : un enregistrement d'erreurs courantes
> s'applique si l'une de ses classes est cochée, aucun des trois autres types
> n'en porte. Les deux motifs de crochets et de parenthèses (D7) ne sont
> commandés que par une seule case au noyau — l'une **ou** l'autre coche
> suffit à lancer le balayage — plutôt que par deux, `withoutHearingImpaired`
> ne sachant traiter les deux styles qu'ensemble ; les distinguer vraiment
> resterait à faire si une issue le demandait.
>
> **Les réglages** (`CorrectionSettings`, dans les réglages de l'ADR 0022,
> seizième bloc après celui du découpage de durées) : une tâche cochée et son
> code par type, les deux classes, les deux cases du balayage, une liste
> ouverte de dérogations d'activation — `<type>:<code>:<nom>:<0|1>`, dans une
> seule clé à liste — les deux réglages du découpage et la case des
> sous-titres vides. Une dérogation qui ne nomme plus rien n'est jamais lue,
> ce qui suffit à l'ignorer sans code dédié — la table des motifs ne connaît
> qu'elle-même.
>
> **`CommandKind::CorrectTexts`** rejoint l'énumération ; `noticeOfCorrection`
> rejoint `core/wording.hpp`, le patron même de Gaupol — « Edited N and
> removed M subtitles » — et compte les textes, jamais les correspondances,
> par construction : une correspondance qui rend un texte inchangé ne franchit
> jamais `proposeCorrections`.

> **Précisé par [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505).**
> Les trois menus d'une page de tâche — écriture, langue, pays — se peuplent de
> ce que le catalogue de motifs porte réellement pour ce type, jamais d'un
> registre de langues externe : `PatternCodeSelector` cascade sur `catalogue`,
> pas sur une liste ISO.
>
> **La partie changée d'une diff se marque en gras, jamais en couleur** —
> `correctionDiffHtml` pose `<b>` autour de chaque `DiffSpan` marqué changé.
> Une teinte se serait mal comportée sous les deux palettes, et de toute façon
> `QStyledItemDelegate` peint le `Qt::DisplayRole` d'une cellule, pas ses
> couleurs de police par plage.
>
> **`applyCorrections` compose sans appliquer**, plutôt que d'ajouter à
> `Session`/`History` une méthode qui enregistrerait une commande déjà
> appliquée : elle rend un `CompositeCommand` par projet, et c'est l'appelant
> — le contrôleur de la fenêtre — qui le fait passer par `Session::apply`,
> comme toute autre opération. La raison tient en une règle : `Session::project()`
> rend une référence `const`, et une commande appliquée hors de `Session::apply`
> l'aurait contournée.
>
> **« Abandonner » est le bouton `Cancel` du wizard, sans mécanisme
> d'interruption propre** : un calcul déjà lancé sur son fil d'arrière-plan
> n'est jamais coupé, seulement jamais lu — **mais la page de progression
> attend sa fin avant d'être détruite**. Ce que le calcul référence — le
> moteur, le catalogue, les projets de ses cibles — n'a donc qu'à survivre à
> la page, jamais au fil : l'annulation bloque, au plus le temps que le moteur
> accorde à une recherche. `Back` pendant le calcul le laisse finir sans
> l'attendre, et son résultat est écarté ; `Next` en relance un.
>
> **La cible et le document ne se retiennent pas** d'une session à l'autre —
> D8 le disait, le périmètre l'a resserré : la première page rouvre toujours
> sur `Current Project` et `Text`. **L'unité du découpage, elle, ne se
> retenait pas non plus, et #506 l'a ajoutée** — voir plus bas. Ne se retient
> que ce que `CorrectionSettings` porte — les tâches cochées et leurs codes,
> les classes, les dérogations d'activation, les réglages du découpage, la
> case des sous-titres vides.
>
> **Une dérogation d'activation disparaît quand sa case revient au défaut**,
> au lieu de rester écrite : pour chaque motif qu'une page montre, la
> dérogation retenue est effacée avant que celles qui s'écartent encore du
> défaut ne soient réécrites (`foldActivations`). Changer de code sur une page
> y replie d'abord l'état des cases du code quitté.
>
> **GUI-CORRECT-06 couvre les trois cas** : un motif qui ne se traduit ou ne compile pas, ou
> ne termine pas (`PatternFailure`), et une ligne de fichier qui ne se lit pas
> (`PatternCatalogue::diagnostics()`), nommés sous la même table.
>
> **Une page de tâche ne porte pas de case « active » qui lui soit propre** :
> c'est celle de la première page, par tâche, qui décide seule si la tâche
> tourne. Une seconde case ici aurait pu la contredire.

> **Précisé par [#506](https://github.com/Guyot-Bertrand/sub-edit/issues/506).**
> **Le saut de Gaupol a ses propres seuils**, et #502 les avait confondus avec
> les limites visées : `max_skip_length` et `max_skip_lines` sont deux
> réglages, chacun avec sa case (`use_skip_max_length`, `use_skip_max_lines`),
> tous deux à `24` et `3` par défaut — d'où la confusion, invisible tant que
> personne ne les change. `breakLines` prend désormais `std::optional<SkipLimits>` :
> absent, aucun saut ; présent, une case décochée est un seuil infini, comme le
> `32768` de Gaupol. `CorrectionSettings` porte les quatre réglages du saut, et
> **`proposeCorrections` les branche** — un sous-titre de moins de `24`
> laissé tel quel est désormais le comportement par défaut de l'assistant, ce
> qu'il n'était pas avant cette issue.
>
> **L'unité se retient**, contrairement à ce que #505 avait resserré :
> `lineBreakInEms`, `true` par défaut, l'unité de Gaupol — les défauts `24` et
> `3` n'ont de sens qu'en ems (D5). Le noyau ne porte que le choix, jamais la
> mesure : la fenêtre construit `EmsLineMeasure` ou `CharacterLineMeasure` à
> chaque calcul, si bien qu'aucune longueur d'une mesure précédente ne survit
> à un changement d'unité. Le seuil de saut en longueur suit l'unité, comme
> chez Gaupol (`skip_unit_combo` et `unit_combo` lisent la même valeur). Les
> réglages passent de seize à vingt et un.

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

> **Précisé par [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501).**
> Six cas balisés, dans `common-error.entrees`, `capitalization.entrees` et
> `hearing-impaired.entrees` : une balise juste après ce qu'une correction
> retire, une balise à l'intérieur d'une correspondance, une correction qui la
> vide, une balise autour de chaque ligne d'un texte à plusieurs lignes — une
> marge, au sens du parseur de Gaupol —, et un mot capitalisé que la balise
> touche. Trois désaccords, aucun corrigé dans `transform` :
>
> - **la balise à l'intérieur d'une correspondance** confirme l'écart que ce
>   paragraphe annonçait déjà : Gaupol la ramène au début du morceau retiré,
>   puis son nettoyage de balises retire la paire devenue vide ; `transform` la
>   garde étirée sur tout le texte qui la remplace, la règle que la phase 10
>   lui a donnée et que `recherche.cas` tient. Revenir dessus referait une
>   règle éprouvée ailleurs pour un seul appelant ;
> - **le nom du locuteur balisé**, dans les mentions, est le même écart : la
>   balise `<b>` qui entourait le nom retiré survit, étirée sur le tiret de
>   dialogue qui le remplace, là où Gaupol ne laisse rien ;
> - **le mot capitalisé balisé** est un écart différent, pas celui annoncé plus
>   haut : `SubRip.clean` déplace un espace collé à une balise avant de la
>   rendre, un nettoyage cosmétique que rien dans `MarkupParser` ne porte. Une
>   capitalisation ne déplace jamais une balise — elle ne réécrit qu'un point de
>   code, jamais à l'intérieur d'une balise —, donc l'écart ne vient que de ce
>   nettoyage absent, volontairement : retoucher l'espacement autour des
>   balises n'est le travail d'aucune des trois corrections de cette tranche.
>
> Les trois sont nommés dans les tests qui les jouent
> (`common_errors_test.cpp`, `capitalization_test.cpp`,
> `hearing_impaired_correction_test.cpp`), pas seulement ici. La marge
> (`<i>` autour de chaque ligne) et les deux autres cas ne montrent, eux,
> aucun désaccord : la mécanique par balise de `transform` suffit à reproduire
> ce que Gaupol range dans un cas spécial.

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
| une balise à l'intérieur d'une correspondance **se ramène au début du morceau retiré** | **reste étirée sur tout le texte qui le remplace** | D9, décidé par #501 : la règle que `transform` tient depuis la phase 10, sur laquelle `recherche.cas` fait foi |
| `SubRip.clean` **retouche l'espacement autour des balises** après coup | **rien ne le fait** | D9, décidé par #501 : aucune des trois corrections ne déplace une balise, ce nettoyage cosmétique n'est le travail d'aucune |

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
