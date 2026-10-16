# Executables.
#
# An executable comes down to a main, the parsing of its arguments, and calls
# into the libraries. No functional logic here: what deserves testing lives in
# src/lib.

add_subdirectory(cli)
add_subdirectory(gui)
