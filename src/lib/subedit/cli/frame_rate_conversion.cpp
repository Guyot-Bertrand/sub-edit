#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/counts.hpp>

#include <expected>
#include <memory>
#include <string>

namespace subedit::cli {

ExitCode convertFrameRateAll(core::FileSystem& files,
                             const std::vector<std::string>& paths,
                             const std::optional<core::Encoding>& reading,
                             core::FrameRate input,
                             core::FrameRate output,
                             const Destination& destination,
                             const Reporter& reporter) {
    // No refusal of its own: the grammar has already established that both
    // texts named a rate, and two rates always define a ratio. This is the one
    // rewriting operation that cannot fail on the contents of a file.
    const Operation retime = [input, output](core::Session& session) -> OperationOutcome {
        session.apply(std::make_unique<core::ConvertFrameRateCommand>(
            session.project(), core::Selection::all(session.project()), input, output));
        const std::size_t total = session.project().count();
        return OperationResult{.sentence = core::substitute(
                                   // TRANSLATORS: %1 subtitles were retimed from the frame rate %2
                                   // to the frame rate %3.
                                   core::translatePlural("%1 subtitle retimed from %2 to %3 fps",
                                                         "%1 subtitles retimed from %2 to %3 fps",
                                                         total),
                                   {std::to_string(total), nameOf(input), nameOf(output)}),
                               .counts = {{"subtitles", static_cast<std::int64_t>(total)}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, Verb::Retimed, retime);
}

} // namespace subedit::cli
