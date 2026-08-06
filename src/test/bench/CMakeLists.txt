# Les benchmarks sont un exécutable distinct, non enregistré dans CTest : ils
# se mesurent en Release, pas pendant une passe de tests instrumentée.

add_executable(subedit_core_bench core/time/timestamp_bench.cpp core/version_bench.cpp)

target_link_libraries(subedit_core_bench PRIVATE subedit::core Catch2::Catch2WithMain)

subedit_set_warnings(subedit_core_bench)
