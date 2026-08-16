# subedit_cli — ce qu'une ligne de commande fait, une fois ses arguments lus.
#
# Cette bibliothèque existe parce que la règle d'architecture du projet
# l'impose : aucune logique fonctionnelle dans src/exe. Rapporter ce qu'un
# fichier contient, décider d'un code de retour, doser ce qui se dit à quel
# niveau — rien de tout cela n'est du câblage, et rien de tout cela n'a sa
# place dans le noyau non plus : ce sont des décisions d'interface.
#
# **Elle ne connaît pas CLI11**, qui reste enfermée dans src/exe/cli/main.cpp.
# Ce qui est ici prend des chemins, des flux et un niveau — des valeurs, pas
# une ligne de commande. C'est ce qui la rend testable sans lancer de processus,
# et ce qui permettrait à un second appelant de s'en servir.

add_library(
    subedit_cli STATIC
    batch.cpp
    destination.cpp
    diagnostics.cpp
    digits.cpp
    frame_rate_conversion.cpp
    frame_rate_grammar.cpp
    inspection.cpp
    conversion.cpp
    index_grammar.cpp
    reporter.cpp
    rewriting.cpp
    hearing_impaired.cpp
    shifting.cpp
    time_grammar.cpp
    transforming.cpp
    verbosity.cpp
    wording.cpp)

add_library(subedit::cli ALIAS subedit_cli)

target_include_directories(subedit_cli PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_cli PUBLIC subedit::core)

subedit_set_warnings(subedit_cli)
