#!/usr/bin/env bash
# Installation de la chaîne d'outils de développement.
#
# Cible Ubuntu 24.04. Le script est idempotent : il n'installe que ce qui
# manque, et peut être relancé sans effet de bord.
#
# Nécessite sudo pour les paquets APT. git-cliff n'étant pas empaqueté, il est
# récupéré depuis les releases GitHub vers ~/.local/bin.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly LOCAL_BIN="${HOME}/.local/bin"
readonly GIT_CLIFF_VERSION="2.10.1"

readonly WITH_QT="${1:-}"

info() { printf '\033[1m%s\033[0m\n' "$*"; }
skip() { printf '  déjà présent : %s\n' "$*"; }

# Paquets APT : nom de commande -> nom de paquet.
declare -A APT_TOOLS=(
    [ninja]=ninja-build
    [clang-tidy]=clang-tidy
    [clang-format]=clang-format
    [gcovr]=gcovr
    [ccache]=ccache
    [gh]=gh
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

    if [[ "${WITH_QT}" == "--with-qt" ]]; then
        # Requis à partir de la phase 5 seulement.
        missing+=(qt6-base-dev qt6-multimedia-dev)
    fi

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
    trap 'rm -rf "${tmp}"' RETURN
    curl -fsSL "${url}" -o "${tmp}/${archive}"
    tar -xzf "${tmp}/${archive}" -C "${tmp}"
    find "${tmp}" -type f -name git-cliff -perm -u+x -exec install -m 755 {} "${LOCAL_BIN}/git-cliff" \;
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
    for cmd in cmake ninja g++ clang-tidy clang-format gcovr ccache git git-cliff gh; do
        if command -v "${cmd}" >/dev/null 2>&1; then
            printf '  \033[32m✓\033[0m %-14s %s\n' "${cmd}" "$("${cmd}" --version 2>/dev/null | head -1)"
        else
            printf '  \033[31m✗\033[0m %-14s absent\n' "${cmd}"
        fi
    done
}

main() {
    check_cxx_alternative
    ensure_gh_repository
    install_apt_tools
    install_git_cliff
    "${REPO_ROOT}/src/scripts/install-hooks.sh"
    "${REPO_ROOT}/src/scripts/reference.sh" lock >/dev/null 2>&1 || true
    report
}

main "$@"
