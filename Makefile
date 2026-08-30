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

# Où le preset release dépose ce qu'il produit. Nommé une fois : `make release`
# l'annonce, et l'empaquetage ira l'y chercher.
RELEASE_DIR := build/release
RELEASE_BIN := $(RELEASE_DIR)/bin

# Charge maximale sous laquelle une mesure de performance compte comme propre.
#
# Une et demie, et non deux : le seuil a été abaissé le jour même où il a été
# posé. Le relevé de la version 0.3.5, pris à 1,88 — sous le seuil, donc admis —
# a fixé un maximum de 853 µs pour l'écriture de 4000 sous-titres, là où
# l'enveloppe des dix-sept relevés précédents allait de 492 à 641. Les relevés
# pris sous 1,4 n'ont posé que des minima.
#
# Le chiffre reste une heuristique, et le journal est là pour l'affiner : chaque
# relevé porte sa charge, donc le rapport entre les deux se lit désormais.
BENCH_MAX_LOAD ?= 1.5

# git-cliff s'installe dans ~/.local/bin, que ~/.profile n'ajoute au PATH qu'à
# l'ouverture de session suivante. On ne dépend pas de la configuration du
# shell de l'utilisateur.
export PATH := $(HOME)/.local/bin:$(PATH)

# Ninja est préféré, sans être requis : le figer dans les presets rendrait le
# projet inconstructible sur une machine qui ne l'a pas encore.
export CMAKE_GENERATOR ?= $(shell command -v ninja >/dev/null 2>&1 && echo Ninja || echo "Unix Makefiles")

SOURCES := $(shell find src -name '*.cpp' -o -name '*.hpp' 2>/dev/null)

# Ce que clang-tidy analyse : ce qui a changé depuis TIDY_BASE, et rien d'autre.
#
# **C'est la seule optimisation qui compte.** Mesuré sur cette machine, ccache
# chaud : l'analyse complète prend 751 s quand tout le reste de la porte —
# format, invariants, trois constructions, tests, couverture — tient en 76.
# Environ 6 s par fichier, donc la restriction rapporte exactement en proportion
# de ce qu'une branche ne touche pas.
#
# src/scripts/tidy-scope.sh calcule la liste, en fermeture transitive des
# en-têtes, en voyant le travail non commité, et **il retombe sur la liste
# complète au moindre doute** — base introuvable, fichier gouvernant l'analyse,
# ou calcul qui ne retient rien alors que des sources ont changé. Il annonce à
# chaque fois ce qu'il a retenu et pourquoi.
#
# Pour tout analyser sans condition, vider la base :
#
#     make check TIDY_BASE=
#
# **Un seul bouton, et c'est voulu.** Il y en a eu deux un moment — une base et
# une liste de fichiers — et la liste était un piège : passée vide sur la ligne
# de commande, elle l'emportait sur le défaut du Makefile et clang-tidy
# n'analysait plus rien, en silence et en vert. Une base vide, elle, veut dire
# « tout », ce qui est le sens sûr.
TIDY_BASE ?= origin/main

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
	$(call step,"binaires (release)")
	@cmake --preset release -DSUBEDIT_LTO_JOBS=$(JOBS)
	@cmake --build --preset release -j $(JOBS) --target subedit-cli subedit-gui
	@printf '  binaires : $(RELEASE_BIN)/subedit-cli, $(RELEASE_BIN)/subedit-gui\n'

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
	@cmake --build --preset release -j $(JOBS) --target subedit_bench
	@load="$$(./src/scripts/await-quiet.sh --below $(BENCH_MAX_LOAD) || true)" ; \
	./build/release/bin/subedit_bench \
		--reporter console \
		--reporter xml::out=build/release/bench.xml && \
	./src/scripts/record-bench.sh --xml build/release/bench.xml --mode Release \
		--load "$$load" --below $(BENCH_MAX_LOAD)

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
# Le périmètre est calculé une fois, dans la recette, et non par une variable :
# une variable récursive relancerait le script à chaque emploi, donc deux fois,
# et annoncerait deux fois ce qu'elle a retenu.
.PHONY: tidy
tidy: ## Exécute clang-tidy sur ce qui a changé depuis TIDY_BASE
	$(call require,$(CLANG_TIDY))
	@cmake --preset dev >/dev/null
	@files="$$(./src/scripts/tidy-scope.sh $(TIDY_BASE))" ; \
	printf '$(BOLD)▸ analyse statique — %s — %s fichier(s)$(RESET)\n' \
		'$(notdir $(CLANG_TIDY))' "$$(printf '%s' "$$files" | grep -c . || true)" ; \
	printf '%s\n' "$$files" \
		| sed '/^$$/d' \
		| xargs -r -P $(JOBS) -I{} \
			$(CLANG_TIDY) -p build/dev --quiet --extra-arg=-Wno-unknown-warning-option {}

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
	$(call step,"fixtures vidéo")
	@./src/scripts/video-fixtures.sh --check
	$(call step,"fixtures de grille")
	@./src/scripts/subtitle-fixtures.py --check

