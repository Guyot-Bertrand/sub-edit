# Façade ergonomique sur CMake.
#
# Ce fichier n'ajoute aucune logique de construction : il abrège des lignes de
# commande. La règle importante est `check`, décrite dans
# docs/specs/00-fondations.md.

.DEFAULT_GOAL := help
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

# Nombre de tâches parallèles, pour la compilation et pour l'analyse statique.
#
# **Deux cœurs par défaut, pas plus.** La machine de développement fait tourner
# d'autres projets en même temps, et une compilation qui prend tous les cœurs y
# provoque des échecs de tests sur délai d'attente : un défaut ailleurs, causé
# ici. Deux est le plafond convenu pour le travail local — une règle, pas un
# jugement au cas par cas, précisément parce que le seuil est invisible : on ne
# voit pas ce qu'on casse chez le voisin avant de l'avoir cassé. Le chiffre
# vient d'une mesure : `make check` à un seul cœur prend environ 17 minutes,
# dont environ 15 pour clang-tidy à lui seul.
#
# Se relève au besoin, sans toucher à ce fichier :
#
#     make build JOBS=8
#     JOBS=$$(nproc) make check
#
# La CI passe `nproc` explicitement : sa machine n'est qu'à elle, le plafond ne
# la concerne pas.
#
# src/scripts/check-parallelism.sh vérifie mécaniquement qu'aucune recette de
# ce fichier ni aucun script de src/scripts/ ne contourne $(JOBS) par un
# parallélisme câblé en dur.
#
# Les tests restent séquentiels quoi qu'il arrive. Ce n'est pas un oubli :
# `ctest` ne parallélise que sur `-j` explicite, et le lui donner ferait
# fusionner les `.gcda` de plusieurs exécutions concurrentes, donc un taux de
# couverture faux sans autre signe qu'un avertissement noyé dans la sortie.
JOBS ?= 2

# **`JOBS` et `BENCH_MAX_LOAD` sont exportés, et lus par les étapes.**
#
# Elles vivent dans src/scripts/gate/ depuis #269, et une variable de recette
# ne leur parviendrait pas. Le défaut de chacune est écrit ici, avec la mesure
# qui le justifie, et les étapes le reprennent sans le redéfinir : deux endroits
# où poser un chiffre en feraient deux vérités.
#
# Une et demie, et non deux, pour la charge : le seuil a été abaissé le jour
# même où il a été posé. Le relevé de la version 0.3.5, pris à 1,88 — sous le
# seuil, donc admis — a fixé un maximum de 853 µs pour l'écriture de 4000
# sous-titres, là où l'enveloppe des dix-sept relevés précédents allait de 492 à
# 641. Les relevés pris sous 1,4 n'ont posé que des minima.
#
# Le chiffre reste une heuristique, et le journal est là pour l'affiner : chaque
# relevé porte sa charge, donc le rapport entre les deux se lit désormais.
BENCH_MAX_LOAD ?= 1.5
export BENCH_MAX_LOAD
export JOBS

# git-cliff s'installe dans ~/.local/bin, que ~/.profile n'ajoute au PATH qu'à
# l'ouverture de session suivante. On ne dépend pas de la configuration du
# shell de l'utilisateur.
export PATH := $(HOME)/.local/bin:$(PATH)

# Ninja est préféré, sans être requis : le figer dans les presets rendrait le
# projet inconstructible sur une machine qui ne l'a pas encore.
export CMAKE_GENERATOR ?= $(shell command -v ninja >/dev/null 2>&1 && echo Ninja || echo "Unix Makefiles")

# **L'analyse statique n'a plus de bouton, et c'est le sujet de #269.**
#
# Elle en avait un — `TIDY_BASE` — et un script de deux cents lignes qui
# calculait un périmètre depuis un diff git, avec une liste de « fichiers
# gouvernants » qui faisait tout reprendre. Ce fichier en était : un commentaire
# ajouté à une recette faisait passer l'analyse de 2 fichiers à 223.
#
# clang-tidy ne lit pourtant jamais un Makefile. Ce dont dépend le résultat
# d'une unité de traduction — sa source, ses en-têtes, sa ligne de commande —,
# le système de construction le connaît déjà, exactement. Il est donc accroché à
# la règle de compilation de chaque source, sous le preset `tidy`, et il n'y a
# plus rien à calculer ni à passer. Voir cmake/Tidy.cmake.
#
# Pour tout réanalyser : `rm -rf build/tidy`.

