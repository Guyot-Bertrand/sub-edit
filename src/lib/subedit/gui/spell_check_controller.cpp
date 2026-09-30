#include <subedit/core/edit/session.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/spell_check_walk.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/spell_dictionary.hpp>
#include <subedit/core/text/spell_replacements.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/correction_target.hpp>
#include <subedit/gui/join_split_page.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/spell_check_controller.hpp>
#include <subedit/gui/spell_check_dialog.hpp>
#include <subedit/gui/spell_check_settings_dialog.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>

#include <QLocale>
#include <QObject>

#include <memory>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

[[nodiscard]] ProjectPage* pageOwning(std::span<const std::unique_ptr<ProjectPage>> pages,
                                      const core::Project& project) {
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        if (&page->session->project() == &project)
            return page.get();
    }
    return nullptr;
}

[[nodiscard]] CorrectionScope scopeOf(core::SpellCheckTarget target) {
    switch (target) {
    case core::SpellCheckTarget::Selection:
        return CorrectionScope::Selection;
    case core::SpellCheckTarget::AllProjects:
        return CorrectionScope::AllProjects;
    case core::SpellCheckTarget::CurrentProject:
        break;
    }
    return CorrectionScope::CurrentProject;
}

[[nodiscard]] core::Document documentOf(core::SpellCheckDocument document) {
    return document == core::SpellCheckDocument::Translation ? core::Document::Translation
                                                             : core::Document::Main;
}

} // namespace

SpellCheckController::SpellCheckController(Prompts& prompts, View& view)
    : m_prompts(&prompts), m_view(&view) {}

std::string SpellCheckController::resolvedLanguage() const {
    if (!m_settings.language.empty())
        return m_settings.language;
    const core::SpellProvider* provider = m_view->spellProvider();
    const std::vector<std::string> offered =
        provider != nullptr ? provider->languages() : std::vector<std::string>{};
    return spellLanguageFor(offered, QLocale::system().name().toStdString());
}

bool SpellCheckController::spellCheckAvailable() const {
    return hasSpellDictionary(m_view->spellProvider(), resolvedLanguage());
}

std::string SpellCheckController::unavailableReason() const {
    const std::string language = resolvedLanguage();
    if (hasSpellDictionary(m_view->spellProvider(), language))
        return {};
    return core::noDictionaryFor(language);
}

void SpellCheckController::configure() {
    const std::span<const std::unique_ptr<ProjectPage>> pages = m_view->pages();
    const bool selectionAvailable =
        selectionOf(*pages[m_view->shownProject()]->tableSelection).count() != 0;
    bool translationAvailable = false;
    for (const std::unique_ptr<ProjectPage>& page : pages) {
        if (page->session->project().translationFile().has_value()) {
            translationAvailable = true;
            break;
        }
    }

    SpellCheckSettingsDialog dialog{
        m_view->spellProvider(), selectionAvailable, translationAvailable, m_view->dialogParent()};
    dialog.apply(m_settings);
    if (!m_prompts->run(dialog))
        return;
    m_settings = dialog.settings();
}

void SpellCheckController::openCheck() {
    const std::span<const std::unique_ptr<ProjectPage>> pages = m_view->pages();
    const core::SpellProvider* provider = m_view->spellProvider();
    const std::string language = resolvedLanguage();
    if (provider == nullptr) {
        m_view->announce(core::noDictionaryFor(language));
        return;
    }

    core::RealFileSystem files;
    const std::filesystem::path replacementFile =
        core::spellReplacementFile(m_view->spellConfigDirectory(), language);
    auto checker = core::openSpellChecker(*provider, language, files, replacementFile);
    if (!checker.has_value()) {
        m_view->announce(core::noDictionaryFor(language));
        return;
    }

    std::vector<core::CorrectionTarget> targets = correctionTargetsOf(
        scopeOf(m_settings.target), documentOf(m_settings.document), pages, m_view->shownProject());
    if (targets.empty()) {
        m_view->announce("Nothing to check.");
        return;
    }

    // Declared before the dialog, which holds a reference to it and is
    // destroyed first.
    core::SpellCheckWalk walk{std::move(*checker), std::move(targets)};
    SpellCheckDialog dialog{walk, m_view->dialogParent()};
    QObject::connect(
        &dialog, &SpellCheckDialog::stopped, &dialog, [this](const core::SpellStop& stop) {
            m_view->reveal(*stop.project, stop.index);
        });

    dialog.start();
    if (!dialog.walkFinished())
        static_cast<void>(m_prompts->run(dialog)); // however it closes, what was done is kept

    // Fetched again: the window may have opened or closed tabs meanwhile.
    const std::span<const std::unique_ptr<ProjectPage>> open = m_view->pages();
    const std::vector<core::ProposedCorrection> corrections = walk.corrections();
    std::vector<core::AppliedCorrection> composed = core::applyCorrections(corrections, false);
    const core::CorrectionTally tally = core::tallyOf(composed);
    for (core::AppliedCorrection& one : composed) {
        ProjectPage* owner = pageOwning(open, *one.project);
        if (owner == nullptr)
            continue; // its project closed while the window was open
        owner->model->applied(owner->session->apply(std::move(one.command)));
    }

    const auto saved = core::saveSpellReplacements(walk.checker(), files, replacementFile);
    if (!saved.has_value())
        m_prompts->reportFailure("Could not save the replacements for " + language + ": " +
                                 std::string{core::reasonOf(saved.error().kind)});

    m_view->announce(core::noticeOfCorrection(tally.corrected, tally.removed));
}

} // namespace subedit::gui
