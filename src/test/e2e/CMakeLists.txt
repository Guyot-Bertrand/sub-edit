# Tests de bout en bout.
#
# Ils ne lient pas la logique qu'ils vérifient : ils lancent le binaire réel et
# lisent ce qu'un utilisateur en verrait — sortie standard, sortie d'erreur,
# code de retour. L'arborescence miroite src/exe, comme celle de unit/ miroite
# src/lib.

add_executable(
    subedit_e2e_test
    cli/cli_run.cpp
    cli/config_home_test.cpp
    cli/usage_test.cpp
    cli/inspect_test.cpp
    cli/narration_test.cpp
    cli/batch_test.cpp
    cli/convert_test.cpp
    cli/hearing_impaired_test.cpp
    cli/shift_test.cpp
    cli/snap_test.cpp
    cli/transform_test.cpp
    cli/framerate_test.cpp
    gui/version_test.cpp)

# subedit::core pour versionString() : l'attendu d'un test se calcule, il ne se
# recopie pas — un numéro recopié est un numéro qui se périme.
target_link_libraries(subedit_e2e_test PRIVATE subedit::core Catch2::Catch2WithMain)

# Le chemin exact du binaire, résolu par CMake, comme SUBEDIT_TEST_DATA_DIR
# l'est pour le corpus : un test ne doit pas dépendre du répertoire d'où on le
# lance.
target_compile_definitions(
    subedit_e2e_test PRIVATE SUBEDIT_CLI_BINARY="$<TARGET_FILE:subedit-cli>"
                             SUBEDIT_GUI_BINARY="$<TARGET_FILE:subedit-gui>"
                             SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data")

# $<TARGET_FILE:…> donne un chemin, pas un ordre de construction. Sans cette
# ligne, rien ne garantit que le binaire existe quand le test le lance.
add_dependencies(subedit_e2e_test subedit-cli subedit-gui)

subedit_set_warnings(subedit_e2e_test)

# Le harnais ne s'enregistre dans CTest que sous les presets asan et release,
# via l'option de cache SUBEDIT_REGISTER_E2E (déclarée dans CMakeLists.txt) —
# OFF par défaut, mise à ON explicitement par CMakePresets.json dans ces deux
# presets et nulle part ailleurs.
#
#   - dev et asan sont tous deux des builds Debug ; asan est strictement plus
#     informatif (AddressSanitizer, UndefinedBehaviorSanitizer,
#     LeakSanitizer), donc enregistrer aussi sous dev n'apporterait rien —
#     une exécution de plus, sans nouveau signal.
#   - release est le mode qui expédie : c'est le binaire qu'un utilisateur
#     lance réellement.
#   - coverage reste exclu : sous ce preset, subedit-cli est lui aussi
#     instrumenté, et chaque invocation gonflerait la couverture de src/lib
#     sans qu'aucun test unitaire ait été écrit pour ce code, juste avant que
#     #50 en fasse un cliquet. Voir docs/adr/0014-registre-d-exigences.md.
#
# La cible se construit dans les quatre presets — ses warnings sont les nôtres
# partout — seule l'exécution est réservée à asan et release.
if(SUBEDIT_REGISTER_E2E)
    # Le défaut de CTest est de 1500 s : un binaire bloqué immobiliserait la CI
    # vingt-cinq minutes avant de dire pourquoi. L'étiquette LABELS e2e permet
    # à `make e2e` de filtrer ces tests par `ctest -L e2e`, sans dépendre de
    # leurs noms — un test unitaire nommé de façon proche ne le tromperait pas.
    # **QT_QPA_PLATFORM=offscreen, et il est indispensable.** Mesuré : dans un
    # environnement sans écran, la construction d'une QApplication avorte sur
    # « could not connect to display » — un échec d'exécution, que nulle
    # compilation ne peut annoncer. La CI n'a pas de serveur graphique.
    #
    # Il se règle ici, sur CTest, plutôt que dans chaque binaire : un réglage
    # posé par le code serait à reposer à chaque nouveau binaire, et son oubli
    # ne se verrait que sur la machine qui n'a pas d'écran — jamais sur celle
    # où on l'écrit.
    catch_discover_tests(
        subedit_e2e_test
        PROPERTIES TIMEOUT 30 LABELS e2e ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()
