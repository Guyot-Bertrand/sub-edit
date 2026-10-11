// The options of `adjust` turned into constraints, and the loop that applies them.

#include <subedit/cli/adjusting.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/index_grammar.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <optional>
#include <sstream>
#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::adjustAll;
using subedit::cli::AdjustOptions;
using subedit::cli::constraintsOf;
using subedit::cli::constraintsRecord;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Json;
using subedit::cli::Range;
using subedit::cli::Reporter;
using subedit::core::Duration;
using subedit::core::DurationAdjustmentSettings;
using subedit::core::DurationConstraints;
using subedit::core::InMemoryFileSystem;
using subedit::core::ReadingSpeed;

namespace {

const std::string kTwo = "1\n00:00:01,000 --> 00:00:01,100\nabcdefghij\n\n"
                         "2\n00:00:05,000 --> 00:00:05,100\nabcdefghij\n\n";

} // namespace

TEST_CASE("no option is Gaupol's four constraints", "[cli][adjusting][CLI-ADJUST-01]") {
    const auto constraints = constraintsOf({});

    REQUIRE(constraints.has_value());
    CHECK(*constraints == DurationConstraints{});
}

TEST_CASE("each constraint is set by its option", "[cli][adjusting][CLI-ADJUST-02]") {
    const auto constraints = constraintsOf({.speed = "12.5",
                                            .minimum = "2",
                                            .maximum = "00:00:06.000",
                                            .gap = "0.25",
                                            .shorten = true,
                                            .noLengthen = true});

    REQUIRE(constraints.has_value());
    CHECK(constraints->speed == ReadingSpeed::create(12.5, false, true));
    CHECK(constraints->minimum == Duration::fromMilliseconds(2000));
    CHECK(constraints->maximum == Duration::fromMilliseconds(6000));
    CHECK(constraints->gap == Duration::fromMilliseconds(250));
}

TEST_CASE("off removes a constraint instead of zeroing it", "[cli][adjusting][CLI-ADJUST-02]") {
    const auto constraints = constraintsOf({.speed = "off", .minimum = "off", .gap = "off"});
    // Everything off but nothing on is refused; a maximum keeps one alive.
    CHECK_FALSE(constraints.has_value());

    const auto kept =
        constraintsOf({.speed = "off", .minimum = "off", .maximum = "3", .gap = "off"});
    REQUIRE(kept.has_value());
    CHECK_FALSE(kept->speed.has_value());
    CHECK_FALSE(kept->minimum.has_value());
    CHECK_FALSE(kept->gap.has_value());
    CHECK(kept->maximum == Duration::fromMilliseconds(3000));
}

TEST_CASE("a minimum or a gap of zero is zero, not off", "[cli][adjusting][CLI-ADJUST-02]") {
    const auto constraints = constraintsOf({.minimum = "0", .gap = "0"});

    REQUIRE(constraints.has_value());
    CHECK(constraints->minimum == Duration::zero());
    CHECK(constraints->gap == Duration::zero());
}

TEST_CASE("nothing active is refused", "[cli][adjusting][CLI-ADJUST-03]") {
    const auto none = constraintsOf({.speed = "off", .minimum = "off", .gap = "off"});
    REQUIRE_FALSE(none.has_value());
    CHECK_THAT(none.error(), ContainsSubstring("nothing to adjust to"));

    // A speed that moves no end constrains nothing.
    CHECK_FALSE(constraintsOf({.minimum = "off", .gap = "off", .noLengthen = true}).has_value());
}

TEST_CASE("what is no constraint is refused, naming the option",
          "[cli][adjusting][CLI-ADJUST-02]") {
    for (const std::string speed :
         {"0", "-3", "fast", "1e3", "inf", "nan", ".5", "5.", "1,5", ""}) {
        if (speed.empty()) {
            continue; // not given
        }
        const auto constraints = constraintsOf({.speed = speed});
        INFO(speed);
        REQUIRE_FALSE(constraints.has_value());
        CHECK_THAT(constraints.error(), ContainsSubstring("--speed: \"" + speed + "\""));
    }

    CHECK_THAT(constraintsOf({.minimum = "-1"}).error(), ContainsSubstring("--minimum:"));
    CHECK_THAT(constraintsOf({.minimum = "soon"}).error(), ContainsSubstring("--minimum:"));
    CHECK_THAT(constraintsOf({.maximum = "0"}).error(), ContainsSubstring("above zero"));
    CHECK_THAT(constraintsOf({.maximum = "x"}).error(), ContainsSubstring("--maximum:"));
    CHECK_THAT(constraintsOf({.gap = "-0.1"}).error(), ContainsSubstring("zero or more"));
    CHECK_THAT(constraintsOf({.gap = "x"}).error(), ContainsSubstring("--gap:"));
}

TEST_CASE("the two directions need a speed to apply to", "[cli][adjusting][CLI-ADJUST-02]") {
    CHECK_FALSE(constraintsOf({.speed = "off", .shorten = true}).has_value());
    CHECK_FALSE(constraintsOf({.speed = "off", .noLengthen = true}).has_value());
}

