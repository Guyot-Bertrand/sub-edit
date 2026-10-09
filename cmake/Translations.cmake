# Les catalogues de messages : `.po` versionné → `.mo` à la construction → installation.
#
# **Ce module compile et installe ; il n'extrait ni ne fusionne.** `make pot` et `make po`
# (`src/scripts/update-pot.sh`, `update-po.sh`) écrivent des fichiers du dépôt, ce qu'une
# construction ne fait jamais — un arbre de construction ne modifie pas les sources. Le
# gabarit est tenu à jour par une étape de `check-local`.
#
# **Les langues installées sont celles de `src/po/LINGUAS`**, une par ligne. Un `.po` qui n'y est
# pas — les graines importées de Gaupol, issue #671 — reste dans le dépôt sans être compilé ni
# livré : une langue entre dans les paquets quand elle est complète et relue (spec 15, D5).
#
# **L'arbre de construction reproduit l'installation**, comme pour les motifs (#566) : les `.mo`
# sont écrits sous `<construction>/share/subedit/locale/<langue>/LC_MESSAGES/subedit.mo`, là où
# un binaire de `<construction>/bin` les trouve à `../share/subedit/locale`. Le chemin installé
# est le même, sous `<préfixe>/share/subedit/locale`.
#
# `msgfmt` est un outil de construction, jamais d'exécution : le lecteur de `core/i18n/` lit le
# `.mo` lui-même (ADR 0042), et rien de gettext n'est requis chez l'utilisateur.

include(GNUInstallDirs)

find_program(SUBEDIT_MSGFMT msgfmt)
if(NOT SUBEDIT_MSGFMT)
    message(
        FATAL_ERROR
        "msgfmt est introuvable, et la construction compile les catalogues de messages.\n"
        "  l'installer avec : ./src/scripts/setup-toolchain.sh")
endif()

set(SUBEDIT_PO_DIR "${CMAKE_SOURCE_DIR}/src/po")
set(SUBEDIT_LOCALE_BUILD_DIR "${CMAKE_BINARY_DIR}/share/subedit/locale")

# Le fichier des langues est une entrée de la configuration : en ajouter une doit reconfigurer.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SUBEDIT_PO_DIR}/LINGUAS")
file(STRINGS "${SUBEDIT_PO_DIR}/LINGUAS" subedit_linguas)

set(subedit_catalogues "")
foreach(language IN LISTS subedit_linguas)
    string(STRIP "${language}" language)
    if(language STREQUAL "" OR language MATCHES "^#")
        continue()
    endif()

    set(po "${SUBEDIT_PO_DIR}/${language}.po")
    if(NOT EXISTS "${po}")
        message(FATAL_ERROR "src/po/LINGUAS nomme « ${language} », et ${po} n'existe pas")
    endif()

    set(mo "${SUBEDIT_LOCALE_BUILD_DIR}/${language}/LC_MESSAGES/subedit.mo")
    # `--check` : l'en-tête est complet, les formats des messages concordent avec ceux de l'original.
    add_custom_command(
        OUTPUT "${mo}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SUBEDIT_LOCALE_BUILD_DIR}/${language}/LC_MESSAGES"
        COMMAND "${SUBEDIT_MSGFMT}" --check --output-file "${mo}" "${po}"
        DEPENDS "${po}"
        COMMENT "msgfmt ${language}.po"
        VERBATIM)
    list(APPEND subedit_catalogues "${mo}")

    install(FILES "${mo}"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/locale/${language}/LC_MESSAGES")
endforeach()

# Dans `all` : un `.mo` qu'aucune cible ne demande ne serait pas construit, donc pas installé.
add_custom_target(subedit_locale ALL DEPENDS ${subedit_catalogues})

# **Les deux exécutables en dépendent**, et c'est ce qui fait tenir la phrase précédente : les
# portes ne construisent que les cibles qu'elles nomment, pas `all`. Sans cette dépendance, un
# binaire construit seul n'aurait aucun catalogue à côté de lui, et `cmake --install` échouerait
# sur un fichier absent — ce que `check-installation.sh` a montré au premier essai.
add_dependencies(subedit-cli subedit_locale)
add_dependencies(subedit-gui subedit_locale)
