#pragma once

// A message catalogue, read from a GNU `.mo` file — ADR 0042.
//
// The format is small and documented: a header, two tables of (length, offset) pairs, and the
// strings. What makes the reader worth its own tests is that the file is not ours — a
// translator's tool wrote it, or a damaged disk, or someone who meant harm. Every length and
// offset is checked against the size of the file before it is followed, and a file that fails
// any check is refused whole rather than read as far as it goes.

#include <subedit/core/i18n/plural_expression.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace subedit::core {

/// Why a catalogue was refused.
enum class CatalogueProblem {
    Unreadable,          ///< the file could not be opened or read
    TooShort,            ///< smaller than the header
    BadMagic,            ///< not a `.mo` file
    UnsupportedRevision, ///< a major revision this reader does not know
    TableOutsideFile,    ///< a table of strings that does not lie inside the file
    StringOutsideFile,   ///< a string whose bytes do not lie inside the file
    UnterminatedString,  ///< a string not followed by its NUL
    BadPluralForms,      ///< a `Plural-Forms` header that does not read
};

struct CatalogueError {
    CatalogueProblem problem;

    /// The path, or the reason the expression did not read. May be empty.
    std::string detail;

    friend bool operator==(const CatalogueError&, const CatalogueError&) = default;
};

class Catalogue {

public:
    /// An empty catalogue translates nothing: the source language.
    Catalogue() = default;

    [[nodiscard]] static std::expected<Catalogue, CatalogueError> fromBytes(std::string_view data);

    [[nodiscard]] static std::expected<Catalogue, CatalogueError>
    fromFile(const std::filesystem::path& path);

    /// The translation of `msgid` under `context` (empty for none), or nothing when the
    /// catalogue does not have it or has it untranslated.
    [[nodiscard]] std::optional<std::string_view> find(std::string_view context,
                                                       std::string_view msgid) const;

    /// The plural form that `n` selects for the message whose singular is `singular`, or
    /// nothing when the catalogue does not have it.
    [[nodiscard]] std::optional<std::string_view> findPlural(std::string_view singular,
                                                             std::uint64_t n) const;

    [[nodiscard]] std::size_t size() const { return m_messages.size(); }

    /// The `nplurals` of the header: two when the catalogue does not say.
    [[nodiscard]] std::size_t pluralCount() const { return m_pluralCount; }

private:
    static constexpr std::size_t kDefaultPluralCount = 2;

    /// The key is `context \x04 msgid`, or the msgid alone when there is none. The forms are
    /// one for a plain message, `nplurals` for a plural one.
    std::unordered_map<std::string, std::vector<std::string>> m_messages;
    PluralExpression m_plural = PluralExpression::germanic();
    std::size_t m_pluralCount = kDefaultPluralCount;
};

} // namespace subedit::core
