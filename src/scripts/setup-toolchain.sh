#!/usr/bin/env bash
# Installation de la chaîne d'outils de développement.
#
# Cible Ubuntu 24.04. Le script est idempotent : il n'installe que ce qui
# manque, et peut être relancé sans effet de bord.
#
# Nécessite sudo pour les paquets APT. git-cliff n'étant pas empaqueté, il est
# récupéré depuis les releases GitHub vers ~/.local/bin.
#
# Deux modes réduits :
#
#   --git-cliff-only   n'installe que git-cliff, et rien d'autre
#   --list-packages    écrit les paquets APT, un par ligne, et n'installe rien

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly LOCAL_BIN="${HOME}/.local/bin"
readonly GIT_CLIFF_VERSION="2.10.1"

readonly MODE="${1:-}"

info() { printf '\033[1m%s\033[0m\n' "$*"; }
skip() { printf '  déjà présent : %s\n' "$*"; }

# Paquets APT : nom de commande -> nom de paquet.
# clang-tidy 20 plutôt que la version par défaut : libstdc++ garde <expected>
# derrière __cpp_concepts >= 202002L, valeur que Clang 18 ne déclare pas. Avec
# lui, std::expected est invisible à l'analyse statique.
declare -A APT_TOOLS=(
    [ninja]=ninja-build
    [clang-tidy-20]=clang-tidy-20
    [clang-format]=clang-format
    [gcovr]=gcovr
    [ccache]=ccache
    [gh]=gh
    # ffprobe est entré avec la phase 6, qui lit la fréquence d'image et la
    # durée dans le conteneur vidéo. Le paquet est ffmpeg, dont il n'est qu'un
    # des exécutables — c'est la commande qu'on sonde, jamais le paquet, sans
    # quoi une installation partielle passerait pour complète.
    #
    # **Le projet ne l'exige pas d'un utilisateur** : il l'exige d'une machine
    # de développement, ce qui n'est pas la même promesse. Le code porte la
    # branche de son absence, et src/test/unit/core/io/find_executable_test.cpp
    # la parcourt sans rien désinstaller.
    [ffprobe]=ffmpeg
    # jq est entré avec l'élagueur d'exécutions d'Actions : la preuve de sa
    # sélection, dans verify-gates.sh, l'exerce sur une liste écrite à la main,
    # donc en local. Les machines GitHub l'ont d'origine, pas forcément un poste
    # fraîchement installé — et une preuve qui ne s'exécute pas n'en est pas une.
    [jq]=jq
    # pkg-config est ce par quoi libmpv se trouve : il ne fournit pas de fichier
    # de configuration CMake, et `find_package(PkgConfig REQUIRED)` échoue sans
    # lui. Les machines GitHub l'ont d'origine ; une Ubuntu de bureau
    # fraîchement installée, pas toujours — et c'est celle-là que ce script
    # promet de rendre constructible.
    [pkg-config]=pkg-config
)

# Paquets APT sans commande à sonder : un fichier dit leur présence.
#
# CLI11 est une bibliothèque d'en-têtes — `command -v` ne la verrait jamais, et
# la ranger dans la table ci-dessus la ferait réinstaller à chaque exécution.
# Le chemin sondé est celui que `find_package(CLI11)` finit par lire.
#
# **Qt y est entré à la phase 5, et le mode `--with-qt` a disparu avec.** Il
# décrivait un monde où Qt était optionnel : ce n'est plus vrai, la porte
# construit tout, donc quiconque lance `make check` en a besoin. Il n'avait
# d'ailleurs jamais servi, et son défaut était de mettre les paquets Qt hors de
# `--list-packages`, donc hors du cache de la CI — qui aurait restauré une
# chaîne d'outils incomplète sans que rien ne le signale.
#
# **libmpv est entré à la phase 6**, qui met un lecteur dans la fenêtre — voir
# docs/adr/0020-libmpv-pour-le-lecteur-integre.md. Le chemin sondé est un
# fichier `.pc` et non un `Qt6Config.cmake` : libmpv ne fournit pas de fichier
# de configuration CMake, seulement de quoi être trouvé par `pkg-config`, et
# c'est donc ce chemin-là que la configuration finit par lire.
#
# **BLAS et LAPACK sont là pour libmpv, et c'est moins absurde qu'il n'y paraît.**
# La chaîne est celle-ci : libmpv tire libavfilter, qui tire le filtre de
# reconnaissance vocale d'ffmpeg, qui tire `libsphinxbase`, qui a besoin de
# `libblas.so.3` et de `liblapack.so.3`.
#
# Ces deux-là sont des **paquets virtuels** — plusieurs paquets réels les
# fournissent, `libblas3`, `libopenblas0-pthread`, `libmkl-rt`. C'est exactement
# ce que la fermeture de dépendances du cache de paquets de la CI ne sait pas
# résoudre : elle restaure `libsphinxbase3` sans fournisseur, et la chaîne arrive
# trouée. L'édition de liens s'arrête alors sur des symboles indéfinis, et le
# chargement de libmpv échouerait de la même façon.
#
# Le chemin sondé est le lien géré par le système d'alternatives : si un
# fournisseur quelconque est déjà là, rien n'est installé — une machine qui a
# déjà OpenBLAS n'en gagne pas un second.
declare -A APT_LIBS=(
    [/usr/include/CLI/CLI.hpp]=libcli11-dev
    [/usr/lib/x86_64-linux-gnu/cmake/Qt6/Qt6Config.cmake]=qt6-base-dev
    [/usr/lib/x86_64-linux-gnu/pkgconfig/mpv.pc]=libmpv-dev
    [/usr/lib/x86_64-linux-gnu/libblas.so.3]=libblas3
    [/usr/lib/x86_64-linux-gnu/liblapack.so.3]=liblapack3
)

