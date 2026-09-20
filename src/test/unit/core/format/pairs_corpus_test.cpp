// The pairs, and what opening a translation over a main document must give.
//
// `src/test/data/paires/` holds one main document — the scene of `formats/` —
// and eight translations of it, each written to separate the two ways Gaupol
// attaches a translation: by number, and by position. Beside each translation
// sit the two results it must lead to, one per method, **written by hand from a
// reading of `aeidon/agents/open.py`** and each as two SubRip files at the same
// positions: the main texts, and the translations.
//
// **Nothing here aligns anything.** The alignment is a later issue's work, and
// it will find these cases already there. What is under test is the data: that
// every file is SubRip that reads and comes back byte for byte, that a case
// holds what the table says it holds, and that the hand-written results keep the
// promises no alignment could break — every translation line lands exactly once,
// every main text survives, and the two methods differ exactly where they are
// said to.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::Newline;
using subedit::core::ReadError;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::RealFileSystem;
using subedit::core::Subtitle;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

/// One case of the directory: what it holds, and how its two results compare.
struct PairCase {
    std::string_view directory;

    /// Whether attaching by number and by position lead to the same result.
    ///
    /// **Three cases agree and five do not**, and the five are the point: a
    /// case where the methods agree teaches nothing about which to choose. The
    /// control (`temoin`) is there to show that they can agree at all.
    bool methodsAgree;

    /// How many subtitles each method's result holds.
    std::size_t byNumber;
    std::size_t byPosition;
};

/// The main document holds four subtitles, so a result of five holds one that a
/// translation line gave birth to.
constexpr std::size_t kMainSubtitles = 4;

constexpr std::array<PairCase, 8> kCases = {
    PairCase{.directory = "temoin", .methodsAgree = true, .byNumber = 4, .byPosition = 4},
    PairCase{.directory = "traduction-plus-courte",
             .methodsAgree = true,
             .byNumber = 4,
             .byPosition = 4},
    PairCase{.directory = "traduction-dans-le-desordre",
             .methodsAgree = true,
             .byNumber = 4,
             .byPosition = 4},
    PairCase{.directory = "une-ligne-de-moins-au-milieu",
             .methodsAgree = false,
             .byNumber = 4,
             .byPosition = 4},
    PairCase{.directory = "une-ligne-de-plus-a-la-fin",
             .methodsAgree = false,
             .byNumber = 5,
             .byPosition = 5},
    PairCase{.directory = "une-ligne-dans-un-intervalle-vide",
             .methodsAgree = false,
             .byNumber = 5,
             .byPosition = 5},
    PairCase{.directory = "deux-lignes-dans-un-meme-sous-titre",
             .methodsAgree = false,
             .byNumber = 5,
             .byPosition = 5},
    PairCase{
        .directory = "positions-decalees", .methodsAgree = false, .byNumber = 4, .byPosition = 7},
};

/// The six files every case holds, and no other.
constexpr std::array<std::string_view, 6> kFiles = {
    "attendu-numero.principal.srt",
    "attendu-numero.traduction.srt",
    "attendu-position.principal.srt",
    "attendu-position.traduction.srt",
    "principal.srt",
    "traduction.srt",
};

/// The one file of the directory that is not a case.
constexpr std::string_view kReadme = "LISEZMOI.md";

[[nodiscard]] std::filesystem::path pairsDirectory() {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "paires";
}

[[nodiscard]] std::string bytesOf(std::string_view directory, std::string_view file) {
    const std::filesystem::path path = pairsDirectory() / directory / file;
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content = files.readFile(path);
    if (!content.has_value()) {
        FAIL("fixture not found: " + path.string());
        return {};
    }
    return *std::move(content);
}

[[nodiscard]] ReadResult readOf(std::string_view directory, std::string_view file) {
    std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf(directory, file));
    if (!result.has_value()) {
        FAIL("fixture unreadable: " + std::string{directory} + "/" + std::string{file});
        return {};
    }
    return *std::move(result);
}

/// The result of one method, as two documents read back.
struct Expected {
    ReadResult main;
    ReadResult translation;
};

[[nodiscard]] Expected expectedOf(std::string_view directory, std::string_view method) {
    return Expected{
        .main = readOf(directory, "attendu-" + std::string{method} + ".principal.srt"),
        .translation = readOf(directory, "attendu-" + std::string{method} + ".traduction.srt"),
    };
}

