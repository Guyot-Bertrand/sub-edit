// `correct`, on an in-memory file system and a catalogue of its own.
//
// What is under test is the layer between the options and the core: what a line
// of options comes to, what it is refused for, and how a file's run is worded.
// The patterns are a handful of records written here, so that nothing depends on
// what the installation ships; the shipped ones are played by the end-to-end
// cases, against the oracle.

#include <subedit/cli/correcting.hpp>
#include <subedit/cli/destination.hpp>
#include <subedit/cli/reporter.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/text/spell_checker.hpp>
#include <subedit/core/text/word_list_spell_provider.hpp>
#include <subedit/core/wording/counts.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::correctIn;
using subedit::cli::CorrectionOptions;
using subedit::cli::correctionSettingsOf;
using subedit::cli::Destination;
using subedit::cli::ExitCode;
using subedit::cli::Pairing;
using subedit::cli::Range;
using subedit::cli::Reporter;
using subedit::core::InMemoryFileSystem;
using subedit::core::PatternCatalogue;
using subedit::core::TranslationMethod;

namespace {

/// Two common errors, one of each class; a capitalization pattern that shares a
/// name with one of them; and the hearing-impaired kind, all of it ticked off
/// by its `.conf` as the shipped `Latn` does.
const PatternCatalogue& catalogue() {
    static const PatternCatalogue found = [] {
        InMemoryFileSystem files;
        files.addFile("/p/Latn.common-error",
                      "[Common Error Pattern]\nName=Ligature\nClasses=OCR;\n"
                      "Pattern=ﬁ\nReplacement=fi\n"
                      "\n[Common Error Pattern]\nName=Double space\nClasses=Human;\n"
                      "Pattern=  +\nReplacement=\\040\n"
                      "\n[Common Error Pattern]\nName=Endless\nClasses=Human;\n"
                      "Pattern=(a+)+$\nReplacement=x\n"
                      "\n[Common Error Pattern]\nName=Broken\nClasses=Human;\n"
                      "Pattern=(\nReplacement=x\n");
        files.addFile("/p/Latn.capitalization",
                      "[Capitalization Pattern]\nName=Double space\nCapitalize=After\n"
                      "Pattern=\\. \nFlags=DOTALL;MULTILINE;\n");
        files.addFile("/p/Latn.hearing-impaired",
                      "[Hearing Impaired Pattern]\nName=Sound in brackets\n"
                      "Pattern=\\[[^\\]]*\\]\nReplacement=\n"
                      "\n[Hearing Impaired Pattern]\nName=Sound in parentheses\n"
                      "Pattern=\\([^)]*\\)\nReplacement=\n"
                      "\n[Hearing Impaired Pattern]\nName=Lyrics\n"
                      "Pattern=#[^#]*#\nReplacement=\n");
        files.addFile("/p/Latn.hearing-impaired.conf",
                      "<patterns>\n"
                      "  <pattern name=\"Sound in brackets\" enabled=\"false\"/>\n"
                      "  <pattern name=\"Sound in parentheses\" enabled=\"false\"/>\n"
                      "  <pattern name=\"Lyrics\" enabled=\"false\"/>\n"
                      "</patterns>\n");
        return subedit::core::readPatternCatalogue(files, "/p", {});
    }();
    return found;
}

[[nodiscard]] CorrectionOptions optionsFor(std::string tasks, std::string code = "Latn") {
    CorrectionOptions options;
    options.tasks = std::move(tasks);
    options.code = std::move(code);
    return options;
}

[[nodiscard]] std::string refusalOf(const CorrectionOptions& options) {
    const auto settings = correctionSettingsOf(options, catalogue());
    return settings ? std::string{} : settings.error();
}

[[nodiscard]] std::string srt(const std::string& text) {
    return "1\n00:00:01,000 --> 00:00:02,000\n" + text + "\n\n";
}

struct Run {
    ExitCode code = ExitCode::Success;
    std::string written;
    std::string errors;
};

Run correct(const CorrectionOptions& options,
            const std::string& content,
            const std::optional<Pairing>& pairing = std::nullopt,
            const std::optional<Range>& range = std::nullopt) {
    InMemoryFileSystem files;
    files.addFile("a.srt", content);
    files.addFile("a.fr.srt", srt("fr  ﬁn"));
    std::ostringstream errors;
    const auto settings = correctionSettingsOf(options, catalogue());
    REQUIRE(settings.has_value());

    const ExitCode code = correctIn(files,
                                    {"a.srt"},
                                    std::nullopt,
                                    catalogue(),
                                    *settings,
                                    range,
                                    Destination::from("", "out", false, 1).value(),
                                    Reporter{errors, 1},
                                    pairing);
    const std::string name = pairing ? "out/a.fr.srt" : "out/a.srt";
    return {.code = code, .written = files.contentOf(name).value_or(""), .errors = errors.str()};
}

} // namespace