install_apt_tools() {
    local missing=()
    local cmd
    for cmd in "${!APT_TOOLS[@]}"; do
        if command -v "${cmd}" >/dev/null 2>&1; then
            skip "${cmd}"
        else
            missing+=("${APT_TOOLS[${cmd}]}")
        fi
    done

    local probe
    for probe in "${!APT_LIBS[@]}"; do
        if [[ -e "${probe}" ]]; then
            skip "${APT_LIBS[${probe}]}"
        else
            missing+=("${APT_LIBS[${probe}]}")
        fi
    done

    if (( ${#missing[@]} == 0 )); then
        info "paquets APT : rien à installer"
        return
    fi

    info "installation des paquets : ${missing[*]}"
    sudo apt-get update -qq
    sudo apt-get install -y "${missing[@]}"
}

# gh n'est pas dans les dépôts de toutes les versions d'Ubuntu. Le test porte
# sur la disponibilité du paquet, pas sur le texte de `apt-cache policy`, qui
# est traduit et donc dépendant de la locale.
ensure_gh_repository() {
    command -v gh >/dev/null 2>&1 && return 0
    apt-cache show gh >/dev/null 2>&1 && return 0
    info "ajout du dépôt GitHub CLI"
    sudo mkdir -p -m 755 /etc/apt/keyrings
    curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg \
        | sudo tee /etc/apt/keyrings/githubcli-archive-keyring.gpg >/dev/null
    sudo chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" \
        | sudo tee /etc/apt/sources.list.d/github-cli.list >/dev/null
}

install_git_cliff() {
    if command -v git-cliff >/dev/null 2>&1; then
        skip "git-cliff"
        return
    fi
    info "installation de git-cliff ${GIT_CLIFF_VERSION} dans ${LOCAL_BIN}"
    mkdir -p "${LOCAL_BIN}"
    local archive="git-cliff-${GIT_CLIFF_VERSION}-x86_64-unknown-linux-gnu.tar.gz"
    local url="https://github.com/orhun/git-cliff/releases/download/v${GIT_CLIFF_VERSION}/${archive}"
    local tmp
    tmp="$(mktemp -d)"

    # Nettoyage explicite plutôt qu'un `trap RETURN` : un tel piège persiste
    # au-delà de la fonction qui l'installe et se redéclenche au retour des
    # suivantes, quand la variable qu'il référence n'existe plus.
    local status=0
    {
        curl -fsSL "${url}" -o "${tmp}/${archive}" &&
            tar -xzf "${tmp}/${archive}" -C "${tmp}" &&
            find "${tmp}" -type f -name git-cliff -perm -u+x \
                -exec install -m 755 {} "${LOCAL_BIN}/git-cliff" \;
    } || status=$?
    rm -rf "${tmp}"
    (( status == 0 )) || die "échec de l'installation de git-cliff (code ${status})"

    if ! command -v git-cliff >/dev/null 2>&1; then
        printf '  ajouter %s au PATH pour utiliser git-cliff\n' "${LOCAL_BIN}"
    fi
}

# Détecte l'alternative « c++ » mal configurée, qui pointe sur gcc au lieu de
# g++ : le C++ compile alors, mais n'édite pas les liens avec libstdc++.
check_cxx_alternative() {
    local target
    target="$(readlink -f /usr/bin/c++ 2>/dev/null || true)"
    [[ "${target}" == *gcc* ]] || return 0

    info "anomalie détectée : /usr/bin/c++ pointe sur ${target}"
    printf '  gcc compile le C++ mais ne lie pas libstdc++ ; toute compilation échouera.\n'
    printf '  correction : sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100\n'
    if [[ -x /usr/bin/g++ ]]; then
        sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100
        sudo update-alternatives --set c++ /usr/bin/g++
        printf '  corrigé : c++ -> %s\n' "$(readlink -f /usr/bin/c++)"
    fi
}

report() {
    info "état de la chaîne d'outils"
    local cmd
    local path_warning=0
    for cmd in cmake ninja g++ clang-tidy-20 clang-format gcovr ccache git git-cliff gh jq ffprobe pkg-config; do
        if command -v "${cmd}" >/dev/null 2>&1; then
            printf '  \033[32m✓\033[0m %-14s %s\n' "${cmd}" "$("${cmd}" --version 2>/dev/null | head -1)"
        elif [[ -x "${LOCAL_BIN}/${cmd}" ]]; then
            # Installé, mais hors du PATH de ce terminal. Sur Ubuntu,
            # ~/.profile n'ajoute ~/.local/bin qu'à l'ouverture de session, et
            # seulement si le répertoire existait déjà — ce qui n'est pas le
            # cas quand ce script vient de le créer.
            printf '  \033[33m!\033[0m %-14s %s\n' "${cmd}" \
                "$("${LOCAL_BIN}/${cmd}" --version 2>/dev/null | head -1)"
            path_warning=1
        else
            printf '  \033[31m✗\033[0m %-14s absent\n' "${cmd}"
        fi
    done

    local probe
    for probe in "${!APT_LIBS[@]}"; do
        if [[ -e "${probe}" ]]; then
            printf '  \033[32m✓\033[0m %-14s %s\n' "${APT_LIBS[${probe}]}" "${probe}"
        else
            printf '  \033[31m✗\033[0m %-14s absent\n' "${APT_LIBS[${probe}]}"
        fi
    done

    if (( path_warning )); then
        printf '\n  \033[33m!\033[0m installé dans %s, absent du PATH de ce terminal.\n' "${LOCAL_BIN}"
        printf '      effectif à la prochaine ouverture de session, ou tout de suite avec :\n'
        printf '        export PATH="%s:$PATH"\n' "${LOCAL_BIN}"
        printf '      les cibles make fonctionnent sans cela.\n'
    fi
}

# Écrit la liste des paquets APT, sans rien installer.
#
# Elle sert au cache de la CI, qui a besoin de nommer ce qu'il restaure. La
# faire dire par ce script plutôt que par le YAML est ce qui garde **une seule
# liste** : une seconde, recopiée dans un workflow, divergerait au premier
# paquet ajouté, et le cache restaurerait alors une chaîne d'outils incomplète
# sans que rien ne le signale.
list_packages() {
    printf '%s\n' "${APT_TOOLS[@]}" "${APT_LIBS[@]}" | sort -u
}

main() {
    if [[ "${MODE}" == "--list-packages" ]]; then
        list_packages
        return
    fi

    # Le job « contrôles de pull request » ne construit rien : de toute la
    # chaîne, il n'a besoin que de git-cliff, pour régénérer le journal et
    # comparer. Lui installer cmake, clang-tidy et gcovr coûterait plusieurs
    # minutes à chaque édition d'un corps de pull request. Le mode existe ici
    # plutôt que dans le YAML pour que le numéro de version de git-cliff reste
    # écrit à un seul endroit.
    if [[ "${MODE}" == "--git-cliff-only" ]]; then
        install_git_cliff
        return
    fi

    check_cxx_alternative
    ensure_gh_repository
    install_apt_tools
    install_git_cliff
    "${REPO_ROOT}/src/scripts/install-hooks.sh"
    "${REPO_ROOT}/src/scripts/reference.sh" lock >/dev/null 2>&1 || true
    report
}

main "$@"
