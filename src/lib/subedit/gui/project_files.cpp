#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/translation_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/video_file.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/open_translation_dialog.hpp>
#include <subedit/gui/project_files.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QCheckBox>
#include <QString>

#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace subedit::gui {

namespace {

constexpr std::array<core::Document, 2> kDocuments = {core::Document::Main,
                                                      core::Document::Translation};

} // namespace

ProjectFiles::ProjectFiles(core::FileSystem& files, Prompts& prompts, View& view)
    : m_files(&files), m_prompts(&prompts), m_view(&view) {}

void ProjectFiles::setLastDirectory(std::filesystem::path directory) {
    m_lastDirectory = std::move(directory);
}

void ProjectFiles::rememberDirectoryOf(const std::filesystem::path& file) {
    if (file.has_parent_path())
        m_lastDirectory = file.parent_path();
}

bool ProjectFiles::save(ProjectPage& page, core::Document document) {
    const core::SourceFile& source = page.session->project().sourceFile(document);
    if (!source.path.has_value())
        return saveAs(page, document);

    const std::expected<void, core::SaveError> written =
        core::saveProject(*m_files, page.session->project(), document, *source.path, source.format);
    if (!written) {
        m_prompts->reportFailure(source.path->string() + ": " +
                                 std::string{core::reasonOf(written.error())});
        return false;
    }

    rememberDirectoryOf(*source.path);
    page.session->markSaved(document);
    m_view->saved(page, document, false);
    return true;
}

bool ProjectFiles::saveIn(int index, core::Document document, bool& shown) {
    ProjectPage& page = m_view->project(index);
    if (!page.session->project().sourceFile(document).path.has_value()) {
        m_view->show(index);
        shown = true;
    }
    return save(page, document);
}

bool ProjectFiles::saveAs(ProjectPage& page, core::Document document) {
    const core::SourceFile& source = page.session->project().sourceFile(document);

    // **The encoding of the file wins over the setting**, and the setting
    // serves the document with no file: rewriting a document one has just
    // opened in another encoding, because a setting three weeks old says so,
    // would be losing what the reading took care to keep.
    const core::Encoding proposed =
        source.path.has_value() ? source.encoding : page.writeEncoding.value_or(source.encoding);

    const std::optional<SaveTarget> target = m_prompts->saveTarget(source, proposed);
    if (!target.has_value())
        return false;

    // **What the arriving format will not carry, said before the writing.** The
    // command line prints the same words afterwards, where they are a report;
    // asked here they are a warning, and the difference is that the answer can
    // still be « no ». ADR 0031: the tags are translated on the way, so the
    // count of what fell is the count of a conversion that really happened.
    const core::SourceFile before = page.session->project().sourceFile(document);
    const std::span<const core::Subtitle> held = page.session->project().subtitles();
    // **The document's own rate, and it is a real answer here.** A file counted
    // in frames was read at it, `Convert Frame Rate…` moves it, and nothing
    // else in this window can leave it unset — so the command line's third
    // case, « no rate and no grid, refuse », cannot arise.
    core::ConvertedProject converted = core::convertProjectFor(
        page.session->project(), document, target->format, page.session->project().frameRate());

    if (const std::string notice = core::noticeOf(converted.loss, before.format, target->format);
        !notice.empty() && !m_prompts->aboutLoss(notice)) {
        return false;
    }

    // What the document becomes, laid down before the writing: `saveProject`
    // writes what the project carries, and what it carries is now what has just
    // been chosen. One act and not two — a format and the texts that speak it —
    // and not a command, for the reasons `Session::becomeFile` writes out.
    const std::vector<core::Subtitle> heldBefore{held.begin(), held.end()};
    core::SourceFile moved = before;
    moved.path = target->path;
    moved.format = target->format;
    moved.encoding = target->encoding;
    moved.newline = target->newline;
    // What the file declares of itself follows the conversion, which is the one
    // place that decides what crosses a format boundary — ADR 0030.
    moved.extras = converted.extras;
    moved.header = converted.header;
    page.session->becomeFile(document, moved, std::move(converted.subtitles));

    const std::expected<void, core::SaveError> written = core::saveProject(
        *m_files, page.session->project(), document, target->path, target->format);
    if (!written) {
        // **And undone when the writing fails.** A document that was not
        // written has not moved: without this step back it aims at a file that
        // does not exist, the title still shows the old name, and `Save` writes
        // somewhere other than where anyone thinks. The case has been reachable
        // since phase 8: a `ł` and a Latin-1 encoding are enough, and it does
        // not even ask the disk to refuse.
        page.session->becomeFile(document, before, heldBefore);
        m_prompts->reportFailure(target->path.string() + ": " +
                                 std::string{core::reasonOf(written.error())});
        return false;
    }

    rememberDirectoryOf(target->path);

    // Kept even if the document already had one: it is a choice that has just
    // been made, and the next document with no file will open on it.
    page.writeEncoding = target->encoding;

    page.session->markSaved(document);

    // The format governs the decimal mark the table shows, and it is the main
    // document's; a translation moved to another format rewrote its own texts.
    // Either way everything on screen is to be read again.
    page.model->refreshAll();
    m_view->saved(page, document, true);
    return true;
}

