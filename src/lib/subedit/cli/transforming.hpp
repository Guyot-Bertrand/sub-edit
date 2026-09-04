#pragma once

// Correcting the positions of a file from two points known to be right.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/core/model/encoding.hpp>

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Two reference points, once they are known to name two subtitles.
///
/// A type rather than two loose parameters, so that the refusal happens where
/// it can: naming one subtitle twice is a mistake in the command line, not in
/// the file, and the spec has usage errors detected **before any processing** —
/// on a batch of ten files, the refusal must be one message and one exit code,
/// not ten.
///
/// What it cannot see, and leaves to [`transformAll`], is whether those two
/// numbers name anything in a given file, and whether the two subtitles they
/// name start at the same moment. Both depend on the file.
class Transform {

public:
    /// Reads the two references, or says why they define no transform.
    [[nodiscard]] static std::expected<Transform, std::string> between(Reference first,
                                                                       Reference last);

    [[nodiscard]] const Reference& first() const { return m_first; }

    [[nodiscard]] const Reference& last() const { return m_last; }

private:
    Transform(Reference first, Reference last) : m_first(first), m_last(last) {}

    Reference m_first;
    Reference m_last;
};

/// Transforms every path from the two references, and says how it went.
///
/// The two named subtitles land **exactly** on their targets and everything
/// else follows affinely, by the single-rounding scaling of ADR 0013.
///
/// Two refusals need the file open, and so happen here: a number past the last
/// subtitle, named with the bound so the caller knows what to write instead,
/// and two subtitles that start at the same moment, which leaves the scaling
/// with a null denominator.
///
/// **A position that would land before the origin is refused**, as it is for a
/// shift and for the same reason: a file holding `-00:00:01,000` is one no
/// player reads, so writing it would be worse than refusing.
[[nodiscard]] ExitCode transformAll(subedit::core::FileSystem& files,
                                    const std::vector<std::string>& paths,
                                    const std::optional<subedit::core::Encoding>& reading,
                                    const Transform& transform,
                                    const Destination& destination,
                                    const Reporter& reporter);

} // namespace subedit::cli
