// What format the project says a file is, on the command line, for the sake of
// scoring it.
//
// **Two candidates made the question invisible; nine make it a classification.**
// `.sub` names both MicroDVD and SubViewer 2, `.txt` both TMPlayer and MPL2,
// and `{0}{25}` differs from `[0][25]` by the shape of a bracket. What was a
// signature to look for becomes a choice to get right, and a choice that can be
// got wrong has a rate — the reasoning `score-encoding-detection.py` was built
// on, one detection over.
//
// **It opens the file rather than calling `detectFormat` on the bytes**, and
// that is not a shortcut: the detection runs on *decoded* text, after the
// encoding has been settled and the mark taken off. Handing it raw bytes would
// refuse every UTF-16 file in the world and call it a detection failure — which
// is exactly what the first version of this program did, and what the private
// corpus caught. Reading adds nothing that can fail on its own: parsing is
// tolerant since ADR 0008, so the only two ways out are a format nobody
// recognised and bytes that do not decode.
//
// It lives here rather than in `src/exe/` for the reason the other tools do:
// nothing that is delivered contains it, and no install rule names it.
//
// One path in, one format name out, nothing else. **Refusing is not failing**:
// the rule of the project is nothing rather than a guess, so an unrecognised
// file exits with 1 and writes no name — which the script counts apart from a
// wrong one. Bytes that do not decode exit with 3, a third answer that is
// neither a name nor a refusal to name one.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/wording.hpp>

#include <expected>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: subedit_detect_format <file>\n";
        return 2;
    }

    const subedit::core::RealFileSystem files;
    const std::expected<std::string, subedit::core::FileError> content = files.readFile(argv[1]);
    if (!content.has_value()) {
        std::cerr << "the file could not be read\n";
        return 2;
    }

    const std::expected<subedit::core::ReadResult, subedit::core::ReadError> result =
        subedit::core::readSubtitles(*content);
    if (!result.has_value())
        return result.error().kind == subedit::core::ReadErrorKind::Undecodable ? 3 : 1;

    std::cout << subedit::core::nameOf(result->format) << '\n';
    return 0;
}
