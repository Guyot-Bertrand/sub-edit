# subedit_gui — la fenêtre, ses modèles et ses dialogues.
#
# Contrainte structurante, symétrique de celle du noyau : tout ce qui n'est pas
# du câblage vit ici et non dans src/exe, parce que check-architecture.sh refuse
# un main qui définit une classe. C'est ce qui rend l'interface testable.
set(CMAKE_AUTOMOC ON)

add_library(
    subedit_gui STATIC subtitle_table_model.cpp cell_delegates.cpp command_label.cpp
                       invocation.cpp opening.cpp saving.cpp
                       diagnostics_panel.cpp qt_prompts.cpp target.cpp
                       operation_dialog.cpp shift_dialog.cpp transform_dialog.cpp
                       frame_rate_dialog.cpp hearing_impaired_dialog.cpp
                       grid_analysis_dialog.cpp snap_dialog.cpp
                       mpv_player.cpp player_factory.cpp main_window.cpp)
add_library(subedit::gui ALIAS subedit_gui)

target_include_directories(subedit_gui PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_gui PUBLIC subedit::core Qt6::Widgets)

# libmpv, derrière la couture `core::VideoPlayer` — voir docs/adr/0020.
#
# **L'implémentation est ici et non au noyau**, alors que ni elle ni libmpv ne
# connaissent Qt. Le lecteur est un élément d'interface : il n'existe que pour
# la fenêtre, il en reçoit la sienne par la propriété `wid`, et rien d'autre ne
# l'appellera. Faire entrer libmpv dans `subedit_core` aurait donné au domaine
# une dépendance d'infrastructure pour la seule commodité d'un harnais de test
# plus léger — ce qui n'est pas une raison d'architecture.
#
# Ce qui reste au noyau est l'interface, et elle seule : `core::VideoPlayer` ne
# nomme ni libmpv ni Qt, et c'est ce qui permet au reste du noyau de raisonner
# sur un lecteur sans en connaître un.
#
# PRIVATE : `mpv_player.hpp` ne fait que déclarer `mpv_handle`, si bien qu'aucun
# fichier qui inclut la bibliothèque n'a besoin des en-têtes de libmpv.
target_link_libraries(subedit_gui PRIVATE PkgConfig::mpv)

subedit_set_warnings(subedit_gui)
