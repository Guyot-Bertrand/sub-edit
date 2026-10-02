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
| les motifs de correction de Gaupol | `/usr/share/subedit/patterns` |
| les pages de manuel de `subedit-cli` et `subedit-gui` | `/usr/share/man/man1` |

`ffmpeg` est **recommandé et non requis** par les deux paquets : sans lui, la
fenêtre cesse seulement de proposer la cadence que le film déclare — voir
[ce que `ffmpeg` change](../subedit-gui/video.md#ffmpeg-nest-pas-requis).

### Ce que les paquets exigent de la distribution

**Installer avec `apt` ou `dnf`, jamais avec `dpkg -i` ni `rpm -i`.** Ces deux
commandes posent le paquet sans résoudre ses dépendances : elles se contentent
de lister celles qui manquent, et s'arrêtent. C'est ce que `rpm -i` répond sur
une Fedora qui a pourtant ce qu'il faut à portée de `dnf` :

```console
$ rpm -i subedit-0.13.0-1.fc43.x86_64.rpm
erreur : Dépendances requises :
        libicui18n.so.77()(64bit) est nécessaire pour subedit-0.13.0-1.fc43.x86_64
        libicuuc.so.77()(64bit) est nécessaire pour subedit-0.13.0-1.fc43.x86_64
```

`sudo dnf install ./subedit-<version>-1.fc43.x86_64.rpm` va chercher ces
bibliothèques, là où elles existent.

**Le `.rpm` à prendre est celui de sa Fedora** : le nom du fichier porte la
version, `fc42`, `fc43`, `fc44`. Chacun est construit sur cette Fedora, contre
son ICU — `cat /etc/fedora-release` dit lequel prendre. Un `.rpm` d'une autre
Fedora demande un ICU que la vôtre n'a pas.

| Dépendance | `.deb` | `.rpm` | Contrainte |
| :--------- | :----- | :----- | :--------- |
| Qt 6 : `Widgets`, `Gui`, `Core` | `libqt6widgets6`, `libqt6gui6`, `libqt6core6` | `qt6-qtbase-gui` | **≥ 6.4**, pour la fenêtre |
| libmpv | `libmpv2` ou `libmpv1` | `mpv-libs` | le binaire lie `libmpv.so.2` |
| Enchant 2 | `libenchant-2-2` | `enchant2` | le correcteur orthographique |
| **ICU** | **`libicu74`** | `libicu`, soit les `libicuuc.so.N` et `libicui18n.so.N` **de la Fedora qui l'a construit** | **la version qu'a la Fedora du `.rpm`**, exactement — voir ci-dessous |
| glibc et libstdc++ | — | — | **assez récentes** : celles d'Ubuntu 24.04 pour le `.deb`, celles de la Fedora du `.rpm` pour celui-ci |

**ICU est la contrainte qui compte, et elle n'est pas assouplissable en
changeant une ligne du paquet.** Les symboles d'ICU portent le numéro de sa
version majeure (`u_strToUTF8_74`) : un binaire construit contre ICU 74 ne
sait pas se lier à ICU 76 ou 77. Une dépendance écrite plus large ferait
installer le paquet, puis échouer au lancement — la déclarer exacte est ce qui
reste honnête. **C'est pourquoi le `.rpm` n'est pas construit une fois, mais sur
chaque Fedora publiée**, contre son ICU ; le `.deb`, lui, l'est sur Ubuntu 24.04.

| Distribution | Ce que cela donne |
| :----------- | :---------------- |
| Ubuntu 24.04 | le `.deb` s'installe : ICU y est en version 74 |
| Fedora 42, 43, 44 | le `.rpm` de sa version (`…fc42…`, `…fc43…`, `…fc44…`) s'installe **par `dnf`**, construit contre l'ICU de cette Fedora |
| Fedora rawhide (la prochaine), les autres distributions à `.rpm` | **non** : aucun `.rpm` n'est construit pour elles — construire depuis les sources |
| toute distribution Debian ou Ubuntu dont ICU n'est pas en version 74 | **non** : le `.deb` exige `libicu74` — construire depuis les sources |

**La construction depuis les sources est le repli, et elle n'a pas cette
contrainte** : elle se lie à l'ICU de la machine, quelle que soit sa version.
C'est aussi ce qu'il faut sur une distribution dont la glibc est plus ancienne
que celle des paquets.

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

**Le contrôle Fedora éprouve le `.rpm` construit sur Ubuntu, pas ceux qu'on
publie.** Cette image n'a pas ICU 74 en propre : le paquet s'y installe parce que
Fedora garde un `libicu74` de compatibilité, et le contrôle serait vert tant que
ce paquet existe. Les `.rpm` publiés sont construits sur leur propre Fedora, et ne
dépendent plus de cette compatibilité. Voir
[ce que les paquets exigent](#ce-que-les-paquets-exigent-de-la-distribution).

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
| CLI11 | la lecture de la ligne de commande | `libcli11-dev` |
| Qt 6 ≥ 6.4, module `Widgets` | la fenêtre | `qt6-base-dev` |
| `libmpv` | le lecteur intégré | `libmpv-dev` |
| ICU | la lecture des encodages | `libicu-dev` |
| Enchant 2 | la bibliothèque du correcteur orthographique | `libenchant-2-dev` |

**Les quatre dernières sont exigées même pour ne construire que
`subedit-cli`** : la configuration CMake les cherche pour tout le projet, et
s'arrête si elles manquent. ICU et Enchant, elles, le sont doublement — l'exécutable les charge, l'une
pour convertir les encodages, l'autre pour le correcteur orthographique — et
ce sont les seules bibliothèques tierces qu'il charge à l'exécution.

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
| les motifs de correction de Gaupol | `<préfixe>/share/subedit/patterns` |
| les pages de manuel des deux binaires | `<préfixe>/share/man/man1` |

**Ce sont les mêmes fichiers que les paquets déposent** : ils en sortent,
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

### Si l'installation du paquet se plaint de `libicu`

```console
erreur : Dépendances requises :
        libicuuc.so.74()(64bit) est nécessaire pour subedit-<version>.x86_64
```

Trois causes, qui se distinguent par la commande tapée et par le nom du fichier :

- **`rpm -i`** : il ne résout rien. Réessayer avec
  `sudo dnf install ./subedit-<version>.fcNN.x86_64.rpm` ;
- **un `.rpm` sans `fcNN` dans son nom**, ou d'une autre Fedora que la vôtre :
  il demande l'ICU de la machine qui l'a construit. Prendre celui dont `fcNN`
  est la sortie de `cat /etc/fedora-release` ;
- **aucun `.rpm` pour votre version** : Fedora rawhide ou une autre distribution.
  Aucun paquet de ce dépôt ne s'y installera ;
  [construire depuis les sources](#construire-depuis-les-sources).

