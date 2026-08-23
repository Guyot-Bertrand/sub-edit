#pragma once

// The answers a scenario gives in a human's place.
//
// This is what the seam of `Prompts` buys: « the user cancelled », « the user
// chose to discard », « the write failed and they read about it » become roads
// a test walks, where a `QDialog::exec` would have left them out of reach.

#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/prompts.hpp>

#include <QDialog>

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace subedit::test {

class FakePrompts final : public gui::Prompts {

public:
    /// What the next question will return. Empty means « the user cancelled »,
    /// which is the default because it is the case one forgets.
    std::optional<std::filesystem::path> nextFileToOpen{};
    std::optional<gui::SaveTarget> nextSaveTarget{};
    gui::UnsavedChoice nextUnsavedChoice = gui::UnsavedChoice::Cancel;

    /// What was asked, and what was said.
    int openAsked = 0;
    int saveTargetAsked = 0;
    int unsavedAsked = 0;
    std::vector<std::string> failures{};

    /// What the last question about the destination was given as a starting
    /// point — a dialog that ignored it would open on nowhere.
    core::SourceFile lastCurrent{};

    [[nodiscard]] std::optional<std::filesystem::path> fileToOpen() override {
        ++openAsked;
        return nextFileToOpen;
    }

    /// What the video chooser answers, and where it was told to open.
    std::optional<std::filesystem::path> nextVideoToOpen{};
    int videoAsked = 0;
    std::filesystem::path lastVideoDirectory{};

    [[nodiscard]] std::optional<std::filesystem::path>
    videoToOpen(const std::filesystem::path& directory) override {
        ++videoAsked;
        lastVideoDirectory = directory;
        return nextVideoToOpen;
    }

    [[nodiscard]] std::optional<gui::SaveTarget>
    saveTarget(const core::SourceFile& current) override {
        ++saveTargetAsked;
        lastCurrent = current;
        return nextSaveTarget;
    }

    [[nodiscard]] gui::UnsavedChoice aboutUnsavedChanges() override {
        ++unsavedAsked;
        return nextUnsavedChoice;
    }

    /// What a dialog becomes without a human: what the scenario put in
    /// `nextRun`, and what it wrote in the fields before getting there.
    bool nextRun = false;
    int runAsked = 0;
    QDialog* lastDialog = nullptr;

    /// What the scenario does with the dialog before answering — filling its
    /// fields, that is, playing the user who types.
    std::function<void(QDialog&)> fill{};

    [[nodiscard]] bool run(QDialog& dialog) override {
        ++runAsked;
        lastDialog = &dialog;
        if (fill)
            fill(dialog);
        return nextRun;
    }

    /// What was reported without being a failure — the account of an
    /// operation, which the user reads and which asks nothing.
    std::vector<std::string> outcomes{};

    void reportFailure(const std::string& message) override { failures.push_back(message); }

    void reportOutcome(const std::string& message) override { outcomes.push_back(message); }
};

} // namespace subedit::test
