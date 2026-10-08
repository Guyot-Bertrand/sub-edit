#pragma once

// Waiting for what a test is about, and not for a time.
//
// Issue #646. A fixed pause is a guess about the machine: too short under a sanitizer or on a
// loaded host, where the case fails for nothing, and too long everywhere else. What a case waits
// for is something that happens — a position reached, a step made, a picture shown — so it asks
// whether it happened, and gives up at a deadline that only a broken case ever reaches.

#include <QTest>

#include <chrono>
#include <functional>

namespace subedit::test {

/// How long a case waits for something that should happen at once, before calling it broken.
inline constexpr std::chrono::seconds kDefaultDeadline{5};

/// How long the event loop runs between two looks at the predicate.
inline constexpr int kLookEveryMilliseconds = 10;

/// Runs the event loop until `done` is true, or until `deadline` has passed, and says whether it
/// is true. The deadline is a failure bound and not a delay: a case that works never waits it out.
[[nodiscard]] inline bool waitUntil(const std::function<bool()>& done,
                                    std::chrono::milliseconds deadline = kDefaultDeadline) {
    const auto end = std::chrono::steady_clock::now() + deadline;
    while (!done() && std::chrono::steady_clock::now() < end)
        QTest::qWait(kLookEveryMilliseconds);
    return done();
}

} // namespace subedit::test
