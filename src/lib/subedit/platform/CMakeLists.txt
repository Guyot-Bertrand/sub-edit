# subedit_platform — où l'exécutable et l'environnement disent que les choses sont.
#
# Ni le noyau, qui ne lit aucune variable et ne demande à aucun exécutable où il
# vit (ADR 0037), ni une interface : ce qui répond une fois, sans Qt, pour la
# ligne de commande comme pour la fenêtre. Issue #566.

add_library(subedit_platform STATIC locations.cpp)
add_library(subedit::platform ALIAS subedit_platform)

target_include_directories(subedit_platform PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_platform PUBLIC subedit::core)

subedit_set_warnings(subedit_platform)