TEST_CASE("the tasks are named, in any order and once each, and none is chosen beforehand",
          "[cli][correct]") {
    const auto settings =
        correctionSettingsOf(optionsFor("capitalization,common-errors,common-errors"), catalogue());

    REQUIRE(settings.has_value());
    CHECK(settings->commonErrors.enabled);
    CHECK(settings->capitalization.enabled);
    CHECK_FALSE(settings->mentions.enabled);
    CHECK_FALSE(settings->lineBreak.enabled);
    CHECK(settings->commonErrors.code == "Latn");
    CHECK(settings->removeBlankSubtitles);

    CHECK_THAT(refusalOf(optionsFor("tidy")), ContainsSubstring("\"tidy\" is not a task"));
    CHECK_THAT(refusalOf(optionsFor("")), ContainsSubstring("is not a task"));
}

TEST_CASE("the code is required, and written as the cascades are", "[cli][correct]") {
    CHECK_THAT(refusalOf(optionsFor("common-errors", "")), ContainsSubstring("--code is required"));

    for (const char* good : {"Zyyy", "Latn", "Latn-en", "Latn-eng", "Latn-en-US"}) {
        INFO(good);
        // Under codes this catalogue holds nothing for it says so; what it does
        // not say is that the code is malformed.
        CHECK_THAT(refusalOf(optionsFor("common-errors", good)),
                   !ContainsSubstring("is not a pattern code"));
    }
    for (const char* bad :
         {"latn", "Lat", "Latn-EN", "Latn-e", "Latn-en-us", "Latn-en-US-x", "Latn-"}) {
        INFO(bad);
        CHECK_THAT(refusalOf(optionsFor("common-errors", bad)),
                   ContainsSubstring("is not a pattern code"));
    }
}

TEST_CASE("the classes are read, and both are on when none is said", "[cli][correct]") {
    CorrectionOptions options = optionsFor("common-errors");
    CHECK(correctionSettingsOf(options, catalogue())->human);
    CHECK(correctionSettingsOf(options, catalogue())->ocr);

    options.classes = "ocr";
    CHECK_FALSE(correctionSettingsOf(options, catalogue())->human);
    CHECK(correctionSettingsOf(options, catalogue())->ocr);

    options.classes = "ocr,human";
    CHECK(correctionSettingsOf(options, catalogue())->human);

    options.classes = "ocr,both";
    CHECK_THAT(refusalOf(options), ContainsSubstring("\"both\" is not a class"));
}

TEST_CASE("a name switches every record that bears it, and the sound patterns the scan",
          "[cli][correct]") {
    CorrectionOptions options = optionsFor("common-errors,mentions");
    options.disable = {"Ligature"};
    options.enable = {"Sound in parentheses", "Lyrics"};

    const auto settings = correctionSettingsOf(options, catalogue());

    REQUIRE(settings.has_value());
    CHECK(settings->soundInParentheses);
    CHECK_FALSE(settings->soundInBrackets);
    REQUIRE(settings->patternActivations.size() == 2);
    CHECK(settings->patternActivations[0].name == "Lyrics");
    CHECK(settings->patternActivations[0].enabled);
    CHECK(settings->patternActivations[1].name == "Ligature");
    CHECK_FALSE(settings->patternActivations[1].enabled);
}

