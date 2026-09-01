# Installation

**Cette page vaut pour les deux programmes** — `subedit-cli` et `subedit-gui`.
Ils sont construits et installés ensemble.

Deux chemins : **un paquet natif**, `.deb` ou `.rpm`, ou bien **la construction
depuis les sources**, qui s'installe ensuite où l'on veut.

## Les paquets

Les deux sont produits par CPack depuis **les mêmes règles d'installation** :
ils déposent les mêmes fichiers aux mêmes endroits, et ne diffèrent que par les
noms des paquets dont ils dépendent — les mêmes bibliothèques s'appellent
autrement chez Debian et chez Fedora.

```bash
sudo apt install ./subedit_<version>_amd64.deb      # Debian, Ubuntu
sudo dnf install ./subedit-<version>.x86_64.rpm     # Fedora, et parentes
```

| Ce que le paquet dépose | Où  |
| :---------------------- | :-- |
| `subedit-cli`, `subedit-gui` | `/usr/bin` |
| l'entrée de menu, l'icône, les métadonnées de logithèque | `/usr/share/applications`, `/usr/share/icons`, `/usr/share/metainfo` |
| ce manuel, en Markdown | `/usr/share/subedit/manual` |
| les pages de manuel de `subedit-cli` et `subedit-gui` | `/usr/share/man/man1` |

`ffmpeg` est **recommandé et non requis** par les deux paquets : sans lui, la
fenêtre cesse seulement de proposer la cadence que le film déclare — voir
[ce que `ffmpeg` change](../subedit-gui/video.md#ffmpeg-nest-pas-requis).

### Ce qui est éprouvé de chacun, et ce qui ne l'est pas

**Les deux ne sont pas vérifiés aussi loin, et il vaut mieux le dire que laisser
croire à une parité qui n'existe pas.** Le développement se fait sur Ubuntu ; un
`.rpm` construit là ne peut pas y être installé. Il l'est ailleurs : sur une
Fedora en conteneur, une fois par semaine.

| | `.deb` | `.rpm` |
| :--- | :----- | :----- |
| la liste des fichiers | vérifiée | vérifiée, et **confrontée à celle du `.deb`** |
| les dépendances déclarées | vérifiées présentes | vérifiées présentes |
| que les noms de dépendances existent dans la distribution | oui, ce sont ceux d'Ubuntu | **oui**, résolus par `dnf` |
| que le paquet s'installe | non — cela demande les droits de l'administrateur | **oui**, sur une Fedora en conteneur |
| que les binaires installés se lancent | oui, depuis un préfixe temporaire | **oui**, depuis le paquet installé |

**La confrontation des deux listes est le contrôle qui compte le plus.** Les
deux paquets sortent de la même installation : un écart entre eux serait un
défaut des règles d'installation, pas du format.

**Ce que le `.deb` ne prouve toujours pas est qu'il s'installe**, et la raison
n'a pas changé : `dpkg -i` demande les droits de l'administrateur, qu'une porte
de qualité n'a pas et ne doit pas demander. Le `.rpm`, lui, s'installe dans un
conteneur qui n'appartient à personne. Voir
[l'ADR 0023](../../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

**Flatpak et AppImage ne sont pas proposés**, et c'est un choix — pas un oubli.
Il est expliqué dans la même ADR, et il est **définitif** : une distribution qui
n'est ni de la famille Debian ni de la famille Fedora se construit depuis les
sources, ce que la section suivante décrit en entier.

## Construire depuis les sources

### Prérequis

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

#### Ce qui n'est pas exigé

| Ce qui est facultatif | Ce qu'on perd sans lui |
| :-------------------- | :--------------------- |
| `ffmpeg`, pour son `ffprobe` | la fenêtre ne propose plus la cadence que le film déclare — [le détail](../subedit-gui/video.md#ffmpeg-nest-pas-requis) |
| un serveur graphique | rien pour `subedit-cli` ; `subedit-gui` en a besoin pour s'afficher |

### Construire

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

### Installer

```bash
make release
cmake --install build/release --prefix ~/.local
```

Ce qui est déposé, aux chemins que `GNUInstallDirs` fixe pour le préfixe
choisi :

| Fichier | Où  |
| :------ | :-- |
| `subedit-cli`, `subedit-gui` | `<préfixe>/bin` |
| l'entrée de menu de bureau | `<préfixe>/share/applications` |
| l'icône | `<préfixe>/share/icons/hicolor/scalable/apps` |
| les métadonnées de logithèque | `<préfixe>/share/metainfo` |
| ce manuel, en Markdown | `<préfixe>/share/subedit/manual` |
| les pages de manuel des deux binaires | `<préfixe>/share/man/man1` |

**Ce sont les six mêmes fichiers que les paquets déposent** : ils en sortent,
plutôt que d'être décrits une seconde fois.

Avec `--prefix ~/.local`, les deux binaires atterrissent dans `~/.local/bin`,
qui est dans le `PATH` de la plupart des distributions. Un préfixe système —
`/usr/local`, par exemple — demande les droits correspondants.

**Une entrée de menu posée sous `~/.local` n'apparaît pas toujours tout de
suite** : les bureaux relisent leur cache à leur rythme. `subedit-gui` se lance
en attendant depuis un terminal, comme n'importe quelle autre commande.

`DESTDIR` est honoré, ce qu'un empaqueteur attend : une installation mise en
scène ne touche rien hors du répertoire de mise en scène.

```bash
DESTDIR=/tmp/scene cmake --install build/release --prefix /usr
```

### Construire l'outil seul

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

### Si la compilation échoue au moment de l'édition des liens

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
