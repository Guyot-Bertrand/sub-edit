#pragma once

// The grid fixtures of `src/test/data/grilles/`, opened for a test.
//
// Three test files needed them before this header existed, and each had written
// the same twenty lines: build the path, read the bytes, parse, fail loudly if
// any step did not work. Reading a fixture is not what any of those files is
// about, so it is here rather than three times over.
//
// `src/test/data/grilles/LISEZMOI.md` says what each fixture is and what each
// one gives on the eight candidates. `subtitle-fixtures.py --check` holds them
// to it.

#include <subedit/core/model/project.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <string_view>
#include <vector>

namespace subedit::test {

/// Opens a grid fixture as a project, timed at `rate`.
///
/// Fails the running test case rather than returning something empty: a fixture
/// that will not open is a broken checkout, not a case to handle.
[[nodiscard]] core::Project gridProject(std::string_view name, core::FrameRate rate);

/// The starts of a grid fixture, which are the whole signal of a deduction.
///
/// Wherever a grid exists the starts sit on it without exception, while the
/// ends run from 55 to 100 — a *cue-out* is often computed by a reading-speed
/// rule rather than placed on a frame.
[[nodiscard]] std::vector<core::Timestamp> gridStarts(std::string_view name);

} // namespace subedit::test
