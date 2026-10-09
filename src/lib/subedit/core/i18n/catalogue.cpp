#include <subedit/core/i18n/catalogue.hpp>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>

namespace subedit::core {

namespace {

constexpr std::uint32_t kMagic = 0x950412de;
constexpr std::uint32_t kMagicSwapped = 0xde120495;
constexpr std::size_t kHeaderSize = 28;
constexpr std::size_t kPairSize = 8;
constexpr std::uint32_t kMajorShift = 16;
constexpr std::uint32_t kLastKnownMajor = 1;
constexpr char kContextSeparator = '\x04';
/// No language needs more, and a header that asks for more is not a plural rule.
constexpr std::size_t kMostPluralForms = 16;
constexpr unsigned kByteBits = 8;
constexpr std::size_t kDecimalBase = 10;
constexpr std::size_t kMostCountDigits = 3;
constexpr std::size_t kReadChunk = 4096;

class Bytes {

public:
    Bytes(std::string_view data, bool swapped) : m_data(data), m_swapped(swapped) {}

    [[nodiscard]] std::uint32_t word(std::size_t offset) const {
        std::uint32_t value = 0;
        for (std::size_t i = 0; i < sizeof(std::uint32_t); ++i) {
            const auto byte = static_cast<std::uint8_t>(m_data[offset + i]);
            const std::size_t shift = m_swapped ? (sizeof(std::uint32_t) - 1 - i) : i;
            value |= static_cast<std::uint32_t>(byte) << (kByteBits * shift);
        }
        return value;
    }

    [[nodiscard]] std::size_t size() const { return m_data.size(); }

    [[nodiscard]] std::string_view slice(std::size_t offset, std::size_t length) const {
        return m_data.substr(offset, length);
    }

    [[nodiscard]] char at(std::size_t offset) const { return m_data[offset]; }

private:
    std::string_view m_data;
    bool m_swapped;
};

/// True when `count` entries of eight bytes starting at `offset` lie inside `size` bytes.
/// Done in 64 bits: two 32-bit values from the file cannot overflow it.
bool tableFits(std::uint64_t offset, std::uint64_t count, std::uint64_t size) {
    return offset + count * kPairSize <= size;
}

/// The string of a (length, offset) pair, or the way it failed.
std::expected<std::string_view, CatalogueProblem> stringAt(const Bytes& bytes, std::size_t pair) {
    const std::uint64_t length = bytes.word(pair);
    const std::uint64_t offset = bytes.word(pair + sizeof(std::uint32_t));
    // The byte after the string is read too: it must be the NUL.
    if (offset + length >= bytes.size()) {
        return std::unexpected(CatalogueProblem::StringOutsideFile);
    }
    if (bytes.at(offset + length) != '\0') {
        return std::unexpected(CatalogueProblem::UnterminatedString);
    }
    return bytes.slice(offset, length);
}

std::vector<std::string> splitOnNul(std::string_view text) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (true) {
        const auto end = text.find('\0', start);
        if (end == std::string_view::npos) {
            parts.emplace_back(text.substr(start));
            return parts;
        }
        parts.emplace_back(text.substr(start, end - start));
        start = end + 1;
    }
}

struct PluralHeader {
    std::size_t count;
    PluralExpression expression;
};

/// `nplurals=3; plural=(…);` out of the header entry, when it has the line.
std::expected<std::optional<PluralHeader>, std::string> pluralHeader(std::string_view header) {
    constexpr std::string_view kKey = "Plural-Forms:";
    std::size_t start = 0;
    while (start <= header.size()) {
        auto end = header.find('\n', start);
        if (end == std::string_view::npos) {
            end = header.size();
        }
        std::string_view line = header.substr(start, end - start);
        start = end + 1;
        if (!line.starts_with(kKey)) {
            continue;
        }
        line.remove_prefix(kKey.size());

        const auto countAt = line.find("nplurals=");
        const auto ruleAt = line.find("plural=");
        if (countAt == std::string_view::npos || ruleAt == std::string_view::npos) {
            return std::unexpected("the header lacks nplurals or plural");
        }

        std::size_t count = 0;
        std::size_t digits = 0;
        for (std::size_t i = countAt + std::string_view("nplurals=").size();
             i < line.size() && line[i] >= '0' && line[i] <= '9' && digits < kMostCountDigits;
             ++i, ++digits) {
            count = count * kDecimalBase + static_cast<std::size_t>(line[i] - '0');
        }
        if (count == 0 || count > kMostPluralForms) {
            return std::unexpected("nplurals is not between 1 and 16");
        }

        auto rule = line.substr(ruleAt + std::string_view("plural=").size());
        if (const auto semicolon = rule.find(';'); semicolon != std::string_view::npos) {
            rule = rule.substr(0, semicolon);
        }
        auto expression = PluralExpression::parse(rule);
        if (!expression) {
            return std::unexpected(expression.error());
        }
        return PluralHeader{.count = count, .expression = std::move(*expression)};
    }
    return std::nullopt;
}

std::string keyOf(std::string_view context, std::string_view msgid) {
    if (context.empty()) {
        return std::string(msgid);
    }
    std::string key(context);
    key += kContextSeparator;
    key += msgid;
    return key;
}

} // namespace

