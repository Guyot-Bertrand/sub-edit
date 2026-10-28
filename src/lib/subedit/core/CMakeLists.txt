# subedit_core — model, formats and operations.
#
# Structuring constraint: no dependency on Qt or on a graphical interface. It
# is checked by src/scripts/check-architecture.sh.

add_library(
    subedit_core STATIC
    analysis/anomaly.cpp
    analysis/frame_rate_deduction.cpp
    analysis/grid_correction.cpp
    analysis/grid_repair.cpp
    command/composite_command.cpp
    command/history.cpp
    edit/append.cpp
    edit/convert_frame_rate_command.cpp
    edit/hearing_impaired_removal.cpp
    edit/insert_command.cpp
    edit/dialogue_dashes_command.cpp
    edit/italics_command.cpp
    edit/letter_case_command.cpp
    edit/merge_split_command.cpp
    edit/clipboard.cpp
    edit/translation.cpp
    edit/translation_drift.cpp
    edit/duration_adjustment.cpp
    edit/search.cpp
    edit/shift_limits.cpp
    edit/split_project.cpp
    edit/video_bounds.cpp
    edit/remove_command.cpp
    edit/replace_subtitles.cpp
    edit/rewrite_texts.cpp
    edit/session.cpp
    edit/snap_command.cpp
    edit/shift_command.cpp
    edit/transform_command.cpp
    edit/set_position_command.cpp
    edit/set_duration_command.cpp
    edit/set_text_command.cpp
    edit/command_preview.cpp
    edit/disjoint_ranges.cpp
    edit/join_repeats.cpp
    edit/sort_command.cpp
    text/capitalization.cpp
    text/enchant_spell_provider.cpp
    text/join_split_words.cpp
    text/spell_check_navigator.cpp
    text/spell_check_walk.cpp
    text/spell_checker.cpp
    text/spell_ranges.cpp
    text/spell_replacements.cpp
    text/spell_tokenizer.cpp
    text/word_list_spell_provider.cpp
    text/common_errors.cpp
    text/correction_pattern.cpp
    text/icu_pattern_engine.cpp
    text/hearing_impaired.cpp
    text/hearing_impaired_correction.cpp
    text/correction_run.cpp
    text/text_diff.cpp
    text/line_breaking.cpp
    text/line_lengths.cpp
    text/line_measure.cpp
    text/pattern_catalogue.cpp
    text/pattern_paths.cpp
    text/python_regex_syntax.cpp
    text/replacement_template.cpp
    text/italics.cpp
    model/encoding.cpp
    model/project.cpp
    model/video_file.cpp
    model/selection.cpp
    io/atomic_write.cpp
    io/json_reader.cpp
    io/find_executable.cpp
    io/find_video.cpp
    video/declared_frame_rate.cpp
    video/frame_step.cpp
    video/replica.cpp
    video/seeking.cpp
    video/showing.cpp
    io/in_memory_file_system.cpp
    text/encoding.cpp
    text/break_marker.cpp
    text/markup.cpp
    text/markup_codec.cpp
    text/markup_parser.cpp
    text/markup_reader.cpp
    text/markup_vocabulary.cpp
    text/html_markup.cpp
    text/sub_station_alpha_markup.cpp
    text/micro_dvd_markup.cpp
    text/markup_conversion.cpp
    text/lines.cpp
    text/dialogue_dashes.cpp
    text/letter_case.cpp
    text/utf8.cpp
    i18n/catalogue.cpp
    i18n/language.cpp
    i18n/language_names.cpp
    i18n/messages.cpp
    i18n/plural_expression.cpp
    format/subtitle_file.cpp
    format/project_file.cpp
    format/translation_file.cpp
    config/settings.cpp
    config/settings_location.cpp
    config/operation_settings.cpp
    config/duration_adjustment_settings.cpp
    config/correction_settings.cpp
    format/format_detection.cpp
    format/degradation.cpp
    format/sub_rip_reader.cpp
    format/sub_rip_writer.cpp
    format/micro_dvd_reader.cpp
    format/micro_dvd_syntax.cpp
    format/micro_dvd_writer.cpp
    format/mpl2_reader.cpp
    format/mpl2_syntax.cpp
    format/mpl2_writer.cpp
    format/deduced_ends.cpp
    format/lrc_reader.cpp
    format/lrc_syntax.cpp
    format/lrc_writer.cpp
    format/tm_player_reader.cpp
    format/tm_player_syntax.cpp
    format/tm_player_writer.cpp
    format/sub_station_alpha_reader.cpp
    format/sub_station_alpha_syntax.cpp
    format/sub_station_alpha_writer.cpp
    format/sub_viewer2_reader.cpp
    format/sub_viewer2_syntax.cpp
    format/sub_viewer2_writer.cpp
    format/web_vtt_reader.cpp
    format/web_vtt_writer.cpp
    io/real_file_system.cpp
    io/temporary_directory.cpp
    libraries.cpp
    process/start_process.cpp
    time/timestamp.cpp
    version.cpp
    wording/analysis.cpp
    wording/conversion.cpp
    wording/correction.cpp
    wording/counts.cpp
    wording/editing.cpp
    wording/formats.cpp
    wording/settings.cpp
    wording/transcription.cpp
    wording/translation.cpp
    wording/version.cpp
    wording/video.cpp
    transcription/engine_capabilities.cpp
    transcription/audio_source.cpp
    transcription/default_engines.cpp
    transcription/engine_registry.cpp
    transcription/language_codes.cpp
    transcription/language_display.cpp
    transcription/subtitle_assembly.cpp
    transcription/remaining_time.cpp
    transcription/time_range.cpp
    transcription/tools_status.cpp
    transcription/transcribe_media.cpp
    transcription/transcription_engine.cpp
    transcription/whisper_cpp_engine.cpp
    transcription/whisper_cpp_output.cpp
    transcription/word_matching.cpp
    transcription/alignment_doubt.cpp
    transcription/alignment_retiming.cpp
    transcription/transcript_matching_aligner.cpp)
