# Jeu de warnings appliqué à toutes les cibles du projet.
#
# Les warnings sont des erreurs : un warning toléré devient un warning ignoré,
# puis un défaut. Voir docs/specs/00-fondations.md.

set(SUBEDIT_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion            # conversions implicites qui perdent de l'information
    -Wsign-conversion
    -Wshadow                # une variable qui en masque une autre est un piège
    -Wnon-virtual-dtor      # suppression polymorphique via un destructeur non virtuel
    -Wold-style-cast        # les casts C contournent le système de types
    -Wcast-align
    -Woverloaded-virtual    # surcharge qui masque une virtuelle au lieu de la redéfinir
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    -Wunused)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND SUBEDIT_WARNINGS -Wduplicated-cond -Wduplicated-branches -Wlogical-op
                                 -Wuseless-cast)
endif()

# Applique le jeu de warnings à une cible, en erreurs.
function(subedit_set_warnings target)
    target_compile_options(${target} PRIVATE ${SUBEDIT_WARNINGS} -Werror)
endfunction()

# Variante sans -Werror, réservée au code que nous ne maîtrisons pas.
function(subedit_set_warnings_lenient target)
    target_compile_options(${target} PRIVATE ${SUBEDIT_WARNINGS})
endfunction()