bool ProjectFiles::isModified(const ProjectPage& page, core::Document document) {
    // A translation counts only while there is one: undoing the opening of it
    // takes its file away, and what is left has nothing to differ from.
    if (document == core::Document::Translation &&
        !page.session->project().translationFile().has_value())
        return false;

    return page.session->hasUnsavedChanges(document);
}

std::vector<ModifiedDocument> ProjectFiles::modifiedDocuments(const ProjectPage& page) const {
    std::vector<ModifiedDocument> modified;

    for (const core::Document document : kDocuments) {
        if (document == core::Document::Translation &&
            !page.session->project().translationFile().has_value())
            continue;

        const core::SourceFile& source = page.session->project().sourceFile(document);
        const bool missing = source.path.has_value() && !m_files->exists(*source.path);
        if (!isModified(page, document) && !missing)
            continue;

        modified.push_back(ModifiedDocument{
            .document = document,
            .name = source.path.has_value() ? source.path->filename().string() : "untitled",
            .missing = missing,
        });
    }

    return modified;
}

bool ProjectFiles::mayDiscard(int index) {
    const std::vector<ModifiedDocument> modified = modifiedDocuments(m_view->project(index));
    return mayDiscard(modified, std::vector<int>(modified.size(), index));
}

bool ProjectFiles::mayDiscardAll() {
    // Every modified document of every project, and the project each one is
    // in. Names are not qualified by project: the kind and the file's name say
    // which document it is, and Gaupol's own list does no more.
    std::vector<ModifiedDocument> modified;
    std::vector<int> owners;
    for (int index = 0; index < m_view->projectCount(); ++index) {
        for (const ModifiedDocument& document : modifiedDocuments(m_view->project(index))) {
            modified.push_back(document);
            owners.push_back(index);
        }
    }

    return mayDiscard(modified, owners);
}

