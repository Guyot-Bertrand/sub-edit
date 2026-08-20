# Le harnais de test d'interface.
#
# Catch2 comme partout ailleurs, et de Qt seulement ses fonctions de pilotage :
# c'est ce qui garde les tags d'exigences visibles par
# check-requirements.sh, qui interroge des binaires Catch2.
#
# Qt6::Test est lié pour QTest::keyClicks et QSignalSpy, sans que le binaire
# soit un binaire QTest : ni QTEST_MAIN, ni slots privés, ni moc.
set(CMAKE_AUTOMOC ON)

add_executable(
    subedit_gui_test window_harness_test.cpp subtitle_table_model_test.cpp
    cell_delegates_test.cpp main_window_test.cpp)

target_link_libraries(
    subedit_gui_test PRIVATE subedit::gui subedit::core Qt6::Widgets Qt6::Test Catch2::Catch2)

subedit_set_warnings(subedit_gui_test)

# Catch2 sans son main : le binaire fournit le sien, parce que la QApplication
# doit vivre sur la pile de main. Détruite parmi les gestionnaires de fin de
# processus — ce que fait une statique — elle plante, et ASan l'a montré.
#
# Le réglage sans écran est posé par le binaire lui-même, et non ici : il est
# aussi lancé hors de CTest, par check-requirements.sh qui lui demande ses tags.
catch_discover_tests(subedit_gui_test PROPERTIES LABELS gui)
