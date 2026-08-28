# 0023 — Un paquet `.deb` pour la première livraison

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
`CMAKE_INSTALL_PREFIX`, et un paquet `.deb` produit par CPack pour la première
livraison.**

Flatpak et AppImage ne sont pas refusés : ils sont **différés**, et leur raison
est écrite plus bas.

L'ordre a un sens et il est la moitié de la décision : **l'installation
d'abord, le format ensuite.** C'est ce que Gaupol montre, et c'est ce qui rend
le choix de format réversible — un second format se sert de la même
installation.

## Pourquoi `.deb`

**C'est la cible déclarée du projet.** `setup-toolchain.sh` dit « Cible Ubuntu
24.04 » en première ligne, l'ADR 0003 cible Linux d'abord, et toute la chaîne
d'outils est décrite en paquets APT. Livrer d'abord pour la plate-forme qu'on
déclare est le plus petit pas honnête.

**C'est le format que la machine sait déjà produire.** `dpkg-deb` est présent ;
`flatpak-builder` et `appimagetool` ne le sont pas. Ce n'est pas un argument de
fond, mais c'est une dépendance de moins à justifier sous l'ADR 0004 pour une
première livraison.

**CPack le produit depuis les règles `install()`**, donc sans description
séparée à tenir en phase. Un paquet qui dérive de l'installation ne peut pas
diverger d'elle.

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

Ce n'est pas une phase de finitions, c'est une phase à elle seule. La différer
est un choix, pas un oubli.

## Pourquoi pas AppImage

Il embarque tout, Qt compris, ce qui en fait le plus gros artefact et le moins
relisible : ce qu'on livre n'est plus ce que le dépôt décrit mais un
empaquetage de l'arbre de construction d'une machine. Pour une première
livraison dont l'objet est justement de vérifier que l'installation est juste,
c'est le format qui le vérifie le moins.

## Ce que l'installation doit poser, et qui n'existe pas encore

Le choix du format ne dispense d'aucun de ces fichiers, et c'est pourquoi ils
appartiennent à l'installation plutôt qu'au paquet :

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

**Deux dépendances d'outillage apparaissent**, et l'issue #239 les porte :
`desktop-file-validate` et `appstreamcli`, qui valident deux des fichiers
ci-dessus. Ils sont présents sur la machine de développement par la base de
bureau d'Ubuntu, mais absents de `setup-toolchain.sh`.

**Le manuel installé décide de ce qu'ouvre `Help ▸ Manual`**, et donc du moment
où cette entrée peut être rallumée — après l'empaquetage, ce qui la place en
dernier dans la phase.

**Les exemples du manuel deviennent vrais.** Ils montrent `$ subedit-cli` comme
si l'outil était dans le `PATH`, ce qui ne l'est qu'à partir d'ici. La
génération, elle, ne change pas : `generate-manual.sh` met `build/dev/bin` en
tête du `PATH`, et cela doit le rester — sans quoi le manuel décrirait la
version installée plutôt que celle du dépôt.

**Le jour où Flatpak est repris, rien de ceci n'est perdu.** Un manifeste se
sert des règles `install()`, comme celui de Gaupol se sert de son `Makefile`.
