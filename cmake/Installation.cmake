# Ce que `cmake --install` dépose, et où.
#
# **Rien de tout cela n'est vérifiable depuis l'arbre de construction.** Un
# fichier de données que le binaire cherche à côté de lui y est présent par
# accident de disposition : il est là parce que le dépôt le contient, pas parce
# qu'une règle l'a copié. Le défaut ne se voit qu'à la première installation
# propre, et c'est `check-installation.sh` qui la fait — issue #239.
#
# Les chemins viennent de `GNUInstallDirs` et jamais d'une chaîne écrite à la
# main : `lib` ou `lib64`, `share` ou autre chose, c'est la distribution qui
# décide, et un empaqueteur qui repositionne `CMAKE_INSTALL_PREFIX` attend que
# tout suive.
#
# **Les six lignes de l'ADR 0023 y sont, depuis #244.** Les deux premières —
# les binaires et le manuel — ont été posées par #239, parce qu'un contrôle
# d'installation n'avait rien à prouver sans elles ; les quatre autres sont
# arrivées avec les fichiers qu'elles déposent, qui n'existaient pas non plus.
#
# **Les noms des trois fichiers de bureau sont l'identifiant de l'application**,
# `io.github.guyot_bertrand.subedit`, et non `subedit`. C'est ce que les bureaux
# attendent : `appstreamcli` refuse un fichier de métadonnées dont le nom ne
# reprend pas l'identifiant, et une icône trouvée par le thème est une icône dont
# le nom est celui que la clé `Icon=` écrit.

include(GNUInstallDirs)

install(TARGETS subedit-cli subedit-gui RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# Le manuel entier, sous-répertoires et captures compris.
#
# **Un répertoire et non une liste de fichiers**, et c'est une décision : une
# liste se périme au premier chapitre ajouté, en silence, et le contrôle
# d'installation ne verrait le manque que si quelqu'un pensait à mettre la
# nouvelle page dans la liste — c'est-à-dire jamais. `DIRECTORY` rend le défaut
# structurellement impossible pour ce qui vit sous `docs/manual`.
#
# `Help ▸ Manual` ouvrira ce qui est déposé ici — issue #245.
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/manual/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/manual")

# L'identifiant de l'application, écrit une fois. Les trois fichiers de bureau
# le portent dans leur nom, et le `.desktop` le porte encore dans sa clé
# `Icon=` : quatre endroits pour un seul nom, donc un seul endroit où le dire.
set(SUBEDIT_APP_ID "io.github.guyot_bertrand.subedit")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/${SUBEDIT_APP_ID}.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/${SUBEDIT_APP_ID}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/${SUBEDIT_APP_ID}.metainfo.xml"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")

# **La page de manuel est engendrée au moment de l'installation, et non à celui
# de la configuration.** Deux valeurs y sont injectées : le numéro de version,
# qui serait faux au premier incrément s'il était recopié dans la source, et le
# répertoire du manuel complet, que la page nomme en toutes lettres.
#
# Le second est la raison du `install(CODE)`. Le préfixe n'est connu qu'à
# l'installation — CPack pose `/usr`, `make install-check` un répertoire
# temporaire, un utilisateur ce qu'il veut. Configurée trop tôt, la page
# annonçait `/usr/local` dans un paquet qui installe sous `/usr` : mesuré sur le
# premier `.deb` produit, et c'est exactement le genre de mensonge qu'un fichier
# engendré est censé rendre impossible.
#
# **Elle est aussi compressée, et ce n'est pas une coquetterie** : la charte
# Debian demande une page de manuel compressée, `rpmbuild` compresse la sienne
# tout seul, et une page nue dans le `.deb` aurait donc été à la fois un
# manquement à la charte et un écart entre les deux paquets — écart que
# `check-installation.sh` refuse. `-n` pour que l'archive ne porte ni horodatage
# ni nom d'origine : sans lui, deux constructions du même fichier donneraient
# deux octets différents.
install(
    CODE "
        set(PROJECT_VERSION \"${PROJECT_VERSION}\")
        set(SUBEDIT_MANUAL_DIR \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/subedit/manual\")
        configure_file(
            \"${CMAKE_CURRENT_SOURCE_DIR}/packaging/subedit-cli.1.in\"
            \"${CMAKE_CURRENT_BINARY_DIR}/subedit-cli.1\"
            @ONLY)
        execute_process(
            COMMAND gzip -9 -n -f \"${CMAKE_CURRENT_BINARY_DIR}/subedit-cli.1\"
            COMMAND_ERROR_IS_FATAL ANY)")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/subedit-cli.1.gz"
        DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