TEST_CASE("a name that names nothing, or two things, or is contradicted, is refused",
          "[cli][correct]") {
    CorrectionOptions options = optionsFor("common-errors,capitalization");

    options.enable = {"Ligatuer"};
    CHECK_THAT(
        refusalOf(options),
        ContainsSubstring(
            "--enable: \"Ligatuer\" names no pattern of the tasks given, under the code Latn"));

    options.enable = {"Double space"};
    CHECK_THAT(refusalOf(options),
               ContainsSubstring("names patterns of several types: write type:name"));

    options.enable = {"capitalization:Double space"};
    CHECK(refusalOf(options).empty());
    options.enable = {"common-error:Double space"};
    CHECK(refusalOf(options).empty());

    // A type that names no such pattern is a name like another, and finds nothing.
    options.enable = {"hearing-impaired:Double space"};
    CHECK_THAT(refusalOf(options), ContainsSubstring("names no pattern"));

    options.enable = {"Ligature"};
    options.disable = {"Ligature"};
    CHECK_THAT(refusalOf(options), ContainsSubstring("is given to --enable and to --disable"));

    // A name of a task that was not asked for is not a name of this run.
    options = optionsFor("capitalization");
    options.disable = {"Ligature"};
    CHECK_THAT(refusalOf(options), ContainsSubstring("--disable: \"Ligature\" names no pattern"));
}

TEST_CASE("a task with nothing active is refused, by the activation or by the class",
          "[cli][correct]") {
    CHECK_THAT(
        refusalOf(optionsFor("mentions")),
        ContainsSubstring(
            "mentions: no pattern is active under the code Latn; switch one on with --enable"));

    CorrectionOptions options = optionsFor("mentions");
    options.enable = {"Lyrics"};
    CHECK(refusalOf(options).empty());

    // The scan alone is something to do; its two records do not count as patterns.
    options.enable = {"Sound in brackets"};
    CHECK(refusalOf(options).empty());

    options = optionsFor("common-errors");
    options.disable = {"Ligature", "Double space", "Endless", "Broken"};
    CHECK_THAT(refusalOf(options), ContainsSubstring("common-errors: no pattern is active"));
}

TEST_CASE("the subtitles the correction empties are kept when asked", "[cli][correct]") {
    CorrectionOptions options = optionsFor("mentions");
    options.enable = {"Lyrics"};
    options.keepBlankSubtitles = true;

    CHECK_FALSE(correctionSettingsOf(options, catalogue())->removeBlankSubtitles);
}

TEST_CASE("a run says what the window says, and writes the file", "[cli][correct]") {
    const Run run = correct(optionsFor("common-errors"), srt("ﬁn"));

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.errors,
               ContainsSubstring("a.srt: Edited 1 and removed 0 subtitles -> out/a.srt"));
    CHECK(run.written == srt("fin"));
}

TEST_CASE("a pattern that gives up is named with its subtitle, and is not a failure",
          "[cli][correct]") {
    const Run run = correct(optionsFor("common-errors"), srt(std::string(40, 'a') + "b"));

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.errors,
               ContainsSubstring("pattern \"Endless\" (timed out) was not applied to subtitle 1"));
    // The one that does not compile is out for every text, and names none.
    CHECK_THAT(run.errors,
               ContainsSubstring("pattern \"Broken\" (will not compile) was not applied\n"));
    CHECK(run.written == srt(std::string(40, 'a') + "b"));
}

TEST_CASE("a range limits the subtitles looked at", "[cli][correct]") {
    const std::string two =
        "1\n00:00:01,000 --> 00:00:02,000\nﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nﬁn\n\n";

    const Run run =
        correct(optionsFor("common-errors"), two, std::nullopt, Range{.first = 2, .last = 2});

    CHECK_THAT(run.errors, ContainsSubstring("Edited 1 and removed 0 subtitles"));
    CHECK_THAT(run.written, ContainsSubstring("\nﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nfin\n"));
}