TEST_CASE("the constraints are recorded in whole milliseconds and a speed as text",
          "[cli][adjusting][CLI-ADJUST-05]") {
    CHECK(constraintsRecord(DurationConstraints{}).dump() ==
          R"({"speed":{"cps":"15","lengthen":true,"shorten":false},)"
          R"("minimum_ms":1500,"maximum_ms":null,"gap_ms":0})");

    DurationConstraints other;
    other.speed = ReadingSpeed::create(12.5, true, true);
    other.minimum.reset();
    other.maximum = Duration::fromMilliseconds(6000);
    other.gap.reset();
    CHECK(constraintsRecord(other).dump() ==
          R"({"speed":{"cps":"12.5","lengthen":true,"shorten":true},)"
          R"("minimum_ms":null,"maximum_ms":6000,"gap_ms":null})");

    other.speed.reset();
    CHECK(constraintsRecord(other).dump().starts_with(R"({"speed":null,)"));
}

TEST_CASE("an option beats the file, which beats the default", "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.charactersPerSecond = 12;
    file.minimumMilliseconds = 2000;

    // File over default: nothing on the command line.
    const auto fromFile = constraintsOf(AdjustOptions{}, file);
    REQUIRE(fromFile.has_value());
    REQUIRE(fromFile->speed.has_value());
    CHECK(fromFile->speed->charactersPerSecond() == 12.0);
    CHECK(fromFile->minimum == Duration::fromMilliseconds(2000));

    // Option over file.
    const auto overridden = constraintsOf(AdjustOptions{.speed = "20", .minimum = "off"}, file);
    REQUIRE(overridden.has_value());
    REQUIRE(overridden->speed.has_value());
    CHECK(overridden->speed->charactersPerSecond() == 20.0);
    CHECK_FALSE(overridden->minimum.has_value());
}

TEST_CASE("the file's directions, maximum and gap are its own until an option says otherwise",
          "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.lengthen = false;
    file.shorten = true;
    file.minimumEnabled = false;
    file.maximumEnabled = true;
    file.maximumMilliseconds = 4000;
    file.gapMilliseconds = 300;

    const auto read = constraintsOf(AdjustOptions{}, file);
    REQUIRE(read.has_value());
    CHECK(read->speed == ReadingSpeed::create(15.0, false, true));
    CHECK_FALSE(read->minimum.has_value());
    CHECK(read->maximum == Duration::fromMilliseconds(4000));
    CHECK(read->gap == Duration::fromMilliseconds(300));

    const auto overridden =
        constraintsOf(AdjustOptions{.maximum = "5", .gap = "off", .noLengthen = true}, file);
    REQUIRE(overridden.has_value());
    CHECK(overridden->speed == ReadingSpeed::create(15.0, false, true));
    CHECK(overridden->maximum == Duration::fromMilliseconds(5000));
    CHECK_FALSE(overridden->gap.has_value());
}

TEST_CASE("default settings give exactly the old behaviour", "[cli][adjusting][settings]") {
    // Worked out by hand: Gaupol's four constraints, as before the file was read.
    DurationConstraints expected;
    expected.speed = ReadingSpeed::create(15.0, true, false);
    expected.minimum = Duration::fromMilliseconds(1500);
    expected.maximum.reset();
    expected.gap = Duration::fromMilliseconds(0);

    const auto constraints = constraintsOf(AdjustOptions{}, DurationAdjustmentSettings{});
    REQUIRE(constraints.has_value());
    CHECK(*constraints == expected);
}

TEST_CASE("each option given replaces the file's value, one by one", "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.charactersPerSecond = 12;
    file.minimumMilliseconds = 2000;
    file.maximumEnabled = true;
    file.maximumMilliseconds = 4000;
    file.gapMilliseconds = 300;

    const auto given =
        constraintsOf(AdjustOptions{.minimum = "1", .maximum = "6", .gap = "0.5"}, file);
    REQUIRE(given.has_value());
    CHECK(given->speed == ReadingSpeed::create(12.0, true, false));
    CHECK(given->minimum == Duration::fromMilliseconds(1000));
    CHECK(given->maximum == Duration::fromMilliseconds(6000));
    CHECK(given->gap == Duration::fromMilliseconds(500));
}

TEST_CASE("a gap disabled in the file is no gap", "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.gapEnabled = false;
    file.gapMilliseconds = 300;

    const auto constraints = constraintsOf(AdjustOptions{}, file);
    REQUIRE(constraints.has_value());
    CHECK_FALSE(constraints->gap.has_value());
}