/// The texts of a document, in order.
[[nodiscard]] std::vector<std::string> textsOf(const ReadResult& result) {
    std::vector<std::string> texts;
    texts.reserve(result.subtitles.size());
    for (const Subtitle& subtitle : result.subtitles)
        texts.push_back(subtitle.mainText);
    return texts;
}

/// The non-empty texts of a document, sorted: a bag of lines, order aside.
[[nodiscard]] std::vector<std::string> bagOf(const ReadResult& result) {
    std::vector<std::string> bag;
    for (const Subtitle& subtitle : result.subtitles)
        if (!subtitle.mainText.empty())
            bag.push_back(subtitle.mainText);
    std::ranges::sort(bag);
    return bag;
}

[[nodiscard]] std::size_t emptyTextsOf(const ReadResult& result) {
    return static_cast<std::size_t>(std::ranges::count_if(
        result.subtitles, [](const Subtitle& subtitle) { return subtitle.mainText.empty(); }));
}

/// The directories of the pairs, the readme aside.
[[nodiscard]] std::vector<std::string> directories() {
    std::vector<std::string> found;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{pairsDirectory()}) {
        const std::string name = entry.path().filename().string();
        if (name != kReadme)
            found.push_back(name);
    }
    std::ranges::sort(found);
    return found;
}

[[nodiscard]] std::vector<std::string> filesOf(std::string_view directory) {
    std::vector<std::string> found;
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{pairsDirectory() / directory})
        found.push_back(entry.path().filename().string());
    std::ranges::sort(found);
    return found;
}

} // namespace

TEST_CASE("the pairs directory and the case table name the same cases", "[format][corpus][pairs]") {
    // Both ways round, and that is the whole guard: a case added without a row
    // would be read by nothing, and a row left without its case would assert on
    // a directory that is gone. Neither shows in a diff.
    std::vector<std::string> declared;
    declared.reserve(kCases.size());
    for (const PairCase& pair : kCases)
        declared.emplace_back(pair.directory);
    std::ranges::sort(declared);

    CHECK(directories() == declared);
}

TEST_CASE("every case holds its six files and no other", "[format][corpus][pairs]") {
    std::vector<std::string> expected;
    expected.reserve(kFiles.size());
    for (const std::string_view file : kFiles)
        expected.emplace_back(file);
    std::ranges::sort(expected);

    for (const PairCase& pair : kCases) {
        INFO("case: " << pair.directory);
        CHECK(filesOf(pair.directory) == expected);
    }
}

TEST_CASE("every file is SubRip that reads cleanly and comes back byte for byte",
          "[format][corpus][pairs]") {
    // **The empty block is what this asks most of.** A subtitle no translation
    // reached, and one no main subtitle holds, are both written as a block with
    // no text: the hand-written results are only usable if such a block reads
    // without a diagnostic and writes back exactly as it was.
    for (const PairCase& pair : kCases) {
        for (const std::string_view file : kFiles) {
            INFO("file: " << pair.directory << "/" << file);
            const std::string original = bytesOf(pair.directory, file);
            const std::expected<ReadResult, ReadError> result = readSubtitles(original);
            REQUIRE(result.has_value());
            CHECK(result->diagnostics.empty());

            // **One convention for the whole directory.** The writer puts back
            // whatever it was read with, so the round trip below cannot tell a
            // file in CRLF or with a byte order mark from one without: it would
            // hand them back unchanged. This is what keeps the fixtures alike.
            CHECK(result->newline == Newline::Lf);
            CHECK(result->encoding == Encoding::utf8(ByteOrderMark::Absent));

            const std::expected<std::string, WriteError> written =
                writeSubtitles(result->format,
                               WriteRequest{
                                   .subtitles = result->subtitles,
                                   .newline = result->newline,
                                   .encoding = result->encoding,
                                   .header = result->header,
                               });
            REQUIRE(written.has_value());
            CHECK(*written == original);
        }
    }
}

TEST_CASE("every case starts from the same main document", "[format][corpus][pairs]") {
    // What varies from case to case is the translation, and only it — the
    // discipline of `formats/`, which varies only the format.
    const std::string scene = bytesOf(kCases.front().directory, "principal.srt");
    REQUIRE(readOf(kCases.front().directory, "principal.srt").subtitles.size() == kMainSubtitles);

    for (const PairCase& pair : kCases) {
        INFO("case: " << pair.directory);
        CHECK(bytesOf(pair.directory, "principal.srt") == scene);
    }
}

