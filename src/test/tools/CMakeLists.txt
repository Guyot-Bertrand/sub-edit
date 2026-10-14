# The programs the tests run, and that the project owns.
#
# They are not tests: they are the third parties a test needs to bring into
# existence. They live here rather than in src/exe/ because nothing that is
# delivered contains them — no install target names them.

add_executable(subedit_fake_player fake_player.cpp)
subedit_set_warnings(subedit_fake_player)

# The screenshots of the manual — #199.
#
# It is here for the same reason as the fake player: nothing that is delivered
# contains it. It is built by `make manual`, never by the gate, and no test
# registers it.
set(CMAKE_AUTOMOC ON)

add_executable(subedit_screenshots screenshots.cpp)

target_link_libraries(subedit_screenshots PRIVATE subedit::gui subedit::platform subedit::core Qt6::Widgets)

# The versioned corpus, and the repository's manual: the manual window is
# photographed on the real one, of which the installed copy is page for page.
target_compile_definitions(
    subedit_screenshots
    PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
            SUBEDIT_MANUAL_DIR="${CMAKE_SOURCE_DIR}/docs/manual")

subedit_set_warnings(subedit_screenshots)

# The window's shortcuts, written one per line — #612. Neither a test nor a deliverable: what
# `check-gui-manual.py` confronts the manual with, on the window's side.
add_executable(subedit_list_shortcuts list_shortcuts.cpp)

target_link_libraries(subedit_list_shortcuts PRIVATE subedit::gui subedit::platform subedit::core Qt6::Widgets)

# Both programs take `--language <name>` (#672) and read the catalogue the build tree lays down:
# building one of them alone builds the catalogues too.
add_dependencies(subedit_screenshots subedit_locale)
add_dependencies(subedit_list_shortcuts subedit_locale)

subedit_set_warnings(subedit_list_shortcuts)

# The encoding detection, on the command line — #296.
#
# Neither a test nor a deliverable: what lets `score-encoding-detection.py` put
# our detection to the same corpus as the other candidates, without the script
# knowing anything of it.
add_executable(subedit_detect_encoding detect_encoding.cpp)

target_link_libraries(subedit_detect_encoding PRIVATE subedit::core)

subedit_set_warnings(subedit_detect_encoding)

# The format detection, on the command line — #340.
#
# The counterpart of the previous one on the other unknown of phase 9: with nine
# candidates, `.sub` and `.txt` each name two formats, and a detection that is
# wrong is measured instead of believed.
add_executable(subedit_detect_format detect_format.cpp)

target_link_libraries(subedit_detect_format PRIVATE subedit::core)

subedit_set_warnings(subedit_detect_format)

# The pattern catalogue, as the command line will read it — #566.
#
# Nothing delivered reads the catalogue yet: this probe resolves the two
# directories **as the command line will** (from its executable and the
# environment, through `subedit::platform`, without Qt) and reads them. It is
# the debugging option the issue allows, outside the delivered binary.
add_executable(subedit_pattern_catalogue pattern_catalogue.cpp)

target_link_libraries(subedit_pattern_catalogue PRIVATE subedit::platform subedit::core)

subedit_set_warnings(subedit_pattern_catalogue)
