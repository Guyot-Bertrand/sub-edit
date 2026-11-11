#include <subedit/cli/reporter.hpp>
#include <subedit/core/wording/counts.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using subedit::cli::Reporter;

TEST_CASE("a line is written when the level reaches its own", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 2};

    reporter.say(1, "coarse");
    reporter.say(2, "finer");

    CHECK(errors.str() == "coarse\nfiner\n");
}

TEST_CASE("a line above the level is dropped", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 1};

    reporter.say(2, "detail");
    reporter.say(3, "debug");

    CHECK(errors.str().empty());
}

TEST_CASE("silence drops every narration", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 0};

    reporter.say(1, "what happened");

    CHECK(errors.str().empty());
}

TEST_CASE("silence keeps the failures", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter{errors, 0};

    reporter.failed("file.srt: does not exist");

    // A command failing in silence leaves its exit code as the only clue, and
    // turns every incident into an investigation.
    CHECK(errors.str() == "file.srt: does not exist\n");
}

TEST_CASE("each level keeps every line of the one below", "[cli][reporter]") {
    const auto narrate = [](int level) {
        std::ostringstream errors;
        const Reporter reporter{errors, level};
        reporter.say(3, "debug");
        reporter.say(2, "detail");
        reporter.say(1, "outcome");
        return errors.str();
    };

    CHECK(narrate(1) == "outcome\n");
    CHECK(narrate(2) == "detail\noutcome\n");
    CHECK(narrate(3) == "debug\ndetail\noutcome\n");
}

TEST_CASE("the level is readable", "[cli][reporter]") {
    std::ostringstream errors;

    CHECK(Reporter{errors, 3}.level() == 3);
}

TEST_CASE("a count agrees with its noun", "[cli][wording]") {
    CHECK(subedit::core::countOf(0, subedit::core::Counted::Subtitle) == "0 subtitles");
    CHECK(subedit::core::countOf(1, subedit::core::Counted::Subtitle) == "1 subtitle");
    CHECK(subedit::core::countOf(2, subedit::core::Counted::Subtitle) == "2 subtitles");
}

TEST_CASE("a result goes to the text output, as it is", "[cli][reporter][CLI-DRYRUN-04]") {
    std::ostringstream errors;
    std::ostringstream out;
    const Reporter reporter = Reporter{errors, 1}.withTextOutput(out);

    reporter.result("a: subtitle 1\n- x\n");

    CHECK(out.str() == "a: subtitle 1\n- x\n");
    CHECK(errors.str().empty());
}

TEST_CASE("a reporter with nowhere to put a result drops it", "[cli][reporter][CLI-DRYRUN-04]") {
    std::ostringstream errors;

    Reporter{errors, 1}.result("text");

    CHECK(errors.str().empty());
}

TEST_CASE("the records are the result when they are wanted, and the text is not written",
          "[cli][reporter][CLI-DRYRUN-05]") {
    std::ostringstream errors;
    std::ostringstream out;
    const Reporter reporter = Reporter{errors, 1}.withTextOutput(out).withRecords(out);

    reporter.result("text\n");

    CHECK(out.str().empty());
}

// The grammar of a line (#826, spec `cli-narration.md`):
// `[subedit] [<command>] [<i>/<n>] <key>: <value>`.

using subedit::cli::Key;

TEST_CASE("a line about the run writes a dash where the element would be", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter = Reporter{errors, 1}.forCommand("shift");

    reporter.tell(1, Key::Summary, "2 of 3 files shifted, 1 failed");

    CHECK(errors.str() == "[subedit] [shift] [-] summary: 2 of 3 files shifted, 1 failed\n");
}

TEST_CASE("a line written before the subcommand is known writes a dash for it", "[cli][reporter]") {
    std::ostringstream errors;

    Reporter{errors, 1}.error("The following argument was not expected: --by");

    CHECK(errors.str() ==
          "[subedit] [-] [-] error: The following argument was not expected: --by\n");
}

TEST_CASE("an element begins with its source, written once", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter = Reporter{errors, 1}.forCommand("hearing-impaired");
    const Reporter item = reporter.forItem(2, 3, "b.srt");

    item.tell(1, Key::Result, "1 subtitle cleaned, 1 removed");
    item.tell(1, Key::Dest, "o/b.srt");

    CHECK(errors.str() ==
          "[subedit] [hearing-impaired] [2/3] source: b.srt\n"
          "[subedit] [hearing-impaired] [2/3] result: 1 subtitle cleaned, 1 removed\n"
          "[subedit] [hearing-impaired] [2/3] dest: o/b.srt\n");
}

