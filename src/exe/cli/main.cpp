// The entry point of subedit-cli.
//
// The call and the last-resort catch, and nothing else. The command line is
// read in application.cpp; what the tool decides lives in subedit_cli.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/platform/locations.hpp>

#include <exception>
#include <iostream>

#include "application.hpp"

namespace {

/// The reporter of a failure nobody foresaw: at level 0, since an error is said whatever the
/// level asked, and under no subcommand.
subedit::cli::Reporter lastResort() {
    return subedit::cli::Reporter{std::cerr, 0};
}

} // namespace

int main(int argc, char** argv) {
    using subedit::cli::ExitCode;
    using subedit::cli::toInt;

    // Nothing escapes. An exception reaching this far is a defect of ours and
    // not an outcome the manual describes, but a caller still deserves a line
    // saying so rather than an unwind. Code 2 is the truthful one of the four:
    // nothing was processed.
    try {
        // **The language, first and once, before anything speaks** (spec 15, D4): read
        // from `LANGUAGE`, `LC_ALL`, `LC_MESSAGES` and `LANG` by our own code, and English
        // — no catalogue — when none of them names an installed one. Never Qt: this
        // binary has none.
        if (auto found = subedit::core::installedLanguage(
                subedit::core::languageCandidates(subedit::core::processEnvironment()),
                subedit::platform::installedLocalePath()))
            subedit::core::installCatalogue(std::move(found->catalogue), found->name);
        return toInt(subedit::cli::run(argc, argv));
    } catch (const std::exception& error) {
        // A line of the standard error like any other, about no subcommand and no element: which
        // one was running is not known here.
        lastResort().error(error.what());
        return toInt(ExitCode::AllFailed);
    } catch (...) {
        lastResort().error(subedit::core::translate("unknown failure"));
        return toInt(ExitCode::AllFailed);
    }
}
