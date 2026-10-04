add_executable(subedit-cli
    main.cpp
    application.cpp
    commands/adjust.cpp
    commands/command.cpp
    commands/convert.cpp
    commands/dialogue_dashes.cpp
    commands/framerate.cpp
    commands/hearing_impaired.cpp
    commands/inspect.cpp
    commands/italics.cpp
    commands/letter_case.cpp
    commands/replace.cpp
    commands/shift.cpp
    commands/snap.cpp
    commands/sort.cpp
    commands/table.cpp
    commands/transform.cpp)

# CLI11 est résolue par le paquet de la distribution, comme le veut l'ADR 0004,
# et n'entre que dans l'exécutable : le noyau reste sans dépendance — voir
# docs/adr/0016-cli11-pour-l-analyse-d-arguments.md.
find_package(CLI11 REQUIRED)

target_link_libraries(subedit-cli PRIVATE subedit::cli CLI11::CLI11)

subedit_set_warnings(subedit-cli)
