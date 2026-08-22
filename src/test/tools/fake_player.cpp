// A video player that plays nothing, and says what it was asked to play.
//
// The phase 6 preview builds a command line for mpv, MPlayer or VLC and starts
// it. What has to be proven is **that the right line is built and started** —
// not that mpv can read a Matroska file. Installing a real player would prove
// the second and cost a film, a screen, and a process that never stops on its
// own; it would also make the test pass on the machine that has the player and
// fail on every other, which is the defect #88 ruled out for the corpus and
// #117 for the screen.
//
// So: a program the project owns, that ends by itself, and whose whole
// behaviour is visible from the file its output was redirected to.
//
//   - every argument it received is written to standard output, one per line,
//     in order — `argv[0]` excluded, since the caller already knows the path
//     it asked for;
//   - `--fail-with=N` makes it complain on standard error and exit with N;
//   - `--linger=N` makes it stay alive N seconds before ending, which is what
//     a test needs to ask after a process that has not finished. Sleeping in a
//     program of ours rather than borrowing `/bin/sleep` keeps the test from
//     depending on a binary the project does not own.
//
// **The knobs travel through argv and not through the environment**, which
// matters more than it looks: the launcher passes the environment of whoever
// started the test, so an environment variable would be global state shared by
// every test in the binary. An argument is local to one launch, and the test
// that asks for it is the test that reads it.

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

constexpr std::string_view kFailFlag = "--fail-with=";
constexpr std::string_view kLingerFlag = "--linger=";

/// What a player says when it cannot play. The text matters little; that it
/// reaches the caller through standard error matters.
constexpr std::string_view kComplaint = "fake player: refusing to play";

} // namespace

int main(int argc, char** argv) {
    const std::vector<std::string> arguments(argv + (argc > 0 ? 1 : 0), argv + argc);

    for (const std::string& argument : arguments)
        std::cout << argument << '\n';
    std::cout.flush();

    for (const std::string& argument : arguments) {
        if (!argument.starts_with(kLingerFlag))
            continue;
        std::this_thread::sleep_for(
            std::chrono::seconds(std::atoi(argument.substr(kLingerFlag.size()).c_str())));
    }

    for (const std::string& argument : arguments) {
        if (!argument.starts_with(kFailFlag))
            continue;
        std::cerr << kComplaint << '\n';
        std::cerr.flush();
        return std::atoi(argument.substr(kFailFlag.size()).c_str());
    }

    return 0;
}
