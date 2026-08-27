cmake_minimum_required(VERSION 3.28)

project(
    subedit
    VERSION 0.6.10
    DESCRIPTION "Éditeur de sous-titres"
    HOMEPAGE_URL "https://github.com/Guyot-Bertrand/sub-edit"
    LANGUAGES CXX)

# Ce projet n'est jamais construit comme sous-projet d'un autre.
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR "subedit se construit seul, pas comme sous-projet.")
endif()

# C++23 pour std::expected — voir docs/adr/0007-cpp23-et-std-expected.md.
# Exige clang-tidy >= 19 : la version 18 ne voit pas <expected>.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Les extensions GNU rendraient le code non portable sans qu'on s'en aperçoive.
set(CMAKE_CXX_EXTENSIONS OFF)

# Consommé par clang-tidy et par les éditeurs.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(SUBEDIT_BUILD_TESTS "Construire les tests et les benchmarks" ON)
option(SUBEDIT_SANITIZERS "Activer AddressSanitizer et UndefinedBehaviorSanitizer" OFF)
option(SUBEDIT_COVERAGE "Instrumenter pour la mesure de couverture" OFF)
# OFF par défaut ; seuls les presets asan et release le mettent à ON. Un
# indicateur déclaratif se lit mieux dans six mois qu'un if() composé sur
# CMAKE_BUILD_TYPE et SUBEDIT_SANITIZERS — voir src/test/e2e/CMakeLists.txt
# pour la raison de ce choix de presets.
option(SUBEDIT_REGISTER_E2E "Enregistrer les tests de bout en bout dans CTest" OFF)

# Date de génération : celle du commit, et non l'horloge. Deux compilations
# d'une même source donnent alors le même binaire — ce que l'horloge
# interdirait, et que ccache paierait en recompilations inutiles. La date se
# rafraîchit à la configuration, ce qui suffit à son usage : dire de quel état
# du source vient un binaire.
set(SUBEDIT_BUILD_DATE "")
# Qt est requis depuis la phase 5, et non plus optionnel : la fenêtre est le
# cœur de ce que le projet livre.
#
# Test accompagne Widgets : il porte les fonctions qui simulent un clic ou une
# frappe, dont le harnais d'interface se sert depuis Catch2 — sans être pour
# autant un binaire QTest.
#
# **Multimedia était déclaré ici, et ne l'est plus.** Il l'avait été « pour la
# prévisualisation de la phase 6 » ; l'ADR 0020 a tranché le backend vidéo en
# faveur de libmpv, et personne ne l'a jamais lié. Une dépendance que rien
# n'utilise n'est pas gratuite : elle est dans le cache de paquets de la CI, et
# la ligne qui la déclarait annonçait un usage qui n'existera pas.
find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

# libmpv — le moteur du lecteur intégré, voir
# docs/adr/0020-libmpv-pour-le-lecteur-integre.md.
#
# Par pkg-config et non par find_package : libmpv ne fournit pas de fichier de
# configuration CMake, seulement un `mpv.pc`. `IMPORTED_TARGET` en tire
# `PkgConfig::mpv`, qui porte ses chemins d'inclusion et ses drapeaux d'édition
# de liens comme le ferait une cible Qt — donc rien à recopier à la main dans
# les cibles qui s'en servent.
#
# Déclaré à la racine, comme Qt, pour que son absence se voie à la
# configuration : une bibliothèque manquante découverte au milieu d'une
# compilation coûte le temps de la compilation.
find_package(PkgConfig REQUIRED)
pkg_check_modules(mpv REQUIRED IMPORTED_TARGET mpv)

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1 --format=%cs
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE SUBEDIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE SUBEDIT_GIT_STATUS)
    if(SUBEDIT_GIT_STATUS EQUAL 0)
        set(SUBEDIT_BUILD_DATE "${SUBEDIT_COMMIT_DATE}")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ToolchainSanity)
include(Ccache)
include(Warnings)
include(Sanitizers)
include(Coverage)
include(Lto)

add_subdirectory(src/lib)
add_subdirectory(src/exe)

if(SUBEDIT_BUILD_TESTS)
    include(CTest)
    include(Dependencies)
    add_subdirectory(src/test)
endif()

message(STATUS "subedit ${PROJECT_VERSION} — ${CMAKE_BUILD_TYPE}")
message(STATUS "  tests       : ${SUBEDIT_BUILD_TESTS}")
message(STATUS "  sanitizers  : ${SUBEDIT_SANITIZERS}")
message(STATUS "  couverture  : ${SUBEDIT_COVERAGE}")
