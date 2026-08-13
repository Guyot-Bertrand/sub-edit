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

# git-cliff s'installe dans ~/.local/bin, que ~/.profile n'ajoute au PATH qu'à
# l'ouverture de session suivante. On ne dépend pas de la configuration du
# shell de l'utilisateur.
export PATH := $(HOME)/.local/bin:$(PATH)

# Ninja est préféré, sans être requis : le figer dans les presets rendrait le
# projet inconstructible sur une machine qui ne l'a pas encore.
export CMAKE_GENERATOR ?= $(shell command -v ninja >/dev/null 2>&1 && echo Ninja || echo "Unix Makefiles")

SOURCES := $(shell find src -name '*.cpp' -o -name '*.hpp' 2>/dev/null)

# libstdc++ garde <expected> derrière __cpp_concepts >= 202002L, valeur que
# Clang 18 ne déclare pas : il ne voit alors pas std::expected. On prend donc la
# version la plus récente disponible, sans exiger qu'elle soit installée.
CLANG_TIDY := $(shell command -v clang-tidy-20 2>/dev/null \
	|| command -v clang-tidy-19 2>/dev/null \
	|| command -v clang-tidy 2>/dev/null || echo clang-tidy)

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
	$(call step,"compilation (dev)")
	@cmake --preset dev
	@cmake --build --preset dev -j $(JOBS)

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
# `JOBS` gouverne aussi le LTO. Sans cela, l'optimisation entre modules se
# déclenche en `-flto=auto`, c'est-à-dire autant de processus que de cœurs, à
# chaque édition de liens — un parallélisme qui n'apparaît dans aucun `-j` et
# qui sature une machine sur laquelle on fait autre chose.
bench: ## Exécute les benchmarks en release et verse les chiffres au journal
	$(call step,"benchmarks (release)")
	@cmake --preset release -DSUBEDIT_LTO_JOBS=$(JOBS)
	@cmake --build --preset release -j $(JOBS) --target subedit_core_bench
	@./build/release/bin/subedit_core_bench \
		--reporter console \
		--reporter xml::out=build/release/bench.xml
	@./src/scripts/record-bench.sh --xml build/release/bench.xml --mode Release

.PHONY: format
format: ## Applique clang-format
	$(call require,clang-format)
	$(call step,"formatage")
	@clang-format -i $(SOURCES)

.PHONY: format-check
format-check: ## Vérifie le format sans modifier
	$(call require,clang-format)
	$(call step,"format")
	@clang-format --dry-run --Werror $(SOURCES)

# compile_commands.json porte les drapeaux de GCC, dont certains n'existent pas
# chez Clang : sans -Wno-unknown-warning-option, clang-tidy échoue sur
# -Wuseless-cast et consorts avant d'avoir analysé la moindre ligne.
.PHONY: tidy
tidy: ## Exécute clang-tidy
	$(call require,$(CLANG_TIDY))
	$(call step,"analyse statique — $(notdir $(CLANG_TIDY))")
	@cmake --preset dev >/dev/null
	@find src -name '*.cpp' -print0 \
		| xargs -0 -P $(JOBS) -I{} \
			$(CLANG_TIDY) -p build/dev --quiet --extra-arg=-Wno-unknown-warning-option {}

.PHONY: arch
arch: ## Vérifie les invariants d'architecture
	$(call step,"invariants d architecture")
	@./src/scripts/check-architecture.sh

.PHONY: parallelism
parallelism: ## Vérifie qu'aucun parallélisme ne contourne $(JOBS)
	$(call step,"parallélisme maîtrisé")
	@./src/scripts/check-parallelism.sh

.PHONY: requirements
requirements: ## Confronte le registre d'exigences aux tests de bout en bout
	$(call step,"exigences")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit_e2e_test
	@./src/scripts/check-requirements.sh

