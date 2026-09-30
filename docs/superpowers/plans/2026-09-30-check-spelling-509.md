# Plan — issue #509, `Tools ▸ Check Spelling…` (D6)

Branche `feat/509`. Spec : `docs/specs/12-correction.md` (D6, GUI-SPELL-01/02/03). Modèle à lire : le
`SpellCheckNavigator` et le `SpellCheckDialog` de Gaupol
(`reference/gaupol/aeidon/spell.py`, `reference/gaupol/gaupol/dialogs/spell_check.py`, LECTURE SEULE ;
jamais de `cd` vers `reference/`).

## Décisions de conception

1. **Noyau (`core/text/`)** :
   - `SpellCheckNavigator` : port fidèle de la classe de Gaupol, positions en **octets UTF-8** (le
     découpeur `tokenizeForSpelling` rend des octets). Il possède un `SpellChecker`.
   - `SpellCheckWalk` : le parcours sur plusieurs cibles (`CorrectionTarget`, déjà là) — sous-titre après
     sous-titre, projet après projet — au-dessus du navigateur. **Sans Qt.** Il produit des
     `ProposedCorrection` (type existant) : l'application passe par `core::applyCorrections`, donc une
     entrée d'historique par projet, sans nouvelle commande.
2. **Fermer au milieu** applique ce qui a été *fait* : les textes déjà quittés **et** les gestes (remplacer,
   joindre…) déjà posés sur le texte courant ; pas les retouches tapées dans la zone de texte et non
   validées (« Save and resume »), pas le reste du parcours. **Écart de Gaupol, à dire** : lui perd le
   texte courant.
3. **Deux fenêtres, comme Gaupol** : `Tools ▸ Check Spelling…` (le parcours) et
   `Tools ▸ Spell-Check Settings…` (langue, cible, document — le `LanguageDialog` de Gaupol). La seconde
   est **toujours active** : c'est elle qui sort d'une langue sans dictionnaire, sans quoi l'entrée grisée
   serait une impasse.
4. **Sans dictionnaire pour la langue réglée**, `Check Spelling…` est **grisée** et son info-bulle / sa
   ligne d'état dit `core::noDictionaryFor(langue)` (« no dictionary for fr ») — GUI-SPELL-02, déjà
   citée à #508, à citer de nouveau ici. La langue réglée vide = celle du système, résolue par
   `spellLanguageFor` (déjà là, `join_split_page.hpp`).
5. **Réglages** : trois clés `spell-check.language`, `spell-check.target`, `spell-check.document`
   (Gaupol : `language`, `target`, `field`), ajoutées à `Settings` sur le modèle de `correction.*` ;
   valeur inconnue → défaut + diagnostic. La taille de la fenêtre n'est pas retenue (hors périmètre).
6. **La liste de remplacements** est écrite à la fermeture (`saveSpellReplacements`), sous
   `spellConfigDirectory()` reçu de la fenêtre — jamais résolu par le contrôleur (ADR 0022).
7. **Le parcours place la vue** : chaque mot inconnu ramène l'onglet du projet et sélectionne la ligne
   (`View::reveal(project, index)`), comme `preview` le fait pour le film.
8. Tests **sur le double** (`WordListSpellProvider`), jamais sur les dictionnaires de la machine.

## Tâches (séquentielles)

- **T1** `SpellCheckNavigator` + tests unitaires (GUI-SPELL-01 côté noyau).
- **T2** `SpellCheckWalk` + tests (multi-cibles, un texte inchangé ne produit rien, fermeture au milieu).
- **T3** Réglages `spell-check.*` (Settings, lecture/écriture, diagnostics, tests, `docs/` des clés).
- **T4** `SpellCheckSettingsDialog` + `SpellCheckDialog` (widgets) + tests de widgets.
- **T5** `SpellCheckController`, les deux actions dans `window_actions`, câblage `MainWindow`
  (`CorrectionSide` ou côté dédié), grisage GUI-SPELL-02, tests du contrôleur, une entrée d'historique
  par projet, liste de remplacements écrite.
- **T6** Spec (D6 « Précisé par #509 », GUI-SPELL-01/02 inscrits et cités), manuel + capture
  (`src/test/tools/screenshots.cpp`), ensuite bump / porte par le coordinateur.

## Consignes à chaque implémenteur

- C++23 ; **commentaires C++ en anglais**, tout le reste en français ; identifiants en anglais ; libellés
  affichés en anglais (mots partagés → `core/wording.hpp`).
- Commits : Conventional Commits en français, `feat(core): …` / `feat(gui): …`, terminés par
  `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`. **Jamais `amend`.** Toute commande git :
  `git -C /home/beber/Projects/subedit …`. Jamais de `cd` dans `reference/`.
- Construire : `cmake --preset dev` ; binaires `build/dev/bin/subedit_core_test`, `subedit_gui_test`.
  Vérifier par étapes ciblées (`./src/scripts/gate.sh <cible> --only <étape>`), **pas de `make check`
  complet** (le coordinateur le lance une fois).
- `moc` ne parse pas toujours `<concepts>`/`<compare>` : déclaration anticipée dans les en-têtes
  `Q_OBJECT`, inclusion réelle dans le `.cpp`.
- TDD : test qui échoue, puis le code.
- Ne jamais nommer un fichier de `src/data/` ; les fixtures vivent dans `src/test/data/`.
- Pas de bump de version, pas de CHANGELOG, pas de manuel avant T6.
