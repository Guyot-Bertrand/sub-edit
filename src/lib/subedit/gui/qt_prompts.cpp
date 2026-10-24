#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/video_file.hpp>
#include <subedit/core/wording/formats.hpp>
#include <subedit/gui/messages.hpp>
#include <subedit/gui/qt_prompts.hpp>
#include <subedit/gui/save_shape.hpp>

#include <QDialog>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::gui {

QString filterFor(core::SubtitleFormat format) {
    return QString::fromUtf8(core::nameOf(format)) + QStringLiteral(" (*") +
           QString::fromUtf8(core::extensionOf(format)) + QLatin1Char(')');
}

QString subtitleFilters() {
    // **Nine entries and not nine patterns.** `.sub` is SubViewer 2 and
    // MicroDVD, `.txt` is MPL2 and TMPlayer, so the first line — the one that
    // shows everything — carries each extension once, while the entries below
    // it name a format each. What tells them apart is the name, never the
    // pattern; `formatOfFilter` reads it back the same way.
    QString patterns;
    QString entries;
    for (const core::SubtitleFormat format : core::kSubtitleFormats) {
        const QString pattern = QStringLiteral("*") + QString::fromUtf8(core::extensionOf(format));
        if (!patterns.contains(pattern)) {
            if (!patterns.isEmpty())
                patterns += QLatin1Char(' ');
            patterns += pattern;
        }
        entries += QStringLiteral(";;") + filterFor(format);
    }

    // TRANSLATORS: a filter of the file dialog; %1 lists the patterns, « *.srt *.vtt ».
    return substitute(translate("Subtitles (%1)"), patterns) + entries + QStringLiteral(";;") +
           translate("All files (*)");
}

QString videoFilters() {
    QString patterns;
    for (const std::string_view extension : core::videoExtensions()) {
        if (!patterns.isEmpty())
            patterns += QLatin1Char(' ');
        patterns +=
            QStringLiteral("*") + QString::fromUtf8(extension.data(), qsizetype(extension.size()));
    }

    // TRANSLATORS: a filter of the file dialog; %1 lists the patterns, « *.mkv *.mp4 ».
    return substitute(translate("Videos (%1)"), patterns) + QStringLiteral(";;") +
           translate("All files (*)");
}

core::SubtitleFormat formatOfFilter(const QString& filter) {
    // **Read from the name and not from the pattern**, because two patterns
    // name two formats each. « Subtitles (…) » and « All files (*) » settle
    // nothing, and SubRip is then the default — it is the format the project
    // writes when nobody asks for another.
    for (const core::SubtitleFormat format : core::kSubtitleFormats) {
        if (filter == filterFor(format))
            return format;
    }
    return core::SubtitleFormat::SubRip;
}

UnsavedChoice choiceOf(int button) {
    switch (button) {
    case QMessageBox::Save:
        return UnsavedChoice::Save;
    case QMessageBox::Discard:
        return UnsavedChoice::Discard;
    default:
        // Closing the box by its cross or by Escape returns something else,
        // and anything that is not an explicit choice means « do nothing ».
        return UnsavedChoice::Cancel;
    }
}

namespace {

/// What a file chooser answered: the path, or nothing when it was closed.
[[nodiscard]] std::optional<std::filesystem::path> pathOf(const QString& chosen) {
    if (chosen.isEmpty())
        return std::nullopt;
    return std::filesystem::path{chosen.toStdString()};
}

} // namespace

std::optional<std::filesystem::path> QtPrompts::fileToOpen(const std::filesystem::path& directory) {
    return pathOf(QFileDialog::getOpenFileName(
        m_owner, translate("Open"), QString::fromStdString(directory.string()), subtitleFilters()));
}

std::optional<std::filesystem::path>
QtPrompts::videoToOpen(const std::filesystem::path& directory) {
    return pathOf(QFileDialog::getOpenFileName(m_owner,
                                               translate("Select Video"),
                                               QString::fromStdString(directory.string()),
                                               videoFilters()));
}

std::optional<std::filesystem::path> QtPrompts::modelToOpen(const std::filesystem::path& current) {
    return pathOf(QFileDialog::getOpenFileName(m_owner,
                                               translate("Choose a model"),
                                               QString::fromStdString(current.string()),
                                               translate("Models (*.bin);;All files (*)")));
}

std::optional<SaveTarget> QtPrompts::saveTarget(const core::SourceFile& current,
                                                const core::Encoding& encoding) {
    const std::unique_ptr<QFileDialog> dialog = saveDialogFor(current, encoding, m_owner);
    if (dialog->exec() != QDialog::Accepted)
        return std::nullopt;

    const std::expected<SaveTarget, std::string> target = targetOf(*dialog);
    if (!target.has_value()) {
        reportFailure(target.error());
        return std::nullopt;
    }

    return *target;
}

QString sentenceAboutUnsaved(const ModifiedDocument& document) {
    // What was always said, for the document that has always been there; and
    // the translation named as what it is, since a project may hold two.
    // Four whole sentences rather than a subject put into two: the subject's form may change
    // with what is said of it.
    if (document.document == core::Document::Translation)
        return document.missing ? translate("The translation is no longer where it was read from.")
                                : translate("The translation has changes that were never written.");
    return document.missing ? translate("The document is no longer where it was read from.")
                            : translate("The document has changes that were never written.");
}

UnsavedChoice QtPrompts::aboutUnsavedChanges(const ModifiedDocument& document) {
    return choiceOf(
        QMessageBox::question(m_owner,
                              translate("Unsaved changes"),
                              sentenceAboutUnsaved(document),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                              QMessageBox::Save));
}

bool QtPrompts::aboutLoss(const std::string& notice) {
    // **A warning and not a notice**, which is the icon `reportFailure` uses:
    // nothing has failed, and something is about to be given up for good. The
    // default is the safe one — a return key pressed without reading keeps the
    // file as it is.
    return QMessageBox::warning(m_owner,
                                translate("Saving will lose something"),
                                QString::fromStdString(notice),
                                QMessageBox::Save | QMessageBox::Cancel,
                                QMessageBox::Cancel) == QMessageBox::Save;
}

bool QtPrompts::proposeShiftedReopening(const std::string& message) {
    // TRANSLATORS: %1 says how the positions of the file were found to be off.
    const QString text =
        substitute(translate("%1\n\nOpen it again, moved?"), QString::fromStdString(message));
    const auto answers = QMessageBox::Yes | QMessageBox::No;
    const auto chosen =
        QMessageBox::question(m_owner, QStringLiteral("subedit"), text, answers, QMessageBox::No);
    return chosen == QMessageBox::Yes;
}

bool QtPrompts::confirmReplacement(const std::string& message) {
    // **A warning, with the safe answer by default**, as the loss of a save is: what is
    // there will be replaced, and a return key pressed without reading keeps it.
    return QMessageBox::warning(m_owner,
                                translate("Replace the subtitles?"),
                                QString::fromStdString(message),
                                QMessageBox::Yes | QMessageBox::Cancel,
                                QMessageBox::Cancel) == QMessageBox::Yes;
}

bool QtPrompts::run(QDialog& dialog) {
    return dialog.exec() == QDialog::Accepted;
}

void QtPrompts::reportFailure(const std::string& message) {
    QMessageBox::warning(m_owner, QStringLiteral("subedit"), QString::fromStdString(message));
}

void QtPrompts::reportOutcome(const std::string& message) {
    QMessageBox::information(m_owner, QStringLiteral("subedit"), QString::fromStdString(message));
}

} // namespace subedit::gui