bool ProjectFiles::mayDiscard(const std::vector<ModifiedDocument>& modified,
                              const std::vector<int>& owners) {
    if (modified.empty())
        return true;

    // One document is the question it always was, whichever tab it is in — and
    // the tab is brought forward first, so that a `Save As…` opens over it.
    if (modified.size() == 1) {
        m_view->show(owners.front());
        switch (m_prompts->aboutUnsavedChanges(modified.front())) {
        case UnsavedChoice::Save:
            return save(m_view->project(owners.front()), modified.front().document);
        case UnsavedChoice::Discard:
            return true;
        case UnsavedChoice::Cancel:
            return false;
        }

        std::unreachable();
    }

    UnsavedDocumentsDialog dialog{modified, m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return false;

    switch (dialog.choice()) {
    case UnsavedChoice::Save: {
        // Read off the boxes and not off `toSave()`: that names documents, and
        // two projects both have a main one.
        bool shown = false;
        for (std::size_t index = 0; index < modified.size(); ++index) {
            if (!dialog.boxes().at(static_cast<qsizetype>(index))->isChecked())
                continue;
            if (!saveIn(owners.at(index), modified.at(index).document, shown))
                return false;
        }
        return true;
    }
    case UnsavedChoice::Discard:
        return true;
    case UnsavedChoice::Cancel:
        return false;
    }

    std::unreachable();
}

void ProjectFiles::saveAll() {
    const int origin = m_view->shownProject();
    const int projects = m_view->projectCount();

    int total = 0;
    for (int index = 0; index < projects; ++index) {
        for (const core::Document document : kDocuments)
            total += isModified(m_view->project(index), document) ? 1 : 0;
    }

    int written = 0;
    bool stopped = false;
    bool shown = false;
    // In the order of the tabs. `save` asks for a name when a document has
    // none, and a `Save As…` given up — or a write that fails — ends the
    // series: what follows would be answering for someone who left.
    for (int index = 0; index < projects && !stopped; ++index) {
        for (const core::Document document : kDocuments) {
            if (!isModified(m_view->project(index), document))
                continue;
            if (!saveIn(index, document, shown)) {
                stopped = true;
                break;
            }
            ++written;
        }
    }

    if (shown)
        m_view->show(origin);

    const auto documents = [](int count) {
        return std::to_string(count) + (count == 1 ? " document" : " documents");
    };
    if (stopped) {
        m_prompts->reportOutcome("Save All stopped: " + std::to_string(written) + " of " +
                                 documents(total) + " saved");
        return;
    }
    m_view->announce(total == 0 ? "Nothing to save" : documents(written) + " saved");
}

bool ProjectFiles::mayReplaceTranslation(ProjectPage& page) {
    if (!isModified(page, core::Document::Translation))
        return true;

    const core::SourceFile& source =
        page.session->project().sourceFile(core::Document::Translation);
    const ModifiedDocument modified{
        .document = core::Document::Translation,
        .name = source.path.has_value() ? source.path->filename().string() : "untitled",
    };

    switch (m_prompts->aboutUnsavedChanges(modified)) {
    case UnsavedChoice::Save:
        return save(page, core::Document::Translation);
    case UnsavedChoice::Discard:
        return true;
    case UnsavedChoice::Cancel:
        return false;
    }

    std::unreachable();
}

std::optional<ProjectFiles::ChosenTranslation> ProjectFiles::chooseTranslation(ProjectPage& page) {
    // Asked before asking what to open, for the reason the main document's is:
    // giving up rather than losing one's work should not require choosing a
    // file first.
    if (!mayReplaceTranslation(page))
        return std::nullopt;

    const std::optional<std::filesystem::path> chosen = m_prompts->fileToOpen(m_lastDirectory);
    if (!chosen.has_value())
        return std::nullopt;

    // Read before the method is asked: a file that will not open, or that is the
    // main document itself, is not worth a question about how to align it.
    std::expected<core::TranslationFile, core::TranslationError> read =
        core::openTranslation(*m_files, page.session->project(), *chosen);
    if (!read) {
        m_prompts->reportFailure(chosen->string() + ": " +
                                 std::string{core::reasonOf(read.error())});
        return std::nullopt;
    }

    OpenTranslationDialog dialog{QString::fromStdString(chosen->filename().string()),
                                 m_view->dialogParent()};
    if (!m_prompts->run(dialog))
        return std::nullopt;

    rememberDirectoryOf(*chosen);
    return ChosenTranslation{.read = std::move(*read), .method = dialog.method()};
}

std::optional<std::filesystem::path> ProjectFiles::askFileToOpen() {
    return m_prompts->fileToOpen(m_lastDirectory);
}

std::expected<core::OpenedFile, std::string> ProjectFiles::read(const std::filesystem::path& path) {
    std::expected<core::OpenedFile, core::OpenError> opened = core::openProject(*m_files, path);
    if (!opened)
        return std::unexpected{path.string() + ": " + std::string{core::reasonOf(opened.error())}};

    // **Kept here and not at the asking**: what counts is where the user
    // works, not where they looked. A box dismissed, or a file that does not
    // open, therefore moves nothing.
    rememberDirectoryOf(path);
    return std::move(*opened);
}

std::optional<int> ProjectFiles::indexOf(const std::filesystem::path& path) {
    // **However the path is spelled** — `film.srt`, `./film.srt` and
    // `../films/film.srt` name one file — the same rule `openTranslation`
    // uses for the main document, compared without asking the disk.
    const std::filesystem::path normalized = path.lexically_normal();
    for (int index = 0; index < m_view->projectCount(); ++index) {
        const std::optional<std::filesystem::path>& open =
            m_view->project(index).session->project().sourceFile().path;
        if (open.has_value() && open->lexically_normal() == normalized)
            return index;
    }
    return std::nullopt;
}

ProjectFiles::Dropped ProjectFiles::sort(std::span<const std::filesystem::path> paths) {
    Dropped dropped;
    for (const std::filesystem::path& path : paths)
        (core::isVideoFile(path) ? dropped.films : dropped.subtitles).push_back(path);
    return dropped;
}

} // namespace subedit::gui
