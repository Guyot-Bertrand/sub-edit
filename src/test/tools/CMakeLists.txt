# Les programmes que les tests lancent, et que le projet possède.
#
# Ce ne sont pas des tests : ce sont les tiers qu'un test a besoin de faire
# exister. Ils vivent ici plutôt que dans src/exe/ parce que rien de ce qui est
# livré ne les contient — aucune cible d'installation ne les nomme.

add_executable(subedit_fake_player fake_player.cpp)
subedit_set_warnings(subedit_fake_player)

# Les captures d'écran du manuel — #199.
#
# Elle est ici pour la même raison que le lecteur de pacotille : rien de ce qui
# est livré ne la contient. Elle est bâtie par `make manual`, jamais par la
# porte, et elle n'est enregistrée dans aucun test.
set(CMAKE_AUTOMOC ON)

add_executable(subedit_screenshots screenshots.cpp)

target_link_libraries(subedit_screenshots PRIVATE subedit::gui subedit::core Qt6::Widgets)

# Le corpus versionné, et le manuel du dépôt : la fenêtre du manuel se
# photographie sur le vrai, dont l'installé est la copie page pour page.
target_compile_definitions(
    subedit_screenshots
    PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
            SUBEDIT_MANUAL_DIR="${CMAKE_SOURCE_DIR}/docs/manual")

subedit_set_warnings(subedit_screenshots)

# La détection d'encodage, sur la ligne de commande — #296.
#
# Ni un test ni un livrable : ce qui permet à `score-encoding-detection.py` de
# passer notre détection au même corpus que les autres candidates, sans que le
# script sache rien d'elle.
add_executable(subedit_detect_encoding detect_encoding.cpp)

target_link_libraries(subedit_detect_encoding PRIVATE subedit::core)

subedit_set_warnings(subedit_detect_encoding)
