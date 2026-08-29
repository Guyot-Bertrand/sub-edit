# Installation

**Cette page vaut pour les deux programmes** — `subedit-cli` et `subedit-gui`.
Ils sont construits et installés ensemble.

subedit n'est pas encore empaqueté : il n'existe ni `.deb` ni `.rpm`. L'outil se
construit depuis les sources, et **s'installe** ensuite où l'on veut.

## Prérequis

| Ce qu'il faut | Pourquoi | Debian, Ubuntu |
| :------------ | :------- | :------------- |
| CMake ≥ 3.28 | la construction | `cmake` |
| un compilateur C++23 — GCC 13 convient | — | `g++` |
| `make`, `pkg-config` | la construction | `make`, `pkg-config` |
| Qt 6, module `Widgets` | la fenêtre | `qt6-base-dev` |
| `libmpv` | le lecteur intégré | `libmpv-dev` |

**Les deux dernières sont exigées même pour ne construire que
`subedit-cli`** : la configuration CMake les cherche pour tout le projet, et
s'arrête si elles manquent.

**Une connexion réseau au premier `cmake`** : la bibliothèque de tests Catch2
est récupérée depuis GitHub à la configuration. Elle n'est plus retéléchargée
ensuite. Pour construire l'outil seul, sans réseau, voir plus bas.

### Ce qui n'est pas exigé

| Ce qui est facultatif | Ce qu'on perd sans lui |
| :-------------------- | :--------------------- |
| `ffmpeg`, pour son `ffprobe` | la fenêtre ne propose plus la cadence que le film déclare — [le détail](../subedit-gui/video.md#ffmpeg-nest-pas-requis) |
| un serveur graphique | rien pour `subedit-cli` ; `subedit-gui` en a besoin pour s'afficher |

## Construire

```bash
git clone git@github.com:Guyot-Bertrand/sub-edit.git subedit
cd subedit
make build
```

Les binaires sont produits dans `build/dev/bin/` — `subedit-cli` et
`subedit-gui`.

Pour une version optimisée :

```bash
make release
```

Les deux binaires sont alors dans `build/release/bin/` — `subedit-cli` et
`subedit-gui`. La cible ne construit rien d'autre : ni le banc de mesures, ni le
harnais de bout en bout, qui ont leurs propres cibles.

**Préférer cette cible aux deux commandes `cmake` équivalentes**, qui n'ont pas
de quoi savoir combien de processus l'optimisation entre modules a le droit de
lancer : à chaque édition de liens, elle en démarre autant qu'il y a de cœurs.
`make release` lui passe `JOBS`, comme au reste de la construction.

## Installer

```bash
make release
cmake --install build/release --prefix ~/.local
```

Ce qui est déposé, aux chemins que `GNUInstallDirs` fixe pour le préfixe
choisi :

| Fichier | Où |
| :------ | :- |
| `subedit-cli`, `subedit-gui` | `<préfixe>/bin` |
| le manuel, en Markdown | `<préfixe>/share/subedit/manual` |

Avec `--prefix ~/.local`, les deux binaires atterrissent dans `~/.local/bin`,
qui est dans le `PATH` de la plupart des distributions. Un préfixe système —
`/usr/local`, par exemple — demande les droits correspondants.

**Ce que l'installation ne pose pas encore** : l'entrée de menu de bureau,
l'icône et les métadonnées de logithèque. Elles viennent avec l'empaquetage, et
d'ici là `subedit-gui` se lance depuis un terminal comme n'importe quelle autre
commande.

## Construire l'outil seul

Les tests sont construits par défaut, et ce sont eux qui réclament le réseau —
`make release` ne les construit pas, mais il configure le projet, et c'est la
configuration qui va chercher Catch2. Les désactiver une fois suffit : le
réglage reste dans le cache CMake, et les constructions suivantes le gardent.

```bash
cmake --preset release -DSUBEDIT_BUILD_TESTS=OFF
make release
```

Cela ne dispense de rien pour qui contribue : la porte de qualité, elle, les
exige.

## Si la compilation échoue au moment de l'édition des liens

Une avalanche de « référence indéfinie vers `std::cout` » sur du code valide
indique en général que l'alternative `c++` du système pointe sur `gcc` au lieu
de `g++` : le C++ compile, mais la bibliothèque standard n'est pas liée.

```bash
ls -l /etc/alternatives/c++                                       # vérifier
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100   # corriger
CXX=g++ make build                                                # contourner
```

La configuration CMake détecte ce cas et s'arrête avec ce message plutôt que de
laisser l'édition des liens échouer.
