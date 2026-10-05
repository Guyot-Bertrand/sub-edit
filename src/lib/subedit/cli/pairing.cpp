#include <subedit/cli/pairing.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/translation_file.hpp>
#include <subedit/core/wording/translation.hpp>

#include <string>
#include <utility>

namespace subedit::cli {

namespace {

[[nodiscard]] std::string_view idOf(core::TranslationMethod method) {
    return method == core::TranslationMethod::Position ? "position" : "number";
}

} // namespace

std::expected<Paired, Failure> pair(const core::FileSystem& files,
                                    core::Session& session,
                                    const Pairing& pairing,
                                    const core::ReadingChoices& reading) {
    std::expected<core::TranslationFile, core::TranslationError> read =
        core::openTranslation(files, session.project(), pairing.translation, reading);
    if (!read) {
        return std::unexpected{
            Failure{idOf(read.error()), std::string{core::reasonOf(read.error())}}};
    }

    core::AttachedTranslation attached =
        core::attachTranslation(session.project(), read->lines, read->source, pairing.method);
    session.apply(std::move(attached.command));
    return Paired{.outcome = attached.outcome, .diagnostics = std::move(read->diagnostics)};
}

Json alignmentOf(const Pairing& pairing, const core::TranslationOutcome& outcome) {
    return Json::object()
        .set("file", pairing.translation)
        .set("method", idOf(pairing.method))
        .set("attached", outcome.attached)
        .set("born", outcome.born)
        .set("untranslated", outcome.untranslated)
        .set("out_of_order", outcome.outOfOrder);
}

} // namespace subedit::cli
