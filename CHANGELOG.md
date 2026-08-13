# Journal des modifications

Les changements notables de subedit. Format inspiré de
[Keep a Changelog](https://keepachangelog.com/fr/1.1.0/), versions suivant
[SemVer](https://semver.org/lang/fr/).

Ce fichier est généré par `make changelog` depuis l'historique des commits :
ne pas l'éditer à la main.

## Non publié

### Ajouts

- **ci** — Compter les lignes non couvertes plutôt qu'un pourcentage
- **ci** — Donner une mémoire aux mesures de performance
- **ci** — Vérifier les trois obligations d'une pull request
- **doc** — Générer les exemples d'appel du manuel
- **scripts** — Élargir la portée du contrôle de parallélisme

### Construction

- Construire en Release, et y mesurer
- **ci** — Garder le contrôle des exigences hors de make check
- **build** — Plafonner JOBS à deux cœurs, contrôle mécanique à l'appui
- Restreindre le harnais e2e à asan/release, enchaîner check-local
- **build** — Câbler le cliquet et le journal dans la façade make
- **build** — Câbler make manual et manual-check dans la façade

### Corrections

- **test** — Corriger la lecture des tubes du harnais de bout en bout
- **test** — Refuser un binaire de test muet ou en échec sur --list-tags
- **test** — Ne plus lire les tags dans les diagnostics du binaire
- **ci** — Corriger COVERAGE_MIN à 99.2 pour que la porte se referme
- **scripts** — Corriger deux failles de check-parallelism.sh
- **scripts** — Retrait de commentaire sensible aux guillemets
- **build** — Ordonner check-local du moins cher au plus cher

### Documentation

- **test** — Décider la forme du registre d'exigences
- **test** — Ouvrir le registre d'exigences sur la commande de version
- **build** — Chiffrer la marge de couverture avant que la porte échoue
- **test** — Décrire le harnais et l'étape exigences de la porte
- **ci** — Dire quelles cibles verify-gates.sh éprouve désormais
- **build** — Documenter le parallélisme maîtrisé des fondations
- **doc** — Corriger le compte d'alternatives écartées de l'ADR 0014
- **doc** — Documenter la façade make et les deux gates
- **doc** — Aligner README et CONTRIBUTING sur le nouveau make test
- **scripts** — Écrire la limite du suivi des guillemets
- **doc** — Corriger les presets du harnais dans l'ADR 0014
- **doc** — Décider la forme de la mémoire des mesures
- **doc** — Décrire le cliquet et le journal, et bumper le patch
- **doc** — Aligner CONTRIBUTING et le manuel, et bumper le patch
- **doc** — Décrire les exemples déclarés, et bumper le patch
- **doc** — Bumper le patch et relever les mesures

### Intégration continue

- **test** — Refuser une exigence implémentée que rien ne démontre
- **ci** — Rendre les contrôles de pull request bloquants
- **ci** — Ne pas exiger encore les contrôles de pull request

### Tests

- **test** — Lancer le binaire réel depuis un harnais de bout en bout
- **ci** — Prouver que le contrôle des exigences se referme
- **build** — Prouver que le parallélisme codé en dur échoue
- **ci** — Prouver que les trois contrôles de pull request refusent
- **ci** — Prouver les quatre refus du générateur d'exemples
- **ci** — Prouver ce que le contrôle attrape, et ce qu'il laisse passer

## 0.2.0 — 2026-08-08

### Ajouts

- **core** — Mise à l'échelle des positions par un rationnel exact
- **core** — Sélection, insertion et suppression dans le projet
- **core** — Session, genre de commande et politique d'ordre
- **core** — Commandes de modification d'un texte et d'une position
- **core** — Commandes d'insertion et de suppression
- **core** — Commande de décalage des positions
- **core** — Transformation des positions par deux repères
- **core** — Commande de conversion de fréquence d'image

### Construction

- Date de génération, garde-fou de version, changelog allégé

### Documentation

- Spec de la phase 2
- Noter la stratégie de version dans les instructions du projet
- Deux ADR pour la phase 2
- Incrémenter le patch à chaque version de PR
- Manuel à jour en fin de ticket, bump de version au dernier moment
- Deux questions ouvertes de la conversion de fréquence, phase 5
- La vidéo associée comme source de la fréquence d'image

### Tests

- **core** — Mesurer les opérations réellement implémentées

## 0.1.0 — 2026-08-08

### Ajouts

- **core** — Types de position, conversions et horodatages
- **core** — Modèle du sous-titre et du projet
- **core** — Historique de commandes réversibles
- **core** — Socle de lecture et écriture atomique
- **core** — Lecture et écriture SubRip
- **core** — Lecture et écriture WebVTT
- **core** — Détection de format, BOM UTF-8 et fins de ligne
- **test** — Corpus de fichiers et benchmarks de référence

### Construction

- **scripts** — Verrou en lecture seule sur le dépôt de référence
- Fondations du projet
- Fermer et vérifier les portes de qualité
- Préparer l'analyse statique du C++23
- Passer à clang-tidy 20 et fiabiliser la couverture
- Limiter le parallélisme à un cœur par défaut

### Documentation

- Spec du sous-projet 0 et configuration GitHub
- Principes de conception permanents
- Feuille de route des huit phases
- Règle critique de lecture seule sur le dépôt de référence
- Préciser la disponibilité de std::expected
- Recadrer la feuille de route sur les priorités de l'utilisateur
- Décrire les rulesets avec les libellés de l'interface GitHub
- Acter la réalisation de la phase 0
- Spec de la phase 1 et cinq décisions d'architecture
- Aligner les références sur l'état réel du projet

### Intégration continue

- Corriger la vérification des messages de commit
- Passer les actions GitHub en v7
- Automatiser la configuration GitHub du dépôt
- Déclencher la porte de qualité sur toutes les branches
- Revenir au déclenchement d'origine, garder la relance manuelle


