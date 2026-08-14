#include <subedit/cli/frame_rate_conversion.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/wording.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <expected>
#include <memory>
#include <string>

namespace subedit::cli {

ExitCode convertFrameRateAll(core::FileSystem& files,
                             const std::vector<std::string>& paths,
                             core::FrameRate input,
                             core::FrameRate output,
                             const Destination& destination,
                             const Reporter& reporter) {
    // No refusal of its own: the grammar has already established that both
    // texts named a rate, and two rates always define a ratio. This is the one
    // rewriting operation that cannot fail on the contents of a file.
    const Operation retime = [input,
                              output](core::Session& session) -> std::expected<void, std::string> {
        session.apply(std::make_unique<core::ConvertFrameRateCommand>(
            session.project(), core::Selection::all(session.project()), input, output));
        return {};
    };

    return rewriteAll(files,
                      paths,
                      destination,
                      reporter,
                      "retimed",
                      "from " + nameOf(input) + " to " + nameOf(output) + " fps",
                      retime);
}

} // namespace subedit::cli
