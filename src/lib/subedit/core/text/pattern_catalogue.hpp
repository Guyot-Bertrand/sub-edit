#pragma once

// Every pattern the program knows, read from Gaupol's files.
//
// The reading follows `aeidon/patternman.py` step by step — ADR 0037 — and, like
// the opening of a subtitle file, it reads as much as it can and says what it
// could not: a record that does not read is a diagnostic naming its file, its
// line and its reason, and the others are read all the same (ADR 0008).

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// What a reading ran into.
enum class PatternProblem {
    DirectoryUnreadable, ///< a directory that could not be listed
    FileUnreadable,      ///< a file that could not be read
    MalformedLine,       ///< a line that is neither a header, a comment nor `Key=Value`
    FieldOutsideRecord,  ///< a `Key=Value` before the first record header
    UnknownField,        ///< a key this kind of pattern has no use for — the record stays
    MissingField,        ///< a key this kind of pattern cannot do without — the record is refused
    InvalidValue, ///< a value that is not one of those the key accepts — the record is refused
    MalformedActivation, ///< a `<pattern>` element of a `.conf` that names nothing
};

/// One thing a reading ran into, and where.
struct PatternDiagnostic {
    PatternProblem problem;
    std::filesystem::path file;

    /// Counted from one. Zero when the diagnostic is about the whole file or
    /// directory.
    int line = 0;

    /// The key, the value or the reason the file system gave. May be empty.
    std::string detail;

    friend bool operator==(const PatternDiagnostic&, const PatternDiagnostic&) = default;
};

/// The patterns read from a shipped directory and a user's, in reading order.
class PatternCatalogue {

public:
    /// Every record, the shipped files' first and then the user's — the order
    /// Gaupol reads them in, which is the order a code's records apply in.
    [[nodiscard]] const std::vector<CorrectionPattern>& patterns() const { return m_patterns; }

    [[nodiscard]] const std::vector<PatternDiagnostic>& diagnostics() const {
        return m_diagnostics;
    }

    /// The patterns of `kind` that apply to `code`, in the order they apply in:
    /// `Zyyy`, the script, the language, the country, each followed by its own.
    ///
    /// `SkipIn` leaves a record out, a name found again is filed after the one
    /// it repeats — or takes its place, under `Policy=Replace`. **Disabled
    /// records are in the answer**: whether to apply one is a setting, and a
    /// page listing the patterns has to show them.
    [[nodiscard]] std::vector<const CorrectionPattern*> cascade(PatternKind kind,
                                                                std::string_view code) const;

private:
    friend PatternCatalogue readPatternCatalogue(const FileSystem& files,
                                                 const std::filesystem::path& shipped,
                                                 const std::filesystem::path& user);

    std::vector<CorrectionPattern> m_patterns;
    std::vector<PatternDiagnostic> m_diagnostics;
};

/// Reads the patterns of `shipped`, then those of `user`, and the activation the
/// shipped `.conf` files give.
///
/// **A directory is received, never resolved here** — ADR 0037: no test reaches
/// the user's real directory by accident, because none is given it. An empty
/// `user` names no directory.
///
/// A shipped directory that is not there is a diagnostic, since it turns every
/// correction off without a word; a user directory that is not there is the
/// ordinary case, and says nothing.
///
/// **Only the shipped `.conf` files are read.** The activation a user chose is
/// a setting of the program (ADR 0022), applied on top of this one.
[[nodiscard]] PatternCatalogue readPatternCatalogue(const FileSystem& files,
                                                    const std::filesystem::path& shipped,
                                                    const std::filesystem::path& user);

} // namespace subedit::core
