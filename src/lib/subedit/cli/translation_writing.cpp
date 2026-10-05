#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/translation_writing.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/wording/translation.hpp>

#include <cstdint>
#include <vector>

namespace subedit::cli {

ExitCode writeTranslationAt(core::FileSystem& files,
                            const std::string& mainPath,
                            const std::optional<core::Encoding>& reading,
                            const Destination& destination,
                            const Reporter& reporter,
                            const Pairing& pairing) {
    // Nothing is changed: what is written is what the pairing made of the
    // translation, and the sentence is the alignment itself.
    const ChangingOperation lay = [](core::Session& session,
                                     const Request& request) -> OperationOutcome {
        return OperationResult{
            .sentence = core::noticeOf(request.alignment.value_or(core::TranslationOutcome{})),
            .counts = {{"subtitles", static_cast<std::int64_t>(session.project().count())}}};
    };
    return rewriteAll(
        files, {mainPath}, reading, destination, reporter, "paired", lay, std::nullopt, pairing);
}

} // namespace subedit::cli
