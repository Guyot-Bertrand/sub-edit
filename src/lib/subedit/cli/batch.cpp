#include <subedit/cli/batch.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording/formats.hpp>

#include <filesystem>
#include <set>
#include <string_view>
#include <utility>

namespace subedit::cli {

std::expected<std::vector<Job>, ExitCode> arrange(core::FileSystem& files,
                                                  const Destination& destination,
                                                  const std::vector<std::string>& inputs,
                                                  std::string_view extension,
                                                  const Reporter& reporter) {
    std::expected<std::vector<Job>, std::string> planned =
        destination.plan(files, inputs, extension);
    if (!planned) {
        reporter.failed(planned.error());
        return std::unexpected{ExitCode::Usage};
    }

    // A dry run creates nothing, and an in-place batch has every directory.
    if (destination.isInPlace() || destination.isDryRun()) {
        return *std::move(planned);
    }

    std::vector<std::filesystem::path> outputs;
    outputs.reserve(planned->size());
    for (const Job& job : *planned) {
        outputs.push_back(job.output);
    }
    if (const std::optional<ExitCode> failed = createDirectoriesFor(files, outputs, reporter)) {
        return std::unexpected{*failed};
    }
    return *std::move(planned);
}

std::optional<ExitCode> createDirectoriesFor(core::FileSystem& files,
                                             std::span<const std::filesystem::path> outputs,
                                             const Reporter& reporter) {
    std::set<std::filesystem::path> made;
    for (const std::filesystem::path& output : outputs) {
        const std::filesystem::path directory = output.parent_path();
        // Empty is the current directory, which is there; and a directory
        // already made for an earlier output is not asked for twice.
        if (directory.empty() || !made.insert(directory).second) {
            continue;
        }
        if (const std::expected<void, core::FileError> created = files.createDirectories(directory);
            !created) {
            reporter.failed(core::substitute(
                core::translate("%1: %2"),
                {directory.string(), core::reasonOfCreating(created.error().kind)}));
            return ExitCode::AllFailed;
        }
    }
    return std::nullopt;
}

ExitCode outcomeOf(std::size_t done, std::size_t total) {
    if (done == total) {
        return ExitCode::Success;
    }
    return done == 0 ? ExitCode::AllFailed : ExitCode::SomeFailed;
}

namespace {

/// The sentence of `verb`, its plural chosen by the number of files.
[[nodiscard]] std::string_view sentenceOf(Verb verb, std::size_t total) {
    switch (verb) {
    case Verb::Adjusted:
        // TRANSLATORS: %1 of %2 files were done; the form is chosen by %2.
        return core::translatePlural("%1 of %2 file adjusted", "%1 of %2 files adjusted", total);
    case Verb::Aligned:
        return core::translatePlural("%1 of %2 file aligned", "%1 of %2 files aligned", total);
    case Verb::Changed:
        return core::translatePlural("%1 of %2 file changed", "%1 of %2 files changed", total);
    case Verb::Cleaned:
        return core::translatePlural("%1 of %2 file cleaned", "%1 of %2 files cleaned", total);
    case Verb::Converted:
        return core::translatePlural("%1 of %2 file converted", "%1 of %2 files converted", total);
    case Verb::Corrected:
        return core::translatePlural("%1 of %2 file corrected", "%1 of %2 files corrected", total);
    case Verb::Inspected:
        return core::translatePlural("%1 of %2 file inspected", "%1 of %2 files inspected", total);
    case Verb::Joined:
        return core::translatePlural("%1 of %2 file joined", "%1 of %2 files joined", total);
    case Verb::MadeDisjoint:
        return core::translatePlural(
            "%1 of %2 file made disjoint", "%1 of %2 files made disjoint", total);
    case Verb::Paired:
        return core::translatePlural("%1 of %2 file paired", "%1 of %2 files paired", total);
    case Verb::Recased:
        return core::translatePlural("%1 of %2 file recased", "%1 of %2 files recased", total);
    case Verb::Repaired:
        return core::translatePlural("%1 of %2 file repaired", "%1 of %2 files repaired", total);
    case Verb::Replaced:
        return core::translatePlural("%1 of %2 file replaced", "%1 of %2 files replaced", total);
    case Verb::Retimed:
        return core::translatePlural("%1 of %2 file retimed", "%1 of %2 files retimed", total);
    case Verb::Shifted:
        return core::translatePlural("%1 of %2 file shifted", "%1 of %2 files shifted", total);
    case Verb::Sorted:
        return core::translatePlural("%1 of %2 file sorted", "%1 of %2 files sorted", total);
    case Verb::Transformed:
        return core::translatePlural(
            "%1 of %2 file transformed", "%1 of %2 files transformed", total);
    }
    std::unreachable();
}

} // namespace

std::string summaryOf(Verb verb, std::size_t done, std::size_t total) {
    if (total < 2) {
        return {};
    }

    const std::string line =
        core::substitute(sentenceOf(verb, total), {std::to_string(done), std::to_string(total)});
    if (done == total) {
        return line;
    }
    const std::size_t failed = total - done;
    // TRANSLATORS: %1 is the summary of a batch, such as « 1 of 3 files shifted »; %2
    // how many files of it failed.
    return core::substitute(core::translatePlural("%1, %2 failed", "%1, %2 failed", failed),
                            {line, std::to_string(failed)});
}

ExitCode tally(const Reporter& reporter, Verb verb, std::size_t done, std::size_t total) {
    const std::string summary = summaryOf(verb, done, total);
    if (!summary.empty()) {
        reporter.say(1, summary);
    }
    return outcomeOf(done, total);
}

} // namespace subedit::cli
