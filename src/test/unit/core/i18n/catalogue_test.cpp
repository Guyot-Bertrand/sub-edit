// The reader of `.mo` files — issue #658, ADR 0042.
//
// Two kinds of file: the ones `msgfmt` compiles from `src/test/data/i18n/*.po`, which are what
// real translators produce, and the ones built here byte by byte, which are what a damaged or
// hostile file looks like. The second kind must be refused, never read out of bounds — the
// suite runs under ASan.

#include <subedit/core/i18n/catalogue.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

using subedit::core::Catalogue;
using subedit::core::CatalogueProblem;

namespace {

std::string compiled(const char* name) {
    return std::string(SUBEDIT_COMPILED_I18N_DIR) + "/" + name;
}

Catalogue loaded(const char* name) {
    auto catalogue = Catalogue::fromFile(compiled(name));
    REQUIRE(catalogue.has_value());
    return std::move(*catalogue);
}

std::string readAll(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

void put(std::string& out, std::uint32_t value, bool bigEndian) {
    for (int i = 0; i < 4; ++i) {
        const int shift = bigEndian ? 8 * (3 - i) : 8 * i;
        out.push_back(static_cast<char>((value >> shift) & 0xffU));
    }
}

struct Entry {
    Entry(std::string originalText, std::string translatedText)
        : original(std::move(originalText)), translation(std::move(translatedText)) {}

    std::string original;
    std::string translation;
};

/// A `.mo` file as `msgfmt` lays it out, minus the hash table, in either byte order.
std::string
build(const std::vector<Entry>& entries, bool bigEndian = false, std::uint32_t revision = 0) {
    const auto count = static_cast<std::uint32_t>(entries.size());
    const std::uint32_t originals = 28;
    const std::uint32_t translations = originals + (8 * count);
    std::uint32_t cursor = translations + (8 * count);

    std::string originalTable;
    std::string translationTable;
    std::string strings;
    for (const auto& entry : entries) {
        put(originalTable, static_cast<std::uint32_t>(entry.original.size()), bigEndian);
        put(originalTable, cursor, bigEndian);
        strings += entry.original;
        strings.push_back('\0');
        cursor += static_cast<std::uint32_t>(entry.original.size()) + 1;
    }
    for (const auto& entry : entries) {
        put(translationTable, static_cast<std::uint32_t>(entry.translation.size()), bigEndian);
        put(translationTable, cursor, bigEndian);
        strings += entry.translation;
        strings.push_back('\0');
        cursor += static_cast<std::uint32_t>(entry.translation.size()) + 1;
    }

    std::string file;
    put(file, 0x950412de, bigEndian);
    put(file, revision, bigEndian);
    put(file, count, bigEndian);
    put(file, originals, bigEndian);
    put(file, translations, bigEndian);
    put(file, 0, bigEndian);
    put(file, 0, bigEndian);
    return file + originalTable + translationTable + strings;
}

void overwrite(std::string& file, std::size_t offset, std::uint32_t value) {
    std::string word;
    put(word, value, false);
    file.replace(offset, 4, word);
}

constexpr const char* kHeader = "Plural-Forms: nplurals=2; plural=(n > 1);\n";

} // namespace

TEST_CASE("a catalogue compiled by msgfmt translates", "[i18n][catalogue]") {
    const auto french = loaded("fr.mo");
    CHECK(french.find({}, "Save") == "Enregistrer");
    CHECK(french.find({}, "Save As…") == "Enregistrer sous…");
    CHECK(french.pluralCount() == 2);
}

TEST_CASE("a key the catalogue does not have is nothing", "[i18n][catalogue]") {
    const auto french = loaded("fr.mo");
    CHECK_FALSE(french.find({}, "Quit").has_value());
    CHECK_FALSE(french.find({}, "").has_value());
    CHECK_FALSE(french.find("verb", "Save").has_value());
}

TEST_CASE("an untranslated message is nothing too", "[i18n][catalogue]") {
    CHECK_FALSE(loaded("fr.mo").find({}, "Close").has_value());
}

TEST_CASE("a context is part of the key", "[i18n][catalogue]") {
    const auto french = loaded("fr.mo");
    CHECK(french.find("verb", "Open") == "Ouvrir");
    CHECK(french.find("adjective", "Open") == "Ouvert");
    CHECK_FALSE(french.find({}, "Open").has_value());
}

TEST_CASE("the plural form is chosen by the header's expression", "[i18n][catalogue]") {
    const auto french = loaded("fr.mo");
    CHECK(french.findPlural("%1 subtitle", 0) == "%1 sous-titre");
    CHECK(french.findPlural("%1 subtitle", 1) == "%1 sous-titre");
    CHECK(french.findPlural("%1 subtitle", 2) == "%1 sous-titres");
    CHECK_FALSE(french.findPlural("%1 line", 2).has_value());

    const auto russian = loaded("ru.mo");
    CHECK(russian.pluralCount() == 3);
    CHECK(russian.findPlural("%1 subtitle", 22) == "%1 субтитра");
    CHECK(russian.findPlural("%1 subtitle", 0) == "%1 субтитров");
}

TEST_CASE("a catalogue without a header uses English's plural rule", "[i18n][catalogue]") {
    const auto bytes = build({{"%1 file", std::string("%1 fichier\0%1 fichiers", 22)}});
    const auto catalogue = Catalogue::fromBytes(bytes);
    REQUIRE(catalogue.has_value());
    CHECK(catalogue->pluralCount() == 2);
    CHECK(catalogue->findPlural("%1 file", 1) == "%1 fichier");
    CHECK(catalogue->findPlural("%1 file", 0) == "%1 fichiers");
    CHECK(catalogue->findPlural("%1 file", 2) == "%1 fichiers");
}

TEST_CASE("a file that cannot be opened is refused", "[i18n][catalogue]") {
    const auto catalogue = Catalogue::fromFile(compiled("absent.mo"));
    REQUIRE_FALSE(catalogue.has_value());
    CHECK(catalogue.error().problem == CatalogueProblem::Unreadable);
}

TEST_CASE("a directory is not a catalogue", "[i18n][catalogue]") {
    const auto catalogue = Catalogue::fromFile(SUBEDIT_COMPILED_I18N_DIR);
    REQUIRE_FALSE(catalogue.has_value());
    CHECK(catalogue.error().problem == CatalogueProblem::Unreadable);
}

TEST_CASE("a catalogue whose plural rule does not read is refused whole", "[i18n][catalogue]") {
    const auto catalogue = Catalogue::fromFile(compiled("bad-plural.mo"));
    REQUIRE_FALSE(catalogue.has_value());
    CHECK(catalogue.error().problem == CatalogueProblem::BadPluralForms);
    CHECK_FALSE(catalogue.error().detail.empty());
}

TEST_CASE("a header with an absurd number of forms is refused", "[i18n][catalogue]") {
    for (const char* header : {"Plural-Forms: nplurals=0; plural=0;\n",
                               "Plural-Forms: nplurals=99; plural=0;\n",
                               "Plural-Forms: nplurals=2;\n",
                               "Plural-Forms: plural=n;\n"}) {
        INFO(header);
        const auto catalogue = Catalogue::fromBytes(build({{"", header}}));
        REQUIRE_FALSE(catalogue.has_value());
        CHECK(catalogue.error().problem == CatalogueProblem::BadPluralForms);
    }
}

TEST_CASE("both byte orders read", "[i18n][catalogue]") {
    for (const bool bigEndian : {false, true}) {
        INFO(bigEndian);
        const auto catalogue =
            Catalogue::fromBytes(build({{"", kHeader}, {"Save", "Enregistrer"}}, bigEndian));
        REQUIRE(catalogue.has_value());
        CHECK(catalogue->find({}, "Save") == "Enregistrer");
    }
}

TEST_CASE("a revision with a later major number is refused", "[i18n][catalogue]") {
    const auto catalogue = Catalogue::fromBytes(build({{"Save", "Enregistrer"}}, false, 2U << 16));
    REQUIRE_FALSE(catalogue.has_value());
    CHECK(catalogue.error().problem == CatalogueProblem::UnsupportedRevision);

    CHECK(Catalogue::fromBytes(build({{"Save", "Enregistrer"}}, false, 1)).has_value());
}

TEST_CASE("a file that is not a catalogue is refused", "[i18n][catalogue]") {
    const auto empty = Catalogue::fromBytes("");
    REQUIRE_FALSE(empty.has_value());
    CHECK(empty.error().problem == CatalogueProblem::TooShort);

    const auto text = Catalogue::fromBytes(std::string(100, 'x'));
    REQUIRE_FALSE(text.has_value());
    CHECK(text.error().problem == CatalogueProblem::BadMagic);
}

TEST_CASE("a hostile count, offset or length is refused, not followed", "[i18n][catalogue]") {
    const auto valid = build({{"Save", "Enregistrer"}});
    REQUIRE(Catalogue::fromBytes(valid).has_value());

    SECTION("a count the file cannot hold") {
        auto file = valid;
        overwrite(file, 8, 0xffffffffU);
        const auto catalogue = Catalogue::fromBytes(file);
        REQUIRE_FALSE(catalogue.has_value());
        CHECK(catalogue.error().problem == CatalogueProblem::TableOutsideFile);
    }
    SECTION("a table that starts past the end") {
        auto file = valid;
        overwrite(file, 12, 0xfffffff0U);
        const auto catalogue = Catalogue::fromBytes(file);
        REQUIRE_FALSE(catalogue.has_value());
        CHECK(catalogue.error().problem == CatalogueProblem::TableOutsideFile);
    }
    SECTION("a table whose end wraps around 32 bits") {
        auto file = valid;
        overwrite(file, 8, 0x20000001U);
        overwrite(file, 12, 0xfffffff8U);
        CHECK_FALSE(Catalogue::fromBytes(file).has_value());
    }
    SECTION("a string longer than the file") {
        auto file = valid;
        overwrite(file, 28, 0x7fffffffU);
        const auto catalogue = Catalogue::fromBytes(file);
        REQUIRE_FALSE(catalogue.has_value());
        CHECK(catalogue.error().problem == CatalogueProblem::StringOutsideFile);
    }
    SECTION("a string whose length and offset add up past 32 bits") {
        auto file = valid;
        overwrite(file, 28, 0xffffffffU);
        overwrite(file, 32, 0xffffffffU);
        CHECK_FALSE(Catalogue::fromBytes(file).has_value());
    }
    SECTION("a string that ends exactly at the end of the file, with no NUL") {
        auto file = valid;
        file.pop_back();
        // The last string loses its terminator: either it is outside, or it is unterminated.
        CHECK_FALSE(Catalogue::fromBytes(file).has_value());
    }
    SECTION("a string whose terminator is not a NUL") {
        auto file = valid;
        file.back() = 'x';
        const auto catalogue = Catalogue::fromBytes(file);
        REQUIRE_FALSE(catalogue.has_value());
        CHECK(catalogue.error().problem == CatalogueProblem::UnterminatedString);
    }
}

TEST_CASE("no prefix of a real catalogue reads out of bounds", "[i18n][catalogue]") {
    const auto file = readAll(compiled("ru.mo"));
    REQUIRE(file.size() > 100);
    std::size_t accepted = 0;
    for (std::size_t length = 0; length < file.size(); ++length) {
        // Refused or, when what is missing is only the tail of the last string's region,
        // accepted: what matters is that nothing is read past the prefix (ASan).
        const auto catalogue = Catalogue::fromBytes(std::string_view(file).substr(0, length));
        if (catalogue.has_value()) {
            ++accepted;
        }
    }
    // A prefix that cuts the strings off is refused; the whole file is the only one that reads.
    CHECK(accepted == 0);
}

TEST_CASE("a catalogue with bytes altered never reads out of bounds", "[i18n][catalogue]") {
    const auto original = readAll(compiled("fr.mo"));
    std::uint32_t state = 12345;
    const auto next = [&state] {
        state = state * 1664525U + 1013904223U;
        return state >> 8;
    };

    for (int round = 0; round < 3000; ++round) {
        auto file = original;
        const int changes = 1 + static_cast<int>(next() % 4);
        for (int i = 0; i < changes; ++i) {
            file[next() % file.size()] = static_cast<char>(next());
        }
        const auto catalogue = Catalogue::fromBytes(file);
        if (catalogue) {
            // Whatever it holds must be safe to look up.
            (void)catalogue->find({}, "Save");
            (void)catalogue->findPlural("%1 subtitle", next());
        }
    }
}

TEST_CASE("a duplicated key keeps the first translation", "[i18n][catalogue]") {
    const auto catalogue = Catalogue::fromBytes(build({{"Save", "first"}, {"Save", "second"}}));
    REQUIRE(catalogue.has_value());
    CHECK(catalogue->find({}, "Save") == "first");
}