# Le répertoire où vivent les images du manuel d'interface, et le seul.
CAPTURES := docs/manual/subedit-gui/captures

# Les captures d'écran du manuel — #199.
#
# **Le même geste que les blocs `console`**, et c'est le fond du ticket : ce qui
# empêche un manuel de mentir est qu'il soit engendré depuis le programme. Les
# blocs viennent de `subedit-cli --help` ; les images viennent de la vraie
# fenêtre, montrée sans écran.
#
# Le programme n'écrit jamais une référence — il écrit `<nom>.new.png`. Le
# comparateur décide ensuite : promouvoir, et git voit une modification qui veut
# dire quelque chose ; ou effacer, et git ne voit rien. Le garde-fou attrape ce
# qu'aucun des deux ne peut voir — une image que le manuel montre et que rien
# n'engendre, ou l'inverse.
.PHONY: screenshots
screenshots: ## Engendre les captures du manuel et promeut celles qui ont bougé
	$(call step,"captures d'écran")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit_screenshots
	@./build/dev/bin/subedit_screenshots --output-dir $(CAPTURES)
	@./src/scripts/compare-screenshots.py --dir $(CAPTURES)
	@./src/scripts/check-screenshots.py

.PHONY: screenshots-check
screenshots-check: ## Vérifie que les captures du manuel sont à jour
	$(call step,"captures d'écran")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit_screenshots
	@./build/dev/bin/subedit_screenshots --output-dir $(CAPTURES)
	@./src/scripts/compare-screenshots.py --dir $(CAPTURES) --check
	@./src/scripts/check-screenshots.py

# Construit `subedit-cli` pour les blocs `console`, et `subedit_screenshots`
# pour les images. La cible a changé de nature avec #199 : elle demande
# désormais l'interface, donc Qt. Le prix est mesuré et il est celui d'une
# compilation incrémentale — la porte de qualité construit déjà tout cela.
.PHONY: manual
manual: ## Régénère les exemples d'appel et les captures du manuel
	$(call step,"exemples du manuel")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit-cli
	@./src/scripts/generate-manual.sh
	@$(MAKE) --no-print-directory screenshots

.PHONY: manual-check
manual-check: ## Vérifie que les exemples, les captures et les renvois du manuel sont à jour
	$(call step,"exemples du manuel")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit-cli
	@./src/scripts/generate-manual.sh --check
	@$(MAKE) --no-print-directory screenshots-check
	# Le troisième filet du manuel, et le dernier qui manquait — #243. Les blocs
	# `console` viennent du binaire, les images de la vraie fenêtre ; les renvois
	# internes, eux, n'étaient vérifiés par rien, et une section renommée laisse
	# derrière elle des ancres qui s'affichent aussi proprement qu'un lien juste.
	@./src/scripts/check-manual-links.py

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

