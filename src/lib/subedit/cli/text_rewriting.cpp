#include <subedit/cli/changes.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/text_rewriting.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/wording/counts.hpp>
#include <subedit/core/wording/formats.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace subedit::cli {

namespace {

/// What differs between the three: the command they build, the sentence for
/// what it did, and what, if anything, makes a file unfit for it.
struct TextRewrite {
    std::function<std::unique_ptr<core::Command>(
        const core::Project&, const core::Selection&, core::Document)>
        build;
    std::function<std::string(std::size_t)> notice;

    /// Why this file cannot be asked, or nothing.
    std::function<std::optional<Failure>(const core::Project&, core::Document)> refusal{};
};

/// The loop they share: set the texts aside when someone reads the list, build
/// the command over the selection, apply it, and say how many texts it rewrote.
///
/// **The count is read from the command**, before it goes, and never counted
/// again after (decision D8 of the window): it is the figure the history would
/// show if the command were undone.
[[nodiscard]] ExitCode rewriteTexts(core::FileSystem& files,
                                    const std::vector<std::string>& paths,
                                    const std::optional<core::Encoding>& reading,
                                    std::string_view verb,
                                    const TextRewrite& rewrite,
                                    const std::optional<Range>& range,
                                    const Destination& destination,
                                    const Reporter& reporter,
                                    const std::optional<Pairing>& pairing) {
    const ChangingOperation operation = [&rewrite](core::Session& session,
                                                   const Request& request) -> OperationOutcome {
        if (rewrite.refusal) {
            if (std::optional<Failure> refused =
                    rewrite.refusal(session.project(), request.document)) {
                return std::unexpected{*std::move(refused)};
            }
        }

        std::vector<std::string> before;
        if (request.changes) {
            before = textsOf(session.project(), request.document);
        }

        std::unique_ptr<core::Command> command =
            rewrite.build(session.project(), request.selection, request.document);
        if (!command) {
            // Every text was already the way it was asked for: nothing is applied,
            // and nothing would be in the history.
            return OperationResult{.sentence = core::nothingToChange(),
                                   .counts = {{"changed", 0}},
                                   .changes = request.changes
                                                  ? std::optional{std::vector<TextChange>{}}
                                                  : std::nullopt};
        }

        const std::size_t rewritten = core::rewrittenCount(*command);
        const std::vector<core::Change> described = command->describe();
        session.apply(std::move(command));

        return OperationResult{
            .sentence = rewrite.notice(rewritten),
            .counts = {{"changed", static_cast<std::int64_t>(rewritten)}},
            .changes = request.changes
                           ? std::optional{changesOfCommand(
                                 session.project(), before, described, request.document)}
                           : std::nullopt};
    };

    return rewriteAll(
        files, paths, reading, destination, reporter, verb, operation, range, pairing);
}

} // namespace

core::LetterCase letterCaseNamed(const std::string& name) {
    if (name == "title") {
        return core::LetterCase::Title;
    }
    if (name == "sentence") {
        return core::LetterCase::Sentence;
    }
    if (name == "upper") {
        return core::LetterCase::Upper;
    }
    return core::LetterCase::Lower;
}

std::expected<bool, std::string> italicsDirectionOf(bool on, bool off) {
    if (on == off) {
        return std::unexpected{
            on ? std::string{"--on and --off say opposite things; give one of them"}
               : std::string{"italics needs --on to put the texts in italics, or "
                             "--off"}};
    }
    return on;
}

std::expected<bool, std::string> dashesDirectionOf(bool add, bool remove) {
    if (add == remove) {
        return std::unexpected{
            add ? std::string{"--add and --remove say opposite things; give one of them"}
                : std::string{"dialogue-dashes needs --add to put dashes on, or --remove"}};
    }
    return add;
}

ExitCode recaseIn(core::FileSystem& files,
                  const std::vector<std::string>& paths,
                  const std::optional<core::Encoding>& reading,
                  core::LetterCase wanted,
                  const std::optional<Range>& range,
                  const Destination& destination,
                  const Reporter& reporter,
                  const std::optional<Pairing>& pairing) {
    const TextRewrite rewrite{
        .build =
            [wanted](const core::Project& project,
                     const core::Selection& selection,
                     core::Document document) {
                return core::setLetterCase(project, selection, document, wanted);
            },
        .notice = [](std::size_t count) { return core::noticeOfRecase(count); }};
    return rewriteTexts(
        files, paths, reading, "recased", rewrite, range, destination, reporter, pairing);
}

ExitCode italicsIn(core::FileSystem& files,
                   const std::vector<std::string>& paths,
                   const std::optional<core::Encoding>& reading,
                   bool italic,
                   const std::optional<Range>& range,
                   const Destination& destination,
                   const Reporter& reporter,
                   const std::optional<Pairing>& pairing) {
    const TextRewrite rewrite{
        .build =
            [italic](const core::Project& project,
                     const core::Selection& selection,
                     core::Document document) {
                return core::setItalics(project, selection, document, italic);
            },
        .notice = [italic](std::size_t count) { return core::noticeOfItalics(count, italic); },
        .refusal = [italic](const core::Project& project,
                            core::Document document) -> std::optional<Failure> {
            // What the format can carry is the question, and not how it spells it:
            // TMPlayer and LRC write no style at all.
            const core::SubtitleFormat format = project.sourceFile(document).format;
            if (core::abilitiesOf(format).italic) {
                return std::nullopt;
            }
            return Failure{"no-style",
                           std::string{core::nameOf(format)} + " writes no style: there are no " +
                               "italics to " + (italic ? "put on" : "take out")};
        }};
    return rewriteTexts(
        files, paths, reading, "changed", rewrite, range, destination, reporter, pairing);
}

ExitCode dialogueDashesIn(core::FileSystem& files,
                          const std::vector<std::string>& paths,
                          const std::optional<core::Encoding>& reading,
                          bool dashed,
                          const std::optional<Range>& range,
                          const Destination& destination,
                          const Reporter& reporter,
                          const std::optional<Pairing>& pairing) {
    const TextRewrite rewrite{
        .build =
            [dashed](const core::Project& project,
                     const core::Selection& selection,
                     core::Document document) {
                return core::setDialogueDashes(project, selection, document, dashed);
            },
        .notice =
            [dashed](std::size_t count) { return core::noticeOfDialogueDashes(count, dashed); }};
    return rewriteTexts(
        files, paths, reading, "changed", rewrite, range, destination, reporter, pairing);
}

} // namespace subedit::cli
