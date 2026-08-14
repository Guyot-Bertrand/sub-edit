# AddressSanitizer et UndefinedBehaviorSanitizer.
#
# ASan embarque LeakSanitizer sous Linux : une fuite fait échouer les tests au
# lieu d'être découverte des mois plus tard. C'est ce qui rend mécaniques les
# règles de propriété mémoire de docs/principes-de-conception.md.
#
# **UBSan ne s'arrête pas de lui-même**, contrairement à ASan : par défaut il
# écrit son diagnostic et laisse le programme continuer. Un test qui déclenche
# un comportement indéfini était donc rapporté comme passé, et la porte restait
# verte avec le message sous les yeux de personne — la sortie de CTest n'est
# lue que lorsqu'elle échoue.
#
# Ce n'était pas une hypothèse : la grammaire du temps a débordé en signé sur
# `shift --by 99999999999999999999` pendant toute la phase 3, puis écrit un
# fichier décalé de deux cents mille milliards de secondes avec un code de
# retour 0. `-fno-sanitize-recover=undefined` fait de ce diagnostic un échec.
# `make verify-gates` en porte la preuve, distincte de celle d'ASan.

if(NOT SUBEDIT_SANITIZERS)
    return()
endif()

set(SUBEDIT_SANITIZER_FLAGS
    -fsanitize=address
    -fsanitize=undefined
    -fno-sanitize-recover=undefined
    -fno-omit-frame-pointer      # traces d'appel exploitables
    -fno-optimize-sibling-calls) # sinon les cadres d'appel disparaissent

add_compile_options(${SUBEDIT_SANITIZER_FLAGS})
add_link_options(${SUBEDIT_SANITIZER_FLAGS})

message(STATUS "sanitizers actifs : address, undefined, leak")
