#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/video_file.hpp>
#include <subedit/core/wording.hpp>
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

    return QStringLiteral("Subtitles (") + patterns + QLatin1Char(')') + entries +
           QStringLiteral(";;All files (*)");
}

QString videoFilters() {
    QString patterns;
    for (const std::string_view extension : core::videoExtensions()) {
        if (!patterns.isEmpty())
            patterns += QLatin1Char(' ');
        patterns +=
            QStringLiteral("*") + QString::fromUtf8(extension.data(), qsizetype(extension.size()));
    }

    return QStringLiteral("Videos (") + patterns + QStringLiteral(");;All files (*)");
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

std::optional<std::filesystem::path> QtPrompts::fileToOpen(const std::filesystem::path& directory) {
    const QString chosen = QFileDialog::getOpenFileName(m_owner,
                                                        QStringLiteral("Open"),
                                                        QString::fromStdString(directory.string()),
                                                        subtitleFilters());
    if (chosen.isEmpty())
        return std::nullopt;

    return std::filesystem::path{chosen.toStdString()};
}

std::optional<std::filesystem::path>
QtPrompts::videoToOpen(const std::filesystem::path& directory) {
    const QString chosen = QFileDialog::getOpenFileName(m_owner,
                                                        QStringLiteral("Select Video"),
                                                        QString::fromStdString(directory.string()),
                                                        videoFilters());
    if (chosen.isEmpty())
        return std::nullopt;

    return std::filesystem::path{chosen.toStdString()};
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

UnsavedChoice QtPrompts::aboutUnsavedChanges() {
    return choiceOf(
        QMessageBox::question(m_owner,
                              QStringLiteral("Unsaved changes"),
                              QStringLiteral("The document has changes that were never written."),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                              QMessageBox::Save));
}

bool QtPrompts::aboutLoss(const std::string& notice) {
    // **A warning and not a notice**, which is the icon `reportFailure` uses:
    // nothing has failed, and something is about to be given up for good. The
    // default is the safe one — a return key pressed without reading keeps the
    // file as it is.
    return QMessageBox::warning(m_owner,
                                QStringLiteral("Saving will lose something"),
                                QString::fromStdString(notice),
                                QMessageBox::Save | QMessageBox::Cancel,
                                QMessageBox::Cancel) == QMessageBox::Save;
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