TEST_CASE("the two files of a result hold the same positions", "[format][corpus][pairs]") {
    // A result is one list of subtitles written twice, once for each of its
    // texts, so the two files can only differ by what they say. A position that
    // drifted between them would be a subtitle that is two things at once.
    for (const PairCase& pair : kCases) {
        for (const auto& [method, count] :
             {std::pair<std::string_view, std::size_t>{"numero", pair.byNumber},
              std::pair<std::string_view, std::size_t>{"position", pair.byPosition}}) {
            INFO("case: " << pair.directory << ", method: " << method);
            const Expected result = expectedOf(pair.directory, method);
            REQUIRE(result.main.subtitles.size() == count);
            REQUIRE(result.translation.subtitles.size() == count);

            for (std::size_t index = 0; index < count; ++index) {
                INFO("subtitle " << index + 1);
                CHECK(result.main.subtitles[index].start ==
                      result.translation.subtitles[index].start);
                CHECK(result.main.subtitles[index].end == result.translation.subtitles[index].end);
            }
        }
    }
}

TEST_CASE("a subtitle no main subtitle holds is born with no main text",
          "[format][corpus][pairs]") {
    // The result holds the four main subtitles, texts intact and in order, and
    // as many more as the table says — each of them with nothing to say in the
    // main document, because a translation line gave it birth.
    const std::vector<std::string> scene =
        textsOf(readOf(kCases.front().directory, "principal.srt"));

    for (const PairCase& pair : kCases) {
        for (const auto& [method, count] :
             {std::pair<std::string_view, std::size_t>{"numero", pair.byNumber},
              std::pair<std::string_view, std::size_t>{"position", pair.byPosition}}) {
            INFO("case: " << pair.directory << ", method: " << method);
            const Expected result = expectedOf(pair.directory, method);

            CHECK(emptyTextsOf(result.main) == count - kMainSubtitles);

            // The main texts that remain, in their order, are the scene's.
            std::vector<std::string> kept;
            for (const std::string& text : textsOf(result.main))
                if (!text.empty())
                    kept.push_back(text);
            CHECK(kept == scene);
        }
    }
}

TEST_CASE("every translation line lands exactly once, whichever way it is attached",
          "[format][corpus][pairs]") {
    // **Nothing dropped, nothing invented, nothing counted twice.** Whatever the
    // method does with a line — the right subtitle, a wrong one, a subtitle of
    // its own — the lines of the result are the lines of the translation. This
    // is what a hand-written result can get wrong without a reader noticing: a
    // line lost in the typing.
    for (const PairCase& pair : kCases) {
        const std::vector<std::string> given = bagOf(readOf(pair.directory, "traduction.srt"));

        for (const std::string_view method : {"numero", "position"}) {
            INFO("case: " << pair.directory << ", method: " << method);
            CHECK(bagOf(expectedOf(pair.directory, method).translation) == given);
        }
    }
}

TEST_CASE("the two methods differ exactly where the table says", "[format][corpus][pairs]") {
    // A case that is meant to separate the methods and does not is a copy that
    // went wrong, and it would keep every other case in this file green.
    for (const PairCase& pair : kCases) {
        INFO("case: " << pair.directory);
        const bool sameMain = bytesOf(pair.directory, "attendu-numero.principal.srt") ==
                              bytesOf(pair.directory, "attendu-position.principal.srt");
        const bool sameTranslation = bytesOf(pair.directory, "attendu-numero.traduction.srt") ==
                                     bytesOf(pair.directory, "attendu-position.traduction.srt");

        CHECK((sameMain && sameTranslation) == pair.methodsAgree);
    }
}

TEST_CASE("only the case about disorder has a translation out of order",
          "[format][corpus][pairs]") {
    // The translation is sorted before it is attached, so the disorder is the
    // one thing that can be asked once and not again: any other case with an
    // unordered file would be testing the sort while claiming to test the
    // attachment.
    for (const PairCase& pair : kCases) {
        INFO("case: " << pair.directory);
        const ReadResult translation = readOf(pair.directory, "traduction.srt");

        const bool ordered = std::ranges::is_sorted(
            translation.subtitles,
            [](const Subtitle& left, const Subtitle& right) { return left.start < right.start; });
        CHECK(ordered == (pair.directory != "traduction-dans-le-desordre"));
    }
}
