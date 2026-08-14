#pragma once

// What the process tells its caller.

namespace subedit::cli {

/// The four values subedit-cli ever returns.
///
/// `AllFailed` and `SomeFailed` are kept apart on purpose: a script must be
/// able to tell "nothing worked" from "one is missing" without reading the
/// output back.
enum class ExitCode : int {
    Success = 0,    ///< everything asked for was done
    Usage = 1,      ///< the command line itself is wrong; nothing was touched
    AllFailed = 2,  ///< no input file could be processed
    SomeFailed = 3, ///< some inputs were processed, others were not
};

[[nodiscard]] constexpr int toInt(ExitCode code) {
    return static_cast<int>(code);
}

} // namespace subedit::cli
