# subedit_cli — what a command line does, once its arguments are read.
#
# This library exists because the project's architecture rule demands it: no
# functional logic in src/exe. Reporting what a file contains, deciding on an
# exit code, measuring out what is said at which level — none of that is
# wiring, and none of that belongs in the core either: these are interface
# decisions.
#
# **It does not know CLI11**, which stays locked in src/exe/cli/main.cpp. What
# is here takes paths, streams and a level — values, not a command line. That
# is what makes it testable without starting a process, and what would let a
# second caller use it.

add_library(
    subedit_cli STATIC
    adjusting.cpp
    appending.cpp
    aligning.cpp
    batch.cpp
    changes.cpp
    json.cpp
    destination.cpp
    expansion.cpp
    encoding_grammar.cpp
    encoding_listing.cpp
    opening.cpp
    options.cpp
    diagnostics.cpp
    digits.cpp
    frame_rate_conversion.cpp
    frame_rate_grammar.cpp
    inspection.cpp
    pairing.cpp
    configuring.cpp
    conversion.cpp
    correcting.cpp
    index_grammar.cpp
    records.cpp
    replacing.cpp
    reporter.cpp
    rewriting.cpp
    writing.cpp
    hearing_impaired.cpp
    shared_settings.cpp
    shifting.cpp
    splitting.cpp
    sorting.cpp
    transcribing.cpp
    disjoining.cpp
    joining.cpp
    text_rewriting.cpp
    time_grammar.cpp
    transforming.cpp
    translation_writing.cpp
    verbosity.cpp
    version_report.cpp)

add_library(subedit::cli ALIAS subedit_cli)

target_include_directories(subedit_cli PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_cli PUBLIC subedit::core subedit::platform)

subedit_set_warnings(subedit_cli)
