#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/project_search.hpp>
#include <subedit/gui/search_dialog.hpp>
#include <subedit/gui/target.hpp>

#include <QString>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace subedit::gui {

ProjectSearch::ProjectSearch(View& view, QWidget* parent) : m_view(&view), m_parent(parent) {}

ProjectSearch::~ProjectSearch() {
    for (const QMetaObject::Connection& connection : m_connections)
        QObject::disconnect(connection);
}

ProjectPage& ProjectSearch::shown() {
    return m_view->project(m_view->shownProject());
}

void ProjectSearch::open() {
    if (m_dialog == nullptr) {
        m_dialog = new SearchDialog{m_parent};
        m_dialog->setOptions(m_options);
        m_connections = {
            QObject::connect(
                m_dialog, &SearchDialog::findNextRequested, m_dialog, [this] { find(true); }),
            QObject::connect(
                m_dialog, &SearchDialog::findPreviousRequested, m_dialog, [this] { find(false); }),
            QObject::connect(
                m_dialog, &SearchDialog::replaceRequested, m_dialog, [this] { replace(); }),
            QObject::connect(
                m_dialog, &SearchDialog::replaceAllRequested, m_dialog, [this] { replaceAll(); }),
            QObject::connect(m_dialog,
                             &SearchDialog::searchChanged,
                             m_dialog,
                             [this] {
                                 m_options = m_dialog->options();
                                 // Every project's, not only the one shown: a match remembered
                                 // by a tab that is not on screen is a match of another pattern.
                                 for (int index = 0; index < m_view->projectCount(); ++index)
                                     m_view->project(index).match.reset();
                                 m_dialog->setStatus({});
                             }),
        };
    }

    refresh();
    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void ProjectSearch::setOptions(core::SearchOptions options) {
    m_options = options;
    if (m_dialog != nullptr)
        m_dialog->setOptions(m_options);
}

void ProjectSearch::refresh() {
    // A match is a place in one text: another text, another search. Compared
    // with the last one aimed at rather than forgotten at every call, because
    // this runs after each operation and the match just written is the one the
    // next `Find Next` starts from.
    ProjectPage& page = shown();
    const core::Document aimed = m_view->targetDocument();
    if (aimed != page.searchDocument) {
        page.searchDocument = aimed;
        page.match.reset();
    }

    if (m_dialog == nullptr)
        return;

    m_dialog->setAllProjectsAvailable(m_view->projectCount() > 1);

    // Two texts, and only then: the box of a window that never opens a
    // translation has nothing to choose between, and says nothing.
    m_dialog->setField(m_view->twoTexts()
                           ? QStringLiteral("Searching in: %1").arg(documentName(aimed))
                           : QString{});
}

std::optional<core::SearchPattern> ProjectSearch::pattern() {
    std::expected<core::SearchPattern, core::PatternError> compiled =
        core::SearchPattern::compile(m_dialog->pattern().toStdString(), m_options);
    if (!compiled.has_value()) {
        m_dialog->setStatus(QString::fromStdString(core::reasonOf(compiled.error())));
        return std::nullopt;
    }
    return std::move(*compiled);
}

core::Selection ProjectSearch::target() {
    ProjectPage& page = shown();
    if (!page.searchTarget.has_value())
        page.searchTarget = m_view->selectionTarget();
    return *page.searchTarget;
}

void ProjectSearch::find(bool forward) {
    const std::optional<core::SearchPattern> compiled = pattern();
    if (!compiled.has_value())
        return;

    if (m_dialog->allProjects()) {
        findAcrossProjects(forward, *compiled);
        return;
    }

    const core::Selection aimed = target();
    ProjectPage& page = shown();
    const core::Document document = m_view->targetDocument();
    const std::optional<core::TextMatch> found =
        forward
            ? core::findNext(page.session->project(), aimed, document, *compiled, page.match)
            : core::findPrevious(page.session->project(), aimed, document, *compiled, page.match);

    // **A search that finds nothing says so, and touches nothing**: the
    // selection stays where it was, and so does the target.
    if (!found.has_value()) {
        page.match.reset();
        m_dialog->setStatus(
            QString::fromStdString(core::notFound(m_dialog->pattern().toStdString())));
        return;
    }

    page.match = found;
    m_dialog->setStatus({});
    m_view->moveTo(static_cast<int>(found->index.value()));
}

void ProjectSearch::findAcrossProjects(bool forward, const core::SearchPattern& compiled) {
    const core::Document document = m_view->targetDocument();
    const auto findIn = [&](const ProjectPage& page, const std::optional<core::TextMatch>& from) {
        const core::Project& project = page.session->project();
        const core::Selection whole = core::Selection::all(project);
        return forward ? core::findNext(project, whole, document, compiled, from)
                       : core::findPrevious(project, whole, document, compiled, from);
    };
    // Whether `match` comes on the far side of `from` in the direction of the
    // search — what a match that wrapped round inside one project does not.
    const auto beyond = [forward](const core::TextMatch& match, const core::TextMatch& from) {
        if (match.index != from.index)
            return forward ? match.index.value() > from.index.value()
                           : match.index.value() < from.index.value();
        return forward ? match.start > from.start : match.start < from.start;
    };

    const int projects = m_view->projectCount();
    const int origin = m_view->shownProject();
    ProjectPage& first = m_view->project(origin);

    // The project shown first: what follows its current match, or its first.
    std::optional<core::TextMatch> found = findIn(first, first.match);
    bool wrapped = found.has_value() && first.match.has_value() && !beyond(*found, *first.match);
    int where = origin;

    // Nothing more in this one — or only the wrapped round of it: the other
    // projects are visited, in the order of the tabs, before coming back.
    if (!found.has_value() || wrapped) {
        for (int step = 1; step < projects; ++step) {
            const int index =
                forward ? (origin + step) % projects : (origin - step + projects) % projects;
            const std::optional<core::TextMatch> other =
                findIn(m_view->project(index), std::nullopt);
            if (!other.has_value())
                continue;

            found = other;
            where = index;
            // Past the last tab, or before the first, is the round coming
            // back to where it began.
            wrapped = forward ? index < origin : index > origin;
            break;
        }
    }

    if (!found.has_value()) {
        first.match.reset();
        m_dialog->setStatus(
            QString::fromStdString(core::notFound(m_dialog->pattern().toStdString())));
        return;
    }

    m_view->show(where);
    m_view->project(where).match = found;
    m_dialog->setStatus(wrapped ? QStringLiteral("Search wrapped around") : QString{});
    m_view->moveTo(static_cast<int>(found->index.value()));
}

void ProjectSearch::replace() {
    const std::optional<core::SearchPattern> compiled = pattern();
    if (!compiled.has_value())
        return;

    // Nothing found yet, or the text moved under the match: find first, as
    // Gaupol does, and let the next press replace what is then shown.
    ProjectPage& page = shown();
    std::optional<core::ReplacedMatch> replaced;
    if (page.match.has_value()) {
        replaced = core::replaceMatch(page.session->project(),
                                      m_view->targetDocument(),
                                      *compiled,
                                      *page.match,
                                      m_dialog->replacement().toStdString());
    }
    if (!replaced.has_value()) {
        find(true);
        return;
    }

    if (replaced->command != nullptr) {
        const core::SubtitleIndex index = replaced->written.index;
        m_view->apply(page, std::move(replaced->command), core::Selection::range(index, index));
    }
    page.match = replaced->written;
    find(true);
}

void ProjectSearch::replaceAll() {
    const std::optional<core::SearchPattern> compiled = pattern();
    if (!compiled.has_value())
        return;

    if (m_dialog->allProjects()) {
        replaceAllAcrossProjects(*compiled);
        return;
    }

    const core::Selection aimed = target();
    ProjectPage& page = shown();
    core::ReplacedAll replaced = core::replaceAll(page.session->project(),
                                                  aimed,
                                                  m_view->targetDocument(),
                                                  *compiled,
                                                  m_dialog->replacement().toStdString());
    page.match.reset();

    // **Not found is not the same as nothing to change**: a pattern that is in
    // the document but is replaced by itself finds matches and writes nothing,
    // and no history entry is made for it either way.
    if (replaced.count == 0) {
        m_dialog->setStatus(QString::fromStdString(
            replaced.matched == 0 ? core::notFound(m_dialog->pattern().toStdString())
                                  : core::nothingToChange()));
        return;
    }

    if (replaced.command != nullptr)
        m_view->apply(page, std::move(replaced.command), aimed);
    m_dialog->setStatus(QString::fromStdString(core::noticeOfReplaceAll(replaced.count)));
}

void ProjectSearch::replaceAllAcrossProjects(const core::SearchPattern& compiled) {
    const core::Document document = m_view->targetDocument();
    const std::string replacement = m_dialog->replacement().toStdString();

    std::size_t matched = 0;
    std::size_t replacedCount = 0;
    std::size_t touched = 0;
    for (int index = 0; index < m_view->projectCount(); ++index) {
        ProjectPage& page = m_view->project(index);
        const core::Project& project = page.session->project();
        const core::Selection whole = core::Selection::all(project);
        core::ReplacedAll replaced =
            core::replaceAll(project, whole, document, compiled, replacement);
        matched += replaced.matched;
        if (replaced.count == 0 || replaced.command == nullptr)
            continue;

        // Given to its page, behind its tab: bringing each tab forward in
        // turn would make the screen flicker and the player open every film
        // on the way, for a gesture made from one tab — issue #461.
        m_view->apply(page, std::move(replaced.command), whole);
        replacedCount += replaced.count;
        ++touched;
    }

    for (int index = 0; index < m_view->projectCount(); ++index)
        m_view->project(index).match.reset();

    if (replacedCount == 0) {
        m_dialog->setStatus(
            QString::fromStdString(matched == 0 ? core::notFound(m_dialog->pattern().toStdString())
                                                : core::nothingToChange()));
        return;
    }
    m_dialog->setStatus(QString::fromStdString(core::noticeOfReplaceAll(replacedCount, touched)));
}

} // namespace subedit::gui