std::expected<Catalogue, CatalogueError> Catalogue::fromBytes(std::string_view data) {
    const auto refuse = [](CatalogueProblem problem, std::string detail = {}) {
        return std::unexpected(CatalogueError{.problem = problem, .detail = std::move(detail)});
    };

    if (data.size() < kHeaderSize) {
        return refuse(CatalogueProblem::TooShort);
    }

    // The magic number tells the byte order; read in the machine's order it is one of two.
    const Bytes little(data, false);
    bool swapped = false;
    if (little.word(0) == kMagic) {
        swapped = false;
    } else if (little.word(0) == kMagicSwapped) {
        swapped = true;
    } else {
        return refuse(CatalogueProblem::BadMagic);
    }
    const Bytes bytes(data, swapped);

    if ((bytes.word(sizeof(std::uint32_t)) >> kMajorShift) > kLastKnownMajor) {
        return refuse(CatalogueProblem::UnsupportedRevision);
    }

    const std::uint64_t count = bytes.word(2 * sizeof(std::uint32_t));
    const std::uint64_t originals = bytes.word(3 * sizeof(std::uint32_t));
    const std::uint64_t translations = bytes.word(4 * sizeof(std::uint32_t));
    if (!tableFits(originals, count, data.size()) || !tableFits(translations, count, data.size())) {
        return refuse(CatalogueProblem::TableOutsideFile);
    }

    Catalogue catalogue;
    for (std::uint64_t i = 0; i < count; ++i) {
        const auto original = stringAt(bytes, originals + (i * kPairSize));
        if (!original) {
            return refuse(original.error());
        }
        const auto translation = stringAt(bytes, translations + (i * kPairSize));
        if (!translation) {
            return refuse(translation.error());
        }

        if (original->empty()) {
            // The header entry: its translation holds the catalogue's metadata.
            auto plural = pluralHeader(*translation);
            if (!plural) {
                return refuse(CatalogueProblem::BadPluralForms, plural.error());
            }
            if (plural->has_value()) {
                catalogue.m_pluralCount = (*plural)->count;
                catalogue.m_plural = std::move((*plural)->expression);
            }
            continue;
        }

        // `context \x04 msgid`, and for a plural message `singular \0 plural`: only the
        // singular is looked up, as gettext does.
        const auto singular = original->substr(0, original->find('\0'));
        catalogue.m_messages.try_emplace(std::string(singular), splitOnNul(*translation));
    }
    return catalogue;
}

std::expected<Catalogue, CatalogueError> Catalogue::fromFile(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return std::unexpected(
            CatalogueError{.problem = CatalogueProblem::Unreadable, .detail = path.string()});
    }
    // `read` reports a failure through the stream's state; an iterator over the buffer would
    // throw instead, on a directory for instance.
    std::string bytes;
    std::array<char, kReadChunk> chunk{};
    while (stream.read(chunk.data(), static_cast<std::streamsize>(chunk.size())) ||
           stream.gcount() > 0) {
        bytes.append(chunk.data(), static_cast<std::size_t>(stream.gcount()));
    }
    if (stream.bad()) {
        return std::unexpected(
            CatalogueError{.problem = CatalogueProblem::Unreadable, .detail = path.string()});
    }
    return fromBytes(bytes);
}

std::optional<std::string_view> Catalogue::find(std::string_view context,
                                                std::string_view msgid) const {
    const auto found = m_messages.find(keyOf(context, msgid));
    if (found == m_messages.end() || found->second.empty() || found->second.front().empty()) {
        return std::nullopt;
    }
    return std::string_view(found->second.front());
}

std::optional<std::string_view> Catalogue::findPlural(std::string_view singular,
                                                      std::uint64_t n) const {
    const auto found = m_messages.find(std::string(singular));
    if (found == m_messages.end()) {
        return std::nullopt;
    }
    const auto index = m_plural.evaluate(n);
    if (index >= m_pluralCount || index >= found->second.size() || found->second[index].empty()) {
        return std::nullopt;
    }
    return std::string_view(found->second[static_cast<std::size_t>(index)]);
}

} // namespace subedit::core
