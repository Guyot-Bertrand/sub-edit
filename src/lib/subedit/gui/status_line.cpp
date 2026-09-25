#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/status_line.hpp>
#include <subedit/gui/target.hpp>

#include <QLabel>
#include <QStatusBar>
#include <QString>

#include <filesystem>
#include <optional>
#include <variant>

namespace subedit::gui {

std::optional<core::FrameRate> rateReadInFrames(const core::Project& project) {
    const core::FileExtras& extras = project.sourceFile().extras;
    if (const auto* frames = std::get_if<core::MicroDvdFile>(&extras))
        return frames->rate;
    return std::nullopt;
}

StatusLine::StatusLine(QStatusBar& bar)
    : m_target(new QLabel{&bar}),
      m_encoding(new QLabel{&bar}),
      m_grid(new QLabel{&bar}),
      m_video(new QLabel{&bar}) {
    // The text an operation aims at first: it is the one that changes with a
    // key press, and it is absent for most users. Then the encoding, being the
    // only one of the other three that describes the file rather than what is
    // deduced from it or associated with it.
    bar.addPermanentWidget(m_target);
    m_target->hide();
    bar.addPermanentWidget(m_encoding);
    bar.addPermanentWidget(m_grid);
    bar.addPermanentWidget(m_video);
}

void StatusLine::showTarget(std::optional<core::Document> aimed) {
    if (aimed.has_value())
        m_target->setText(QStringLiteral("Text: %1").arg(documentName(*aimed)));
    else
        m_target->clear();
    m_target->setVisible(aimed.has_value());
}

void StatusLine::refreshEncoding(const core::Project& project) {
    // **What the document is, and not what its reading did** — issue #313. The
    // window said the encoding in a diagnostic and nowhere else, so it said it
    // only when the encoding had been guessed: a file that declares its own
    // with a mark showed nothing at all, where `inspect` writes « UTF-16LE,
    // from its byte order mark ». The command line had three answers, the
    // window one and a half.
    //
    // Where the answer came from stays with the diagnostics panel, which exists
    // to say what happened; this line says what is, permanently, as the grid's
    // and the film's do.
    m_encoding->setText(
        QString::fromStdString(core::encodingStatusOf(project.sourceFile().encoding)));
}

void StatusLine::refreshGrid(const core::Project& project) {
    // **A document counted in frames gets its rate, not a grid.** Deducing one
    // from positions that were computed *from* frames at that very rate would
    // answer with the number it was given — the same choice `inspect` makes.
    if (rateReadInFrames(project).has_value()) {
        // **The document's rate and not the file's**, so that correcting it
        // through `Convert Frame Rate…` shows: what the line says is what the
        // positions are counted at now, and what writing MicroDVD back will use.
        m_grid->setText(QString::fromStdString(core::framesStatusOf(project.frameRate())));
        return;
    }

    const core::FrameRateDeduction deduced = core::deduceFrameRate(project);
    const std::optional<core::FrameRate> retained = deduced.verdict == core::GridVerdict::Silent
                                                        ? std::nullopt
                                                        : std::optional{deduced.retained.rate};

    m_grid->setText(QString::fromStdString(core::gridStatusOf(deduced.verdict, retained)));
}

void StatusLine::refreshVideo(const core::Project& project) {
    const std::optional<core::AssociatedVideo>& associated = project.video();
    const std::optional<std::filesystem::path> path =
        associated.has_value() ? std::optional{associated->path} : std::nullopt;
    const std::optional<core::FrameRate> declared =
        associated.has_value() ? associated->declared : std::nullopt;

    m_video->setText(QString::fromStdString(core::videoStatusOf(path, declared)));
}

} // namespace subedit::gui
