#include <subedit/cli/encoding_grammar.hpp>
#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::cli {

std::expected<core::Encoding, std::string> encodingNamed(std::string_view text) {
    // Absent rather than present: the mark of a file is what the file says, and
    // an option names an encoding, not a mark. `--bom` and `--no-bom` are how a
    // mark is asked for.
    const std::optional<core::Encoding> encoding =
        core::Encoding::create(text, core::ByteOrderMark::Absent);
    if (!encoding.has_value())
        return std::unexpected("no encoding is named \"" + std::string{text} + "\"");

    return *encoding;
}

} // namespace subedit::cli
