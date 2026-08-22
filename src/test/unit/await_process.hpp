#pragma once

#include <subedit/core/process/start_process.hpp>

#include <chrono>
#include <optional>

namespace subedit::test {

/// How long a test waits for a child before giving up on it.
///
/// Generous on purpose: what is measured here is never speed, and a bound that
/// is too tight turns a slow machine — or a build under AddressSanitizer —
/// into a test that fails once in a while for no reason anybody can reproduce.
inline constexpr std::chrono::milliseconds kProcessBound{5000};

/// A started process that cannot outlive the test that started it.
///
/// Two things a test needs and neither the launcher nor Catch2 provides.
///
/// **A bounded wait.** `outcomeOf` never blocks — that is what it is for — so
/// a test that wants an answer has to ask again until it comes.
/// A loop written once has a bound; a loop rewritten in every test has one
/// nine times out of ten, and the tenth is a test that hangs until CTest kills
/// it and says nothing about why.
///
/// **A guaranteed end.** A failing `REQUIRE` throws, and would step over any
/// cleanup written after it. The destructor is the only place that runs
/// whatever happens, so it is where the killing goes: under
/// AddressSanitizer, a child still alive at exit is noise on top of the real
/// failure.
class RunningProcess {

public:
    explicit RunningProcess(core::ProcessHandle handle) : m_handle(handle) {}

    RunningProcess(const RunningProcess&) = delete;
    RunningProcess(RunningProcess&&) = delete;
    RunningProcess& operator=(const RunningProcess&) = delete;
    RunningProcess& operator=(RunningProcess&&) = delete;

    ~RunningProcess();

    /// Waits for the process to end, at most `bound`.
    ///
    /// Nothing means the bound was reached and the process is still running —
    /// which a caller states as a failure, `REQUIRE` in hand. The distinction
    /// matters: a process that ended badly has an outcome, and only a process
    /// that never ended has none.
    [[nodiscard]] std::optional<core::ProcessOutcome>
    await(std::chrono::milliseconds bound = kProcessBound);

private:
    core::ProcessHandle m_handle;
    bool m_collected = false;
};

} // namespace subedit::test
