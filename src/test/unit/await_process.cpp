#include "await_process.hpp"

#include <subedit/core/process/start_process.hpp>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <optional>
#include <sys/wait.h>
#include <thread>

namespace subedit::test {

namespace {

/// How long the loop sleeps between two questions.
///
/// Short enough that a program which ends at once is not waited on for
/// anything measurable, long enough not to spin a core while it does.
constexpr std::chrono::milliseconds kPollInterval{1};

} // namespace

std::optional<core::ProcessOutcome> RunningProcess::await(std::chrono::milliseconds bound) {
    const std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + bound;

    while (true) {
        if (const std::optional<core::ProcessOutcome> outcome = core::outcomeOf(m_handle)) {
            m_collected = true;
            return outcome;
        }
        if (std::chrono::steady_clock::now() >= deadline)
            return std::nullopt;
        std::this_thread::sleep_for(kPollInterval);
    }
}

RunningProcess::~RunningProcess() {
    if (m_collected || m_handle.id < 0)
        return;

    // SIGKILL and not SIGTERM: this runs when a test is already going wrong,
    // and a program that chose to ignore the polite signal would hang the
    // blocking wait below — trading one stuck test for another.
    ::kill(m_handle.id, SIGKILL);

    // Blocking, and deliberately: killing without collecting leaves a zombie,
    // which is exactly the thing this class exists to prevent. The wait cannot
    // last, the process having just been killed.
    int status = 0;
    while (::waitpid(m_handle.id, &status, 0) < 0 && errno == EINTR) {
    }
}

} // namespace subedit::test
