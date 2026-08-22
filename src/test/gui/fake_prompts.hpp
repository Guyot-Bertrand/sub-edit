#pragma once

// Les réponses qu'un scénario donne à la place d'un humain.
//
// C'est ce que la couture de `Prompts` achète : « l'utilisateur a annulé »,
// « l'utilisateur a choisi d'abandonner », « l'écriture a échoué et il l'a lu »
// deviennent des chemins qu'un test parcourt, là où un `QDialog::exec` les
// aurait rendus inatteignables.

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
    /// Ce que la prochaine question rendra. Vide vaut « l'utilisateur a
    /// annulé », qui est le défaut parce que c'est le cas qu'on oublie.
    std::optional<std::filesystem::path> nextFileToOpen{};
    std::optional<gui::SaveTarget> nextSaveTarget{};
    gui::UnsavedChoice nextUnsavedChoice = gui::UnsavedChoice::Cancel;

    /// Ce qui a été demandé, et ce qui a été dit.
    int openAsked = 0;
    int saveTargetAsked = 0;
    int unsavedAsked = 0;
    std::vector<std::string> failures{};

    /// Ce que la dernière question sur la destination a reçu comme point de
    /// départ — un dialogue qui ne l'utiliserait pas s'ouvrirait sur nulle part.
    core::SourceFile lastCurrent{};

    [[nodiscard]] std::optional<std::filesystem::path> fileToOpen() override {
        ++openAsked;
        return nextFileToOpen;
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

    /// Ce qu'un dialogue devient sans humain : ce que le scénario a posé dans
    /// `nextRun`, et ce qu'il a écrit dans les champs avant d'y arriver.
    bool nextRun = false;
    int runAsked = 0;
    QDialog* lastDialog = nullptr;

    /// Ce que le scénario fait du dialogue avant de répondre — remplir ses
    /// champs, c'est-à-dire jouer l'utilisateur qui tape.
    std::function<void(QDialog&)> fill{};

    [[nodiscard]] bool run(QDialog& dialog) override {
        ++runAsked;
        lastDialog = &dialog;
        if (fill)
            fill(dialog);
        return nextRun;
    }

    /// Ce qui a été rapporté sans être un échec — le compte rendu d'une
    /// opération, que l'utilisateur lit et qui ne demande rien.
    std::vector<std::string> outcomes{};

    void reportFailure(const std::string& message) override { failures.push_back(message); }

    void reportOutcome(const std::string& message) override { outcomes.push_back(message); }
};

} // namespace subedit::test