TEST_CASE("the copies of an element share its source, said once", "[cli][reporter]") {
    std::ostringstream errors;
    std::ostringstream out;
    const Reporter item = Reporter{errors, 1}.forCommand("shift").forItem(1, 1, "a.srt");
    // A reporter derived from the element, as a subcommand derives one to write a result.
    const Reporter copy = item.withTextOutput(out);

    item.tell(1, Key::Result, "1 subtitle shifted");
    copy.tell(1, Key::Dest, "o/a.srt");

    CHECK(errors.str() == "[subedit] [shift] [1/1] source: a.srt\n"
                          "[subedit] [shift] [1/1] result: 1 subtitle shifted\n"
                          "[subedit] [shift] [1/1] dest: o/a.srt\n");
}

TEST_CASE("under --quiet an error still comes with its source", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter item = Reporter{errors, 0}.forCommand("shift").forItem(2, 3, "bad.srt");

    item.tell(1, Key::Result, "not said");
    item.error("is in no format this tool knows");

    CHECK(errors.str() == "[subedit] [shift] [2/3] source: bad.srt\n"
                          "[subedit] [shift] [2/3] error: is in no format this tool knows\n");
}

TEST_CASE("an element that says nothing at the level asked does not name its source",
          "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter item = Reporter{errors, 1}.forCommand("shift").forItem(1, 2, "a.srt");

    item.tell(2, Key::Format, "SubRip");
    item.tell(3, Key::BytesRead, "131");

    CHECK(errors.str().empty());
}

TEST_CASE("each level keeps every line of the one below, under the new grammar",
          "[cli][reporter]") {
    const auto narrate = [](int level) {
        std::ostringstream errors;
        const Reporter item = Reporter{errors, level}.forCommand("shift").forItem(1, 1, "a.srt");
        item.tell(2, Key::Format, "SubRip");
        item.tell(3, Key::BytesRead, "131");
        item.tell(1, Key::Result, "done");
        return errors.str();
    };

    const std::string source = "[subedit] [shift] [1/1] source: a.srt\n";
    const std::string format = "[subedit] [shift] [1/1] format: SubRip\n";
    const std::string bytes = "[subedit] [shift] [1/1] bytes-read: 131\n";
    const std::string result = "[subedit] [shift] [1/1] result: done\n";
    CHECK(narrate(0).empty());
    CHECK(narrate(1) == source + result);
    CHECK(narrate(2) == source + format + result);
    CHECK(narrate(3) == source + format + bytes + result);
}

TEST_CASE("a second input of one element is a source line of its own", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter item = Reporter{errors, 1}.forCommand("append").forItem(1, 1, "a.srt");

    item.tell(1, Key::Source, "b.srt");

    CHECK(errors.str() == "[subedit] [append] [1/1] source: a.srt\n"
                          "[subedit] [append] [1/1] source: b.srt\n");
}

TEST_CASE("every physical line of a sentence carries the prefix and the key", "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter reporter = Reporter{errors, 1}.forCommand("transcribe");

    reporter.error("whisper-cli was not found\nsee the installation page\n");

    CHECK(errors.str() == "[subedit] [transcribe] [-] error: whisper-cli was not found\n"
                          "[subedit] [transcribe] [-] error: see the installation page\n");
}

TEST_CASE("a line end in a path is written as \\n, so that the path stays on one line",
          "[cli][reporter]") {
    std::ostringstream errors;
    const Reporter item = Reporter{errors, 1}.forCommand("shift").forItem(1, 1, "a\nb.srt");

    item.tell(1, Key::Dest, "o/a\nb.srt");

    CHECK(errors.str() == "[subedit] [shift] [1/1] source: a\\nb.srt\n"
                          "[subedit] [shift] [1/1] dest: o/a\\nb.srt\n");
}

TEST_CASE("the keys are the closed list of the spec, by their written name", "[cli][reporter]") {
    using subedit::cli::nameOf;
    CHECK(nameOf(Key::Source) == "source");
    CHECK(nameOf(Key::Translation) == "translation");
    CHECK(nameOf(Key::Media) == "media");
    CHECK(nameOf(Key::Format) == "format");
    CHECK(nameOf(Key::Encoding) == "encoding");
    CHECK(nameOf(Key::Bom) == "bom");
    CHECK(nameOf(Key::LineEndings) == "line-endings");
    CHECK(nameOf(Key::BytesRead) == "bytes-read");
    CHECK(nameOf(Key::Diagnostic) == "diagnostic");
    CHECK(nameOf(Key::Alignment) == "alignment");
    CHECK(nameOf(Key::Progress) == "progress");
    CHECK(nameOf(Key::Warning) == "warning");
    CHECK(nameOf(Key::Error) == "error");
    CHECK(nameOf(Key::Result) == "result");
    CHECK(nameOf(Key::BytesWritten) == "bytes-written");
    CHECK(nameOf(Key::Dest) == "dest");
    CHECK(nameOf(Key::Tail) == "tail");
    CHECK(nameOf(Key::Settings) == "settings");
    CHECK(nameOf(Key::Walk) == "walk");
    CHECK(nameOf(Key::Summary) == "summary");
}
