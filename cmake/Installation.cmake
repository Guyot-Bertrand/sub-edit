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
# **Ce qui manque encore est nommé plutôt que passé sous silence** : l'icône, le
# fichier `.desktop`, les métadonnées AppStream et la page de manuel de
# `subedit-cli` n'existent pas dans le dépôt. Ils viennent avec l'empaquetage,
# issue #244, et leurs règles s'ajoutent ici.

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
