# Journal des modifications

Les changements notables de subedit. Format inspiré de
[Keep a Changelog](https://keepachangelog.com/fr/1.1.0/), versions suivant
[SemVer](https://semver.org/lang/fr/).

Ce fichier est généré par `make changelog` depuis l'historique des commits :
ne pas l'éditer à la main.

## Non publié

### Ajouts

- **core** — Mise à l'échelle des positions par un rationnel exact
- **core** — Sélection, insertion et suppression dans le projet
- **core** — Session, genre de commande et politique d'ordre
- **core** — Commandes de modification d'un texte et d'une position
- **core** — Commandes d'insertion et de suppression
- **core** — Commande de décalage des positions
- **core** — Transformation des positions par deux repères

### Construction

- Date de génération, garde-fou de version, changelog allégé

### Documentation

- Spec de la phase 2
- Noter la stratégie de version dans les instructions du projet
- Deux ADR pour la phase 2
- Incrémenter le patch à chaque version de PR
- Manuel à jour en fin de ticket, bump de version au dernier moment

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