TEST_CASE("the direction options act on the file's directions", "[cli][adjusting][settings]") {
    // An active file speed that only lengthens, plus --shorten: both ways.
    DurationAdjustmentSettings lengthening;
    lengthening.charactersPerSecond = 12;
    CHECK(constraintsOf(AdjustOptions{.shorten = true}, lengthening)->speed ==
          ReadingSpeed::create(12.0, true, true));

    // A file speed switched off, plus --no-lengthen: the directions start from
    // the default (lengthen only), and --no-lengthen takes that away. What is
    // left is a speed that moves no end, which constrains nothing; the minimum
    // and the gap of the file keep the request alive.
    DurationAdjustmentSettings off;
    off.lengthen = false;
    off.shorten = false;
    const auto noLengthen = constraintsOf(AdjustOptions{.noLengthen = true}, off);
    REQUIRE(noLengthen.has_value());
    CHECK(noLengthen->speed == ReadingSpeed::create(15.0, false, false));
    CHECK(noLengthen->minimum == Duration::fromMilliseconds(1500));
}

TEST_CASE("a speed given replaces a hand-edited speed of zero", "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.charactersPerSecond = 0;
    CHECK(constraintsOf(AdjustOptions{.speed = "12"}, file)->speed ==
          ReadingSpeed::create(12.0, true, false));
}

TEST_CASE("the directions with the speed switched off are refused, whatever the file says",
          "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.charactersPerSecond = 12;
    file.shorten = true;
    const auto refused = constraintsOf(AdjustOptions{.speed = "off", .shorten = true}, file);
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("--speed off switches the reading speed off"));
}

TEST_CASE("the speed switched off by option beats a speed in the file",
          "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.charactersPerSecond = 12;
    const auto constraints = constraintsOf(AdjustOptions{.speed = "off"}, file);
    REQUIRE(constraints.has_value());
    CHECK_FALSE(constraints->speed.has_value());
}

TEST_CASE("a file with the speed switched off keeps it off until asked",
          "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.lengthen = false;
    file.shorten = false;
    CHECK_FALSE(constraintsOf(AdjustOptions{}, file)->speed.has_value());
    CHECK(constraintsOf(AdjustOptions{.speed = "12"}, file)->speed ==
          ReadingSpeed::create(12.0, true, false));
    CHECK(constraintsOf(AdjustOptions{.shorten = true}, file)->speed ==
          ReadingSpeed::create(15.0, true, true));
}

TEST_CASE("a hand-edited speed of zero is read as the speed being off",
          "[cli][adjusting][settings]") {
    DurationAdjustmentSettings file;
    file.charactersPerSecond = 0;
    const auto constraints = constraintsOf(AdjustOptions{}, file);
    REQUIRE(constraints.has_value());
    CHECK_FALSE(constraints->speed.has_value());
}

TEST_CASE("adjusting writes the ends and counts what gave way", "[cli][adjusting][CLI-ADJUST-04]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = adjustAll(files,
                                    {"a.srt"},
                                    std::nullopt,
                                    DurationConstraints{},
                                    Json{},
                                    std::nullopt,
                                    Destination::from("", "out", false, 1).value(),
                                    Reporter{errors, 1}.withRecords(records).forCommand("adjust"));

    CHECK(code == ExitCode::Success);
    // Both are too short: 10 characters need 0.667 s and the minimum is 1.5 s.
    CHECK_THAT(files.contentOf("out/a.srt").value_or(""),
               ContainsSubstring("00:00:01,000 --> 00:00:02,500"));
    CHECK_THAT(errors.str(), ContainsSubstring("adjusted the durations of 2 subtitles"));
    CHECK_THAT(records.str(),
               ContainsSubstring("\"counts\":{\"subtitles\":2,\"adjusted\":2,"
                                 "\"sacrificed\":{\"speed\":0,\"minimum\":0,\"gap\":0}}"));
    CHECK_THAT(records.str(), ContainsSubstring("\"constraints\":{\"speed\":{\"cps\":\"15\""));
    // No settings file read: the record says so rather than leave the field out.
    CHECK_THAT(records.str(), ContainsSubstring("\"settings\":null"));
}

TEST_CASE("a range limits what is adjusted and what is counted",
          "[cli][adjusting][CLI-ADJUST-02]") {
    InMemoryFileSystem files;
    files.addFile("a.srt", kTwo);
    std::ostringstream errors;
    std::ostringstream records;

    const ExitCode code = adjustAll(files,
                                    {"a.srt"},
                                    std::nullopt,
                                    DurationConstraints{},
                                    Json{},
                                    Range{.first = 2, .last = 2},
                                    Destination::from("", "out", false, 1).value(),
                                    Reporter{errors, 1}.withRecords(records).forCommand("adjust"));

    CHECK(code == ExitCode::Success);
    const std::string written = files.contentOf("out/a.srt").value_or("");
    CHECK_THAT(written, ContainsSubstring("00:00:01,000 --> 00:00:01,100"));
    CHECK_THAT(written, ContainsSubstring("00:00:05,000 --> 00:00:06,500"));
    CHECK_THAT(records.str(), ContainsSubstring("\"subtitles\":1,\"adjusted\":1"));
}
