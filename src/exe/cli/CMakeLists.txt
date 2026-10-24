add_executable(subedit-cli
    main.cpp
    application.cpp
    commands/adjust.cpp
    commands/append.cpp
    commands/command.cpp
    commands/config.cpp
    commands/convert.cpp
    commands/correct.cpp
    commands/dialogue_dashes.cpp
    commands/disjoint.cpp
    commands/framerate.cpp
    commands/hearing_impaired.cpp
    commands/inspect.cpp
    commands/italics.cpp
    commands/join_repeats.cpp
    commands/letter_case.cpp
    commands/pair.cpp
    commands/repair.cpp
    commands/replace.cpp
    commands/shift.cpp
    commands/snap.cpp
    commands/sort.cpp
    commands/transcribe.cpp
    commands/split_file.cpp
    commands/table.cpp
    commands/transform.cpp)

# CLI11 is resolved through the distribution's package, as ADR 0004 wants,
# and enters only the executable: the core stays free of dependencies — see
# `docs/adr/0016-cli11-pour-l-analyse-d-arguments.md`.
find_package(CLI11 REQUIRED)

target_link_libraries(subedit-cli PRIVATE subedit::cli CLI11::CLI11)

subedit_set_warnings(subedit-cli)
