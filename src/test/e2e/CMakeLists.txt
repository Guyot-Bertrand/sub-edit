# Tests de bout en bout.
#
# Ils ne lient pas la logique qu'ils vérifient : ils lancent le binaire réel et
# lisent ce qu'un utilisateur en verrait — sortie standard, sortie d'erreur,
# code de retour. L'arborescence miroite src/exe, comme celle de unit/ miroite
# src/lib.

add_executable(subedit_e2e_test cli/cli_run.cpp cli/version_test.cpp)

# subedit::core pour versionString() : l'attendu d'un test se calcule, il ne se
# recopie pas — un numéro recopié est un numéro qui se périme.
target_link_libraries(subedit_e2e_test PRIVATE subedit::core Catch2::Catch2WithMain)

# Le chemin exact du binaire, résolu par CMake, comme SUBEDIT_TEST_DATA_DIR
# l'est pour le corpus : un test ne doit pas dépendre du répertoire d'où on le
# lance.
target_compile_definitions(subedit_e2e_test
                           PRIVATE SUBEDIT_CLI_BINARY="$<TARGET_FILE:subedit-cli>")

# $<TARGET_FILE:…> donne un chemin, pas un ordre de construction. Sans cette
# ligne, rien ne garantit que le binaire existe quand le test le lance.
add_dependencies(subedit_e2e_test subedit-cli)

subedit_set_warnings(subedit_e2e_test)

# Sous le preset coverage, subedit-cli est lui aussi instrumenté : chaque
# invocation gonflerait la couverture de src/lib sans qu'aucun test unitaire
# ait été écrit pour ce code, juste avant que #50 en fasse un cliquet. La cible
# se construit dans les quatre presets — ses warnings sont les nôtres partout —
# mais ne s'exécute pas sous celui-là. Voir docs/adr/0014-registre-d-exigences.md.
if(NOT SUBEDIT_COVERAGE)
    # Le défaut de CTest est de 1500 s : un binaire bloqué immobiliserait la CI
    # vingt-cinq minutes avant de dire pourquoi.
    catch_discover_tests(subedit_e2e_test PROPERTIES TIMEOUT 30)
endif()
