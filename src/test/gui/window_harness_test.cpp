// The chain a window test needs, proved before any window exists.
//
// It tests no feature — there is none yet. It tests that everything a phase-5
// test will rely on is in place: a `QApplication` constructs where there is no
// screen, a widget takes an event, and a signal can be watched. Each of those
// fails at run time and never at compilation, so nothing but running them can
// tell us.
//
// **Catch2, and Qt only for its driving helpers.** `QTest` comes in two parts:
// macros that build a test binary of their own — `QTEST_MAIN`, private slots,
// moc — and free functions that simulate clicks and keystrokes. Only the second
// part is used here, so the requirements registry keeps seeing the tags it
// confronts to its lines.
//
// **A window test shows its window.** The rule is written here because this is
// where somebody reading the harness will look for it, and it was paid for:
// libmpv adopts the window it is handed at the moment it loads a file, and a
// window that is not on screen is adopted and never mapped. The whole of
// phase 5 and most of phase 6 drove windows nobody had ever shown, so no test
// could see it — the video panel stayed empty until a human ran the program.
//
// A window that was never shown is not a window a user has. Everything Qt only
// does on display — `showEvent`, the real geometry, a layout that has actually
// run — is out of reach until `show()` is called, and what is out of reach of
// the tests is where the next defect of that family will sit.

#include <QApplication>
#include <QByteArray>
#include <QLineEdit>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QtGlobal>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

/// The one thing the offscreen platform says every time a window is shown.
///
/// It is true and it is harmless: nothing here has a window manager to
/// propagate a size hint to. Showing every window makes it ninety-six lines of
/// a test run, which is how a real warning stops being read.
constexpr const char* kOffscreenSizeHints = "This plugin does not support propagateSizeHints()";

/// Whether this message is that one, and nothing near it.
///
/// **Une égalité et non un préfixe** — a filter that matched loosely would
/// grow into one that hides what it was not meant to hide, and a test harness
/// that swallows warnings is worse than one that shouts. Out here rather than
/// inside the handler so that a case can hold it to that.
[[nodiscard]] bool isOffscreenNoise(const QString& text) {
    return text == QLatin1StringView{kOffscreenSizeHints};
}

/// Passes every message through but that one.
void withoutOffscreenNoise(QtMsgType type, const QMessageLogContext& context, const QString& text) {
    if (isOffscreenNoise(text))
        return;

    // `qt_message_output` is what the default handler ends in, and calling it
    // keeps a real warning printed exactly as it would have been.
    qt_message_output(type, context, text);
}

} // namespace

/// The binary owns its `QApplication`, and owns it **on the stack of `main`**.
///
/// Qt allows exactly one, so it cannot be built per case; and it must be gone
/// **before** the process runs its exit handlers. A function-local static does
/// neither — it is destroyed among those handlers, by which time Qt has torn
/// down what its destructor still reaches. That is not a theory: it segfaulted
/// under ASan, in `QInputDevice::~QInputDevice` called from
/// `__run_exit_handlers`.
///
/// Which is why this binary provides its own `main` rather than linking
/// Catch2's.
int main(int argc, char** argv) {
    // Offscreen by default, and only by default. CTest could set it, but this
    // binary is also run outside CTest — `check-requirements.sh` asks it for
    // its tags — and constructing a `QApplication` without a platform plugin
    // aborts. Whoever wants to watch the window sets the variable, and this
    // leaves it alone.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    // Installed before the `QApplication`, which warns on its own account too.
    qInstallMessageHandler(withoutOffscreenNoise);

    const QApplication application{argc, argv};
    return Catch::Session().run(argc, argv);
}

TEST_CASE("a widget takes keystrokes where there is no screen", "[gui]") {
    QLineEdit field;
    QTest::keyClicks(&field, "00:00:01,000");

    CHECK(field.text().toStdString() == "00:00:01,000");
}

TEST_CASE("a signal can be watched", "[gui]") {
    // What a table test will do on every edit: act, then assert on what the
    // widget announced rather than on what it holds.
    QLineEdit field;
    const QSignalSpy edited{&field, &QLineEdit::textEdited};

    QTest::keyClicks(&field, "abc");

    CHECK(edited.count() == 3);
}

// The harness silences one sentence of the offscreen platform, and this is what
// keeps that silence honest: a handler that hid a warning nobody asked it to
// hide would make every test run quieter and less true.
TEST_CASE("le silence du harnais ne porte que sur une phrase", "[gui]") {
    CHECK(isOffscreenNoise(QStringLiteral("This plugin does not support propagateSizeHints()")));

    CHECK_FALSE(isOffscreenNoise(QString{}));
    CHECK_FALSE(isOffscreenNoise(QStringLiteral("This plugin does not support windows")));
    // Ce qu'un préfixe aurait avalé.
    CHECK_FALSE(
        isOffscreenNoise(QStringLiteral("This plugin does not support propagateSizeHints() — et "
                                        "autre chose d'important")));
    CHECK_FALSE(isOffscreenNoise(QStringLiteral("QObject::connect: invalid nullptr parameter")));
}
