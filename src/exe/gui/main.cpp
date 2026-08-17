// The entry point of subedit-gui.
//
// **A seed, and deliberately almost empty.** The window of phase 5 is not
// written yet; what this proves is the chain underneath it — that Qt is found,
// that a `QApplication` constructs, and that it does so **without a screen**.
// That last point is the one worth a binary: a platform plugin that fails to
// load fails at construction, not at compilation, so no build can tell us.
//
// `--version` and nothing else, exactly as subedit-cli started in phase 1: an
// invocation that constructs the application, writes one line, and leaves.
// It is what lets a test run the whole chain in a place that has no display.

#include <subedit/core/version.hpp>

#include <QApplication>

#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        const QApplication application{argc, argv};

        std::cout << "subedit " << subedit::core::versionString() << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "subedit-gui: " << error.what() << "\n";
        return 2;
    }
}
