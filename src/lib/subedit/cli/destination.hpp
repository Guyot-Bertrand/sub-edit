#pragma once

// Where a subcommand writes what it produces.

#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace subedit::cli {

/// The destination three mutually exclusive options describe.
///
/// **Nothing is ever written without one of them.** A harness that overwrites
/// its own input because none was given is a harness one stops using after the
/// second time; the refusal costs a line and saves the file.
///
/// Shared by every subcommand that writes, so that `--output` means the same
/// thing everywhere. `inspect` writes no file and takes none of them.
class Destination {

public:
    /// Reads the three options, or says why they cannot be honoured.
    ///
    /// `output` and `outputDir` are empty when not given. `inputCount` is what
    /// makes `--output` a mistake on a batch: writing the last input over the
    /// previous ones is the outcome this refuses.
    [[nodiscard]] static std::expected<Destination, std::string>
    from(std::string_view output, std::string_view outputDir, bool inPlace, std::size_t inputCount);

    /// The path `input` is written to.
    ///
    /// `extension` — with its dot, empty to keep the one the input has — is how
    /// a format change reaches the name. Writing WebVTT into a file named
    /// `.srt` would produce a file that lies about itself, and every other tool
    /// would trip on it.
    ///
    /// Ignored when the caller named the output file: they named it.
    [[nodiscard]] std::filesystem::path pathFor(const std::filesystem::path& input,
                                                std::string_view extension) const;

    /// Whether the inputs are written back over themselves.
    [[nodiscard]] bool isInPlace() const { return m_inPlace; }

private:
    Destination() = default;

    std::filesystem::path m_output{};
    std::filesystem::path m_outputDir{};
    bool m_inPlace = false;
};

} // namespace subedit::cli
