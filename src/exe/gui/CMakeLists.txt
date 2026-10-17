# The window — wiring only, everything else is in subedit::gui.
#
# AUTOMOC is on from now: switching it on later, when a first Q_OBJECT
# arrives, would make linking fail on a missing symbol with nothing to say it
# comes from a build setting.
set(CMAKE_AUTOMOC ON)

add_executable(subedit-gui main.cpp)

target_link_libraries(subedit-gui PRIVATE subedit::gui subedit::core Qt6::Widgets)

subedit_set_warnings(subedit-gui)
