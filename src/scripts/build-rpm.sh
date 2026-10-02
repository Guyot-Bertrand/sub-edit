#!/usr/bin/env bash
# Construit le `.rpm` sur la Fedora où l'on est, contre l'ICU qu'elle porte.
#
#   build-rpm.sh
#
# **Pourquoi ce script existe** : un `.rpm` construit sur Ubuntu demande
# `libicuuc.so.74` et `libicui18n.so.74`, dont les symboles portent le numéro
# de la version majeure d'ICU (`u_strToUTF8_74`). Une Fedora qui n'a pas ICU 74
# — ni son paquet de compatibilité — refuse de l'installer, et rien dans le
# paquet ne permet d'y remédier. Le `.rpm` publié est donc construit **sur
# chaque Fedora qu'on publie**, contre son ICU, et nommé d'après elle
# (`subedit-X.Y.Z-1.fc43.x86_64.rpm`). Voir ADR 0023.
#
# À lancer dans une image `fedora:N`, en root : il installe ce qu'il lui faut.
# Le `.deb` n'est pas concerné, il se construit comme avant.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

[[ -r /etc/fedora-release ]] || { echo "ce script se lance sur une Fedora" >&2; exit 1; }

# Les mêmes bibliothèques que `setup-toolchain.sh` installe côté Debian ;
# `mpv-devel` porte `mpv.pc`, `cli11-devel` la bibliothèque d'en-têtes.
dnf install -y --setopt=install_weak_deps=False \
    gcc-c++ cmake ninja-build git rpm-build pkgconf-pkg-config python3 \
    qt6-qtbase-devel mpv-devel libicu-devel enchant2-devel cli11-devel

# Le plafond vient de l'environnement, comme pour les autres scripts : le workflow
# le pose à `nproc`, un appel local garde le défaut prudent.
readonly JOBS="${JOBS:-2}"
export CMAKE_GENERATOR="Ninja"

cd "${REPO_ROOT}"
cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
cmake --build --preset release -j "${JOBS}" --target subedit-cli subedit-gui
(cd build/release && cpack -G RPM >/dev/null)

ls -1 build/release/subedit*.rpm
