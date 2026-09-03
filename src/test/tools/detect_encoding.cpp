// The detection of the core, on the command line, for the sake of scoring it.
//
// **A detection is a classifier, not a function** — it has a rate, and
// `score-encoding-detection.py` is what measures it. That script takes the
// detector as an argument, so that every candidate is passed the same corpus in
// the same terms; this program is what makes ours one of those candidates.
//
// It lives here rather than in `src/exe/` for the reason the other two tools
// do: nothing that is delivered contains it, and no install rule names it.
//
// One path in, one encoding name out, nothing else — the script reads the first
// line of the standard output and refuses anything that is not shaped like the
// name of an encoding. Its silence about paths is deliberate, and this program
// keeps it: it never writes the name of the file it was given.

#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/text/encoding.hpp>

#include <expected>
#include <iostream>
#include <optional>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: subedit_detect_encoding <file>\n";
        return 2;
    }

    const subedit::core::RealFileSystem files;
    const std::expected<std::string, subedit::core::FileError> content = files.readFile(argv[1]);
    if (!content.has_value()) {
        std::cerr << "the file could not be read\n";
        return 2;
    }

    const std::optional<subedit::core::DetectedEncoding> detected =
        subedit::core::detectEncoding(*content);
    if (!detected.has_value())
        return 1;

    std::cout << detected->encoding.charset() << '\n';
    return 0;
}
