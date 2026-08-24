# Les benchmarks sont un exécutable distinct, non enregistré dans CTest : ils
# se mesurent en Release, pas pendant une passe de tests instrumentée.
#
# **Un seul binaire, noyau et interface confondus.** `record-bench.sh` ne lit
# qu'une sortie XML, et deux binaires demanderaient soit de la fusionner, soit
# deux relevés au journal pour une même version. Qt est de toute façon requis
# pour construire le projet.
set(CMAKE_AUTOMOC ON)

add_executable(
    subedit_bench
    full_length_project.cpp
    core/edit/operations_bench.cpp
    core/format/subtitle_file_bench.cpp
    core/time/timestamp_bench.cpp
    core/version_bench.cpp
    gui/subtitle_table_model_bench.cpp
    gui/video_bench.cpp)

target_link_libraries(
    subedit_bench PRIVATE subedit::gui subedit::core Qt6::Widgets Catch2::Catch2WithMain)

# La fixture partagée vit à la racine de bench/, comme text_cases.hpp vit à
# celle de unit/. Sans ce chemin d'inclusion, un benchmark rangé dans un
# sous-répertoire la désignerait par une suite de « ../ » qui changerait à
# chaque déplacement de fichier.
target_include_directories(subedit_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Les fixtures vidéo de #163, pour la seule mesure qui ouvre un vrai fichier :
# ce que coûtent l'ouverture et la recherche, c'est-à-dire ce qui se passe à
# chaque changement de ligne.
target_compile_definitions(subedit_bench
                           PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data")

subedit_set_warnings(subedit_bench)
