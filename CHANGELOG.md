# Journal des modifications

Les changements notables de subedit. Format inspiré de
[Keep a Changelog](https://keepachangelog.com/fr/1.1.0/), versions suivant
[SemVer](https://semver.org/lang/fr/).

Ce fichier est généré par `make changelog` depuis l'historique des commits :
ne pas l'éditer à la main.

## 0.7.0 — 2026-08-27

### Ajouts

- **core** — Déduire la fréquence d'image des positions
- **core** — Aligner les positions sur une fréquence d'image
- **core** — Ramener un fichier sur la grille qu'il a quittée
- **cli** — Inspect rapporte la grille d'images du fichier
- **cli** — Aligner sur une fréquence, ramener sur la grille
- **gui** — Le verdict dans la barre d'état, et l'analyse de la grille
- **gui** — La mesure dans le dialogue de conversion
- **gui** — Aligner sur une fréquence, ramener sur la grille

### Construction

- **scripts** — Refuser un fichier laissé par les tests

### Corrections

- **gui** — Six corrections d'usage de la fenêtre

### Documentation

- **doc** — Cadrage de la phase 16, la déduction de fréquence
- **doc** — Relecture de fin de phase 16

### Intégration continue

- Débrancher les portes, le quota du mois étant épuisé

### Remaniements

- **core** — Déduire la fréquence depuis un projet
- **gui** — Une seule liste des huit fréquences
- **core** — Ranger les anomalies parmi les analyses

### Tests

- **test** — Des fixtures de sous-titres sur grille connue
- **core** — Éprouver les seuils de la déduction à leur frontière

## 0.6.0 — 2026-08-25

### Ajouts

- **core** — Chercher un exécutable sur un chemin de recherche injecté
- **core** — Lancer un programme extérieur, et ne pas l'attendre
- **scripts** — L'allure d'un relevé, écrite dans son en-tête
- **core** — La vidéo associée à un document, et la convention de nom
- **core** — Lire la fréquence déclarée par le conteneur avec ffprobe
- **core** — La couture du lecteur, et libmpv derrière
- **core** — Avertir de ce qui dépasse la fin de la vidéo
- **gui** — Choisir une vidéo, et voir laquelle est associée
- **gui** — Jouer la vidéo dans la fenêtre, la réplique dessinée
- **gui** — Proposer la fréquence lue, signaler la fin du film

### Construction

- **build** — Ffprobe dans la chaîne d'outils et dans le cache de la CI
- **build** — Libmpv dans la chaîne d'outils, et un lecteur sans écran

### Corrections

- **gui** — Montrer la vidéo, et non un panneau vide
- **scripts** — Ne poser un extrême que sur un relevé propre

### Documentation

- **doc** — La règle du titre de pull request, dans CLAUDE.md
- **doc** — Les six étapes de check-local, dont les fixtures vidéo
- **doc** — Les programmes que les tests lancent, et le cliquet relevé
- **doc** — Le seuil de charge, mis en doute, mesuré, et gardé
- **doc** — La spec de la phase 6, autour du lecteur intégré
- **doc** — Libmpv pour le lecteur intégré, et la phase 14 réduite
- **doc** — Un gabarit pour les relectures de fin de phase
- **doc** — Relecture de fin de phase 6

### Remaniements

- **core** — Les détails d'erreur du système de fichiers, en anglais

### Tests

- **test** — Deux fixtures vidéo minuscules, vérifiables plutôt que crues
- **test** — Un faux lecteur vidéo, et l'attente bornée d'un enfant
- **test** — Montrer la fenêtre que les tests pilotent

## 0.5.0 — 2026-08-22

### Ajouts

- **build** — Restreindre clang-tidy au périmètre modifié, en local aussi
- **build** — Prouver le périmètre restreint et relever son gain
- **build** — Qt dans la chaîne d'outils, dans la CI, et sans écran
- **build** — Tenir la raison qui rend les quatre portes correctes
- **test** — Un harnais de test d'interface, en Catch2
- **ci** — Élaguer les exécutions d'Actions, et borner leur rétention
- **gui** — La fenêtre et son modèle de table
- **gui** — L'édition en place des cellules
- **gui** — Annuler et rétablir, et le libellé de l'action
- **gui** — Ouvrir, enregistrer, enregistrer sous
- **gui** — Décaler, transformer, convertir la fréquence d'image
- **gui** — Retirer les mentions pour malentendants
- **gui** — Marquer les anomalies dans la table

### Corrections

- **scripts** — Voir le travail non commité dans le périmètre de tidy
- **build** — Écarter un arbre de couverture périmé par un déplacement
- **gui** — Un dialogue qui annonce la cible, et non le fichier
- **gui** — Des boîtes modales posées sur la fenêtre

### Documentation

- Cadrer la phase 5, et ouvrir la phase 16 des fréquences d'image
- Numéroter le découpage en issues de la phase 5
- Mesurer les grilles d'images par leur phase
- **core** — Trois commentaires qui décrivaient un état révolu
- **doc** — Les écarts de la phase 5, et ses renvois qui atterrissent

### Performance

- **core** — Sélections et changements décrits par plages

### Remaniements

- **core** — Ranger le vocabulaire des formats dans le modèle
- **core** — Séparer les anomalies d'un document de sa lecture
- Une seule langue pour l'interface, et l'anglais

### Tests

- **test** — Mesurer la réinitialisation du modèle, une fois pour de bon
- **test** — Ouvrir un fichier du corpus, sur un vrai disque

## 0.4.0 — 2026-08-16

### Ajouts

- **cli** — Nommer chaque diagnostic de lecture, partout
- **scripts** — Un relevé pris sous charge ne fixe plus d'extrême
- **scripts** — N'attendre la machine que si sa charge baisse
- **scripts** — Interdire au code de lire le dépôt de référence
- **scripts** — Mesurer les mentions d'un corpus de sous-titres
- **text** — Retirer les mentions pour malentendants
- **scripts** — Calculer ce que clang-tidy doit analyser
- **core** — La commande de retrait des mentions, et sa mesure
- **cli** — La sous-commande hearing-impaired

### Corrections

- **ci** — Passer le périmètre de clang-tidy sur une seule ligne

### Documentation

- **doc** — La lecture au mieux au manuel, et bumper le patch
- **doc** — Recalculer la table des extrêmes sans le relevé chargé
- **test** — Écrire la spécification des mentions, cas par cas
- **doc** — Ne pas reprendre les motifs de Gaupol, et dire pourquoi
- **doc** — Cadrer la phase 4 et décider l'analyseur
- **doc** — Inscrire les exigences de la phase 4 et les renvois
- Préciser quand un bump de version rouvre la porte
- **doc** — Une balise ne borde pas une mention
- Inscrire le périmètre de clang-tidy et l'ordre du manuel
- **doc** — Le manuel de hearing-impaired
- **doc** — Inscrire les écarts de la phase 4 et compléter les renvois

### Intégration continue

- **ci** — Une exécution par pull request, et rien pour la documentation

### Remaniements

- **cli** — Une seule accumulation gardée pour les trois grammaires
- **cli** — Un seul câblage de destination, et un refus nommé
- **scripts** — Lire ce qui change dans les fichiers de build
- **cli** — Laisser chaque opération formuler son résultat

### Tests

- **cli** — Prouver ce que chaque diagnostic dit, et à quel niveau
- **cli** — Épingler la borne de l'accumulation des deux côtés
- **test** — Loger corpus, contentOf et Scratch dans le harnais
- **test** — Un dossier par test, et plus aucune copie de helper
- **test** — Un corpus de cas de texte, lu plutôt que compilé
- **test** — Une fixture de mesure dont le texte mord
- **test** — Écrire les cas que le cadrage et le corpus ont tranchés
- **test** — La mention à cheval manquait aux tests de bout en bout

## 0.3.0 — 2026-08-14

### Ajouts

- **ci** — Compter les lignes non couvertes plutôt qu'un pourcentage
- **ci** — Donner une mémoire aux mesures de performance
- **ci** — Vérifier les trois obligations d'une pull request
- **doc** — Générer les exemples d'appel du manuel
- **scripts** — Élargir la portée du contrôle de parallélisme
- **cli** — Ossature de la ligne de commande et sous-commande inspect
- **core** — Seconde lecture du désordre, choisie par un paramètre
- **cli** — --order-report, pour choisir la lecture du désordre
- **cli** — Convert, et la grammaire de destination
- **cli** — Shift, et la grammaire du temps
- **cli** — Transform, et la grammaire des indices
- **cli** — Framerate, et la grammaire des cadences

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
- **doc** — Cadrer la phase 3 et décider CLI11
- **doc** — Inscrire les exigences de la phase 3 et les renvois
- **doc** — Bumper le patch et relever les mesures
- **doc** — Manuel de la CLI, et bumper le patch
- **doc** — Bumper le patch et relever les mesures
- **doc** — Documenter les deux lectures, et bumper le patch
- **doc** — Manuel de convert, et bumper le patch
- **doc** — Manuel de shift, et bumper le patch
- **doc** — Manuel de transform, et bumper le patch
- **doc** — Manuel de framerate, et bumper le patch

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
- **cli** — Prouver l'ossature, inspect et les quatre niveaux
- **ci** — Interdire les noms de test qui passent pour une option
- **core** — Prouver que les deux lectures diffèrent, et où
- **cli** — Prouver les deux lectures jusqu'au binaire
- **cli** — Prouver la conversion, ses six formes et ses refus
- **cli** — Prouver le décalage, sa grammaire et ses refus
- **cli** — Prouver la transformation, ses bornes et ses refus
- **cli** — Prouver la conversion de cadence et son arrondi unique

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


