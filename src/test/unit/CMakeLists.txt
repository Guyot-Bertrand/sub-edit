# Un binaire de test par bibliothèque, chaque cas étant enregistré
# individuellement dans CTest par catch_discover_tests.

add_executable(subedit_core_test core/version_test.cpp)

target_link_libraries(subedit_core_test PRIVATE subedit::core Catch2::Catch2WithMain)

subedit_set_warnings(subedit_core_test)

catch_discover_tests(subedit_core_test)
