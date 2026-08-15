cmake_minimum_required(VERSION 3.28)

project(
    subedit
    VERSION 0.3.3
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
