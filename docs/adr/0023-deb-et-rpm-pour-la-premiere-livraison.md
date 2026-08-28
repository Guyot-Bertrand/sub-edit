# 0023 — Deux paquets natifs, `.deb` et `.rpm`, pour la première livraison

Statut : acceptée — 2026-08-28
Décidée au cadrage de la phase 7 (#237).

## Contexte

La phase 7 est celle qui rend l'outil installable. Le projet n'a aujourd'hui
**aucune règle `install()`** — vérifié, aucune occurrence dans l'arbre — et
`docs/manual/subedit-cli/installation.md` dit encore que l'outil se lance depuis
l'arbre de construction.

La feuille de route pose la question sous la forme d'un choix entre trois
formats : Flatpak, `.deb`, AppImage. Elle a été laissée ouverte depuis le
cadrage de la phase 3, volontairement — « une cible écrite avant que le format
d'empaquetage soit tranché préjugerait de la réponse ».

### Ce que fait Gaupol

Un `Makefile` avec `DESTDIR` et `PREFIX`, qui installe le lanceur dans `BINDIR`,
les icônes sous `$DATADIR/icons/hicolor/{scalable,symbolic}/apps`, le fichier
`.desktop` dans `$DATADIR/applications`, le fichier AppStream dans
`$DATADIR/metainfo`, la page de manuel dans `$MANDIR/man1`, et les traductions
sous `LOCALEDIR`. Il appelle `update-desktop-database` seulement quand
`DESTDIR` est vide, c'est-à-dire quand l'installation n'est pas mise en scène
pour un paquet.

Et, **à côté**, un manifeste Flatpak. `PACKAGING.md` s'adresse aux empaqueteurs
de distributions et décrit deux découpages possibles.

Autrement dit : Gaupol ne choisit pas un format. Il produit une **installation
correcte**, et les formats se servent dessus.

## Décision

**Des règles `install()` dans CMake, honorant `DESTDIR` et
`CMAKE_INSTALL_PREFIX`, puis deux paquets natifs produits par CPack : un `.deb`
et un `.rpm`.**

Flatpak et AppImage ne sont pas refusés : ils sont **différés**, et leur raison
est écrite plus bas.

L'ordre a un sens et il est la moitié de la décision : **l'installation
d'abord, les formats ensuite.** C'est ce que Gaupol montre, et c'est ce qui rend
le choix de format réversible — un format de plus se sert de la même
installation.

## Pourquoi deux, et pourquoi ces deux-là

**Deux familles couvrent l'essentiel du bureau Linux**, et ce sont celles-là :
Debian et ses dérivées d'un côté, Fedora et ses parentes de l'autre. Livrer pour
une seule reviendrait à choisir entre deux moitiés d'utilisateurs sur un critère
qui n'a rien à voir avec le logiciel.

**Le second paquet est ce qui met la décision à l'épreuve.** « L'installation
d'abord, les formats ensuite » est une affirmation tant qu'un seul format
l'exerce. Un second format construit depuis les mêmes règles `install()` la
vérifie : **s'il demandait de retoucher l'installation, c'est que l'installation
était fausse.** C'est la meilleure raison d'en faire deux tout de suite plutôt
qu'un maintenant et un plus tard.

**Ubuntu reste la cible déclarée du développement** — `setup-toolchain.sh` le
dit en première ligne, l'ADR 0003 cible Linux d'abord, et la chaîne d'outils est
décrite en paquets APT. C'est la plate-forme sur laquelle le paquet sera vérifié
le plus complètement, et c'est une asymétrie qu'il faut écrire plutôt que
laisser croire à une parité qui n'existe pas — voir « ce qu'on ne saura pas
prouver ».

**CPack produit les deux depuis les mêmes règles `install()`**, donc sans
description séparée à tenir en phase. Un paquet qui dérive de l'installation ne
peut pas diverger d'elle.

## Ce qui diffère réellement entre les deux, et c'est le seul vrai travail

Pas la liste des fichiers : elle vient de l'installation, une fois pour les
deux. **Les dépendances déclarées**, en revanche, ne se traduisent pas :
`CPACK_DEBIAN_PACKAGE_DEPENDS` et `CPACK_RPM_PACKAGE_REQUIRES` nomment les mêmes
bibliothèques par des noms de paquets différents, et ces noms sont propres à
chaque famille — Qt 6 et libmpv en premier lieu.

C'est là que le paquet peut être faux sans que rien ne le montre à la
construction : un nom de dépendance erroné produit un `.rpm` parfaitement valide
qui refuse de s'installer, ou pire, qui s'installe et ne se lance pas.

## Ce qu'on ne saura pas prouver, et qui doit être écrit

**Un `.rpm` construit sur Ubuntu ne peut pas y être installé.** `rpmbuild` sait
le produire — le paquet `rpm` le fournit —, mais l'éprouver demande une machine
Fedora, que ni la porte ni la CI n'ont.

Ce qui reste vérifiable sans Fedora, et qui n'est pas rien :

| Vérifiable ici | Non vérifiable ici |
| :------------- | :----------------- |
| la liste des fichiers, par `rpm -qlp` | que le paquet s'installe |
| les dépendances déclarées, par `rpm -qp --requires` | que les noms de dépendances existent chez Fedora |
| que les chemins sont ceux de `GNUInstallDirs` | que le binaire se lance une fois installé |

**La conséquence est une asymétrie assumée** : le `.deb` est prouvé de bout en
bout par #239, le `.rpm` est prouvé jusqu'à son contenu. Le dire vaut mieux que
livrer les deux en laissant croire qu'ils ont été éprouvés pareil.

Ce qui refermerait l'écart : un conteneur Fedora. C'est une dépendance
d'exécution lourde pour la porte, et le quota d'Actions du mois est déjà épuisé
(#232). À rouvrir quand quelqu'un installera vraiment le `.rpm`.

## Pourquoi pas Flatpak, pour l'instant

C'est le meilleur format pour une diffusion large, et c'est précisément
pourquoi il ne convient pas ici : **il demande des décisions qui n'ont rien à
voir avec cette phase.** Un choix de runtime et de sa version, un manifeste qui
redéclare toutes les dépendances, un bac à sable de construction, et surtout la
question de l'accès aux fichiers de l'utilisateur et à la vidéo — un éditeur de
sous-titres ouvre des fichiers arbitraires et joue un film à côté.

**libmpv rend cette question plus lourde, pas plus légère.** L'ADR 0020 l'a fait
entrer par `pkg-config` ; dans un bac à sable, c'est le runtime qui décide de ce
que le lecteur peut décoder.

Et deux paquets natifs réduisent son urgence : ce que Flatpak apporterait en
plus est la couverture des distributions qu'on ne nomme pas. C'est un vrai
besoin, ce n'est pas celui d'une première livraison.

## Pourquoi pas AppImage

Il embarque tout, Qt compris, ce qui en fait le plus gros artefact et le moins
relisible : ce qu'on livre n'est plus ce que le dépôt décrit mais un
empaquetage de l'arbre de construction d'une machine. Pour une première
livraison dont l'objet est justement de vérifier que l'installation est juste,
c'est le format qui le vérifie le moins.

## Ce que l'installation doit poser, et qui n'existe pas encore

Le choix des formats ne dispense d'aucun de ces fichiers, et c'est pourquoi ils
appartiennent à l'installation plutôt qu'aux paquets :

| Fichier | Où |
| :------ | :- |
| `subedit-gui`, `subedit-cli` | `${CMAKE_INSTALL_BINDIR}` |
| une icône | `${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps` |
| un `.desktop` | `${CMAKE_INSTALL_DATADIR}/applications` |
| un fichier AppStream | `${CMAKE_INSTALL_DATADIR}/metainfo` |
| le manuel | `${CMAKE_INSTALL_DATADIR}/subedit/manual` |
| une page de manuel `subedit-cli.1` | `${CMAKE_INSTALL_MANDIR}/man1` |

Ces chemins viennent de `GNUInstallDirs`, et non de constantes écrites à la
main : c'est ce qui fait qu'un empaqueteur peut les déplacer.

## Conséquences

**Trois dépendances d'outillage apparaissent**, et l'issue #239 les porte :
`desktop-file-validate` et `appstreamcli`, qui valident deux des fichiers
ci-dessus, et **`rpm`, qui fournit `rpmbuild`** et sans lequel le générateur RPM
de CPack ne produit rien. Les deux premiers sont présents sur la machine de
développement par la base de bureau d'Ubuntu ; `rpm` ne l'est pas. Aucun des
trois n'est déclaré dans `setup-toolchain.sh`, et l'ADR 0004 demande qu'une
dépendance se justifie.

**Le manuel installé décide de ce qu'ouvre `Help ▸ Manual`**, et donc du moment
où cette entrée peut être rallumée — après l'empaquetage, ce qui la place en
dernier dans la phase.

**Les exemples du manuel deviennent vrais.** Ils montrent `$ subedit-cli` comme
si l'outil était dans le `PATH`, ce qui ne l'est qu'à partir d'ici. La
génération, elle, ne change pas : `generate-manual.sh` met `build/dev/bin` en
tête du `PATH`, et cela doit le rester — sans quoi le manuel décrirait la
version installée plutôt que celle du dépôt.

**Le manuel doit dire ce qui est éprouvé et ce qui ne l'est pas.** Un
utilisateur Fedora a le droit de savoir que son paquet est produit par les mêmes
règles que l'autre et vérifié moins loin.

**Le jour où Flatpak est repris, rien de ceci n'est perdu.** Un manifeste se
sert des règles `install()`, comme celui de Gaupol se sert de son `Makefile`.
