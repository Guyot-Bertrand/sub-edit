#include "full_length_project.hpp"

#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::test {
namespace {

using core::FrameRate;
using core::Project;
using core::StandardFrameRate;
using core::Subtitle;
using core::Timestamp;

constexpr std::int64_t kStepMilliseconds = 2500;
constexpr std::int64_t kDurationMilliseconds = 2000;

/// What a subtitle's text is made of.
///
/// The eight shapes that carry a mention are the cases `mentions.cas` settled,
/// and each costs the operation that will read them a different amount of
/// work — `Sole` is the expensive one, since the subtitle itself goes.
enum class Shape {
    Plain,      ///< no mention at all — four subtitles in five
    Sole,       ///< the text is the mention, and nothing survives it
    SoleLine,   ///< two lines, one of them being the mention
    Head,       ///< the mention opens the line, text follows
    Dialogue,   ///< two voices, one of them a mention — the dash goes with it
    Middle,     ///< the mention sits between two neighbours
    Tail,       ///< the mention closes the line
    Twice,      ///< two mentions in the same subtitle
    Straddling, ///< the mention is cut by the line break
};

/// The composition, in subtitles out of `kSubtitleCount`.
///
/// Every figure is a corpus proportion of the header, rounded to a whole
/// subtitle. Their sum is checked against `kSubtitleCount` rather than derived
/// from it: a table that stops adding up is a mistake to report, not a
/// remainder to absorb silently into `Plain`.
constexpr std::array<std::pair<Shape, std::size_t>, 9> kComposition{{
    {Shape::Plain, 3200},
    {Shape::Sole, 464},
    {Shape::SoleLine, 176},
    {Shape::Head, 74},
    {Shape::Dialogue, 28},
    {Shape::Middle, 24},
    {Shape::Tail, 22},
    {Shape::Twice, 10},
    {Shape::Straddling, 2},
}};

/// Ordinary replies, of the lengths a real subtitling has — from four
/// characters to sixty, six of the sixteen on two lines.
constexpr std::array<std::string_view, 16> kReplies{
    "Oui.",
    "Quoi ?",
    "Attends une seconde !",
    "Je ne sais pas du tout.",
    "- Tu viens avec nous ?\n- J'arrive.",
    "Il est parti sans rien dire à personne.",
    "On se retrouve\ndevant la gare, à sept heures.",
    "Non, pas encore, il faut attendre.",
    "Ça ne me plaît pas du tout,\net je le dis depuis le début.",
    "Regarde-moi quand je te parle.",
    "Je t'avais prévenu,\net tu n'as rien voulu entendre.",
    "C'est bon, j'ai compris.",
    "- Et alors ?\n- Alors rien.",
    "Personne n'a rien vu ni rien entendu,\net le dossier est vide.",
    "Laisse-le tranquille.",
    "Tu crois qu'ils vont nous laisser passer ?",
};

/// Single-line replies, for the shapes that must end up on two lines.
constexpr std::array<std::string_view, 6> kVoices{
    "Ça suffit.",
    "Attention, il arrive !",
    "Personne ne bouge.",
    "Restez groupés.",
    "Je n'ai rien entendu.",
    "Trop tard.",
};

/// Replies split in two, so that a mention can be dropped between the halves.
constexpr std::array<std::pair<std::string_view, std::string_view>, 4> kAround{{
    {"Il s'est arrêté", "au milieu de la rue."},
    {"Elle a répondu", "sans lever les yeux."},
    {"On rentre", "avant la nuit."},
    {"Il a raccroché", "sans un mot de plus."},
}};

/// Mentions cut by the line break, written whole: nothing can assemble them
/// without saying twice where the break falls.
constexpr std::array<std::string_view, 2> kStraddling{
    "Reculez ! [Il\nhurle] Tout de suite !",
    "Le train arrive. (bruit de\nfreins) Ne bougez plus.",
};

/// Mentions that are a subtitle on their own.
constexpr std::array<std::string_view, 8> kSoleMentions{
    "[Bruit de pas]",
    "(soupir)",
    "[Musique]",
    "[Il tousse]",
    "(rires)",
    "[Sonnerie de téléphone]",
    "[Grincement]",
    "(chuchotements)",
};

/// Mentions that share their subtitle with a reply.
constexpr std::array<std::string_view, 6> kMentions{
    "[Il murmure]",
    "(bas)",
    "[En criant]",
    "[Voix off]",
    "(hors champ)",
    "[Soupir]",
};

/// A deterministic scramble — splitmix64.
///
/// The shapes have to be laid down irregularly: a real file clusters its
/// mentions, and a regular one-in-five would let the branch predictor learn
/// the fixture rather than measure the work. They also have to fall in the
/// same order on every machine and at every run, or two figures of the journal
/// would not compare. Arithmetic gives both, where a seeded `std::mt19937`
/// would give the second only, at the price of a state nobody can read.
[[nodiscard]] constexpr std::uint64_t scramble(std::uint64_t value) {
    constexpr std::uint64_t kGoldenGap = 0x9E3779B97F4A7C15ULL;
    constexpr std::uint64_t kFirstFactor = 0xBF58476D1CE4E5B9ULL;
    constexpr std::uint64_t kSecondFactor = 0x94D049BB133111EBULL;
    constexpr unsigned kFirstShift = 30;
    constexpr unsigned kSecondShift = 27;
    constexpr unsigned kFinalShift = 31;

    std::uint64_t mixed = value + kGoldenGap;
    mixed = (mixed ^ (mixed >> kFirstShift)) * kFirstFactor;
    mixed = (mixed ^ (mixed >> kSecondShift)) * kSecondFactor;
    return mixed ^ (mixed >> kFinalShift);
}

/// The shapes of `kComposition`, laid down by a Fisher-Yates the scramble drives.
[[nodiscard]] std::vector<Shape> shuffledShapes() {
    std::vector<Shape> order;
    order.reserve(kSubtitleCount);
    for (const auto& [shape, count] : kComposition)
        order.insert(order.end(), count, shape);

    for (std::size_t index = order.size(); index-- > 1;)
        std::swap(order[index], order[scramble(index) % (index + 1)]);
    return order;
}

[[nodiscard]] std::string joined(std::initializer_list<std::string_view> parts) {
    std::size_t size = 0;
    for (const std::string_view part : parts)
        size += part.size();

    std::string text;
    text.reserve(size);
    for (const std::string_view part : parts)
        text.append(part);
    return text;
}

/// Builds the text of the `index`-th subtitle, of shape `shape`.
///
/// The tables are walked by the index rather than drawn: two neighbouring
/// subtitles then differ, which is the point, and the same index always yields
/// the same text.
[[nodiscard]] std::string textFor(Shape shape, std::size_t index) {
    const std::string_view reply = kReplies[index % kReplies.size()];
    const std::string_view voice = kVoices[index % kVoices.size()];
    const std::string_view mention = kMentions[index % kMentions.size()];
    const std::string_view other = kMentions[(index + 1) % kMentions.size()];
    const auto& [before, after] = kAround[index % kAround.size()];

    switch (shape) {
    case Shape::Plain:
        return std::string{reply};
    case Shape::Sole:
        return std::string{kSoleMentions[index % kSoleMentions.size()]};
    case Shape::SoleLine:
        // On the first line as often as on the second: the files hold both,
        // and a removal does not leave the same thing behind.
        return index % 2 == 0 ? joined({mention, "\n", voice}) : joined({voice, "\n", mention});
    case Shape::Head:
        return joined({mention, " ", reply});
    case Shape::Dialogue:
        return joined({"- ", mention, "\n- ", voice});
    case Shape::Middle:
        return joined({before, " ", mention, " ", after});
    case Shape::Tail:
        return joined({before, " ", after, " ", mention});
    case Shape::Twice:
        return joined({mention, " ", reply, " ", other});
    case Shape::Straddling:
        return std::string{kStraddling[index % kStraddling.size()]};
    }
    return {};
}

/// Removes what a mention looks like — brackets or parentheses, within a line.
///
/// **Deliberately naive, and not the operation phase 4 will write.** It has
/// one job: to count what the tables above actually produce, so that the
/// composition documented in the header cannot quietly stop being true. It
/// knows nothing of numeric references, of the space a removal leaves behind,
/// nor of a mention cut by the line break — the two of those in the fixture
/// come out of it untouched, and the count in the test case says so.
[[nodiscard]] std::string withoutMentions(std::string_view text) {
    std::string kept;
    kept.reserve(text.size());

    std::size_t index = 0;
    while (index < text.size()) {
        const char opening = text[index];
        if (opening != '[' && opening != '(') {
            kept.push_back(opening);
            ++index;
            continue;
        }

        const std::size_t closing = text.find(opening == '[' ? ']' : ')', index);
        const std::size_t lineBreak = text.find('\n', index);
        const bool onOneLine = closing != std::string_view::npos &&
                               (lineBreak == std::string_view::npos || lineBreak > closing);
        if (!onOneLine) {
            kept.push_back(opening);
            ++index;
            continue;
        }
        index = closing + 1;
    }
    return kept;
}

/// Tells whether nothing of substance is left — the dash of a dialogue that
/// lost its only other voice counts as nothing.
[[nodiscard]] bool isBlank(std::string_view text) {
    return std::ranges::all_of(text, [](char character) {
        return character == ' ' || character == '\n' || character == '\t' || character == '-';
    });
}

} // namespace

