# Les deux paquets natifs — ADR 0023, issue #244.
#
# **Ils sortent de l'installation, pas d'une description séparée.** CPack
# empaquette ce que `cmake --install` dépose ; il n'y a donc pas de liste de
# fichiers à tenir en phase, et un paquet ne peut pas diverger de ce que le
# projet installe. C'est la moitié de la décision de l'ADR : l'installation
# d'abord, les formats ensuite.
#
# **Le second format est ce qui met cette moitié à l'épreuve.** « L'installation
# d'abord » est une affirmation tant qu'un seul format l'exerce ; si le `.rpm`
# demandait de retoucher les règles `install()`, c'est qu'elles étaient fausses.
# Il ne l'a pas demandé, et `check-installation.sh` le vérifie en confrontant
# les deux listes de fichiers.
#
# **Ce qui diffère réellement entre les deux est ici, et c'est tout** : les noms
# des paquets de dépendances. Les mêmes bibliothèques s'appellent autrement chez
# Debian et chez Fedora, et c'est là que le paquet peut être faux sans que rien
# ne le montre à la construction — un nom erroné produit un paquet parfaitement
# valide qui refuse de s'installer.

include(GNUInstallDirs)

set(CPACK_PACKAGE_NAME "subedit")
set(CPACK_PACKAGE_VENDOR "Bertrand Guyot")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_CONTACT "Bertrand Guyot <bertrand.guyot.vpc@orange.fr>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# **En anglais, comme tout ce que l'utilisateur lit.** Ce résumé s'affiche dans
# une logithèque et dans `apt show` ; c'est la même phrase que le `summary`
# AppStream et que la ligne de description du binaire.
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Read, inspect and retime subtitle files")

set(CPACK_GENERATOR "DEB;RPM")

# **Chaque famille nomme ses paquets à sa façon, et on lui laisse le faire.**
# `subedit_0.7.12_amd64.deb` d'un côté, `subedit-0.7.12-1.x86_64.rpm` de
# l'autre. Sans ces deux lignes, CPack donne aux deux le même nom neutre —
# `subedit-0.7.12-Linux` — qui ne dit ni l'architecture ni la révision, et qu'un
# outil de la distribution ne reconnaît pas.
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

# ## Ce qui diffère : les dépendances
#
# **Ni l'un ni l'autre n'est déduit.** CPack sait déduire les dépendances Debian
# par `dpkg-shlibdeps`, mais seulement sur la machine où le paquet est construit
# et pour les bibliothèques que l'éditeur de liens a nommées ; il ne sait rien de
# Fedora. Les deux listes sont donc écrites, et elles sont la seule chose que ce
# fichier tient à la main.
#
# Qt 6 et libmpv en premier lieu : ce sont les deux dépendances d'exécution que
# l'ADR 0020 et la phase 5 ont fait entrer.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 (>= 6.4), libqt6gui6 (>= 6.4), libqt6core6 (>= 6.4), libmpv2 | libmpv1")
set(CPACK_DEBIAN_PACKAGE_SECTION "video")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ffmpeg")

# `ffmpeg` en recommandation et non en dépendance : l'outil fonctionne sans lui
# — la fenêtre cesse seulement de proposer la cadence que le film déclare. En
# faire une dépendance ferait tirer un paquet lourd pour une commodité, et
# mentirait sur ce que le programme exige.
#
# **Le `.rpm` n'en porte aucune, et pour deux raisons qui vont dans le même
# sens.** La première est mécanique : CPack sonde `rpm --suggests`, qui échoue
# sur le `rpmbuild` d'Ubuntu, et retire alors les quatre étiquettes de
# dépendance faible — un `SUGGESTS` posé ici serait donc écarté en silence sur
# la machine même qui construit le paquet. La seconde est de fond : `ffmpeg`
# n'est pas dans les dépôts par défaut de Fedora, et nommer le paquet qui le
# remplace serait deviner, sur une distribution que ce dépôt ne sait pas
# éprouver. Le manuel dit ce que `ffmpeg` apporte ; c'est le bon endroit pour le
# dire à qui n'a pas de gestionnaire de paquets pour l'apprendre.
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase-gui >= 6.4, mpv-libs")
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")

# ## Le préfixe d'empaquetage, écrit plutôt que supposé
#
# `/usr` est le défaut des deux générateurs, et le laisser tacite avait un coût
# précis : la liste d'exclusions ci-dessous s'écrit dans les chemins **du
# paquet**, donc préfixés, et il n'y avait rien à quoi les préfixer. Le nommer
# ici donne aux deux le même point de départ.
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

# **Les répertoires que la distribution possède déjà ne sont pas au paquet.**
# Un `.rpm` qui déclare posséder `/usr/share/applications` entre en conflit avec
# `filesystem`, qui le possède vraiment, et `dnf` refuse la transaction. Le
# générateur Debian n'a pas ce problème : un `.deb` ne possède pas ses
# répertoires.
#
# **Cette liste existait depuis #244 et n'excluait rien** — issue #266. Elle
# était écrite en chemins relatifs, `share/applications`, parce que
# `CMAKE_INSTALL_DATADIR` est relatif ; CPack compare des chemins de paquet,
# donc absolus, et n'a jamais trouvé une seule correspondance. Le `.rpm`
# possédait ses huit répertoires partagés, et l'installation échouait sur six
# conflits :
#
#     file /usr/share/applications from install of subedit-0.8.4-1.x86_64
#     conflicts with file from package filesystem-3.18-47.fc42.x86_64
#
# Rien ne pouvait le dire : `rpm -qlp` lit une liste de fichiers, pas une
# transaction, et le dépôt n'avait pas de Fedora pour en jouer une. Il en a une
# depuis `src/scripts/check-rpm.sh`.
#
# **Le conflit se joue sur le mode**, et c'est ce qui le rend invisible à l'œil :
# CPack déclare ces répertoires en 0775, `filesystem` les possède en 0755. Deux
# paquets peuvent partager un répertoire tant qu'ils le décrivent pareil ; ceux-ci
# ne le décrivaient pas pareil.
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/metainfo"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")

# **Aucun script de post-installation, et c'est une décision.** Gaupol appelle
# `update-desktop-database` depuis son Makefile, et seulement quand `DESTDIR` est
# vide — parce qu'une installation mise en scène ne doit rien toucher au-dehors.
# Un paquet natif n'a pas ce souci pour une autre raison : `dpkg` et `rpm`
# appellent eux-mêmes les déclencheurs des répertoires de bureau. Un script qui
# le referait ferait le travail deux fois, et le ferait mal le jour où la
# distribution changerait de mécanisme.

include(CPack)
