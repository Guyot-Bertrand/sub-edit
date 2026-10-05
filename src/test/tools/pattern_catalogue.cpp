// The pattern catalogue a command line reads, on the command line, for the sake
// of proving where it finds its files.
//
// **Nothing delivered reads the catalogue yet** — `correct` is a later issue —
// and the resolution cannot wait for it: it touches the packaging and the
// harness, and what touches those is tried early. This program is the smallest
// thing that resolves the two directories **as the command line will** — from
// its own executable and the real environment, through `subedit::platform`,
// without Qt — and reads them with the core's reader. It is the debug option the
// issue allows, kept out of the shipped binary.
//
// It prints the two directories, the number of patterns read, one line per
// pattern (`code: name`), and the diagnostics with their file and line — nothing
// it could not print from the catalogue alone.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/platform/locations.hpp>

#include <filesystem>
#include <iostream>

int main() {
    const std::filesystem::path shipped = subedit::platform::installedPatternsPath();
    const std::filesystem::path user = subedit::platform::resolvedUserPatternsPath();
    const subedit::core::RealFileSystem files;
    const subedit::core::PatternCatalogue catalogue =
        subedit::core::readPatternCatalogue(files, shipped, user);

    std::cout << "shipped: " << shipped.string() << '\n';
    std::cout << "user: " << user.string() << '\n';
    std::cout << "patterns: " << catalogue.patterns().size() << '\n';
    for (const subedit::core::CorrectionPattern& pattern : catalogue.patterns()) {
        std::cout << pattern.code << ": " << pattern.name << '\n';
    }
    std::cout << "diagnostics: " << catalogue.diagnostics().size() << '\n';
    for (const subedit::core::PatternDiagnostic& diagnostic : catalogue.diagnostics()) {
        std::cout << "  " << diagnostic.file.string() << ':' << diagnostic.line << " "
                  << diagnostic.detail << '\n';
    }
    return 0;
}
