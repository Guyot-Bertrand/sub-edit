#include <subedit/cli/changes.hpp>
#include <subedit/cli/replacing.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/editing.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace subedit::cli {

std::expected<core::SearchPattern, std::string> compilePattern(std::string_view pattern,
                                                               core::SearchOptions options) {
    std::expected<core::SearchPattern, core::PatternError> compiled =
        core::SearchPattern::compile(pattern, options);
    if (!compiled) {
        return std::unexpected{"pattern: " + core::reasonOf(compiled.error())};
    }
    return std::move(*compiled);
}

ExitCode replaceIn(core::FileSystem& files,
                   const std::vector<std::string>& paths,
                   const std::optional<core::Encoding>& reading,
                   const core::SearchPattern& pattern,
                   std::string_view patternText,
                   std::string_view replacement,
                   const std::optional<Range>& range,
                   const Destination& destination,
                   const Reporter& reporter) {
    // The compiled pattern is read, never changed, and outlives the batch; the
    // operation is copied into a `std::function` and borrows it.
    const ChangingOperation replace =
        [&pattern, text = std::string{patternText}, with = std::string{replacement}](
            core::Session& session, const Request& request) -> OperationOutcome {
        std::vector<std::string> before;
        if (request.changes) {
            before = mainTextsOf(session.project());
        }

        core::ReplacedAll replaced = core::replaceAll(
            session.project(), request.selection, core::Document::Main, pattern, with);

        const auto count = [](std::size_t number) { return static_cast<std::int64_t>(number); };
        std::vector<Count> counts{{"replaced", count(replaced.count)},
                                  {"matched", count(replaced.matched)}};

        // **Not found is not nothing to change**: a pattern that is there and is
        // replaced by itself finds matches and writes nothing, and the two are told
        // apart by `matched`, as the window tells them apart.
        if (replaced.count == 0 || replaced.command == nullptr) {
            return OperationResult{
                .sentence = replaced.matched == 0 ? core::notFound(text) : core::nothingToChange(),
                .counts = std::move(counts),
                .changes =
                    request.changes ? std::optional{std::vector<TextChange>{}} : std::nullopt};
        }

        const std::vector<core::Change> described = replaced.command->describe();
        session.apply(std::move(replaced.command));
        return OperationResult{
            .sentence = core::noticeOfReplaceAll(replaced.count),
            .counts = std::move(counts),
            .changes = request.changes
                           ? std::optional{changesOfCommand(session.project(), before, described)}
                           : std::nullopt};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "replaced", replace, range);
}

} // namespace subedit::cli
