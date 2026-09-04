#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/wording.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace subedit::cli {

std::expected<core::Encoding, std::string> encodingNamed(std::string_view text) {
    // Absent rather than present: the mark of a file is what the file says, and
    // an option names an encoding, not a mark. `--bom` and `--no-bom` are how a
    // mark is asked for.
    const std::expected<core::Encoding, core::EncodingRefusal> encoding =
        core::Encoding::create(text, core::ByteOrderMark::Absent);
    if (!encoding.has_value())
        return std::unexpected(core::refusalOf(encoding.error(), text));

    return *encoding;
}

} // namespace subedit::cli
