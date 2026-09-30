# Lot 2 — Commandes et fenêtre — Plan d'implémentation

> **Pour qui exécute :** SOUS-COMPÉTENCE REQUISE : suivre
> superpowers:subagent-driven-development (recommandé) ou
> superpowers:executing-plans pour dérouler ce plan tâche par tâche. Les étapes
> utilisent la syntaxe case à cocher (`- [ ]`) pour le suivi.

**Objectif :** fermer les cinq issues du lot 2 défini par #411 — #396, #404,
#398, #397, #409 — dans une seule pull request, un commit par issue, dans cet
ordre.

**Architecture :** cinq correctifs indépendants mais touchant les mêmes
fichiers à fort taux d'inclusion (`main_window.cpp`/`.hpp`, `wording.hpp`,
`command_kind.hpp`), d'où le regroupement en une seule PR décidé par #411.
Chaque tâche ci-dessous correspond à un commit. #404 est la plus grosse : elle
factorise en une fonction libre la boucle « construire, sauter l'inchangé,
empiler, grouper » que quatre opérations recopiaient, et corrige au passage un
compte fantôme dans la recherche.

**Tech Stack :** C++23, Qt 6, Catch2, ICU (recherche).

**Spec :** issue #411 (découpage des lots) et les cinq issues elles-mêmes —
#396, #404, #398, #397, #409, chacune avec son corps et, pour #398 et #409, une
décision déjà actée en commentaire. Pas de spec de phase séparée : ce sont des
corrections de fin de phase 10.

## Contraintes globales

- **Un commit par issue**, avec une ligne `Closes #N` par issue dans le corps
  de la pull request (pas dans les commits individuels), en anglais.
- **Le C++ s'écrit en anglais**, identifiants et commentaires ; les messages de
  commit et ce document sont en français.
- **`make check-local` puis `make check`** doivent passer avant d'ouvrir la
  pull request ; le bump de patch, `make manual` et le relevé de banc suivent
  l'ordre décrit dans `CLAUDE.md` — bumper une seule fois, à la toute fin du
  lot, pas après chaque tâche.
- **Aucun test ne doit lire `reference/gaupol`** ni y écrire quoi que ce soit.
- Le manuel (`docs/manual/`) se met à jour à l'étape 7 de l'ordre de
  `CLAUDE.md`, après la porte, pour chaque tâche qui change ce que
  l'utilisateur voit — donc en fin de lot, avant de commiter le dernier
  correctif ou dans un commit dédié juste avant la pull request.

---

## Tâche 1 — #396 : la cible de recherche survit à une annulation structurelle

**Fichiers :**
- Modifier : `src/lib/subedit/gui/main_window.cpp` (`MainWindow::openOn`,
  autour de la ligne 592)
- Modifier : `src/lib/subedit/gui/main_window.hpp` (commentaire de `m_match`,
  ligne 661)
- Test : `src/test/gui/window_search_test.cpp`