# Installe pour de vrai, ailleurs, et lance ce qui vient d'être déposé.
#
# **Aucun autre contrôle ne dit cela.** Tout le reste s'exécute depuis l'arbre de
# construction, où un fichier de données est présent par accident de
# disposition — il est là parce que le dépôt le contient, pas parce qu'une règle
# l'a copié. Le défaut ne se voit qu'à la première installation propre.
#
# Elle installe l'arbre `release`, celui que l'empaquetage prendra — ADR 0023,
# issue #244 — et `make release` ne recompile rien si `e2e` ou `bench` l'ont
# déjà construit. Le préfixe est temporaire et ne laisse rien derrière lui, ce
# que le contrôle de #226 verrait.
# Les deux paquets natifs — ADR 0023.
#
# **Une cible à part, comme `bench` et pour la même raison** : elle n'entre dans
# aucune porte. `install-check` construit déjà les deux paquets et les confronte
# — c'est là qu'ils sont éprouvés. Celle-ci existe pour les avoir sous la main,
# nommés et rangés, le jour où on les publie.
.PHONY: packages
packages: ## Produit le .deb et le .rpm depuis l'arbre release
	$(call require,cpack)
	$(call require,rpmbuild)
	@$(MAKE) --no-print-directory release
	$(call step,"paquets natifs")
	@cd build/release && cpack -G "DEB;RPM" >/dev/null
	@ls -1 build/release/subedit*.deb build/release/subedit*.rpm \
		| sed 's/^/  /'

.PHONY: install-check
install-check: ## Installe dans un préfixe temporaire et lance le binaire installé
	$(call step,"installation dans un préfixe temporaire")
	@$(MAKE) --no-print-directory release
	@./src/scripts/check-installation.sh --build-dir $(RELEASE_DIR)

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
	# Le pendant du nettoyage des .gcda plus bas, pour l'autre moitié du
	# problème. Celui-là naît d'une recompilation ; celui-ci d'un déplacement
	# de source, qui laisse un .gcno que plus rien ne rattache à une source —
	# et gcovr échoue dessus sur un message qui ne nomme ni le fichier fautif
	# ni le remède. Le script porte le raisonnement et la raison d'écarter
	# l'arbre plutôt que d'y retrancher.
	@./src/scripts/clean-stale-coverage.sh
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

# Le relevé encadre tout le reste de la porte, et non les seuls tests : ce qui
# est surveillé est ce que la porte laisse derrière elle, d'où qu'il vienne.
# Le contrôle est ici et non dans `check-local` parce que **c'est ici que les
# tests de bout en bout tournent** — le preset asan les exécute, sans filtre
# d'étiquette — et ce sont eux qui écrivent. Voir src/scripts/check-untracked.sh
# pour ce qu'il compare et pourquoi le corpus privé lui échappe.
.PHONY: untracked
untracked: ## Refuse un fichier non suivi apparu depuis le dernier relevé
	$(call step,"fichiers laissés derrière")
	@./src/scripts/check-untracked.sh --compare

# Le pendant du précédent, de l'autre côté de la frontière du dépôt. Un fichier
# écrit dans le répertoire personnel échappe entièrement à `git ls-files`, et
# c'est pourtant le même défaut. Voir src/scripts/check-config-home.sh pour ce
# qu'il surveille et pourquoi il ne surveille que ce qui porte notre nom.
.PHONY: config-home
config-home: ## Refuse qu'une exécution touche la configuration de l'utilisateur
	$(call step,"configuration de l'utilisateur")
	@./src/scripts/check-config-home.sh --compare

.PHONY: check
check: ## Porte de qualité — format, warnings, tidy, tests sous ASan, couverture
	@printf '$(BOLD)porte de qualité$(RESET)\n'
	@./src/scripts/check-untracked.sh --record
	@./src/scripts/check-config-home.sh --record
	@$(MAKE) --no-print-directory format-check
	@$(MAKE) --no-print-directory arch
	@$(MAKE) --no-print-directory build
	@$(MAKE) --no-print-directory tidy
	@$(MAKE) --no-print-directory asan
	@$(MAKE) --no-print-directory coverage
	@$(MAKE) --no-print-directory untracked
	@$(MAKE) --no-print-directory config-home
	@printf '$(GREEN)✓ porte franchie$(RESET)\n'

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
	@$(MAKE) --no-print-directory parallelism
	@$(MAKE) --no-print-directory fixtures
	@$(MAKE) --no-print-directory manual-check
	@$(MAKE) --no-print-directory requirements
	@$(MAKE) --no-print-directory e2e
	@$(MAKE) --no-print-directory install-check
	@$(MAKE) --no-print-directory bench

.PHONY: verify-gates
verify-gates: ## Prouve que chaque porte se referme sur son défaut (trente-neuf preuves)
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
