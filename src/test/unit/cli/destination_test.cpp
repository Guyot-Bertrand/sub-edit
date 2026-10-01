#include <subedit/cli/destination.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::Destination;
using subedit::core::InMemoryFileSystem;

TEST_CASE("no destination at all is refused", "[cli][destination]") {
    const auto destination = Destination::from("", "", false, 1);

    REQUIRE_FALSE(destination.has_value());
    // A harness that overwrites its own input without being asked is a harness
    // one stops using: the refusal names the three ways to say where to write.
    CHECK_THAT(destination.error(), ContainsSubstring("--output"));
    CHECK_THAT(destination.error(), ContainsSubstring("--output-dir"));
    CHECK_THAT(destination.error(), ContainsSubstring("--in-place"));
}

TEST_CASE("two destinations at once are refused", "[cli][destination]") {
    CHECK_FALSE(Destination::from("a.vtt", "out", false, 1).has_value());
    CHECK_FALSE(Destination::from("a.vtt", "", true, 1).has_value());
    CHECK_FALSE(Destination::from("", "out", true, 1).has_value());
}

TEST_CASE("a single output file cannot take several inputs", "[cli][destination]") {
    const auto destination = Destination::from("a.vtt", "", false, 2);

    REQUIRE_FALSE(destination.has_value());
    // Writing the last input over the previous ones is the outcome this
    // refusal exists to prevent, so the message points at the option that works.
    CHECK_THAT(destination.error(), ContainsSubstring("--output-dir"));
}

TEST_CASE("a named output file is used as it is", "[cli][destination]") {
    const Destination destination = Destination::from("out/renamed.vtt", "", false, 1).value();

    // The caller named the file; nothing is added to it, not even an extension
    // that would match the format.
    CHECK(destination.pathFor("input/a.srt", ".vtt") == "out/renamed.vtt");
}

TEST_CASE("in place writes back over the input", "[cli][destination]") {
    const Destination destination = Destination::from("", "", true, 3).value();

    CHECK(destination.pathFor("input/a.srt", "") == "input/a.srt");
    CHECK(destination.isInPlace());
}

TEST_CASE("a directory keeps the base name", "[cli][destination]") {
    const Destination destination = Destination::from("", "out", false, 2).value();

    CHECK(destination.pathFor("input/a.srt", "") == "out/a.srt");
}

TEST_CASE("a directory takes the extension of the format written", "[cli][destination]") {
    const Destination destination = Destination::from("", "out", false, 2).value();

    // Writing WebVTT into a file named .srt would produce a file that lies
    // about itself — every other tool would trip on it.
    CHECK(destination.pathFor("input/a.srt", ".vtt") == "out/a.vtt");
}

TEST_CASE("a directory accepts a single input too", "[cli][destination]") {
    CHECK(Destination::from("", "out", false, 1).has_value());
}

TEST_CASE("a plan computes each destination once, in the order of the inputs",
          "[cli][destination][CLI-BATCH-03]") {
    const InMemoryFileSystem files;
    const Destination destination = Destination::from("", "out", false, 2).value();

    const auto jobs = destination.plan(files, {"a/one.srt", "b/two.srt"}, ".vtt");

    REQUIRE(jobs.has_value());
    REQUIRE(jobs->size() == 2);
    CHECK((*jobs)[0].input == "a/one.srt");
    CHECK((*jobs)[0].output == "out/one.vtt");
    CHECK((*jobs)[1].output == "out/two.vtt");
}

TEST_CASE("two inputs of one destination are refused, both named",
          "[cli][destination][CLI-BATCH-03]") {
    const InMemoryFileSystem files;
    const Destination destination = Destination::from("", "out", false, 2).value();

    const auto jobs = destination.plan(files, {"a/film.srt", "b/film.srt"}, "");

    REQUIRE_FALSE(jobs.has_value());
    CHECK(jobs.error() == "out/film.srt: would be written by both a/film.srt and b/film.srt");
}

TEST_CASE("the extension of the destination decides whether two inputs collide",
          "[cli][destination][CLI-BATCH-03]") {
    const InMemoryFileSystem files;
    const Destination destination = Destination::from("", "out", false, 2).value();

    // The same base name, two extensions: apart when kept, together once converted.
    CHECK(destination.plan(files, {"a/film.srt", "b/film.vtt"}, "").has_value());
    CHECK_FALSE(destination.plan(files, {"a/film.srt", "b/film.vtt"}, ".vtt").has_value());
}

TEST_CASE("the same input given twice collides with itself", "[cli][destination][CLI-BATCH-03]") {
    const InMemoryFileSystem files;
    const Destination destination = Destination::from("", "", true, 2).value();

    CHECK_FALSE(destination.plan(files, {"a.srt", "./a.srt"}, "").has_value());
}

TEST_CASE("a destination that is an input is refused without --in-place",
          "[cli][destination][CLI-BATCH-04]") {
    const InMemoryFileSystem files;
    const Destination here = Destination::from("", "in", false, 1).value();

    const auto refused = here.plan(files, {"in/film.srt"}, "");

    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("is itself the input in/film.srt"));
    CHECK_THAT(refused.error(), ContainsSubstring("use --in-place"));

    // Another spelling of the same place, and the other input of the pair.
    CHECK_FALSE(here.plan(files, {"in/../in/film.srt"}, "").has_value());
    CHECK_FALSE(Destination::from("", "in", false, 2)
                    ->plan(files, {"x/a.srt", "in/a.srt"}, "")
                    .has_value());
    CHECK_FALSE(Destination::from("in/film.srt", "", false, 1)
                    ->plan(files, {"in/film.srt"}, "")
                    .has_value());
}

TEST_CASE("writing in place is the one gesture that writes over an input",
          "[cli][destination][CLI-BATCH-04]") {
    const InMemoryFileSystem files;
    const Destination destination = Destination::from("", "", true, 2).value();

    const auto jobs = destination.plan(files, {"in/a.srt", "in/b.srt"}, "");

    REQUIRE(jobs.has_value());
    CHECK((*jobs)[1].output == "in/b.srt");
}
