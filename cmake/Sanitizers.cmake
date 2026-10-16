# AddressSanitizer and UndefinedBehaviorSanitizer.
#
# ASan embeds LeakSanitizer on Linux: a leak makes the tests fail instead of
# being discovered months later. That is what makes the memory ownership rules
# of docs/principes-de-conception.md mechanical.
#
# **UBSan does not stop by itself**, unlike ASan: by default it writes its
# diagnostic and lets the program go on. A test that triggered undefined
# behaviour was therefore reported as passed, and the gate stayed green with
# the message in front of nobody's eyes — CTest's output is read only when it
# fails.
#
# This was not a hypothesis: the time grammar overflowed as signed on
# `shift --by 99999999999999999999` throughout phase 3, then wrote a file
# shifted by two hundred thousand billion seconds with an exit code of 0.
# `-fno-sanitize-recover=undefined` makes that diagnostic a failure.
# `make verify-gates` carries the proof of it, distinct from ASan's.

if(NOT SUBEDIT_SANITIZERS)
    return()
endif()

set(SUBEDIT_SANITIZER_FLAGS
    -fsanitize=address
    -fsanitize=undefined
    -fno-sanitize-recover=undefined
    -fno-omit-frame-pointer      # usable call stacks
    -fno-optimize-sibling-calls) # otherwise call frames disappear

add_compile_options(${SUBEDIT_SANITIZER_FLAGS})
add_link_options(${SUBEDIT_SANITIZER_FLAGS})

message(STATUS "sanitizers enabled: address, undefined, leak")
