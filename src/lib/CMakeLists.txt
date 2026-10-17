# The project's libraries.
#
# Each direct child of src/lib/subedit/ is a target. All the useful code lives
# here: the executables of src/exe only do wiring.

add_subdirectory(subedit/core)
add_subdirectory(subedit/platform)
add_subdirectory(subedit/cli)
add_subdirectory(subedit/gui)
