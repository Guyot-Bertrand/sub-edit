# AddressSanitizer et UndefinedBehaviorSanitizer.
#
# ASan embarque LeakSanitizer sous Linux : une fuite fait échouer les tests au
# lieu d'être découverte des mois plus tard. C'est ce qui rend mécaniques les
# règles de propriété mémoire de docs/principes-de-conception.md.

if(NOT SUBEDIT_SANITIZERS)
    return()
endif()

set(SUBEDIT_SANITIZER_FLAGS
    -fsanitize=address
    -fsanitize=undefined
    -fno-omit-frame-pointer      # traces d'appel exploitables
    -fno-optimize-sibling-calls) # sinon les cadres d'appel disparaissent

add_compile_options(${SUBEDIT_SANITIZER_FLAGS})
add_link_options(${SUBEDIT_SANITIZER_FLAGS})

message(STATUS "sanitizers actifs : address, undefined, leak")