TEST_CASE("a translation is corrected and written, and not the main file", "[cli][correct]") {
    const Run run =
        correct(optionsFor("common-errors"),
                srt("Hello"),
                Pairing{.translation = "a.fr.srt", .method = TranslationMethod::Number});

    CHECK(run.code == ExitCode::Success);
    CHECK_THAT(run.errors,
               ContainsSubstring("a.fr.srt: Edited 1 and removed 0 subtitles -> out/a.fr.srt"));
    CHECK(run.written == srt("fr fin"));
}

TEST_CASE("the changes a record carries name the subtitle, and a removal has no text after it",
          "[cli][correct]") {
    InMemoryFileSystem files;
    files.addFile("a.srt",
                  "1\n00:00:01,000 --> 00:00:02,000\n# la #\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\nﬁn\n\n");

    CorrectionOptions options = optionsFor("mentions,common-errors");
    options.enable = {"Lyrics"};
    const auto removed = correctionSettingsOf(options, catalogue());
    options.keepBlankSubtitles = true;
    const auto kept = correctionSettingsOf(options, catalogue());
    REQUIRE((removed.has_value() && kept.has_value()));

    const auto changesOf = [&](const subedit::core::CorrectionSettings& settings) {
        std::ostringstream errors;
        std::ostringstream records;
        const Reporter reporter = Reporter{errors, 0}.withRecords(records).forCommand("correct");
        REQUIRE(correctIn(files,
                          {"a.srt"},
                          std::nullopt,
                          catalogue(),
                          settings,
                          std::nullopt,
                          Destination::from("", "", false, 1, true).value(),
                          reporter) == ExitCode::Success);
        return records.str();
    };

    // Taken away: the text after is null.
    CHECK_THAT(changesOf(*removed),
               ContainsSubstring("\"changes\":[{\"subtitle\":1,\"document\":\"main\","
                                 "\"before\":\"# la #\",\"after\":null},"
                                 "{\"subtitle\":2,\"document\":\"main\",\"before\":\"ﬁn\","
                                 "\"after\":\"fin\"}]"));
    // Kept: the subtitle is there, with no text.
    CHECK_THAT(changesOf(*kept), ContainsSubstring("\"before\":\"# la #\",\"after\":\"\"}"));
}

TEST_CASE("the line break is in characters, has no default length, and its skip follows its bounds",
          "[cli][correct]") {
    CorrectionOptions options = optionsFor("line-break");
    CHECK_THAT(refusalOf(options),
               ContainsSubstring("--max-length is required by the task line-break"));

    options.maxLength = "30";
    const auto settings = correctionSettingsOf(options, catalogue());
    REQUIRE(settings.has_value());
    CHECK(settings->lineBreak.enabled);
    CHECK(settings->lineBreak.code == "Latn");
    CHECK(settings->lineBreakMaxLength == 30.0);
    CHECK(settings->lineBreakMaxLines == 3);
    CHECK_FALSE(settings->lineBreakInEms);
    // The bounds of the skip are those of the break, unless said otherwise.
    CHECK(settings->lineBreakSkipOnLength);
    CHECK(settings->lineBreakSkipMaxLength == 30.0);
    CHECK(settings->lineBreakSkipOnLines);
    CHECK(settings->lineBreakSkipMaxLines == 3);

    options.maxLines = "2";
    options.skipLength = "off";
    options.skipLines = "1";
    const auto given = correctionSettingsOf(options, catalogue());
    REQUIRE(given.has_value());
    CHECK(given->lineBreakMaxLines == 2);
    CHECK_FALSE(given->lineBreakSkipOnLength);
    CHECK(given->lineBreakSkipOnLines);
    CHECK(given->lineBreakSkipMaxLines == 1);

    options.maxLength = "12.5";
    CHECK(correctionSettingsOf(options, catalogue())->lineBreakMaxLength == 12.5);
}

