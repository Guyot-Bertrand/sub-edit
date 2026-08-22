# subedit_gui — la fenêtre, ses modèles et ses dialogues.
#
# Contrainte structurante, symétrique de celle du noyau : tout ce qui n'est pas
# du câblage vit ici et non dans src/exe, parce que check-architecture.sh refuse
# un main qui définit une classe. C'est ce qui rend l'interface testable.
set(CMAKE_AUTOMOC ON)

add_library(
    subedit_gui STATIC subtitle_table_model.cpp cell_delegates.cpp command_label.cpp
                       opening.cpp saving.cpp
                       diagnostics_panel.cpp qt_prompts.cpp target.cpp
                       operation_dialog.cpp shift_dialog.cpp transform_dialog.cpp
                       frame_rate_dialog.cpp hearing_impaired_dialog.cpp
                       main_window.cpp)
add_library(subedit::gui ALIAS subedit_gui)

target_include_directories(subedit_gui PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_gui PUBLIC subedit::core Qt6::Widgets)

subedit_set_warnings(subedit_gui)
