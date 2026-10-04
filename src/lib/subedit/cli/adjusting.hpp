#pragma once

// Bringing the durations of a file within limits.

#include <subedit/cli/exit_code.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/json.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
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

/// What `adjust` was asked, as written: nothing parsed yet.
///
/// An empty text is "not given", and takes the default of `DurationConstraints`
/// — which is Gaupol's, and the window's on its first opening.
struct AdjustOptions {
    std::string speed{};
    std::string minimum{};
    std::string maximum{};
    std::string gap{};
    bool shorten = false;
    bool noLengthen = false;
};

/// The constraints those options describe, or why they describe none.
///
/// **`off` switches a constraint off, which is not setting it to zero**: an
/// absent constraint is not tested, and a minimum of zero is a minimum of zero.
/// The maximum is off by default and has no `off`: not giving it is how it is
/// switched off.
///
/// Refused here, before any file is read:
/// - a number that is not a positive reading speed, a duration that is not a
///   time or is negative, a maximum that is not above zero;
/// - `--shorten` or `--no-lengthen` with `--speed off`: they say which way the
///   speed may move an end, and there is no speed to move it;
/// - **no constraint left active** — nothing to adjust *to*, and an adjustment
///   that does nothing and says it succeeded is the worse answer.
[[nodiscard]] std::expected<subedit::core::DurationConstraints, std::string>
constraintsOf(const AdjustOptions& options);

/// The constraints in the form the record carries them: the reading speed as a
/// string of characters per second and the two ways it moves an end, or `null`;
/// each duration in whole milliseconds, or `null` when off.
///
/// **So that a reader can check two computations used the same settings**
/// without reading the command line (ADR 0008, the cross-check of #407).
[[nodiscard]] Json constraintsRecord(const subedit::core::DurationConstraints& constraints);

/// Adjusts the durations of every path to `constraints`, and says how it went.
///
/// Only the ends move. The report says what was adjusted and — the one thing
/// Gaupol does not say — which constraints some subtitle could not have, once the
/// others had had their say. `range` limits the subtitles adjusted, in every file.
[[nodiscard]] ExitCode adjustAll(subedit::core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<subedit::core::Encoding>& reading,
                                 const subedit::core::DurationConstraints& constraints,
                                 const std::optional<Range>& range,
                                 const Destination& destination,
                                 const Reporter& reporter);

} // namespace subedit::cli
