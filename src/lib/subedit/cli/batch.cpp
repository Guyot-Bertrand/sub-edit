#include <subedit/cli/batch.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/wording/formats.hpp>

#include <filesystem>
#include <set>
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
            reporter.failed(directory.string() + ": " +
                            std::string{core::reasonOfCreating(created.error().kind)});
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

std::string summaryOf(std::string_view verb, std::size_t done, std::size_t total) {
    if (total < 2) {
        return {};
    }

    std::string line =
        std::to_string(done) + " of " + std::to_string(total) + " files " + std::string{verb};
    if (done < total) {
        line += ", " + std::to_string(total - done) + " failed";
    }
    return line;
}

ExitCode
tally(const Reporter& reporter, std::string_view verb, std::size_t done, std::size_t total) {
    const std::string summary = summaryOf(verb, done, total);
    if (!summary.empty()) {
        reporter.say(1, summary);
    }
    return outcomeOf(done, total);
}

} // namespace subedit::cli