TEST_CASE("the options of the line break are refused when they cannot be read, or are stray",
          "[cli][correct]") {
    CorrectionOptions options = optionsFor("line-break");
    for (const char* bad : {"", "wide", "0", "-3", "1e999", "nan", "12x"}) {
        options.maxLength = bad;
        CHECK_FALSE(refusalOf(options).empty());
    }
    options.maxLength = "20";
    options.maxLines = "2.5";
    CHECK_THAT(refusalOf(options),
               ContainsSubstring("--max-lines: \"2.5\" is not a number of lines"));
    options.maxLines = "";
    options.skipLength = "soon";
    CHECK_THAT(refusalOf(options), ContainsSubstring("--skip-length: \"soon\" is not a bound"));
    options.skipLength = "";
    options.skipLines = "soon";
    CHECK_THAT(refusalOf(options), ContainsSubstring("--skip-lines: \"soon\" is not a bound"));
    options.skipLines = "1.5";
    CHECK_THAT(refusalOf(options),
               ContainsSubstring("--skip-lines: \"1.5\" is not a number of lines"));

    // Given without the task, each is a mistake.
    using Member = std::string CorrectionOptions::*;
    const std::vector<std::pair<std::string, Member>> strays{
        {"--max-length", &CorrectionOptions::maxLength},
        {"--max-lines", &CorrectionOptions::maxLines},
        {"--skip-length", &CorrectionOptions::skipLength},
        {"--skip-lines", &CorrectionOptions::skipLines},
    };
    for (const auto& [name, member] : strays) {
        CorrectionOptions stray = optionsFor("common-errors");
        stray.*member = "5";
        CHECK_THAT(refusalOf(stray), ContainsSubstring(name + " is for the task line-break"));
    }
}

TEST_CASE("a text is broken in characters, and the skip gate leaves one that fits",
          "[cli][correct]") {
    CorrectionOptions options = optionsFor("line-break");
    options.maxLength = "10";
    options.maxLines = "3";
    const std::string text = "one two three four";

    const Run broken = correct(options, srt(text));
    CHECK(broken.code == ExitCode::Success);
    CHECK_THAT(broken.errors, ContainsSubstring("Edited 1 and removed 0 subtitles"));
    CHECK(broken.written == srt("one two\nthree four"));

    // Within its bounds: ten characters in one line.
    CHECK(correct(options, srt("one two")).written == srt("one two"));
}

TEST_CASE("the tasks that check words need a language, and no code", "[cli][correct]") {
    CorrectionOptions options;
    options.tasks = "join-words";
    CHECK_THAT(
        refusalOf(options),
        ContainsSubstring("--language is required by the tasks that check words: join-words"));

    options.language = "French";
    CHECK_THAT(refusalOf(options), ContainsSubstring("\"French\" is not a language"));

    options.language = "fr_FR";
    const auto joining = correctionSettingsOf(options, catalogue());
    REQUIRE(joining.has_value());
    CHECK(joining->joinSplitEnabled);
    CHECK(joining->joinWords);
    CHECK_FALSE(joining->splitWords);
    CHECK(joining->spellLanguage == "fr_FR");
    CHECK_FALSE(joining->commonErrors.enabled);

    options.tasks = "split-words";
    const auto splitting = correctionSettingsOf(options, catalogue());
    REQUIRE(splitting.has_value());
    CHECK_FALSE(splitting->joinWords);
    CHECK(splitting->splitWords);

    options.tasks = "join-words,split-words";
    const auto both = correctionSettingsOf(options, catalogue());
    CHECK((both->joinWords && both->splitWords));

    // A pattern task beside them still wants its code, and the words do not need it.
    options.tasks = "join-words,common-errors";
    CHECK_THAT(refusalOf(options), ContainsSubstring("--code is required"));
    options.code = "Latn";
    CHECK(refusalOf(options).empty());
}

