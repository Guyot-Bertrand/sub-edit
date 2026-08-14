add_executable(subedit-cli main.cpp application.cpp)

# CLI11 est résolue par le paquet de la distribution, comme le veut l'ADR 0004,
# et n'entre que dans l'exécutable : le noyau reste sans dépendance — voir
# docs/adr/0016-cli11-pour-l-analyse-d-arguments.md.
find_package(CLI11 REQUIRED)

target_link_libraries(subedit-cli PRIVATE subedit::cli CLI11::CLI11)

subedit_set_warnings(subedit-cli)
