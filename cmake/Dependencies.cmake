# Dépendances externes.
#
# Règle du projet : `find_package` sur les paquets système, afin qu'adopter
# Conan ou vcpkg plus tard reste presque gratuit — tous deux fonctionnent en
# s'interposant sur ce mécanisme. Voir docs/adr/0004-gestion-des-dependances.md.
#
# Catch2 fait exception : il se compile en quelques secondes et sa version doit
# être identique partout, sous peine qu'une mise à jour de distribution casse
# les tests.

include(FetchContent)

set(CATCH2_VERSION "v3.5.2")

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG ${CATCH2_VERSION}
    GIT_SHALLOW TRUE
    SYSTEM) # en-têtes traités comme système : leurs warnings ne sont pas les nôtres

FetchContent_MakeAvailable(Catch2)

# **Catch2 n'est pas notre code, et le preset `tidy` ne le lint pas.**
# `CMAKE_CXX_CLANG_TIDY` est une variable de répertoire : toute cible ajoutée
# après elle en hérite, y compris celles qu'une dépendance ajoute pour son
# compte. Sans cette boucle, l'analyse porterait sur un sur-ensemble de `src/`
# et signalerait des défauts que personne ici ne peut corriger.
#
# Le `SYSTEM` de la déclaration ci-dessus ne suffit pas : il traite les en-têtes
# de Catch2 comme systèmes pour *nos* unités de traduction, il ne dit rien des
# siennes propres.
foreach(subedit_foreign IN ITEMS Catch2 Catch2WithMain)
    if(TARGET ${subedit_foreign})
        set_target_properties(${subedit_foreign} PROPERTIES CXX_CLANG_TIDY "")
    endif()
endforeach()

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(Catch)
