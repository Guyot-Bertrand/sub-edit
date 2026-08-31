# L'analyse statique, accrochée à la compilation de chaque source.
#
# **Ce module ne s'active que sous le preset `tidy`**, et il ne fait rien
# ailleurs : `SUBEDIT_TIDY` est `OFF` partout sinon, et la boucle de
# développement ne paie donc jamais les six secondes par fichier que clang-tidy
# coûte.
#
# ## Pourquoi la compilation, et pas un script qui choisit
#
# Le périmètre de l'analyse était calculé jusqu'ici par `tidy-scope.sh`, depuis
# un diff git : ce qui a changé depuis une base, plus la fermeture des en-têtes,
# plus une liste de « fichiers gouvernants » qui faisait tout reprendre.
# Il approchait, avec deux défauts que la mesure a montrés :
#
#   * la fermeture d'en-têtes se faisait par `grep` du **nom de fichier** dans
#     les sources — un en-tête mentionné dans un commentaire comptait ;
#   * un commentaire ajouté au `Makefile` faisait passer l'analyse de 2 fichiers
#     à 223, soit 72 s contre 827. Issue #269.
#
# Or clang-tidy ne lit jamais le `Makefile`. Ce dont dépend le résultat d'une
# unité de traduction est exactement : sa source, ses en-têtes transitifs, sa
# ligne de commande de compilation, la configuration `.clang-tidy`, et le
# binaire clang-tidy. Les trois premiers, **le système de construction les
# connaît déjà**, exactement et sans heuristique : Ninja rejoue une règle quand
# la source change, quand un en-tête de son fichier de dépendances change, ou
# quand la ligne de commande change.
#
# Reste à lui apprendre les deux derniers, ce que la clé ci-dessous fait.

option(SUBEDIT_TIDY "Analyse clang-tidy à la compilation de chaque source" OFF)

if(NOT SUBEDIT_TIDY)
    return()
endif()

# libstdc++ garde <expected> derrière __cpp_concepts >= 202002L, valeur que
# Clang 18 ne déclare pas : il ne voit alors pas std::expected. On prend donc la
# version la plus récente disponible.
find_program(
    SUBEDIT_CLANG_TIDY
    NAMES clang-tidy-20 clang-tidy-19 clang-tidy
    DOC "Binaire clang-tidy utilisé par le preset tidy")

if(NOT SUBEDIT_CLANG_TIDY)
    message(
        FATAL_ERROR
        "clang-tidy est introuvable, et le preset tidy n'a rien à faire sans lui.\n"
        "  l'installer avec : ./src/scripts/setup-toolchain.sh")
endif()

# ## Les deux entrées que Ninja ne voit pas, et la clé qui les lui montre
#
# Ninja rejoue une règle quand sa ligne de commande change. La configuration
# `.clang-tidy` n'y est pas — clang-tidy la lit lui-même, au vol —, et le
# binaire non plus, dont seul le chemin apparaît. Modifier l'une ou mettre à
# jour l'autre ne rejouerait donc **rien**, et l'analyse resterait verte sur une
# réponse périmée.
#
# **C'est le pire mode d'échec possible** : plus discret que celui qu'on
# remplace, puisque l'ancien se trompait en analysant trop. On le boucle en
# posant l'empreinte des deux dans les **drapeaux de compilation**, sous forme
# d'une définition dont personne ne se sert.
#
# Dans les drapeaux et non dans les arguments de clang-tidy, et la nuance
# décide : « la ligne de commande a changé, donc je rejoue » est une propriété
# de **Ninja**, qui en garde l'empreinte. Le générateur Makefile ne la connaît
# pas — mais il fait dépendre chaque objet de son `flags.make`, donc une
# définition de compilation qui change le recompile. Passer par les drapeaux
# rend la garantie vraie avec les deux, et le projet ne fige pas son générateur
# (voir `CMAKE_GENERATOR` dans le Makefile : Ninja est préféré, pas exigé).
execute_process(
    COMMAND "${SUBEDIT_CLANG_TIDY}" --version
    OUTPUT_VARIABLE subedit_tidy_version
    ERROR_VARIABLE subedit_tidy_version
    OUTPUT_STRIP_TRAILING_WHITESPACE)

file(READ "${CMAKE_SOURCE_DIR}/.clang-tidy" subedit_tidy_config)
string(SHA256 subedit_tidy_key "${subedit_tidy_version}${subedit_tidy_config}")
string(SUBSTRING "${subedit_tidy_key}" 0 16 subedit_tidy_key)

# Et pour que la clé soit recalculée quand le fichier bouge, la configuration
# doit se rejouer : sans cette ligne, CMake ne regarde jamais `.clang-tidy`.
set_property(
    DIRECTORY "${CMAKE_SOURCE_DIR}"
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.clang-tidy")

# `compile_commands.json` porte les drapeaux de GCC, dont certains n'existent
# pas chez Clang : sans -Wno-unknown-warning-option, clang-tidy échoue sur
# -Wuseless-cast et consorts avant d'avoir analysé la moindre ligne.
#
# `WarningsAsErrors: '*'` est dans `.clang-tidy` : une trouvaille fait donc
# échouer la compilation, et la porte garde sa dent.
#
# Pas de `PARENT_SCOPE` : `include()` ne crée pas de portée, donc ce qui est
# posé ici l'est dans le fichier qui inclut, et toutes les cibles ajoutées
# ensuite en héritent.
add_compile_definitions(__SUBEDIT_TIDY_KEY=${subedit_tidy_key})

set(CMAKE_CXX_CLANG_TIDY
    "${SUBEDIT_CLANG_TIDY}"
    "--quiet"
    "--extra-arg=-Wno-unknown-warning-option")

message(STATUS "clang-tidy   : ${SUBEDIT_CLANG_TIDY} (clé ${subedit_tidy_key})")