**Cause exacte.** `SubtitleTableModel::applied()` ne fait `beginResetModel()` /
`endResetModel()` que pour une opération structurelle (insertion, suppression,
et donc une annulation qui rejoue l'inverse d'une fusion ou d'une scission).
`QItemSelectionModel::reset()`, appelé alors par Qt, vide la sélection **sans
émettre `selectionChanged`** — le connect de la ligne 612 de `main_window.cpp`,
qui oublie `m_searchTarget`/`m_match` sur un changement de sélection, ne se
déclenche donc jamais dans ce cas précis. La cible garde des indices qui
n'existent plus, et `Project::subtitleAt` lève `std::out_of_range` au prochain
`Find Next` ou `Replace All`.

Le signal Qt natif `QAbstractItemModel::modelReset` est émis, lui, dans tous
les cas — c'est exactement ce que `beginResetModel`/`endResetModel` déclenchent
— et rien d'autre ne s'y accroche aujourd'hui.

- [ ] **Étape 1 — écrire le test qui échoue aujourd'hui**

Dans `src/test/gui/window_search_test.cpp`, ajouter, à la suite des cas
existants (le fichier a déjà les fonctions utilitaires `withFour`, `fourIn`,
`searching`, `selectedRows`, `statusOf` — les réutiliser) :

```cpp
TEST_CASE("an undo that resets the model forgets a stale search target",
          "[gui][GUI-SEARCH-03]") {
    // Four subtitles; select the fourth, split it into a fifth. The search
    // then captures a target of {3, 4} before the split is undone.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    selectRow(window, 3);
    window.splitAction()->trigger();
    CHECK(selectedRows(window) == std::vector<int>{3, 4});

    const SearchDialog& dialog = searching(window, "marie");
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});

    // The split is undone: back to four subtitles, and the model was reset
    // rather than told which rows changed — Qt clears the selection without a
    // `selectionChanged`.
    window.undoAction()->trigger();

    // Before the fix, this throws `std::out_of_range` out of `spansAt`.
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});
}
```

- [ ] **Étape 2 — vérifier qu'il échoue**

`./src/scripts/gate.sh check --only build-test` puis lancer le binaire de
tests de fenêtre filtré sur `[GUI-SEARCH-03]` (voir `--list-tests` du binaire
concerné pour le nom exact de la cible CTest). Avant le correctif, le test doit
planter (`std::out_of_range` non rattrapé) plutôt qu'échouer proprement sur une
assertion — c'est le crash que #396 décrit.

- [ ] **Étape 3 — corriger `main_window.cpp`**

Dans `MainWindow::openOn`, juste après le connect existant de la ligne 592
(`historyChanged`), ajouter :

```cpp
    // A structural undo or redo resets the model rather than reporting which
    // rows changed — Qt then clears the selection without a
    // `selectionChanged`, which is otherwise what forgets a stale target. This
    // catches that one case directly on Qt's own reset signal.
    connect(model.get(), &QAbstractItemModel::modelReset, this, [this] {
        m_searchTarget.reset();
        m_match.reset();
    });
```

(`QAbstractItemModel` est déjà inclus via `<QAbstractItemModel>` dans ce
fichier, à vérifier — sinon ajouter l'include.)

- [ ] **Étape 4 — corriger le commentaire de `main_window.hpp`**

Ligne 661, remplacer :

```cpp
    /// The match last found, which `Find Next` starts after and `Replace`
    /// rewrites. Forgotten when the pattern, an option or the document changes.
```

par :

```cpp
    /// The match last found, which `Find Next` starts after and `Replace`
    /// rewrites. Forgotten when the pattern, an option or the document
    /// changes — including a structural undo or redo, which resets the model
    /// rather than reporting the change.
```

- [ ] **Étape 5 — vérifier que le test passe**

Relancer le même test ciblé ; il doit passer sans lever d'exception.

- [ ] **Étape 6 — commiter**

```bash
git -C /home/beber/Projects/subedit add src/lib/subedit/gui/main_window.cpp \
    src/lib/subedit/gui/main_window.hpp src/test/gui/window_search_test.cpp
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
fix(gui): oublier la cible de recherche sur une réinitialisation du modèle

Closes #396
EOF
)"
```

(Le message de commit ci-dessus est illustratif — `Closes #N` se place dans le
corps de la pull request, pas nécessairement dans chaque commit ; suivre la
convention déjà en usage dans le dépôt pour ce détail.)

---

## Tâche 2 — #404 : une seule réécriture de textes, et des comptes qui disent la même chose

C'est la tâche la plus grosse du lot. Elle se découpe en cinq mouvements, dans
l'ordre : factoriser la boucle, migrer les quatre opérations simples, migrer
les comptes rendus vers `wording.hpp`, corriger le compte fantôme de
`Replace All`, et deux nettoyages annexes que l'issue mentionne explicitement.

**Ce qui reste hors périmètre, délibérément :** `replaceAll` garde sa propre
boucle (elle a besoin du moteur ICU de `rewrite()` et compte des
*correspondances*, pas des sous-titres — un sous-titre à deux correspondances
compte pour deux dans `noticeOfReplaceAll`, comme le test
`"replace all is one entry in the history, and says how many"` le fixe déjà :
« replaced 3 matches » pour deux sous-titres touchés). Le
`Document::Main` en dur de `replaceAll` n'est pas non plus touché : l'issue
l'observe mais ne le met pas dans ses « ce qu'on pourrait faire », et rien
n'indique qu'un remplacement doive un jour toucher la traduction.

### 2.1 — Factoriser la boucle commune

**Fichiers :**
- Créer : `src/lib/subedit/core/edit/rewrite_texts.hpp`
- Créer : `src/lib/subedit/core/edit/rewrite_texts.cpp`
- Modifier : `src/lib/subedit/core/CMakeLists.txt` (ajouter les deux fichiers
  à la bibliothèque)
- Test : `src/test/unit/core/edit/rewrite_texts_test.cpp` (nouveau)

**Interfaces produites**, que les tâches 2.2 et 2.3 consomment :

```cpp
[[nodiscard]] std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform);

[[nodiscard]] std::size_t rewrittenCount(const Command& command);
```

- [ ] **Étape 1 — écrire le test qui échoue**

```cpp
#include <subedit/core/edit/rewrite_texts.hpp>

#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

using subedit::core::CommandKind;
using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::rewrittenCount;
using subedit::core::rewriteTexts;
using subedit::core::Selection;
using subedit::core::SubtitleIndex;

namespace {

constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                             "2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n";

} // namespace

TEST_CASE("rewriteTexts skips what the transformation leaves unchanged",
          "[core][edit]") {
    InMemoryFileSystem files;
    files.addFile("f.srt", kTwo);
    auto opened = openProject(files, "f.srt");
    REQUIRE(opened.has_value());

    const Selection all = Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(1));
    std::unique_ptr<subedit::core::Command> command =
        rewriteTexts(opened->project, all, Document::Main, CommandKind::ChangeCase,
                     [](const std::string& text) { return text; });

    CHECK(command == nullptr);
}

TEST_CASE("rewriteTexts groups every real change under the kind it is given",
          "[core][edit]") {
    InMemoryFileSystem files;
    files.addFile("f.srt", kTwo);
    auto opened = openProject(files, "f.srt");
    REQUIRE(opened.has_value());

    const Selection all = Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(1));
    std::unique_ptr<subedit::core::Command> command =
        rewriteTexts(opened->project, all, Document::Main, CommandKind::ChangeCase,
                     [](const std::string& text) {
                         std::string upper = text;
                         for (char& letter : upper)
                             letter = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));
                         return upper;
                     });

    REQUIRE(command != nullptr);
    CHECK(command->kind() == CommandKind::ChangeCase);
    CHECK(rewrittenCount(*command) == 2);
}
```

(Adapter les inclusions exactes de `InMemoryFileSystem`/`openProject` sur le
modèle de `italics_command_test.cpp`, qui construit déjà un `Project` de test
de cette manière — s'y référer plutôt que deviner les signatures.)

- [ ] **Étape 2 — vérifier que la compilation échoue** (le fichier `.hpp`
  n'existe pas encore)

- [ ] **Étape 3 — écrire `rewrite_texts.hpp`**

```cpp
#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/document.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace subedit::core {

class Project;
class Selection;

/// Rewrites the text of `document` for every subtitle of `selection` through
/// `transform`, and groups what changed under `kind`.
///
/// **The one loop every text-rewriting operation shared** — italics, letter
/// case, dialogue dashes, cut. Every command is built before any is applied,
/// so that each captures the text as it stands now rather than the one its
/// predecessor left. A subtitle `transform` leaves unchanged contributes
/// neither a command nor a count.
///
/// Returns **nothing when no text changes**. An empty group would apply
/// without doing anything and still push an entry the user would meet in
/// "undo" without understanding it.
[[nodiscard]] std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform);

/// How many subtitles `command` rewrote, read from what it describes rather
/// than counted again — decision D8: the count a caller reports is the count
/// the history would show if the command were undone.
[[nodiscard]] std::size_t rewrittenCount(const Command& command);

} // namespace subedit::core
```

- [ ] **Étape 4 — écrire `rewrite_texts.cpp`**

```cpp
#include <subedit/core/edit/rewrite_texts.hpp>

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/set_text_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

std::unique_ptr<Command>
rewriteTexts(const Project& project,
             const Selection& selection,
             Document document,
             CommandKind kind,
             const std::function<std::string(const std::string&)>& transform) {
    std::vector<std::unique_ptr<Command>> commands;

    for (const SubtitleIndex index : selection.indices()) {
        const std::string& text = project.subtitleAt(index).text(document);

        std::string written = transform(text);
        if (written == text)
            continue;

        commands.push_back(
            std::make_unique<SetTextCommand>(project, index, document, std::move(written)));
    }

    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(kind, std::move(commands));
}

std::size_t rewrittenCount(const Command& command) {
    std::size_t rewritten = 0;
    for (const Change& change : command.describe())
        rewritten += change.subtitles.count();
    return rewritten;
}

} // namespace subedit::core
```

- [ ] **Étape 5 — enregistrer les deux fichiers dans `CMakeLists.txt`** du
  noyau (à côté des autres fichiers de `core/edit`) et le test dans celui de
  `src/test`.

- [ ] **Étape 6 — vérifier que le test passe**

- [ ] **Étape 7 — commiter** (voir message-type en fin de tâche 2).

### 2.2 — Migrer italique, casse, tirets et couper vers `rewriteTexts`

**Fichiers :**
- Modifier : `src/lib/subedit/core/edit/italics_command.hpp`/`.cpp`
- Modifier : `src/lib/subedit/core/edit/letter_case_command.hpp`/`.cpp`
- Modifier : `src/lib/subedit/core/edit/dialogue_dashes_command.cpp`
- Modifier : `src/lib/subedit/core/edit/clipboard.cpp`
- Modifier : `src/lib/subedit/gui/main_window.cpp` (trois sites : lignes 1317,
  1340, 1361)
- Tests existants à faire passer sans régression :
  `src/test/unit/core/edit/italics_command_test.cpp`,
  `text_commands_test.cpp`, tests de tirets et de couper, tests de fenêtre
  correspondants.

**Interfaces consommées :** `rewriteTexts(...)` et `rewrittenCount(...)` de
2.1.

- [ ] **Étape 1 — adapter `italics_command_test.cpp`**

La ligne `CHECK(subedit::core::italicisedCount(*command) == 2);` devient
`CHECK(subedit::core::rewrittenCount(*command) == 2);`, et l'inclusion de
`<subedit/core/edit/rewrite_texts.hpp>` s'ajoute.

- [ ] **Étape 2 — vérifier que ce test échoue à la compilation** (la fonction
  n'existe pas encore sous ce nom à cet appel, `italicisedCount` va être
  retirée à l'étape suivante — donc soit ce test casse *après* l'étape 3, soit
  on avance les deux ensemble ; dans les faits, avancer 1 et 3 dans le même
  geste est plus simple ici que de forcer un échec intermédiaire artificiel).

- [ ] **Étape 3 — réécrire `italics_command.cpp`**

```cpp
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/italics.hpp>

#include <algorithm>
#include <string>

namespace subedit::core {

bool wouldItalicise(const Project& project, const Selection& selection, Document document) {
    const SubtitleFormat format = project.sourceFile().format;

    return std::ranges::any_of(selection.indices(), [&](SubtitleIndex index) {
        const std::string& text = project.subtitleAt(index).text(document);
        return !withoutItalics(text, format).empty() && !opensInItalics(text, format);
    });
}

std::unique_ptr<Command>
setItalics(const Project& project, const Selection& selection, Document document, bool italic) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project,
                        selection,
                        document,
                        italic ? CommandKind::Italicise : CommandKind::Unitalicise,
                        [italic, format](const std::string& text) {
                            return italic ? inItalics(text, format) : withoutItalics(text, format);
                        });
}

} // namespace subedit::core
```

Retirer `italicisedCount` du `.hpp` et du `.cpp`.

- [ ] **Étape 4 — réécrire `letter_case_command.cpp`** de la même manière :

```cpp
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>

namespace subedit::core {

std::unique_ptr<Command> setLetterCase(const Project& project,
                                       const Selection& selection,
                                       Document document,
                                       LetterCase wanted) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project, selection, document, CommandKind::ChangeCase,
                        [wanted, format](const std::string& text) {
                            return recased(text, wanted, format);
                        });
}

} // namespace subedit::core
```

Retirer `recasedCount` du `.hpp` et du `.cpp`. Dans `text_commands_test.cpp`,
`recasedCount(*command)` devient `rewrittenCount(*command)`.

- [ ] **Étape 5 — réécrire `dialogue_dashes_command.cpp`** :

```cpp
std::unique_ptr<Command> setDialogueDashes(const Project& project,
                                           const Selection& selection,
                                           Document document,
                                           bool dashed) {
    const SubtitleFormat format = project.sourceFile().format;

    return rewriteTexts(project,
                        selection,
                        document,
                        dashed ? CommandKind::AddDialogueDashes : CommandKind::RemoveDialogueDashes,
                        [dashed, format](const std::string& text) {
                            return dashed ? withDialogueDashes(text, format)
                                         : withoutDialogueDashes(text, format);
                        });
}
```

(`wouldAddDialogueDashes` ne change pas.)

- [ ] **Étape 6 — réécrire `cutTexts` dans `clipboard.cpp`** :

```cpp
std::unique_ptr<Command>
cutTexts(const Project& project, const Selection& selection, Document document) {
    return rewriteTexts(project, selection, document, CommandKind::Cut,
                        [](const std::string&) { return std::string{}; });
}
```

(Le saut des textes déjà vides est déjà assuré par `rewriteTexts`, puisque
`written == text` quand les deux sont vides.)

- [ ] **Étape 7 — mettre à jour les trois sites de `main_window.cpp`**

Ligne 1317 : `core::italicisedCount(*command)` → `core::rewrittenCount(*command)`.
Ligne 1340 : `core::recasedCount(*command)` → `core::rewrittenCount(*command)`.
Ligne 1361 : `core::recasedCount(*command)` → `core::rewrittenCount(*command)`
(les tirets empruntaient déjà celui de la casse ; ils lisent maintenant leur
propre commande, ce qui règle au passage l'emprunt que #404 relève).

- [ ] **Étape 8 — lancer les tests unitaires et de fenêtre concernés**, les
  faire tous passer.

- [ ] **Étape 9 — commiter** (dans le même commit que 2.1, ou séparément selon
  ce qui rend la revue la plus claire — les deux sont un seul mouvement
  logique et peuvent être un seul commit d'issue).

### 2.3 — Déplacer les comptes rendus vers `wording.hpp`

**Fichiers :**
- Modifier : `src/lib/subedit/core/wording.hpp`
- Modifier : `src/lib/subedit/core/wording.cpp`
- Modifier : `src/lib/subedit/gui/main_window.cpp` (mêmes trois fonctions)
- Modifier : `CLAUDE.md` (portée de la règle sur `wording.hpp`)

**Interfaces produites :**

```cpp
[[nodiscard]] std::string_view nothingToChange();
[[nodiscard]] std::string noticeOfItalics(std::size_t count, bool italic);
[[nodiscard]] std::string noticeOfRecase(std::size_t count);
[[nodiscard]] std::string noticeOfDialogueDashes(std::size_t count, bool dashed);
```

- [ ] **Étape 1 — ajouter les déclarations à `wording.hpp`**, à côté de
  `noticeOfReplaceAll` (ligne 368), avec une doc Doxygen anglaise sur le modèle
  des fonctions voisines :

```cpp
/// What no rewrite had to do — italics, case or dialogue dashes alike. Never
/// followed by a history entry.
[[nodiscard]] std::string_view nothingToChange();

/// What toggling italics did: "N subtitle(s) put in italics" or "… taken out
/// of italics".
[[nodiscard]] std::string noticeOfItalics(std::size_t count, bool italic);

/// What re-casing did: "N subtitle(s) recased".
[[nodiscard]] std::string noticeOfRecase(std::size_t count);

/// What toggling dialogue dashes did: "N subtitle(s) dashed" or "… undashed".
[[nodiscard]] std::string noticeOfDialogueDashes(std::size_t count, bool dashed);
```

- [ ] **Étape 2 — les implémenter dans `wording.cpp`**, à côté de
  `noticeOfReplaceAll` :

```cpp
std::string_view nothingToChange() {
    return "nothing to change";
}

std::string noticeOfItalics(std::size_t count, bool italic) {
    return countOf(count, "subtitle") + (italic ? " put in italics" : " taken out of italics");
}

std::string noticeOfRecase(std::size_t count) {
    return countOf(count, "subtitle") + " recased";
}

std::string noticeOfDialogueDashes(std::size_t count, bool dashed) {
    return countOf(count, "subtitle") + (dashed ? " dashed" : " undashed");
}
```

- [ ] **Étape 3 — mettre à jour `main_window.cpp`**

```cpp
void MainWindow::toggleItalicsOnTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());
    const bool italic = core::wouldItalicise(m_session->project(), target, core::Document::Main);

    std::unique_ptr<core::Command> command =
        core::setItalics(m_session->project(), target, core::Document::Main, italic);
    if (!command) {
        m_prompts->reportOutcome(std::string{core::nothingToChange()});
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(std::move(command), target);
    m_prompts->reportOutcome(core::noticeOfItalics(rewritten, italic));
}
```

(et de même pour `changeCaseOfTarget`/`core::noticeOfRecase`,
`toggleDialogueDashesOnTarget`/`core::noticeOfDialogueDashes`). Les trois
`"nothing to change"` littéraux disparaissent au profit de
`core::nothingToChange()`. **Note :** la tâche 3 (#398) remplacera ces mêmes
appels `reportOutcome` par `statusBar()->showMessage(...)` — ne pas s'étonner
que ce code soit retouché une deuxième fois deux tâches plus loin ; c'est
l'ordre des commits que #411 fixe.

- [ ] **Étape 4 — corriger la portée de la règle dans `CLAUDE.md`**

Le passage actuel :

> Anglais aussi pour tout ce que le binaire écrit à un utilisateur — sortie
> standard, sortie d'erreur, en-têtes de colonne, libellés de menu, messages de
> dialogue. Les deux surfaces disent les mêmes mots, et
> [`core/wording.hpp`](src/lib/subedit/core/wording.hpp) est l'unique endroit
> où ils sont écrits.

devient (préciser que `wording.hpp` porte ce que les *deux* surfaces peuvent
écrire, pas les libellés propres à une seule) :

> Anglais aussi pour tout ce que le binaire écrit à un utilisateur — sortie
> standard, sortie d'erreur, en-têtes de colonne, libellés de menu, messages de
> dialogue. **Ce que les deux surfaces peuvent dire toutes les deux** — le nom
> d'un format, un compte rendu d'opération, une raison d'échec — vit dans
> [`core/wording.hpp`](src/lib/subedit/core/wording.hpp), l'unique endroit où
> c'est écrit ; **un libellé de menu ou de dialogue, propre à la fenêtre et
> sans équivalent en ligne de commande, reste dans le fichier de la fenêtre**
> — la règle porte sur les mots partagés, pas sur la surface qui les affiche.

(Le texte exact ci-dessus est une proposition ; l'important est que la règle
cesse d'affirmer que `wording.hpp` est l'unique endroit où *tout* libellé de
menu est écrit, ce que #404 a trouvé faux par lecture — 42 `QStringLiteral`
dans `main_window.cpp`.)

- [ ] **Étape 5 — tests**, faire passer ceux qui lisent aujourd'hui les
  chaînes littérales dans `FakePrompts::outcomes` (ils continuent de le faire
  jusqu'à la tâche 3, qui les fera lire la barre d'état à la place) ; vérifier
  qu'ils comparent toujours aux mêmes mots exacts (`"2 subtitles put in
  italics"`, etc. — inchangés, seul l'endroit où ils sont composés bouge).

- [ ] **Étape 6 — commiter.**

### 2.4 — Corriger le compte fantôme de `Replace All`

**Fichiers :**
- Modifier : `src/lib/subedit/core/edit/search.cpp` (fonction `rewrite`, ligne
  ~243-277)
- Test : `src/test/unit/core/edit/search_test.cpp` (ou fichier équivalent —
  vérifier son nom exact avant d'écrire)

**Le bug.** `rewrite()` incrémente `rewritten.count` à chaque correspondance
trouvée, même quand le remplacement produit exactement le même texte que celui
trouvé (remplacer « Marie » par « Marie »). `replaceAll` additionne ce compte
sans jamais vérifier qu'un texte a réellement changé, si bien que
`replaceAllInTarget` (`main_window.cpp:1629`) annonce « replaced 1 match » sans
qu'aucune commande n'entre dans l'historique.

- [ ] **Étape 1 — écrire le test qui échoue**

Sur le modèle des tests existants de `search.cpp` (vérifier le nom exact du
fichier et des inclusions avant d'écrire — probablement
`src/test/unit/core/edit/search_test.cpp`) :

```cpp
TEST_CASE("replacing a match by itself changes nothing and counts nothing",
          "[core][edit][search]") {
    InMemoryFileSystem files;
    files.addFile("f.srt", "1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie.\n\n");
    auto opened = openProject(files, "f.srt");
    REQUIRE(opened.has_value());

    const std::expected<SearchPattern, PatternError> pattern =
        SearchPattern::compile("Marie", SearchOptions{});
    REQUIRE(pattern.has_value());

    const Selection all = Selection::range(SubtitleIndex::fromValue(0), SubtitleIndex::fromValue(0));
    const ReplacedAll replaced = replaceAll(opened->project, all, *pattern, "Marie");

    CHECK(replaced.count == 0);
    CHECK(replaced.command == nullptr);
}
```

- [ ] **Étape 2 — vérifier qu'il échoue** (`replaced.count` vaut 1 aujourd'hui).

- [ ] **Étape 3 — corriger `rewrite()` dans `search.cpp`**

```cpp
    Rewritten rewritten{.text = std::string{text}};
    MarkupParser parser{text, format};

    std::size_t at = only.has_value() ? only->start : 0;
    while (const std::optional<Found> found =
               firstFrom(parser.visible(), compiled, at, replacement)) {
        if (only.has_value() && found->span != *only)
            break;

        // A match that replaces itself by the same text is not a change: not
        // counted, and not what pushes a command past `rewrite`'s caller.
        const std::string_view matched =
            parser.visible().substr(found->span.start, found->span.end - found->span.start);
        const bool changes = found->replacement != matched;

        parser.replace(found->span.start, found->span.end - found->span.start, found->replacement);
        if (changes)
            ++rewritten.count;

        const std::size_t length = MarkupParser{found->replacement, format}.visible().size();
        rewritten.written = {.start = found->span.start, .end = found->span.start + length};
        if (only.has_value())
            break;

        at = found->span.end > found->span.start
                 ? rewritten.written.end
                 : nextCharacter(parser.visible(), rewritten.written.end);
    }

    if (rewritten.count > 0)
        rewritten.text = parser.text();
    return rewritten;
```

(Seuls les cinq lignes autour du `++rewritten.count` changent ; le reste de la
fonction, cité ici en entier pour le contexte, ne bouge pas.)

- [ ] **Étape 4 — vérifier que le nouveau test passe, et qu'aucun test
  existant de recherche/remplacement ne casse** — en particulier
  `window_search_test.cpp`, dont « replace all is one entry in the history,
  and says how many » attend toujours « replaced 3 matches » pour un vrai
  remplacement : ce test ne doit pas bouger, puisque ses trois correspondances
  changent bien le texte.

- [ ] **Étape 5 — commiter.**

### 2.5 — Deux nettoyages annexes que l'issue nomme explicitement

**a) Retirer `replacingAll`, code mort.**

- [ ] Vérifier qu'aucun appelant de production n'existe (déjà confirmé par la
  recherche : seul `markup_parser_test.cpp` l'appelle).
- [ ] Retirer la déclaration de `markup_parser.hpp` (ligne 104) et
  l'implémentation de `markup_parser.cpp` (lignes 597-616).
- [ ] Retirer le ou les cas de test qui l'exercaient dans
  `markup_parser_test.cpp`.
- [ ] Vérifier que la compilation et les tests du fichier passent toujours.

**b) `SetDurationCommand` devient une fonction bâtie sur `SetPositionCommand`.**

**Fichiers :**
- Modifier : `src/lib/subedit/core/edit/set_duration_command.hpp`/`.cpp`
- Modifier : `src/lib/subedit/gui/subtitle_table_model.cpp` (seul appelant de
  production)
- Modifier : `src/test/unit/core/edit/set_duration_command_test.cpp`

- [ ] Remplacer la classe par une fonction libre :

```cpp
// set_duration_command.hpp
[[nodiscard]] std::unique_ptr<Command>
setDuration(const Project& project, SubtitleIndex index, Duration duration);
```

```cpp
// set_duration_command.cpp
std::unique_ptr<Command>
setDuration(const Project& project, SubtitleIndex index, Duration duration) {
    const Timestamp newEnd = project.subtitleAt(index).start + duration;
    std::vector<std::unique_ptr<Command>> commands;
    commands.push_back(
        std::make_unique<SetPositionCommand>(project, index, Boundary::End, newEnd));
    return std::make_unique<CompositeCommand>(CommandKind::SetDuration, std::move(commands));
}
```

- [ ] Dans `subtitle_table_model.cpp`, remplacer
  `std::make_unique<SetDurationCommand>(project, index, duration)` par
  `setDuration(project, index, duration)` (le type de retour change de
  `std::unique_ptr<SetDurationCommand>` à `std::unique_ptr<Command>` — vérifier
  que l'appelant ne dépend pas du type concret).
- [ ] Adapter `set_duration_command_test.cpp` à la nouvelle signature ; les
  assertions sur `kind()` et `describe()` doivent continuer de passer sans
  changement de valeurs attendues, puisque `CompositeCommand` les délègue.

- [ ] **Commiter 2.5 séparément ou avec 2.4** — ce sont deux nettoyages
  indépendants l'un de l'autre, tous deux rattachés au même commit d'issue
  #404 si la revue préfère un seul commit pour toute la tâche 2.

### Message de commit de la tâche 2

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
refactor(core): une seule réécriture de textes pour italique, casse, tirets et couper

Closes #404
EOF
)"
```

---

## Tâche 3 — #398 : les gestes sans dialogue parlent dans la barre d'état

**Décision déjà actée** (commentaire du 2026-09-15 sur #398) : Italic, les
quatre Case, Dialogue et leur « nothing to change » passent en barre d'état ;
tout le reste, dialogues compris et `Paste Texts` avec pertes, garde une
boîte.

**Fichiers :**
- Modifier : `src/lib/subedit/gui/main_window.cpp` (les trois fonctions
  retouchées en 2.3)
- Modifier : `docs/manual/subedit-gui/operations.md` (lignes 23-25)
- Modifier : `src/test/gui/window_italics_test.cpp`,
  `src/test/gui/window_text_test.cpp` (ou fichiers équivalents portant les
  tests de casse/tirets — confirmer les noms exacts)

Aucune interface ne change : `MainWindow` hérite publiquement de `QMainWindow`,
donc `statusBar()` est déjà accessible, y compris depuis un test qui tient une
`MainWindow&`.

- [ ] **Étape 1 — adapter les tests d'abord**

Dans `window_italics_test.cpp` et `window_text_test.cpp`, remplacer chaque
`prompts.outcomes.back() == "..."` concernant Italic/Case/Dialogue par
`window.statusBar()->currentMessage().toStdString() == "..."`. Ajouter
`#include <QStatusBar>` si absent. Les messages attendus ne changent pas
(mêmes chaînes que produit `wording.hpp` depuis la tâche 2.3).

- [ ] **Étape 2 — vérifier que ces tests échouent** (`FakePrompts::outcomes`
  reste vide puisque le code de production appelle encore `reportOutcome`).

- [ ] **Étape 3 — définir un délai d'affichage**

Dans l'espace de noms anonyme de `main_window.cpp`, à côté des autres
constantes du fichier :

```cpp
// Long enough to be read without a click, short enough not to survive past
// the next gesture — Qt's own convention for a transient status.
constexpr int kOperationStatusTimeoutMs = 5000;
```

- [ ] **Étape 4 — remplacer les trois `reportOutcome` visés**

```cpp
void MainWindow::toggleItalicsOnTarget() {
    ...
    if (!command) {
        statusBar()->showMessage(QString::fromStdString(std::string{core::nothingToChange()}),
                                 kOperationStatusTimeoutMs);
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(std::move(command), target);
    statusBar()->showMessage(QString::fromStdString(core::noticeOfItalics(rewritten, italic)),
                             kOperationStatusTimeoutMs);
}
```

(même geste pour `changeCaseOfTarget`/`noticeOfRecase` et
`toggleDialogueDashesOnTarget`/`noticeOfDialogueDashes`). Les quinze autres
appels à `reportOutcome`/`reportFailure` du fichier — `Adjust Durations…`,
`Remove Hearing-Impaired Mentions…`, `Paste Texts`, la recherche, etc. — ne
changent pas : ils gardent leur dialogue.

- [ ] **Étape 5 — vérifier que les tests passent.**

- [ ] **Étape 6 — mettre à jour le manuel**

`docs/manual/subedit-gui/operations.md`, lignes 23-25, remplacer :

```
**Ce qu'une opération a fait se dit dans une boîte d'information**, qu'il faut
fermer — y compris après un `Ctrl+I`. Les messages sont cités dans la section de
chaque opération.
```

par :

```
**Ce qu'une opération a fait se dit dans une boîte d'information qu'il faut
fermer, sauf pour `Italic`, `Case` et `Dialogue`** : ces trois-là n'ouvrent pas
de dialogue, et leur compte rendu — « nothing to change » compris — s'affiche
dans la barre d'état, sans rien à fermer. Les messages sont cités dans la
section de chaque opération.
```

Les blocs de messages cités plus bas (lignes 393-395, 399, 466-468, 471-472)
ne changent pas de mots, seulement de support — rien à corriger dans leur
contenu.

- [ ] **Étape 7 — commiter.**

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
feat(gui): les gestes sans dialogue parlent dans la barre d'état

Closes #398
EOF
)"
```

---

## Tâche 4 — #397 : valider la cellule en cours d'édition avant un geste sans dialogue

**Ce que l'issue demande en premier : rejouer, pas supposer.** Les deux
scénarios sont qualifiés de « plausibles, non rejoués » — la tâche commence
donc par deux tests qui établissent, avant tout correctif, ce qui se passe
réellement.

**Fichiers :**
- Modifier : `src/lib/subedit/gui/main_window.cpp` (nouvelle méthode privée, et
  sept sites d'appel)
- Modifier : `src/lib/subedit/gui/main_window.hpp` (déclaration privée)
- Test : `src/test/gui/window_italics_test.cpp` (scénario clavier `Ctrl+I`),
  nouveau fichier ou section dédiée pour le second scénario si un geste de
  souris est nécessaire à le distinguer

- [ ] **Étape 1 — écrire le scénario « une ligne »**

Sur le modèle de `window_player_test.cpp:415-434`, qui ouvre déjà un éditeur
avec `window.table()->edit(index)` :

```cpp
TEST_CASE("Ctrl+I while a cell is being edited commits the edit first",
          "[gui][GUI-EDIT-01]") {
    InMemoryFileSystem files = withFile("f.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "f.srt"), prompts};
    window.show();

    const QModelIndex edited = window.table()->model()->index(0, kTextColumn);
    window.table()->edit(edited);
    REQUIRE(window.table()->isEditing());

    auto* editor = qobject_cast<QWidget*>(window.table()->indexWidget(edited));
    // The delegate's editor is a QPlainTextEdit for the text column; typed
    // characters land wherever Qt's own focus already sits — a persistent
    // editor opened by `edit()` takes it as part of opening.
    QTest::keyClicks(editor, " tout");

    window.italicAction()->trigger();

    CHECK_FALSE(window.table()->isEditing());
    CHECK(window.table()
              ->model()
              ->data(edited, Qt::DisplayRole)
              .toString()
              .toStdString()
              .find("tout") != std::string::npos);
}
```

(Adapter le nom exact du délégué/widget d'édition et l'inclusion de
`<QTest>`/`<QPlainTextEdit>` en s'appuyant sur `cell_delegates.hpp` — vérifier
sur place quel widget `TextDelegate::createEditor` construit avant d'écrire ce
test, plutôt que de deviner.)

- [ ] **Étape 2 — lancer ce test et noter ce qui se passe réellement**
  (c'est la « lecture » que l'issue demande avant de corriger : soit la saisie
  est perdue — bug 1 — soit l'italique posé est écrasé — bug 2 — soit rien ne
  casse parce que Qt committe déjà tout seul dans ce cas précis). Consigner le
  résultat observé dans le message du commit ou dans la description de la
  pull request, puisque c'est une information que l'issue elle-même n'avait
  pas.

- [ ] **Étape 3 — ajouter la méthode commune**

Dans `main_window.hpp`, section privée (à côté des autres méthodes utilitaires
de gestes) :

```cpp
/// Commits and closes the cell editor open on the table, if any — the same
/// validation a modal dialog already gets by taking the focus away.
void commitCellEditor();
```

Dans `main_window.cpp` :

```cpp
void MainWindow::commitCellEditor() {
    // Ahead of every gesture without a dialog, so that the target it reads
    // and the command it builds see the edit already applied rather than
    // racing it — issue #397. Giving the table itself the focus is what a
    // click elsewhere already does, and the delegate's own focus-out handling
    // — Qt's, unmodified — takes it from there: it validates rather than
    // discards, the same answer a dialog already gave.
    if (m_table->isEditing())
        m_table->setFocus();
}
```

- [ ] **Étape 4 — appeler `commitCellEditor()` en tête des sept gestes sans
  dialogue**

`toggleItalicsOnTarget()`, `changeCaseOfTarget()`, `toggleDialogueDashesOnTarget()`,
`cutTexts()`, `pasteTexts()`, `mergeSubtitles()`, `splitSubtitle()` — une ligne
`commitCellEditor();` comme première instruction de chacune.

- [ ] **Étape 5 — vérifier que le test de l'étape 1 passe désormais**, et
  qu'aucun test existant de ces sept gestes ne régresse (en particulier ceux
  qui vérifient la sélection ou le focus après le geste).

- [ ] **Étape 6 — écrire un second test si l'étape 2 a montré un second bug
  distinct** (le scénario « plusieurs lignes » de l'issue, où l'italique posé
  serait écrasé plutôt que la saisie perdue) — sinon, noter explicitement dans
  la pull request que ce second scénario ne reproduit pas et pourquoi, plutôt
  que d'écrire un test qui ne prouve rien.

- [ ] **Étape 7 — commiter.**

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
fix(gui): valider la cellule en cours d'édition avant un geste sans dialogue

Closes #397
EOF
)"
```

---

## Tâche 5 — #409 : l'ajustement des durées valide sa vitesse et garde ses réglages

**Décision déjà actée** (commentaire du 2026-09-15 sur #409) : quatre volets —
une `ReadingSpeed` validée à la construction, les quatre défauts au noyau,
l'état du formulaire distinct de la requête, et la persistance dans
`settings.conf` comme les deux options de la recherche.

### 5.1 — `ReadingSpeed` validée à la construction

**Fichiers :**
- Modifier : `src/lib/subedit/core/edit/duration_adjustment.hpp`/`.cpp`
- Modifier : `src/lib/subedit/gui/duration_adjust_dialog.cpp` (deux
  constructions, lignes 98 et 132)
- Modifier : `src/test/unit/core/edit/duration_adjustment_test.cpp` (cinq
  constructions), `src/test/gui/window_durations_test.cpp` (une)

Suivre le patron déjà en usage pour `FrameRate`
(`src/lib/subedit/core/time/frame_rate.hpp:46-73`) : constructeur privé,
fabrique publique renvoyant `std::optional`.

- [ ] **Étape 1 — écrire le test qui échoue**

```cpp
TEST_CASE("a reading speed of zero or less is refused", "[core][edit]") {
    CHECK_FALSE(core::ReadingSpeed::create(0.0, true, false).has_value());
    CHECK_FALSE(core::ReadingSpeed::create(-1.0, true, false).has_value());
    CHECK(core::ReadingSpeed::create(15.0, true, false).has_value());
}
```

- [ ] **Étape 2 — vérifier qu'il échoue à la compilation** (`create` n'existe
  pas).

- [ ] **Étape 3 — réécrire `ReadingSpeed`** dans `duration_adjustment.hpp` :

```cpp
struct ReadingSpeed {
    /// Compiles `charactersPerSecond`, or refuses zero and the negatives.
    [[nodiscard]] static std::optional<ReadingSpeed>
    create(double charactersPerSecond, bool lengthen, bool shorten);

    /// Visible characters per second. Strictly positive — `create` refuses
    /// anything else.
    [[nodiscard]] double charactersPerSecond() const { return m_charactersPerSecond; }

    [[nodiscard]] bool lengthen() const { return m_lengthen; }

    [[nodiscard]] bool shorten() const { return m_shorten; }

    friend bool operator==(const ReadingSpeed&, const ReadingSpeed&) = default;

private:
    ReadingSpeed(double charactersPerSecond, bool lengthen, bool shorten)
        : m_charactersPerSecond(charactersPerSecond), m_lengthen(lengthen), m_shorten(shorten) {}

    double m_charactersPerSecond;
    bool m_lengthen;
    bool m_shorten;
};
```

**Attention** : ce changement transforme des champs publics en accesseurs, et
casse donc toute construction agrégée existante (`ReadingSpeed{...}`), pas
seulement les deux du dialogue. Mettre à jour dans la foulée, au même commit :
`duration_adjustment.cpp` (fonctions `boundsOf`, `endFor`, `countSacrifices`,
qui lisent `speed.charactersPerSecond`, `speed.lengthen`, `speed.shorten` —
passer aux accesseurs), et le `.value_or(ReadingSpeed{})` de
`duration_adjust_dialog.cpp:98`, qui doit devenir
`.value_or(*ReadingSpeed::create(core::kDefaultReadingSpeed, true, false))` ou
une constante par défaut équivalente construite une fois.

- [ ] **Étape 4 — dans `duration_adjustment.cpp`**, remplacer chaque accès
  `speed.charactersPerSecond`/`.lengthen`/`.shorten` par
  `speed.charactersPerSecond()`/`.lengthen()`/`.shorten()`.

- [ ] **Étape 5 — dans `duration_adjust_dialog.cpp`**, ligne 98-101 :

```cpp
    const core::ReadingSpeed speed =
        initial.speed.value_or(*core::ReadingSpeed::create(core::kDefaultReadingSpeed, true, false));
    m_speed->setValue(speed.charactersPerSecond());
    m_lengthen->setChecked(initial.speed.has_value() && speed.lengthen());
    m_shorten->setChecked(initial.speed.has_value() && speed.shorten());
```

et ligne 129-135 (`constraints()`), en tenant compte du fait que la vitesse
saisie dans un `QDoubleSpinBox` borné à `[1.0, 99.0]` (ligne 93 du même
fichier) ne peut jamais produire un `create()` refusé en pratique — le
`.value()` y est donc sûr, avec un commentaire qui le dit plutôt qu'un silence
qui laisserait croire à un oubli :

```cpp
    std::optional<core::ReadingSpeed> speed;
    if (m_lengthen->isChecked() || m_shorten->isChecked()) {
        // The box is bounded to [1, 99]: `create` cannot refuse what it holds.
        speed = core::ReadingSpeed::create(
            m_speed->value(), m_lengthen->isChecked(), m_shorten->isChecked());
    }
```

- [ ] **Étape 6 — adapter les six tests existants** (`duration_adjustment_test.cpp`
  lignes 96, 110, 124, 140, 177 ; `window_durations_test.cpp` ligne 144) au
  nouveau constructeur : `ReadingSpeed{...}` → `*ReadingSpeed::create(...)`.

- [ ] **Étape 7 — vérifier que tout compile et que les tests passent.**

### 5.2 — Le maximum par défaut migre au noyau

**Fichiers :**
- Modifier : `src/lib/subedit/core/edit/duration_adjustment.hpp`
- Modifier : `src/lib/subedit/gui/duration_adjust_dialog.cpp`

- [ ] Ajouter dans `duration_adjustment.hpp`, à côté de
  `kDefaultMinimumMilliseconds` :

```cpp
/// Gaupol's default maximum duration, in milliseconds — off by default, and
/// this is only what a switched-off field shows.
inline constexpr std::int64_t kDefaultMaximumMilliseconds = 6000;
```

- [ ] Dans `duration_adjust_dialog.cpp`, retirer
  `constexpr double kDefaultMaximumSeconds = 6.0;` (ligne 32) et son usage
  (ligne 110), le remplacer par
  `secondsOf(core::Duration::fromMilliseconds(core::kDefaultMaximumMilliseconds))`.
  Les trois autres défauts (`kDefaultReadingSpeed`, `kDefaultMinimumMilliseconds`,
  le zéro de `gap`) sont déjà au noyau — rien à y changer.

- [ ] Vérifier la compilation.

### 5.3 — L'état du formulaire, distinct de la requête

**Fichiers :**
- Modifier : `src/lib/subedit/gui/duration_adjust_dialog.hpp`/`.cpp`
- Modifier : `src/lib/subedit/gui/main_window.cpp`/`.hpp`
- Test : `src/test/gui/window_durations_test.cpp`

**Le problème exact** (confirmé par lecture) : `main_window.hpp:652` stocke un
`core::DurationConstraints` — dont les champs optionnels décochés valent
`std::nullopt` — comme `initial` de la prochaine ouverture. La valeur qu'une
case décochée portait avant d'être décochée n'est jamais conservée nulle part
en dehors du widget Qt détruit avec le dialogue. Décision #409 : séparer un
type « ce que le formulaire affiche » — quatre valeurs, toujours présentes, et
leurs cases — de `DurationConstraints`, qui reste la requête que le noyau
reçoit.

- [ ] **Étape 1 — écrire le test qui échoue**

```cpp
TEST_CASE("an unchecked minimum keeps its value across two openings of the dialog",
          "[gui][GUI-DUR-01]") {
    InMemoryFileSystem files = withFile("f.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "f.srt"), prompts};
    window.show();

    prompts.fill = [](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        adjust.minimumBox()->setValue(2.0);
        adjust.minimumCheck()->setChecked(false);
    };
    prompts.nextRun = true;
    window.adjustDurationsAction()->trigger();

    prompts.fill = [](QDialog& dialog) {
        auto& adjust = dynamic_cast<DurationAdjustDialog&>(dialog);
        CHECK(adjust.minimumBox()->value() == 2.0);
    };
    window.adjustDurationsAction()->trigger();
}
```

(Vérifier le nom exact de l'action — `adjustDurationsAction()` est une
supposition à confirmer dans `main_window.hpp` avant d'écrire ce test ; si
l'accesseur n'existe pas encore, l'ajouter comme les autres `*Action()`
publics du fichier.)

- [ ] **Étape 2 — vérifier qu'il échoue** (le champ affiche 1,5 s, le défaut,
  et non 2,0 s).

- [ ] **Étape 3 — créer le type d'état de formulaire**

Dans `src/lib/subedit/core/config/duration_adjustment_settings.hpp` (nouveau
fichier, aux côtés de `search_options.hpp`, dont il suit le patron) :

```cpp
#pragma once

#include <subedit/core/edit/duration_adjustment.hpp>

#include <cstdint>

namespace subedit::core {

/// The form of `Adjust Durations…`, as the window keeps it from one opening to
/// the next and from one session to the next — decision of issue #409.
///
/// **Every value is always here, whether its case is checked or not.**
/// `DurationConstraints` turns an unchecked case into `std::nullopt`, which is
/// right for the core and wrong for a form: unchecking a field must not erase
/// the number it held, or checking it back on asks the user for it again.
struct DurationAdjustmentSettings {
    double charactersPerSecond = kDefaultReadingSpeed;
    bool lengthen = true;
    bool shorten = false;

    bool minimumEnabled = true;
    std::int64_t minimumMilliseconds = kDefaultMinimumMilliseconds;

    bool maximumEnabled = false;
    std::int64_t maximumMilliseconds = kDefaultMaximumMilliseconds;

    bool gapEnabled = true;
    std::int64_t gapMilliseconds = 0;

    friend bool operator==(const DurationAdjustmentSettings&,
                           const DurationAdjustmentSettings&) = default;
};

/// The request the core receives, built from what the form shows — an
/// unchecked field becomes absent, never zero by accident.
[[nodiscard]] DurationConstraints constraintsOf(const DurationAdjustmentSettings& settings);

} // namespace subedit::core
```

et son `.cpp` :

```cpp
#include <subedit/core/config/duration_adjustment_settings.hpp>

namespace subedit::core {

DurationConstraints constraintsOf(const DurationAdjustmentSettings& settings) {
    std::optional<ReadingSpeed> speed;
    if (settings.lengthen || settings.shorten) {
        // A settings file edited by hand could carry zero: the core has
        // nothing useful to do with it, so it is read as the speed being off
        // rather than refused outright — the tolerance ADR 0022 already
        // chooses for every other option.
        speed = ReadingSpeed::create(settings.charactersPerSecond, settings.lengthen, settings.shorten);
    }

    return DurationConstraints{
        .speed = speed,
        .minimum = settings.minimumEnabled
                      ? std::optional{Duration::fromMilliseconds(settings.minimumMilliseconds)}
                      : std::nullopt,
        .maximum = settings.maximumEnabled
                      ? std::optional{Duration::fromMilliseconds(settings.maximumMilliseconds)}
                      : std::nullopt,
        .gap = settings.gapEnabled ? std::optional{Duration::fromMilliseconds(settings.gapMilliseconds)}
                                   : std::nullopt};
}

} // namespace subedit::core
```

- [ ] **Étape 4 — le dialogue prend `DurationAdjustmentSettings` en entrée et
  en sortie**

`DurationAdjustDialog` : le constructeur reçoit
`const core::DurationAdjustmentSettings& initial` au lieu de
`const core::DurationConstraints&`. Une nouvelle méthode publique :

```cpp
[[nodiscard]] core::DurationAdjustmentSettings settings() const;
```

remplace `constraints()` comme ce que la fenêtre lit après `exec()` ;
`isComplete()` appelle `core::constraintsOf(settings()).isAny()`. Le
remplissage initial des champs (lignes 98-112 actuelles) se lit directement
sur `initial.*` plutôt que sur des `value_or` — plus besoin de `value_or`
puisque `DurationAdjustmentSettings` n'a pas de champ optionnel.

- [ ] **Étape 5 — `MainWindow` garde l'état plutôt que la requête**

`main_window.hpp:652` : `core::DurationConstraints m_durationConstraints;`
devient `core::DurationAdjustmentSettings m_durationSettings;`.
`main_window.cpp:1255-1272` (`adjustDurationsOfTarget`) :

```cpp
void MainWindow::adjustDurationsOfTarget() {
    commitCellEditor();
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    DurationAdjustDialog dialog{target.count(), m_durationSettings, this};
    if (!m_prompts->run(dialog))
        return;

    m_durationSettings = dialog.settings();

    core::DurationAdjustment adjustment = core::adjustDurations(
        m_session->project(), target, core::constraintsOf(m_durationSettings));
    if (adjustment.command != nullptr)
        applyOperation(std::move(adjustment.command), target);

    m_prompts->reportOutcome(core::noticeOfAdjustment(adjustment.adjusted, adjustment.sacrificed));
}
```

(`commitCellEditor()` n'apparaît ici que si la tâche 4 a déjà été intégrée —
`Adjust Durations…` a un dialogue, donc #397 ne l'exigeait pas à proprement
parler ; l'ajouter est sans risque et cohérent, mais n'est pas requis par
#397. Le laisser de côté si l'on préfère ne pas mélanger les deux tâches.)

- [ ] **Étape 6 — vérifier que le test de l'étape 1 passe**, et que les tests
  existants de `window_durations_test.cpp` passent toujours (adapter les
  éventuelles constructions de `DurationConstraints` en
  `DurationAdjustmentSettings` là où le test construisait l'état initial).

### 5.4 — Persistance dans `settings.conf`

**Fichiers :**
- Modifier : `src/lib/subedit/core/config/settings.hpp` (ajouter le champ)
- Modifier : `src/lib/subedit/core/config/settings.cpp` (clés, lecture,
  écriture)
- Modifier : `src/lib/subedit/gui/main_window.cpp` (`applySettings`,
  `settings()`)
- Modifier : `docs/manual/subedit-gui/operations.md` (ligne 222-223) et
  `docs/manual/subedit-gui/preferences.md` (lignes 170, 177-182)
- Test : `src/test/unit/core/config/settings_test.cpp` (confirmer le nom
  exact)

Suivre exactement le patron de `SearchOptions` (`settings.hpp:119`,
`settings.cpp:34-35,256-259,384-385`).

- [ ] **Étape 1 — écrire le test qui échoue**, sur le modèle des tests
  existants de `search.regex`/`search.ignore-case` dans
  `settings_test.cpp` — un aller-retour écriture/lecture qui vérifie que
  `durationAdjustment` survit, et qu'un fichier qui ne le mentionne pas
  retombe sur les défauts.

- [ ] **Étape 2 — ajouter le champ à `Settings`**

```cpp
/// The form of `Adjust Durations…`, kept from one opening of the dialog to
/// the next and from one session to the next — issue #409, the same
/// treatment as `search` just above.
DurationAdjustmentSettings durationAdjustment{};
```

(avec l'inclusion de `<subedit/core/config/duration_adjustment_settings.hpp>`
dans `settings.hpp`.)

- [ ] **Étape 3 — les clés**, à côté de `kSearchIgnoreCaseKey` :

```cpp
constexpr std::string_view kDurationSpeedKey = "duration-adjust.speed";
constexpr std::string_view kDurationLengthenKey = "duration-adjust.lengthen";
constexpr std::string_view kDurationShortenKey = "duration-adjust.shorten";
constexpr std::string_view kDurationMinimumEnabledKey = "duration-adjust.minimum-enabled";
constexpr std::string_view kDurationMinimumKey = "duration-adjust.minimum-ms";
constexpr std::string_view kDurationMaximumEnabledKey = "duration-adjust.maximum-enabled";
constexpr std::string_view kDurationMaximumKey = "duration-adjust.maximum-ms";
constexpr std::string_view kDurationGapEnabledKey = "duration-adjust.gap-enabled";
constexpr std::string_view kDurationGapKey = "duration-adjust.gap-ms";
```

**Un nombre à virgule est à lire différemment d'un entier** : `speed` est un
`double`, alors que `integerOf`/`booleanOf` couvrent le reste. Ajouter une
petite fonction voisine de `integerOf` :

```cpp
[[nodiscard]] std::optional<double> decimalOf(std::string_view text) {
    double value = 0.0;
    const char* const first = std::to_address(text.begin());
    const char* const last = std::to_address(text.end());
    const std::from_chars_result read = std::from_chars(first, last, value);
    if (read.ec != std::errc{} || read.ptr != last)
        return std::nullopt;
    return value;
}
```

- [ ] **Étape 4 — `applyOption`**

Le commentaire de la fonction dit déjà que la septième option l'a fait
franchir le seuil de complexité que la porte tolère — en ajouter neuf de plus
d'affilée le refranchirait presque certainement. **Avant d'ajouter les
branches, lancer `clang-tidy` sur `settings.cpp` seul** pour mesurer l'effet :

```console
$ ./src/scripts/gate.sh check --only tidy
```

Si le seuil est dépassé après ajout, ne pas empiler des `else if`
supplémentaires : regrouper les neuf clés de l'ajustement des durées dans une
fonction dédiée `applyDurationAdjustmentOption(read, key, value)` appelée
depuis `applyOption` par un unique `else if (key.starts_with("duration-adjust."))`
— une décomposition plutôt qu'une gonflette de la fonction existante, dans le
même esprit que ce qui a déjà été fait pour la faire tenir sous le seuil au
septième réglage.

```cpp
void applyDurationAdjustmentOption(SettingsRead& read, std::string_view key, std::string_view value) {
    const auto take = [&read, key, value](auto parsed, auto& field) {
        if (!parsed.has_value()) {
            read.diagnostics.push_back({.key = std::string{key}, .value = std::string{value}});
            return;
        }
        field = *std::move(parsed);
    };

    DurationAdjustmentSettings& duration = read.settings.durationAdjustment;
    if (key == kDurationSpeedKey)
        take(decimalOf(value), duration.charactersPerSecond);
    else if (key == kDurationLengthenKey)
        take(booleanOf(value), duration.lengthen);
    else if (key == kDurationShortenKey)
        take(booleanOf(value), duration.shorten);
    else if (key == kDurationMinimumEnabledKey)
        take(booleanOf(value), duration.minimumEnabled);
    else if (key == kDurationMinimumKey)
        take(integerOf(value), duration.minimumMilliseconds);
    else if (key == kDurationMaximumEnabledKey)
        take(booleanOf(value), duration.maximumEnabled);
    else if (key == kDurationMaximumKey)
        take(integerOf(value), duration.maximumMilliseconds);
    else if (key == kDurationGapEnabledKey)
        take(booleanOf(value), duration.gapEnabled);
    else if (key == kDurationGapKey)
        take(integerOf(value), duration.gapMilliseconds);
}
```

(`integerOf` rend un `int` ; `minimumMilliseconds` etc. sont des
`std::int64_t` — vérifier la conversion, ou changer `integerOf` pour un
gabarit couvrant les deux tailles si la porte l'exige. Un détail à trancher à
l'implémentation plutôt qu'ici : les deux options sont raisonnables, et le
choix dépend de ce que `clang-tidy`/`cppcoreguidelines` acceptent.)

Puis, dans `applyOption` :

```cpp
    else if (key.starts_with("duration-adjust."))
        applyDurationAdjustmentOption(read, key, value);
```

- [ ] **Étape 5 — `renderSettings`**, à côté du bloc de `search` :

```cpp
    const DurationAdjustmentSettings duration = settings.durationAdjustment;
    const DurationAdjustmentSettings durationDefaults;
    writeOption(out, kDurationSpeedKey, std::to_string(duration.charactersPerSecond),
                duration.charactersPerSecond == durationDefaults.charactersPerSecond);
    writeOption(out, kDurationLengthenKey, duration.lengthen ? "true" : "false",
                duration.lengthen == durationDefaults.lengthen);
    writeOption(out, kDurationShortenKey, duration.shorten ? "true" : "false",
                duration.shorten == durationDefaults.shorten);
    writeOption(out, kDurationMinimumEnabledKey, duration.minimumEnabled ? "true" : "false",
                duration.minimumEnabled == durationDefaults.minimumEnabled);
    writeOption(out, kDurationMinimumKey, std::to_string(duration.minimumMilliseconds),
                duration.minimumMilliseconds == durationDefaults.minimumMilliseconds);
    writeOption(out, kDurationMaximumEnabledKey, duration.maximumEnabled ? "true" : "false",
                duration.maximumEnabled == durationDefaults.maximumEnabled);
    writeOption(out, kDurationMaximumKey, std::to_string(duration.maximumMilliseconds),
                duration.maximumMilliseconds == durationDefaults.maximumMilliseconds);
    writeOption(out, kDurationGapEnabledKey, duration.gapEnabled ? "true" : "false",
                duration.gapEnabled == durationDefaults.gapEnabled);
    writeOption(out, kDurationGapKey, std::to_string(duration.gapMilliseconds),
                duration.gapMilliseconds == durationDefaults.gapMilliseconds);
```

- [ ] **Étape 6 — `main_window.cpp`**, dans `applySettings` (à côté de la
  ligne 1830) :

```cpp
    m_durationSettings = settings.durationAdjustment;
```

et dans `settings()` (à côté de la ligne 1866) :

```cpp
    settings.durationAdjustment = m_durationSettings;
```

- [ ] **Étape 7 — vérifier que le test de l'étape 1 passe, et que
  `make check` reste vert sur `settings.cpp`** (complexité comprise).

- [ ] **Étape 8 — mettre à jour le manuel**

`docs/manual/subedit-gui/operations.md`, lignes 222-223, remplacer :

```
Le dialogue rouvre sur les valeurs du dernier ajustement, le temps que la fenêtre
reste ouverte ; il ne les garde pas d'une session à l'autre.
```

par :

```
Le dialogue rouvre sur les valeurs du dernier ajustement, **et les garde d'une
session à l'autre** — comme les deux options de la recherche.
```

`docs/manual/subedit-gui/preferences.md` :

- ligne 170, « Cinq réglages » devient « Six réglages », et la liste gagne
  l'ajustement des durées :

```
**Six réglages retenus se choisissent dans un dialogue**, et chacun dans celui où
il sert : le thème, ici ; le côté d'une insertion, dans le dialogue
d'insertion ; les deux options de la recherche, dans le dialogue de
recherche ; l'encodage et la marque d'ordre des octets, dans `Save As…` ; la
vitesse de lecture, les durées et l'écart de l'ajustement des durées, dans son
propre dialogue.
```

- lignes 177-182, la phrase sur `Adjust Durations…` comme seule exception
  disparaît puisqu'il n'en est plus une :

```
Et **aucun nombre qui appartient à un document n'est gardé d'une session à
l'autre** : une durée de décalage, deux repères de transformation sont vrais
d'un fichier et faux du suivant. Les dialogues les redemandent à chaque
lancement, délibérément. Les réglages de `Adjust Durations…` font exception au
même titre que les deux options de la recherche : une vitesse de lecture, une
durée minimale ou un écart sont des choix de méthode plutôt que des mesures
d'un fichier, et persistent d'une session à l'autre pour cette raison.
```

- [ ] **Étape 9 — commiter.**

```
git -C /home/beber/Projects/subedit commit -m "$(cat <<'EOF'
feat(core): valider la vitesse de lecture et garder les réglages d'ajustement

Closes #409
EOF
)"
```

---

## Fin de lot — ce qui reste dû, une fois les cinq commits en place

Suivre l'ordre déjà décrit dans `CLAUDE.md` :

- [ ] relire l'ensemble du diff des cinq tâches ;
- [ ] bumper le patch dans `CMakeLists.txt` (une seule fois, à la fin) ;
- [ ] `make manual` ;
- [ ] `make check-local`, relever le banc ;
- [ ] `make check` ;
- [ ] vérifier que toute la documentation touchée est à jour (manuel,
  `CLAUDE.md`) — fait au fil des tâches ci-dessus, à relire une dernière fois ;
- [ ] régénérer le `CHANGELOG` ;
- [ ] ouvrir la pull request avec les cinq lignes `Closes #N` (#396, #404,
  #398, #397, #409) dans son corps, titre à la portée dominante (probablement
  `feat(gui):` ou `fix(gui):` selon ce qui domine — cinq issues touchant à la
  fois `core` et `gui`, à trancher à l'écriture du titre) et sous la limite de
  72 caractères, vérifié par
  `./src/scripts/check-commit-message.sh "<titre>"` avant de l'ouvrir.
