# The set of warnings applied to every target of the project.
#
# Warnings are errors: a tolerated warning becomes an ignored warning, then a
# defect. See docs/specs/00-fondations.md.

set(SUBEDIT_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion            # implicit conversions that lose information
    -Wsign-conversion
    -Wshadow                # a variable that hides another is a trap
    -Wnon-virtual-dtor      # polymorphic deletion through a non-virtual destructor
    -Wold-style-cast        # C casts bypass the type system
    -Wcast-align
    -Woverloaded-virtual    # an overload that hides a virtual instead of overriding it
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    -Wunused)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND SUBEDIT_WARNINGS -Wduplicated-cond -Wduplicated-branches -Wlogical-op
                                 -Wuseless-cast)
endif()

# Applies the set of warnings to a target, as errors.
function(subedit_set_warnings target)
    target_compile_options(${target} PRIVATE ${SUBEDIT_WARNINGS} -Werror)
endfunction()

# Variant without -Werror, reserved for code we do not control.
function(subedit_set_warnings_lenient target)
    target_compile_options(${target} PRIVATE ${SUBEDIT_WARNINGS})
endfunction()
