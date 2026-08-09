# Plafonne le parallélisme de l'optimisation entre modules.
#
# `CMAKE_INTERPROCEDURAL_OPTIMIZATION` traduit le LTO en `-flto=auto` chez GCC,
# c'est-à-dire « autant de processus que de cœurs », **à chaque édition de
# liens**. Ce parallélisme-là n'apparaît dans aucun `-j` : il échappe au réglage
# sobre du Makefile, qui vaut 1 par défaut, et sature une machine sur laquelle
# on fait autre chose — des tests d'un autre projet, par exemple, qui échouent
# alors sur des délais qu'ils n'auraient pas dû dépasser.
#
# Ce fichier donne donc au LTO le même bouton que le reste : bas par défaut,
# relevé explicitement par qui sait ce que sa machine peut prendre.
#
#   cmake --preset release -DSUBEDIT_LTO_JOBS=8
#
# Le drapeau est ajouté après celui que CMake pose ; chez GCC, le dernier
# `-flto=` l'emporte.

set(SUBEDIT_LTO_JOBS
    1
    CACHE STRING "Processus parallèles pour l'optimisation entre modules")

if(CMAKE_INTERPROCEDURAL_OPTIMIZATION AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_link_options("-flto=${SUBEDIT_LTO_JOBS}")
    message(STATUS "  LTO         : ${SUBEDIT_LTO_JOBS} processus")
endif()
