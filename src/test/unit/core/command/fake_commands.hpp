#pragma once

// Stand-in commands for the tests of this module.
//
// Phase 1 provides no concrete command — those arrive with the operations —
// so the foundation is exercised with the smallest doubles that still prove
// something: one that really changes a project, and one that records the order
// in which it was called.

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <string>
#include <utility>
#include <vector>

namespace subedit::test {

/// Sets the main text of one subtitle, remembering the previous one.
///
/// Written the way a real command will be: the state needed to undo is
/// captured at construction, and limited to what is strictly necessary.
class SetMainText final : public core::Command {

public:
    SetMainText(const core::Project& project, core::SubtitleIndex index, std::string text)
        : m_index(index),
          m_newText(std::move(text)),
          m_oldText(project.subtitleAt(index).mainText) {}

    void apply(core::Project& project) override {
        project.subtitleAt(m_index).mainText = m_newText;
    }

    void revert(core::Project& project) override {
        project.subtitleAt(m_index).mainText = m_oldText;
    }

    [[nodiscard]] core::CommandKind kind() const override { return core::CommandKind::SetText; }

    [[nodiscard]] std::vector<core::Change> describe() const override {
        return {core::Change{.kind = core::ChangeKind::MainText,
                             .subtitles = core::Selection::range(m_index, m_index)}};
    }

private:
    core::SubtitleIndex m_index;
    std::string m_newText;
    std::string m_oldText;
};

/// Touches nothing, and reports the nature of change it was built with.
///
/// Enough to drive the per-document modification counters without needing a
/// project that could actually hold the change.
class Declaring final : public core::Command {

public:
    explicit Declaring(core::ChangeKind kind,
                       core::CommandKind commandKind = core::CommandKind::SetText)
        : m_kind(kind), m_commandKind(commandKind) {}

    void apply(core::Project& /*project*/) override {}

    void revert(core::Project& /*project*/) override {}

    [[nodiscard]] core::CommandKind kind() const override { return m_commandKind; }

    [[nodiscard]] std::vector<core::Change> describe() const override {
        return {core::Change{.kind = m_kind, .subtitles = core::Selection::of({})}};
    }

private:
    core::ChangeKind m_kind;
    core::CommandKind m_commandKind;
};

/// Appends a line to a trace shared with the test, so that the order of calls
/// becomes an assertion instead of a guess.
///
/// Holds a reference to a vector the test owns: an observer, whose lifetime is
/// guaranteed by the test outliving the command.
class Tracing final : public core::Command {

public:
    Tracing(std::vector<std::string>& trace, std::string name)
        : m_trace(trace), m_name(std::move(name)) {}

    void apply(core::Project& /*project*/) override { m_trace.push_back("apply " + m_name); }

    void revert(core::Project& /*project*/) override { m_trace.push_back("revert " + m_name); }

    [[nodiscard]] core::CommandKind kind() const override { return core::CommandKind::SetText; }

    [[nodiscard]] std::vector<core::Change> describe() const override {
        return {
            core::Change{.kind = core::ChangeKind::MainText, .subtitles = core::Selection::of({})}};
    }

private:
    std::vector<std::string>& m_trace;
    std::string m_name;
};

} // namespace subedit::test
