# Instrumentation pour la mesure de couverture.
#
# Le rapport est produit par gcovr, piloté depuis le Makefile : CMake se
# contente d'instrumenter.

if(NOT SUBEDIT_COVERAGE)
    return()
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "La couverture n'est configurée que pour GCC.")
endif()

add_compile_options(--coverage -fprofile-abs-path -fno-inline -fno-elide-constructors)
add_link_options(--coverage)

message(STATUS "couverture instrumentée")
