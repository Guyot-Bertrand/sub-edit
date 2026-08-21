#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/qt_prompts.hpp>

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

#include <filesystem>
#include <optional>
#include <string>

namespace subedit::gui {

QString subtitleFilters() {
    return QStringLiteral("Subtitles (*.srt *.vtt);;SubRip (*.srt);;WebVTT (*.vtt);;All files (*)");
}

core::SubtitleFormat formatOfFilter(const QString& filter) {
    // WebVTT seulement si le filtre le nomme seul : « Subtitles (*.srt *.vtt) »
    // et « All files (*) » ne tranchent pas, et SubRip est alors le défaut —
    // c'est le format que le projet écrit sans qu'on lui demande.
    return filter.contains(QStringLiteral("*.vtt")) && !filter.contains(QStringLiteral("*.srt"))
               ? core::SubtitleFormat::WebVtt
               : core::SubtitleFormat::SubRip;
}

UnsavedChoice choiceOf(int button) {
    switch (button) {
    case QMessageBox::Save:
        return UnsavedChoice::Save;
    case QMessageBox::Discard:
        return UnsavedChoice::Discard;
    default:
        // Fermer la boîte par la croix ou par Échap rend autre chose, et tout
        // ce qui n'est pas un choix explicite vaut « ne rien faire ».
        return UnsavedChoice::Cancel;
    }
}

std::optional<std::filesystem::path> QtPrompts::fileToOpen() {
    const QString chosen =
        QFileDialog::getOpenFileName(m_owner, QStringLiteral("Open"), {}, subtitleFilters());
    if (chosen.isEmpty())
        return std::nullopt;

    return std::filesystem::path{chosen.toStdString()};
}

std::optional<SaveTarget> QtPrompts::saveTarget(const core::SourceFile& current) {
    const QString start =
        current.path.has_value() ? QString::fromStdString(current.path->string()) : QString{};

    QString picked = subtitleFilters().section(QStringLiteral(";;"), 1, 1);
    const QString chosen = QFileDialog::getSaveFileName(
        m_owner, QStringLiteral("Save As"), start, subtitleFilters(), &picked);
    if (chosen.isEmpty())
        return std::nullopt;

    return SaveTarget{.path = std::filesystem::path{chosen.toStdString()},
                      .format = formatOfFilter(picked)};
}

UnsavedChoice QtPrompts::aboutUnsavedChanges() {
    return choiceOf(
        QMessageBox::question(m_owner,
                              QStringLiteral("Unsaved changes"),
                              QStringLiteral("The document has changes that were never written."),
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                              QMessageBox::Save));
}

bool QtPrompts::run(QDialog& dialog) {
    return dialog.exec() == QDialog::Accepted;
}

void QtPrompts::reportFailure(const std::string& message) {
    QMessageBox::warning(m_owner, QStringLiteral("subedit"), QString::fromStdString(message));
}

} // namespace subedit::gui
