// The bench binary's own `main`, replacing `Catch2::Catch2WithMain` — issue
// #503. `QFontMetricsF` needs a `QApplication` to reach a font database, and
// `EmsLineMeasure` is the first bench to ask for one.
//
// **The reasoning is `window_harness_test.cpp`'s**, for `subedit_gui_test: a
// `QApplication` is built once, on the stack of `main`, offscreen by default
// and only by default — CTest does not run this binary, but a human measuring
// by hand might set `QT_QPA_PLATFORM` before doing so.
//
// **Nothing here shows a widget or writes a setting**, unlike that binary: no
// message handler to silence a widget's offscreen noise, no `XDG_CONFIG_HOME`
// of its own to protect. Should either become true of a future bench, borrow
// both from `window_harness_test.cpp` — the reasons are documented there.

#include <QApplication>
#include <catch2/catch_session.hpp>

int main(int argc, char** argv) {
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    const QApplication application{argc, argv};
    return Catch::Session().run(argc, argv);
}