# Ne construit et n'exécute que le harnais de bout en bout, filtré par
# l'étiquette CTest `e2e` plutôt que par nom de test — un nom de test unitaire
# qui s'en approcherait ne le tromperait pas. Partage le build release avec
# `make bench` : la seconde invocation ne recompile rien.
.PHONY: e2e
e2e: ## Exécute uniquement les tests de bout en bout (release)
	$(call step,"tests de bout en bout (release)")
	@cmake --preset release -DSUBEDIT_LTO_JOBS=$(JOBS)
	@cmake --build --preset release -j $(JOBS) --target subedit_e2e_test
	@ctest --preset release -L e2e

.PHONY: asan
asan: ## Exécute les tests sous ASan et UBSan
	$(call step,"tests sous sanitizers")
	@cmake --preset asan
	@cmake --build --preset asan -j $(JOBS)
	@ctest --preset asan

.PHONY: coverage
coverage: ## Mesure la couverture des bibliothèques
	$(call require,gcovr)
	$(call step,"couverture")
	@cmake --preset coverage
	@cmake --build --preset coverage -j $(JOBS)
	# Les .gcda d'une exécution antérieure survivent à la recompilation et
	# gcov les fusionne : un fichier modifié depuis produirait un taux de
	# couverture faux, sans autre signe qu'un avertissement noyé dans la sortie.
	@find build/coverage -name '*.gcda' -delete
	@ctest --preset coverage
	@mkdir -p build/coverage-report
	@gcovr --root . build/coverage/src \
		--filter 'src/lib/' \
		--exclude-unreachable-branches --exclude-throw-branches \
		--print-summary \
		--json-summary build/coverage-report/summary.json \
		--html-details build/coverage-report/index.html \
		--txt build/coverage-report/summary.txt
	@printf 'rapport : build/coverage-report/index.html\n'
	@./src/scripts/check-coverage.sh --summary build/coverage-report/summary.json

.PHONY: ratchet
ratchet: ## Enregistre la couverture mesurée comme nouveau cliquet
	$(call step,"cliquet de couverture")
	@./src/scripts/check-coverage.sh \
		--summary build/coverage-report/summary.json --record

.PHONY: check
check: ## Porte de qualité — format, warnings, tidy, tests sous ASan, couverture
	@printf '$(BOLD)porte de qualité$(RESET)\n'
	@$(MAKE) --no-print-directory format-check
	@$(MAKE) --no-print-directory arch
	@$(MAKE) --no-print-directory build
	@$(MAKE) --no-print-directory tidy
	@$(MAKE) --no-print-directory asan
	@$(MAKE) --no-print-directory coverage
	@printf '$(GREEN)✓ porte franchie$(RESET)\n'

# `check` est ce que la CI exécute — .github/workflows/ci.yml n'appelle que
# cette cible, rien d'autre. Tout ce qui y entre gate donc chaque push, de
# tout le monde ; on n'y ajoute rien à la légère.
#
# `check-local` est l'unique commande à lancer avant d'ouvrir une pull
# request : elle enchaîne tout ce qu'on veut voir passer en local sans le
# voir gater la CI. L'ordre va du moins cher au plus cher, pour qu'un échec
# coûte des secondes plutôt que la totalité de la chaîne : parallélisme
# maîtrisé (un grep, sous la seconde), exigences (compilation incrémentale
# dev), tests de bout en bout (build release), puis benchmarks. `parallelism`
# passe en premier précisément parce qu'elle ne construit rien — la faire
# attendre derrière `requirements`, qui compile, coûterait à un `-j 8` codé en
# dur le temps d'un build entier avant qu'on l'entende. `bench` n'a pas de
# verdict binaire — c'est voulu, la règle du projet impose de rejouer les
# benchmarks à chaque issue, et les chaîner ici est ce qui le garantit plutôt
# que de compter sur la mémoire de qui ouvre la pull request.
.PHONY: check-local
check-local: ## Unique commande locale à lancer avant une pull request
	@$(MAKE) --no-print-directory parallelism
	@$(MAKE) --no-print-directory requirements
	@$(MAKE) --no-print-directory e2e
	@$(MAKE) --no-print-directory bench

.PHONY: verify-gates
verify-gates: ## Prouve que chaque porte se referme sur son défaut (onze)
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
