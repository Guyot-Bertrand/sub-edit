#pragma once

// What a subcommand's shared options come to, once read: the destination, the range, the
// translation, the inputs, and the choices a reading is given. No CLI11 here — these take
// the values the command line was parsed into, so that they are tried without a process.

#include <subedit/cli/destination.hpp>
#include <subedit/cli/expansion.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/pairing.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

/// Where a subcommand writes, as the three options that say it, and `--dry-run`,
/// which says it writes nowhere.
///
/// A type of its own rather than three fields repeated in four structs: they
/// always travel together, they are always declared the same way, and they are
/// always turned into a `Destination` by the same call. A fifth subcommand that
/// writes now costs none of the three.
struct DestinationOptions {
    std::string output{};
    std::string outputDir{};
    bool inPlace = false;
    bool dryRun = false;
};

/// The destination those options describe, or why they cannot be honoured.
///
/// Told about the inputs because two things depend on them: whether `--output`
/// is a mistake, and which of them came from walking a directory — those keep
/// their place in the tree under `--output-dir`.
[[nodiscard]] std::expected<Destination, std::string>
destinationOf(const DestinationOptions& options, const Inputs& inputs);

/// The range that option says, **nothing when it was not given**, or why it
/// cannot be honoured. The refusal names the option, so that it reads the same
/// from every subcommand.
[[nodiscard]] std::expected<std::optional<Range>, std::string> rangeOf(const std::string& range);

/// The translation a subcommand is given: `-t`, `--align-method`, and — for
/// the subcommands that change texts — `--document`.
struct TranslationOptions {
    std::string file{};
    std::string document{};
    std::string alignMethod{};
};

/// The pairing these options ask for, **nothing when the main document is the
/// target**, or why they cannot be honoured. Read before any file is.
///
/// `withDocument` says whether `--document` was declared. When it was,
/// `--document translation` demands `-t` and `-t` demands `--document
/// translation`: a file read that no gesture uses is an omission, not a
/// preference. Either way, `-t` names one file and so goes with **one input**
/// that was given by name: a batch, or a directory walked, is refused as
/// `--output` is.
[[nodiscard]] std::expected<std::optional<Pairing>, std::string>
pairingOf(const TranslationOptions& options, bool withDocument, const Inputs& inputs);

/// What a subcommand's options come to once they have been read, **in the one
/// order every subcommand reads them**.
struct Prepared {
    Inputs inputs;

    /// Nothing when the subcommand has no `--range`, or none was given.
    std::optional<Range> range;

    /// Nothing when no translation was given (`-t`), or the subcommand takes none.
    std::optional<Pairing> pairing;
};

/// The same, for a subcommand that writes: with the destination it settled.
struct PreparedWriting {
    Inputs inputs;
    std::optional<Range> range;
    std::optional<Pairing> pairing;
    Destination destination;
};

/// Which of the shared options a subcommand has — the ones `prepare` reads.
///
/// Pointers, so that a subcommand without `--range` or without `-t` says so by
/// leaving them null, and the options stay where CLI11 filled them.
struct Preparation {
    const std::vector<std::string>& files;
    bool recursive = false;
    const std::string* range = nullptr;
    const TranslationOptions* translation = nullptr;

    /// Whether `--document` was declared next to `-t`: `pairingOf`'s second argument.
    bool withDocument = true;
};

/// Reads the range, expands the inputs and reads the translation — **in that order,
/// and the first refusal wins**. For the subcommand that writes no file (`inspect`).
///
/// The order was decided seventeen times, once by each subcommand, and was not the
/// same in all of them; it is decided here. What depends on the command line alone
/// (`--on` against `--off`, a pattern that does not compile) is a subcommand's own and
/// comes before: it needs no file, and `prepare` touches the file system.
[[nodiscard]] std::expected<Prepared, std::string>
prepare(const core::FileSystem& files, const Reporter& reporter, const Preparation& wanted);

/// `prepare`, then the destination `options` describe — **last**, since it is judged
/// against the inputs. A subcommand that writes has no other way to get one.
[[nodiscard]] std::expected<PreparedWriting, std::string>
prepareWriting(const core::FileSystem& files,
               const Reporter& reporter,
               const Preparation& wanted,
               const DestinationOptions& options);

/// The two things a reading can be told, once both have been checked.
///
/// **Named once because two subcommands say it**, and because the refusal has
/// to be the same sentence in both: a rate that names nothing is a usage error,
/// not a file that failed.
[[nodiscard]] std::expected<core::ReadingChoices, std::string>
readingWith(const std::optional<core::Encoding>& encoding, const std::string& frameRate);

} // namespace subedit::cli
