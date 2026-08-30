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
    cell_delegates_test.cpp command_label_test.cpp
    window_files_test.cpp qt_prompts_test.cpp target_test.cpp
    operation_dialogs_test.cpp window_operations_test.cpp
    window_hearing_impaired_test.cpp window_insert_test.cpp anomaly_marking_test.cpp
    manual_window_test.cpp
    main_window_test.cpp mpv_player_test.cpp window_video_test.cpp
    window_player_test.cpp player_factory_test.cpp invocation_test.cpp
    window_video_bounds_test.cpp window_grid_test.cpp
    window_appearance_test.cpp settings_path_test.cpp window_settings_test.cpp theme_test.cpp
    ${CMAKE_SOURCE_DIR}/src/test/unit/grid_fixtures.cpp)

target_link_libraries(
    subedit_gui_test PRIVATE subedit::gui subedit::core Qt6::Widgets Qt6::Test Catch2::Catch2)

subedit_set_warnings(subedit_gui_test)

# Le corpus versionné, pour le seul test qui ouvre un vrai fichier à travers un
# vrai système de fichiers. Les autres passent par InMemoryFileSystem, ce qui
# est le bon choix pour éprouver une fenêtre — mais laissait `openProject` sans
# preuve sur un fichier qui existe.
target_compile_definitions(subedit_gui_test
                           PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data")

# Les fixtures de grille, partagées avec les tests du noyau et ceux de la ligne
# de commande : il n'y a qu'un jeu de fichiers, et qu'une façon de l'ouvrir.
target_include_directories(subedit_gui_test PRIVATE ${CMAKE_SOURCE_DIR}/src/test/unit)

# Catch2 sans son main : le binaire fournit le sien, parce que la QApplication
# doit vivre sur la pile de main. Détruite parmi les gestionnaires de fin de
# processus — ce que fait une statique — elle plante, et ASan l'a montré.
#
# Le réglage sans écran est posé par le binaire lui-même, et non ici : il est
# aussi lancé hors de CTest, par check-requirements.sh qui lui demande ses tags.
catch_discover_tests(subedit_gui_test PROPERTIES LABELS gui)
