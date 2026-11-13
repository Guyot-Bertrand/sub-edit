// The parser's complaints, read back out of CLI11's sentences.
//
// The sentences below are CLI11's own, as `subedit-cli` raises them: each is what the binary
// wrote before #830 for one mistake on its command line. In English the complaint is the same
// sentence, which is the point — it shows the reading found every name and value, since a
// hole would show.

#include <subedit/cli/complaint.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

using subedit::cli::complaintIn;
using subedit::cli::ComplaintKind;
using subedit::cli::helpHint;

TEST_CASE("an unexpected argument is said again", "[cli][complaint]") {
    CHECK(complaintIn(ComplaintKind::Unexpected,
                      "The following argument was not expected: --bogus") ==
          "The following argument was not expected: --bogus");
    CHECK(
        complaintIn(ComplaintKind::Unexpected, "The following arguments were not expected: a b") ==
        "The following arguments were not expected: a b");
}

TEST_CASE("a missing argument is said again", "[cli][complaint]") {
    CHECK(complaintIn(ComplaintKind::Required, "--to is required") == "--to is required");
    CHECK(complaintIn(ComplaintKind::Required, "files is required") == "files is required");
    CHECK(complaintIn(ComplaintKind::Required, "A subcommand is required") ==
          "A subcommand is required");
}

TEST_CASE("two options that exclude each other are said again", "[cli][complaint]") {
    CHECK(complaintIn(ComplaintKind::Excludes, "--config excludes --no-config") ==
          "--config excludes --no-config");
}

TEST_CASE("a wrong count of values is said again", "[cli][complaint]") {
    CHECK(complaintIn(ComplaintKind::Count, "--by: 1 required TEXT missing") ==
          "--by: 1 required TEXT missing");
    CHECK(complaintIn(ComplaintKind::Count, "--by: At Most 1 required but received 2") ==
          "--by: At Most 1 required but received 2");
    CHECK(complaintIn(ComplaintKind::Count, "--x: At least 2 required but received 1") ==
          "--x: At least 2 required but received 1");
}

TEST_CASE("a refused value is said again", "[cli][complaint]") {
    CHECK(complaintIn(ComplaintKind::Validation, "--format: yaml not in {text,json}") ==
          "--format: yaml not in {text,json}");
    CHECK(
        complaintIn(ComplaintKind::Validation, "--min-match: Value 99999 not in range [1 - 100]") ==
        "--min-match: Value 99999 not in range [1 - 100]");
}

TEST_CASE("a value below zero is not told the largest double", "[cli][complaint]") {
    // `CLI::NonNegativeNumber` is a range up to the largest double, and CLI11 prints it.
    CHECK(complaintIn(ComplaintKind::Validation,
                      "--audio-track: Value -3 not in range [0 - 1.79769e+308]") ==
          "--audio-track: Value -3 is not a number zero or greater");
}

TEST_CASE("a sentence that is not recognised is kept as written", "[cli][complaint]") {
    // Rather than a translated sentence with a hole in it.
    for (const ComplaintKind kind : {ComplaintKind::Unexpected,
                                     ComplaintKind::Required,
                                     ComplaintKind::Excludes,
                                     ComplaintKind::Count,
                                     ComplaintKind::Validation,
                                     ComplaintKind::Other}) {
        CHECK(complaintIn(kind, "something else entirely") == "something else entirely");
    }
    CHECK(complaintIn(ComplaintKind::Count, "--x: something") == "--x: something");
    CHECK(complaintIn(ComplaintKind::Count, "--x: At Most one") == "--x: At Most one");
    CHECK(complaintIn(ComplaintKind::Count, "--x: one missing") == "--x: one missing");
    CHECK(complaintIn(ComplaintKind::Validation, "--x: y not in {a") == "--x: y not in {a");
    CHECK(complaintIn(ComplaintKind::Validation, "--x: something") == "--x: something");
    CHECK(complaintIn(ComplaintKind::Validation, "--x: Value y") == "--x: Value y");
    CHECK(complaintIn(ComplaintKind::Validation, "--x: Value y not in range [1]") ==
          "--x: Value y not in range [1]");
    CHECK(complaintIn(ComplaintKind::Validation, "--x: Value y not in range [1 - 2") ==
          "--x: Value y not in range [1 - 2");
}

TEST_CASE("the help hint is CLI11's", "[cli][complaint]") {
    CHECK(helpHint() == "Run with --help for more information.");
}
