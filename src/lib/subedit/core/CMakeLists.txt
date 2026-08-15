# subedit_core — modèle, formats et opérations.
#
# Contrainte structurante : aucune dépendance à Qt ni à une interface
# graphique. Elle est vérifiée par src/scripts/check-architecture.sh.

add_library(
    subedit_core STATIC
    command/composite_command.cpp
    command/history.cpp
    edit/convert_frame_rate_command.cpp
    edit/hearing_impaired_removal.cpp
    edit/insert_command.cpp
    edit/remove_command.cpp
    edit/session.cpp
    edit/shift_command.cpp
    edit/transform_command.cpp
    edit/set_position_command.cpp
    edit/set_text_command.cpp
    edit/sort_command.cpp
    text/hearing_impaired.cpp
    model/project.cpp
    model/selection.cpp
    format/atomic_write.cpp
    format/in_memory_file_system.cpp
    format/encoding.cpp
    format/lines.cpp
    format/subtitle_file.cpp
    format/subtitle_format.cpp
    format/sub_rip_reader.cpp
    format/sub_rip_writer.cpp
    format/web_vtt_reader.cpp
    format/web_vtt_writer.cpp
    format/real_file_system.cpp
    time/timestamp.cpp
    version.cpp)
add_library(subedit::core ALIAS subedit_core)

# La racine d'inclusion est src/lib/, d'où des inclusions toujours qualifiées :
# #include <subedit/core/version.hpp>
target_include_directories(subedit_core PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_compile_definitions(
    subedit_core
    PRIVATE SUBEDIT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
            SUBEDIT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
            SUBEDIT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
            SUBEDIT_BUILD_DATE="${SUBEDIT_BUILD_DATE}")

subedit_set_warnings(subedit_core)
