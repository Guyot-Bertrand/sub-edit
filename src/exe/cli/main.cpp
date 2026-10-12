// The entry point of subedit-cli.
//
// The call and the last-resort catch, and nothing else. The command line is
// read in application.cpp; what the tool decides lives in subedit_cli.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/i18n/language.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/platform/locations.hpp>

#include <exception>
#include <iostream>

#include "application.hpp"

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
        subedit::core::installCatalogue(subedit::core::installedCatalogue(
            subedit::core::languageCandidates(subedit::core::processEnvironment()),
            subedit::platform::installedLocalePath()));
        return toInt(subedit::cli::run(argc, argv));
    } catch (const std::exception& error) {
        // TRANSLATORS: %1 is the reason of a failure the program did not foresee.
        std::cerr << subedit::core::substitute(subedit::core::translate("subedit-cli: %1"),
                                               {error.what()})
                  << '\n';
        return toInt(ExitCode::AllFailed);
    } catch (...) {
        std::cerr << subedit::core::translate("subedit-cli: unknown failure") << '\n';
        return toInt(ExitCode::AllFailed);
    }
}
