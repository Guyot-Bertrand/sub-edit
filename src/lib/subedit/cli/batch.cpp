#include <subedit/cli/batch.hpp>
#include <subedit/cli/reporter.hpp>

namespace subedit::cli {

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