TEST_CASE("a language or a code nothing uses is a mistake", "[cli][correct]") {
    CorrectionOptions options = optionsFor("common-errors");
    options.language = "fr";
    CHECK_THAT(refusalOf(options),
               ContainsSubstring("--language is for the tasks join-words and split-words"));

    options = CorrectionOptions{};
    options.tasks = "join-words";
    options.language = "fr";
    options.code = "Latn";
    CHECK_THAT(refusalOf(options), ContainsSubstring("--code is for the tasks that read patterns"));
}

TEST_CASE("words are joined and split with the checker the caller opened", "[cli][correct]") {
    subedit::core::WordList list;
    list.words = {"hello", "there", "world"};
    list.suggestions = {{"hellothere", {"hello there"}}};
    subedit::core::WordListSpellProvider provider;
    provider.add("zz", std::move(list));
    const InMemoryFileSystem none;
    const auto checker = subedit::core::openSpellChecker(provider, "zz", none, "/none.repl");
    REQUIRE(checker.has_value());

    CorrectionOptions options;
    options.tasks = "join-words,split-words";
    options.language = "zz";
    const auto settings = correctionSettingsOf(options, catalogue());
    REQUIRE(settings.has_value());

    InMemoryFileSystem files;
    files.addFile("a.srt", srt("hel lo hellothere"));
    std::ostringstream errors;
    const ExitCode code = correctIn(files,
                                    {"a.srt"},
                                    std::nullopt,
                                    catalogue(),
                                    *settings,
                                    std::nullopt,
                                    Destination::from("", "out", false, 1).value(),
                                    Reporter{errors, 1},
                                    std::nullopt,
                                    &*checker);

    CHECK(code == ExitCode::Success);
    CHECK(files.contentOf("out/a.srt").value_or("") == srt("hello hello there"));
}

namespace {

/// A provider of one dictionary, `en`, whose words are those of the test.
subedit::cli::SpellProviderFactory englishOnly(int& opened) {
    return [&opened] {
        ++opened;
        auto provider = std::make_unique<subedit::core::WordListSpellProvider>();
        subedit::core::WordList list;
        list.words = {"hello", "world"};
        provider->add("en", std::move(list));
        return provider;
    };
}

} // namespace

TEST_CASE("a run that joins words opens its dictionary before anything is read",
          "[cli][correct][CLI-CORRECT-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    int opened = 0;
    CorrectionOptions options;
    options.tasks = "join-words";
    options.language = "en";

    const auto run = subedit::cli::prepareCorrection(
        files, Reporter{errors, 1}, options, "/shipped", "/user", englishOnly(opened));

    REQUIRE(run.has_value());
    CHECK(opened == 1);
    CHECK(run->settings.joinWords);
    REQUIRE(run->provider != nullptr);
    CHECK(run->spellChecker.has_value());
    // No patterns under the paths given: said, at level one, and the run goes on.
    CHECK_THAT(errors.str(), ContainsSubstring("patterns: "));
}

TEST_CASE("a language nobody has is refused in the words of the window",
          "[cli][correct][CLI-CORRECT-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    int opened = 0;
    CorrectionOptions options;
    options.tasks = "split-words";
    options.language = "fr";

    const auto run = subedit::cli::prepareCorrection(
        files, Reporter{errors, 1}, options, "/shipped", "/user", englishOnly(opened));

    REQUIRE_FALSE(run.has_value());
    CHECK(run.error() == subedit::core::noDictionaryFor("fr"));
}

TEST_CASE("a run that checks no words asks for no provider, and a bad option is refused first",
          "[cli][correct][CLI-CORRECT-01]") {
    InMemoryFileSystem files;
    std::ostringstream errors;
    int opened = 0;
    CorrectionOptions options;
    options.tasks = "common-errors";

    // No code: refused before a dictionary is thought of.
    const auto refused = subedit::cli::prepareCorrection(
        files, Reporter{errors, 1}, options, "/shipped", "/user", englishOnly(opened));
    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("--code is required"));
    CHECK(opened == 0);
}