Project fullLengthProject() {
    const std::vector<Shape> shapes = shuffledShapes();

    std::vector<Subtitle> subtitles;
    subtitles.reserve(kSubtitleCount);
    for (std::size_t index = 0; index < kSubtitleCount; ++index) {
        const std::int64_t start = static_cast<std::int64_t>(index) * kStepMilliseconds;
        subtitles.push_back(
            Subtitle{.start = Timestamp::fromMilliseconds(start),
                     .end = Timestamp::fromMilliseconds(start + kDurationMilliseconds),
                     .mainText = textFor(shapes[index], index)});
    }

    Project project;
    project.setSubtitles(std::move(subtitles));
    project.setFrameRate(FrameRate{StandardFrameRate::Fps25});
    return project;
}

// The two test cases below run in the benchmark binary, which is the only
// place this fixture is built. They measure nothing; they hold the two claims
// the header makes — the composition, and the positions that may not move —
// so that a table edited later says so rather than shifting the figures of the
// journal without a word.

TEST_CASE("the measurement fixture is composed as its header says") {
    const Project project = fullLengthProject();

    std::size_t declared = 0;
    for (const auto& [shape, count] : kComposition)
        declared += count;
    REQUIRE(declared == kSubtitleCount);
    REQUIRE(project.count() == kSubtitleCount);

    std::size_t withMention = 0;
    std::size_t emptied = 0;
    std::size_t onTwoLines = 0;
    std::size_t brackets = 0;
    std::size_t parentheses = 0;
    for (const Subtitle& subtitle : project.subtitles()) {
        const std::string kept = withoutMentions(subtitle.mainText);
        if (kept.size() != subtitle.mainText.size()) {
            ++withMention;
            if (isBlank(kept))
                ++emptied;
        }
        if (subtitle.mainText.contains('\n'))
            ++onTwoLines;
        brackets += static_cast<std::size_t>(std::ranges::count(subtitle.mainText, '['));
        parentheses += static_cast<std::size_t>(std::ranges::count(subtitle.mainText, '('));
    }

    // 798 and not 800: the two mentions cut by the line break are invisible to
    // the naive scanner above, which is exactly why they are in the fixture.
    CHECK(withMention == 798);
    CHECK(emptied == 464);
    CHECK(onTwoLines == 1439);
    CHECK(brackets == 528);
    CHECK(parentheses == 282);
}

TEST_CASE("the measurement fixture keeps the positions it has always had") {
    constexpr std::int64_t kLastStart = 9997500;
    constexpr std::int64_t kLastEnd = 9999500;

    const Project project = fullLengthProject();

    REQUIRE(project.count() == kSubtitleCount);
    CHECK(project.subtitles().front().start == Timestamp::origin());
    CHECK(project.subtitles().front().end == Timestamp::fromMilliseconds(kDurationMilliseconds));
    CHECK(project.subtitles().back().start == Timestamp::fromMilliseconds(kLastStart));
    CHECK(project.subtitles().back().end == Timestamp::fromMilliseconds(kLastEnd));
}

} // namespace subedit::test