add_library(subedit::core ALIAS subedit_core)

# The include root is src/lib/, hence always-qualified includes:
# #include <subedit/core/version.hpp>
target_include_directories(subedit_core PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

# **On `version.cpp` alone, and not on the target** (#686). A target definition enters the compile
# command line of each of its sources: the patch bump of every pull request, and the commit date at
# every commit, therefore changed the command of the three hundred or so files of the core, and
# Ninja recompiled them all — in every tree, `tidy` included, whose analysis is hooked to that
# command. Only `version.cpp` reads these macros; they now live there alone.
set_source_files_properties(
    version.cpp
    PROPERTIES COMPILE_DEFINITIONS
               "SUBEDIT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR};SUBEDIT_VERSION_MINOR=${PROJECT_VERSION_MINOR};SUBEDIT_VERSION_PATCH=${PROJECT_VERSION_PATCH};SUBEDIT_BUILD_DATE=\"${SUBEDIT_BUILD_DATE}\"")

# **The version of Enchant the build found, on `libraries.cpp` alone** (#716), for the reason the
# version macros are on `version.cpp` alone: Enchant's headers carry no version of their own, so the
# one `pkg-config` gave at configuration is what the report calls the version it was built with.
set_source_files_properties(libraries.cpp PROPERTIES COMPILE_DEFINITIONS
                                                     "SUBEDIT_ENCHANT_VERSION=\"${enchant_VERSION}\"")

# ICU as private: no header of the core shows `unicode/`, only implementation
# files include it. What depends on subedit_core therefore
# inherits the link, and nothing else.
#
# Enchant likewise: only `enchant_spell_provider.cpp` and `libraries.cpp`
# include it, and nothing of their headers shows `enchant.h`.
target_link_libraries(subedit_core PRIVATE ICU::uc ICU::i18n PkgConfig::enchant)

subedit_set_warnings(subedit_core)
