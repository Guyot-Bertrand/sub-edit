// The entry point of subedit-cli.
//
// The call and the last-resort catch, and nothing else. The command line is
// read in application.cpp; what the tool decides lives in subedit_cli.

#include <subedit/cli/exit_code.hpp>

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
        return toInt(subedit::cli::run(argc, argv));
    } catch (const std::exception& error) {
        std::cerr << "subedit-cli: " << error.what() << '\n';
        return toInt(ExitCode::AllFailed);
    } catch (...) {
        std::cerr << "subedit-cli: unknown failure\n";
        return toInt(ExitCode::AllFailed);
    }
}