BOLD := \033[1m
GREEN := \033[32m
RED := \033[31m
RESET := \033[0m

define step
	@printf '$(BOLD)▸ %s$(RESET)\n' $(1)
endef

# Échoue avec un message utile plutôt qu'une erreur « command not found ».
define require
	@command -v $(1) >/dev/null 2>&1 || { \
		printf '$(RED)outil manquant : %s$(RESET)\n' $(1) >&2; \
		printf '  l installer avec : ./src/scripts/setup-toolchain.sh\n' >&2; \
		exit 1; \
	}
endef

.PHONY: help
help: ## Affiche cette aide
	@printf '$(BOLD)subedit$(RESET) — cibles disponibles\n\n'
	@grep -hE '^[a-z0-9-]+:.*?## ' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  $(BOLD)%-12s$(RESET) %s\n", $$1, $$2}'
	@printf '\ngénérateur : $(CMAKE_GENERATOR)\n'
	@printf 'tâches parallèles : $(JOBS)   (relever avec JOBS=N)\n'

.PHONY: setup
setup: ## Installe la chaîne d'outils et les hooks git
	@./src/scripts/setup-toolchain.sh

.PHONY: build
build: ## Compile le preset dev
	@./src/scripts/gate.sh build

# Le pendant release de `build`, et il ne produit que ce qu'un utilisateur lance.
#
# **Elle existe parce qu'on faisait sans.** Le contournement était de lancer
# `make bench` et de l'interrompre une fois l'édition de liens passée : ça
# marche, et ça a trois défauts — il compile aussi `subedit_bench`, dont on n'a
# que faire ; il laisse une exécution à moitié faite, dont on ne sait plus si
# elle a écrit au journal des mesures ; et il faut le surveiller pour savoir
# quand interrompre.
#
# Ni le banc ni le harnais de bout en bout, donc : ils ont leurs propres cibles.
# Ce qui est demandé ici est ce qu'un utilisateur lance, pas ce qui l'éprouve.
#
# `JOBS` gouverne le LTO, pour la raison écrite sur `bench` : sans lui,
# l'optimisation entre modules part en `-flto=auto`, c'est-à-dire autant de
# processus que de cœurs, à chaque édition de liens — un parallélisme qui
# n'apparaît dans aucun `-j` et qui sature une machine sur laquelle on fait
# autre chose. C'est aussi ce qui rend cette cible préférable aux deux commandes
# `cmake` écrites à la main, qui n'en savent rien.
#
# **Elle n'exécute rien.** C'est ce que l'empaquetage installera — ADR 0023,
# issue #244.
.PHONY: release
release: ## Produit les deux binaires optimisés, et rien d'autre
	@./src/scripts/gate.sh release

# N'exécute pas les tests de bout en bout : ils ne s'enregistrent dans CTest
# que sous les presets asan et release — voir SUBEDIT_REGISTER_E2E dans
# src/test/e2e/CMakeLists.txt. `make asan` est la façon la plus rapide de les
# voir tourner en boucle de développement ; `make e2e` cible release seul.
.PHONY: test
test: ## Compile et exécute les tests (hors bout en bout — voir make asan)
	$(call step,"tests (dev)")
	@cmake --preset dev
	@cmake --build --preset dev -j $(JOBS)
	@ctest --preset dev

.PHONY: bench
bench: ## Exécute les benchmarks en release et verse les chiffres au journal
	@./src/scripts/gate.sh bench

.PHONY: format
format: ## Applique clang-format
	@./src/scripts/gate.sh format

.PHONY: format-check
format-check: ## Vérifie le format sans modifier
	@./src/scripts/gate.sh format-check

.PHONY: tidy
tidy: ## Analyse statique — clang-tidy à la compilation de chaque source
	@./src/scripts/gate.sh tidy

.PHONY: arch
arch: ## Vérifie les invariants d'architecture
	$(call step,"invariants d architecture")
	@./src/scripts/check-architecture.sh

.PHONY: parallelism
parallelism: ## Vérifie qu'aucun parallélisme ne contourne $(JOBS)
	$(call step,"parallélisme maîtrisé")
	@./src/scripts/check-parallelism.sh

.PHONY: fixtures
fixtures: ## Vérifie que les fixtures engendrées sont ce que leur table dit
	@./src/scripts/gate.sh fixtures

.PHONY: manual
manual: ## Régénère les exemples d'appel et les captures du manuel
	@./src/scripts/gate.sh manual

.PHONY: manual-check
manual-check: ## Vérifie que les exemples, les captures et les renvois du manuel sont à jour
	@./src/scripts/gate.sh manual-check

.PHONY: requirements
requirements: ## Confronte le registre d'exigences aux tests et aux specs
	@./src/scripts/gate.sh requirements

# Ne construit et n'exécute que le harnais de bout en bout, filtré par
# l'étiquette CTest `e2e` plutôt que par nom de test — un nom de test unitaire
# qui s'en approcherait ne le tromperait pas. Partage le build release avec
# `make bench` : la seconde invocation ne recompile rien.
.PHONY: e2e
e2e: ## Exécute uniquement les tests de bout en bout (release)
	@./src/scripts/gate.sh e2e

.PHONY: packages
packages: ## Produit le .deb et le .rpm depuis l'arbre release
	@./src/scripts/gate.sh packages

.PHONY: install-check
install-check: ## Installe dans un préfixe temporaire et lance le binaire installé
	@./src/scripts/gate.sh install-check

# **Hors de `check-local`, et c'est délibéré** — issue #266. Elle télécharge
# près de trois cents mégaoctets et demande le réseau ; une porte qui dépend du
# réseau est rouge un jour de panne, pour une raison étrangère au dépôt. Ce qui
# garde les pull requests est le contrôle local que `install-check` porte
# désormais : un `.rpm` ne possède que ses propres répertoires.
.PHONY: rpm-check
rpm-check: ## Installe le .rpm sur une Fedora en conteneur, et lance l'installé
	@./src/scripts/gate.sh rpm-check

.PHONY: asan
asan: ## Exécute les tests sous ASan et UBSan
	@./src/scripts/gate.sh asan

.PHONY: coverage
coverage: ## Mesure la couverture des bibliothèques
	@./src/scripts/gate.sh coverage

.PHONY: ratchet
ratchet: ## Enregistre la couverture mesurée comme nouveau cliquet
	$(call step,"cliquet de couverture")
	@./src/scripts/check-coverage.sh \
		--summary build/coverage-report/summary.json --record

.PHONY: untracked
untracked: ## Refuse un fichier non suivi apparu depuis le dernier relevé
	@./src/scripts/gate.sh untracked

.PHONY: config-home
config-home: ## Refuse qu'une exécution touche la configuration de l'utilisateur
	@./src/scripts/gate.sh config-home

# **L'ordre des étapes n'est plus ici mais dans src/scripts/gate.sh**, avec la
# raison de chacune. Ce qui se gagne à l'avoir déplacé : on sait reprendre au
# milieu, ce qu'un enchaînement de sous-`make` ne savait pas faire.
#
#     ./src/scripts/gate.sh check --from coverage
#     ./src/scripts/gate.sh check --only tidy
#     ./src/scripts/gate.sh --list
#
# Une couverture qui casse à la trente-cinquième minute faisait repayer les
# trente-quatre premières.
.PHONY: check
check: ## Porte de qualité — format, warnings, tidy, tests sous ASan, couverture
	@./src/scripts/gate.sh check

# `check` est ce que la CI exécute — .github/workflows/ci.yml n'appelle que
# cette cible, rien d'autre. Tout ce qui y entre gate donc chaque push, de
# tout le monde ; on n'y ajoute rien à la légère.
#
# `check-local` est l'unique commande à lancer avant d'ouvrir une pull
# request : elle enchaîne tout ce qu'on veut voir passer en local sans le
# voir gater la CI. L'ordre va du moins cher au plus cher, pour qu'un échec
# coûte des secondes plutôt que la totalité de la chaîne : parallélisme
# maîtrisé (un grep, sous la seconde), fixtures vidéo (deux appels à ffprobe,
# sous la seconde aussi), exemples du manuel (le seul binaire de
# la CLI), exigences (compilation incrémentale dev), tests de bout en bout
# (build release), installation dans un préfixe temporaire — qui partage cet
# arbre release et ne le reconstruit donc pas —, puis benchmarks. `parallelism`
# passe en premier précisément parce qu'elle ne construit rien — la faire
# attendre derrière `requirements`, qui compile, coûterait à un `-j 8` codé en
# dur le temps d'un build entier avant qu'on l'entende.
#
# `manual-check` est ici et non dans `check`, ce qui a une conséquence à
# connaître : **la CI ne verra pas un manuel périmé.** L'y mettre ferait entrer
# `docs/manual/**` dans le périmètre de la porte, dont `docs/**` est exclu, et
# une retouche de prose rouvrirait quinze minutes de compilation. `bench` n'a pas de
# verdict binaire — c'est voulu, la règle du projet impose de rejouer les
# benchmarks à chaque issue, et les chaîner ici est ce qui le garantit plutôt
# que de compter sur la mémoire de qui ouvre la pull request.
.PHONY: check-local
check-local: ## Unique commande locale à lancer avant une pull request
	@./src/scripts/gate.sh check-local

.PHONY: verify-gates
verify-gates: ## Prouve que chaque porte se referme sur son défaut (cinquante et une preuves)
	@./src/scripts/verify-gates.sh

.PHONY: changelog
changelog: ## Régénère CHANGELOG.md depuis l'historique des commits
	$(call require,git-cliff)
	$(call step,"changelog")
	@git-cliff --output CHANGELOG.md

.PHONY: clean
clean: ## Supprime les répertoires de construction
	@rm -rf build
	@printf 'build/ supprimé\n'
