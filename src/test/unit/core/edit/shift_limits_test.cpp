// Ce qu'un décalage ne peut pas faire — issue #132.
//
// La règle vivait dans la ligne de commande, dans la boucle de `shiftAll`. La
// fenêtre en a besoin du même, et deux copies d'une même règle dérivent — on
// vient d'en payer une avec le vocabulaire.

#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace {

using subedit::core::Duration;
using subedit::core::firstBeforeOrigin;
using subedit::core::Project;
using subedit::core::Selection;
using subedit::core::Subtitle;
using subedit::core::SubtitleIndex;
using subedit::core::Timestamp;

[[nodiscard]] Subtitle at(std::int64_t start) {
    return Subtitle{.start = Timestamp::fromMilliseconds(start),
                    .end = Timestamp::fromMilliseconds(start + 1000),
                    .mainText = "x"};
}

/// Départs à une, trois et cinq secondes.
[[nodiscard]] Project three() {
    Project project;
    project.setSubtitles({at(1000), at(3000), at(5000)});
    return project;
}

[[nodiscard]] Selection only(std::size_t value) {
    const SubtitleIndex index = SubtitleIndex::fromValue(value);
    return Selection::range(index, index);
}

} // namespace

TEST_CASE("a shift that keeps everything on the timeline is allowed", "[edit][shift]") {
    const Project project = three();

    CHECK_FALSE(
        firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(-1000))
            .has_value());
}

TEST_CASE("a shift that would take a subtitle before the origin names the first", "[edit][shift]") {
    // Le premier, et non n'importe lequel : c'est celui que l'utilisateur doit
    // regarder pour comprendre de combien il s'est trompé.
    const Project project = three();

    // L'option entière plutôt que son contenu : clang-tidy ne reconnaît pas le
    // REQUIRE de Catch2 comme une vérification.
    CHECK(firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(-2000)) ==
          SubtitleIndex::fromNumber(1));
}

TEST_CASE("only the selected subtitles are looked at", "[edit][shift]") {
    // Décaler le troisième de quatre secondes vers l'arrière le laisse à une
    // seconde ; le premier passerait avant l'origine, mais on ne le décale pas.
    const Project project = three();

    CHECK_FALSE(firstBeforeOrigin(project, only(2), Duration::fromMilliseconds(-4000)).has_value());
    CHECK(firstBeforeOrigin(project, only(0), Duration::fromMilliseconds(-4000)).has_value());
}

TEST_CASE("landing exactly on the origin is allowed", "[edit][shift]") {
    // Zéro est une position, et la refuser ferait de la borne un interdit.
    const Project project = three();

    CHECK_FALSE(
        firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(-1000))
            .has_value());
}

TEST_CASE("a shift forward is never refused", "[edit][shift]") {
    const Project project = three();

    CHECK_FALSE(
        firstBeforeOrigin(project, Selection::all(project), Duration::fromMilliseconds(3600000))
            .has_value());
}

TEST_CASE("an empty selection has nothing to refuse", "[edit][shift]") {
    const Project project = three();

    CHECK_FALSE(firstBeforeOrigin(project, Selection::of({}), Duration::fromMilliseconds(-99999))
                    .has_value());
}
