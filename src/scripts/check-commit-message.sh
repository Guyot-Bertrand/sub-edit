#!/usr/bin/env bash
# Valide un message de commit contre la grammaire Conventional Commits.
#
# Utilisé par le hook commit-msg en local et par la CI sur l'historique poussé.
# Argument : un fichier contenant le message, ou le message lui-même.

set -euo pipefail

readonly TYPES='feat|fix|perf|refactor|test|docs|build|ci|chore|revert'
readonly SCOPES='build|ci|cli|core|doc|format|gui|i18n|scripts|test|text|video'
readonly SUBJECT_MAX_LENGTH=72

readonly RED=$'\033[31m'
readonly RESET=$'\033[0m'

input="${1:?usage : $(basename "$0") <fichier|message>}"
if [[ -f "${input}" ]]; then
    message="$(cat "${input}")"
else
    message="${input}"
fi

# Les lignes de commentaire de git ne font pas partie du message.
subject="$(printf '%s\n' "${message}" | grep -v '^#' | sed '/^[[:space:]]*$/d' | head -1)"

reject() {
    printf '%smessage de commit invalide%s\n\n' "${RED}" "${RESET}" >&2
    printf '  %s\n\n' "${subject}" >&2
    printf '%s\n' "$*" >&2
    cat >&2 <<EOF

Grammaire attendue :

  <type>(<scope>): <description>

  types  : ${TYPES//|/, }
  scopes : ${SCOPES//|/, }   (facultatif)

Exemples :

  feat(core): lecture des fichiers SubRip
  fix(cli): code de retour non nul en cas d'échec partiel
  docs: feuille de route des huit phases
EOF
    exit 1
}

# Les commits de fusion sont produits par git, pas par un humain.
[[ "${subject}" =~ ^Merge\  ]] && exit 0

[[ -n "${subject}" ]] || reject "le message est vide."

[[ "${subject}" =~ ^(${TYPES})(\((${SCOPES})\))?!?:\ .+ ]] \
    || reject "l'en-tête ne suit pas la grammaire, ou le scope n'appartient pas à la liste."

(( ${#subject} <= SUBJECT_MAX_LENGTH )) \
    || reject "l'en-tête fait ${#subject} caractères, maximum ${SUBJECT_MAX_LENGTH}."

[[ ! "${subject}" =~ \.$ ]] \
    || reject "l'en-tête ne se termine pas par un point."
