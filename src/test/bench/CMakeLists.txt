# Les benchmarks sont un exécutable distinct, non enregistré dans CTest : ils
# se mesurent en Release, pas pendant une passe de tests instrumentée.

add_executable(
    subedit_core_bench
    full_length_project.cpp
    core/edit/operations_bench.cpp
    core/format/subtitle_file_bench.cpp
    core/time/timestamp_bench.cpp
    core/version_bench.cpp)

target_link_libraries(subedit_core_bench PRIVATE subedit::core Catch2::Catch2WithMain)

# La fixture partagée vit à la racine de bench/, comme text_cases.hpp vit à
# celle de unit/. Sans ce chemin d'inclusion, un benchmark rangé dans un
# sous-répertoire la désignerait par une suite de « ../ » qui changerait à
# chaque déplacement de fichier.
target_include_directories(subedit_core_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

subedit_set_warnings(subedit_core_bench)
