#pragma once

// Where a subcommand writes what it produces.

#include <cstddef>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

/// Whether two paths name one file: by their absolute normalised spelling, and
/// by what the system says when they exist (`FileSystem::equivalent`).
[[nodiscard]] bool sameFile(const core::FileSystem& files,
                            const std::filesystem::path& first,
                            const std::filesystem::path& second);

/// One input of a batch and the path it is written to, **computed once**.
///
/// The validation reads `output` to refuse collisions, the creation of the
/// output directory reads it, and the writing reads it again: three readers of
/// one decision, none of them recomputing it.
struct Job {
    std::string input;
    std::filesystem::path output;
};

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

    /// The same destination, knowing which inputs were found by walking the
    /// directories `roots`.
    ///
    /// **A file found under a root is written at the path it has relative to
    /// that root**, below `--output-dir`: `films/a/x.srt`, `films` being given,
    /// becomes `out/a/x.srt`. The name of the root is not part of it — the
    /// meaning of a trailing slash for `rsync`, not that of `cp -r`: the
    /// caller names the destination and adds the level they want.
    [[nodiscard]] Destination withRoots(std::vector<std::filesystem::path> roots) const;

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

    /// Every input with its destination, or why the batch must not start.
    ///
    /// **Judged whole, before the first write** — the rule of `CLI-USAGE-03`: a
    /// usage error never leaves a batch half written. Two refusals:
    ///
    /// - two inputs that end up at the same destination, **extension
    ///   included**: `convert` changes it, so `a/film.srt` and `b/film.vtt`
    ///   converted to WebVTT both become `out/film.vtt`, which comparing the
    ///   inputs' names would not see. The last would silently erase the first;
    /// - without `--in-place`, a destination that is one of the inputs: writing
    ///   over one's own input is a gesture one names, including through
    ///   `--output-dir .`.
    ///
    /// Two paths are the same file by `FileSystem::equivalent` when they exist,
    /// and by their normalised absolute spelling when they do not yet.
    [[nodiscard]] std::expected<std::vector<Job>, std::string>
    plan(const core::FileSystem& files,
         const std::vector<std::string>& inputs,
         std::string_view extension) const;

    /// Whether the inputs are written back over themselves.
    [[nodiscard]] bool isInPlace() const { return m_inPlace; }

private:
    Destination() = default;

    std::filesystem::path m_output{};
    std::filesystem::path m_outputDir{};
    std::vector<std::filesystem::path> m_roots{};
    bool m_inPlace = false;
};

} // namespace subedit::cli
