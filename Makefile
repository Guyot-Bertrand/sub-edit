# Façade ergonomique sur CMake.
#
# Ce fichier n'ajoute aucune logique de construction : il abrège des lignes de
# commande. La règle importante est `check`, décrite dans
# docs/specs/00-fondations.md.

.DEFAULT_GOAL := help
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

JOBS ?= $(shell nproc)

# git-cliff s'installe dans ~/.local/bin, que ~/.profile n'ajoute au PATH qu'à
# l'ouverture de session suivante. On ne dépend pas de la configuration du
# shell de l'utilisateur.
export PATH := $(HOME)/.local/bin:$(PATH)

# Ninja est préféré, sans être requis : le figer dans les presets rendrait le
# projet inconstructible sur une machine qui ne l'a pas encore.
export CMAKE_GENERATOR ?= $(shell command -v ninja >/dev/null 2>&1 && echo Ninja || echo "Unix Makefiles")

COVERAGE_MIN := 80
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
	@grep -hE '^[a-z-]+:.*?## ' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  $(BOLD)%-12s$(RESET) %s\n", $$1, $$2}'
	@printf '\ngénérateur : $(CMAKE_GENERATOR)\n'

.PHONY: setup
setup: ## Installe la chaîne d'outils et les hooks git
	@./src/scripts/setup-toolchain.sh

.PHONY: build
build: ## Compile le preset dev
	$(call step,"compilation (dev)")
	@cmake --preset dev
	@cmake --build --preset dev -j $(JOBS)

.PHONY: test
test: ## Compile et exécute les tests
	$(call step,"tests (dev)")
	@cmake --preset dev
	@cmake --build --preset dev -j $(JOBS)
	@ctest --preset dev

.PHONY: bench
bench: ## Exécute les benchmarks en release
	$(call step,"benchmarks (release)")
	@cmake --preset release
	@cmake --build --preset release -j $(JOBS) --target subedit_core_bench
	@./build/release/bin/subedit_core_bench

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
		--print-summary --fail-under-line $(COVERAGE_MIN) \
		--html-details build/coverage-report/index.html \
		--txt build/coverage-report/summary.txt
	@printf 'rapport : build/coverage-report/index.html\n'

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

.PHONY: verify-gates
verify-gates: ## Prouve que make check échoue sur chaque type de défaut
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
