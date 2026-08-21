#pragma once

// Les réponses qu'un scénario donne à la place d'un humain.
//
// C'est ce que la couture de `Prompts` achète : « l'utilisateur a annulé »,
// « l'utilisateur a choisi d'abandonner », « l'écriture a échoué et il l'a lu »
// deviennent des chemins qu'un test parcourt, là où un `QDialog::exec` les
// aurait rendus inatteignables.

#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/prompts.hpp>

#include <filesystem>
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

    void reportFailure(const std::string& message) override { failures.push_back(message); }
};

} // namespace subedit::test
