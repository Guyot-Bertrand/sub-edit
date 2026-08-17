# La fenêtre — pour l'instant, la graine qui prouve que Qt est là.
#
# AUTOMOC est actif dès maintenant : l'allumer plus tard, quand un premier
# Q_OBJECT arrivera, ferait échouer l'édition de liens sur un symbole manquant
# dont rien ne dit qu'il vient d'un réglage de construction.
set(CMAKE_AUTOMOC ON)

add_executable(subedit-gui main.cpp)

target_link_libraries(subedit-gui PRIVATE subedit::core Qt6::Widgets)

subedit_set_warnings(subedit-gui)
