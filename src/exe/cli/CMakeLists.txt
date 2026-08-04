add_executable(subedit-cli main.cpp)

target_link_libraries(subedit-cli PRIVATE subedit::core)

subedit_set_warnings(subedit-cli)
