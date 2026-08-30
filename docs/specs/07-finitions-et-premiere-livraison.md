# Phase 7 — Finitions et première livraison

Cadrage : #237. Initialisation : #236.

## Objectif

Ce qui manque pour qu'un tiers installe et utilise l'outil.

Six phases ont produit un logiciel qui marche et qui ne s'installe pas. Celle-ci
ne cherche pas une fonctionnalité de plus : elle cherche la distance entre
« ça marche chez moi » et « quelqu'un d'autre s'en sert ».

## Ce qui la distingue

**Son livrable n'est pas du code mais une installation.** Aucun test unitaire ne
dit qu'un tiers peut installer l'outil ; cela se prouve en installant, dans un
préfixe qui n'est pas l'arbre de construction. C'est l'objet de #239.

**Une bonne part de ce qu'elle change ne se voit qu'à l'œil.** Un thème clair et
un thème sombre, une géométrie retenue d'une session à l'autre, un manuel
complet : trois choses dont la vérification par défaut est « quelqu'un regarde
et trouve que c'est bien ». C'est l'objet de #199.

**Son analyse préalable est en partie déjà faite**, et c'est la première fois.
`gaupol/config.py` avait été lu au cadrage de la phase 5 ; ce cadrage l'a relu
et étendu, et le résultat est dans [l'ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md).

## L'analyse préalable, et ce qu'elle a donné

Lu dans `reference/gaupol` : `gaupol/config.py`, `gaupol/style.py`,
`gaupol/applicationman.py`, `gaupol/agents/help.py`, `gaupol/actions/edit.py`,
`gaupol/dialogs/insert.py`, `data/`, `PACKAGING.md`, `Makefile`, `flatpak/`.

Quatre résultats, dont deux corrigent ce que la feuille de route attendait.

**`style.py` ne parle pas du thème.** La feuille de route le listait au titre du
thème clair et sombre ; il ne contient que du CSS et une police d'éditeur
personnalisable. Le thème vit dans `applicationman.py`, et il y vit bien.

**Gaupol a exactement notre contrainte, une version en avance.** Son thème est
une énumération à trois valeurs — système, clair, sombre — persistée dans la
configuration. « Système » n'est résolu que sur GTK 4.20 et plus ; en dessous,
`_update_theme` **ne fait rien du tout** et laisse la boîte à outils décider.
C'est la jumelle de notre situation sous Qt 6.4.2, et c'est ce qui transforme
notre contrainte en comportement conçu plutôt qu'en impasse.

**`Help ▸ Browse documentation` ouvre une URL**, pas un fichier local.
`gaupol.util.show_uri(gaupol.DOCUMENTATION_URL)`. Le seul document que Gaupol
installe est une page de manuel, `data/gaupol.1`.

**L'installation précède le format.** Le `Makefile` installe binaire, icônes,
`.desktop`, AppStream, page de manuel et traductions avec `DESTDIR` et
`PREFIX` ; le manifeste Flatpak se sert par-dessus. Gaupol ne choisit pas un
format, il produit une installation correcte. C'est ce que reprend
[l'ADR 0023](../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

## Portée

| Dans la phase | Hors de la phase |
| :------------ | :--------------- |
| préférences persistées | les préférences que personne ne réclame |
| thème clair, sombre, système | un thème propre à l'application |
| insertion et suppression depuis la fenêtre | fusionner, scinder — phase 10 |
| manuel utilisateur complet | la traduction du manuel — phase 15 |
| règles `install()`, paquets `.deb` et `.rpm` | Flatpak, AppImage — écartés, ADR 0023 et #265 |
| `Help ▸ Manual` rallumée | une aide contextuelle |
| une seule recette d'ouverture (#154) | |

## Décisions applicables

**D1 — la configuration est un type du noyau, tolérante option par option.**
[ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md). Elle
lit et écrit par `core::FileSystem`, **reçoit** son emplacement, et n'a ni
numéro de version ni migration. Une clé inconnue est ignorée, une valeur
illisible laisse le défaut en place avec un diagnostic, et **une option restée à
son défaut est réécrite commentée**.

**D2 — deux paquets natifs, `.deb` et `.rpm`, après des règles `install()`.**
[ADR 0023](../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md). L'installation
d'abord, les formats ensuite ; Flatpak et AppImage différés avec leur raison —
et **écartés depuis**, la question ayant été instruite après la phase : #265.

**Deux, et le second est ce qui met la décision à l'épreuve.** « L'installation
d'abord » est une affirmation tant qu'un seul format l'exerce ; un `.rpm`
construit depuis les mêmes règles la vérifie, puisque **s'il demandait de
retoucher l'installation, c'est que l'installation était fausse.**

**Ce qui diffère réellement entre les deux n'est pas la liste des fichiers mais
les dépendances déclarées** : Qt 6 et libmpv ne portent pas les mêmes noms de
paquets chez Debian et chez Fedora. C'est là que le paquet peut être faux sans
que rien ne le montre à la construction.

**Et une asymétrie assumée, écrite plutôt que tue** : un `.rpm` construit sur
Ubuntu ne peut pas y être installé. Le `.deb` est prouvé de bout en bout par
#239 ; le `.rpm` l'est jusqu'à son contenu — liste des fichiers et dépendances
déclarées, par `rpm -qlp` et `rpm -qp --requires`. Livrer les deux en laissant
croire qu'ils ont été éprouvés pareil serait le vrai défaut.

**D3 — le thème est une préférence à trois valeurs, et « système » ne fait
rien.** Sous Qt 6.4.2 il n'existe aucune API de schéma de couleurs —
`QStyleHints::colorScheme` est arrivée en 6.5 — donc l'application ne peut ni
demander au système ce qu'il préfère, ni être prévenue quand il change d'avis.
« Système » signifie donc **laisser la palette au thème de plate-forme**, ce que
la fenêtre fait déjà aujourd'hui. « Clair » et « sombre » posent une palette
explicite.

C'est le comportement de Gaupol sous GTK antérieur à 4.20, et il est honnête :
il livre les deux thèmes que l'utilisateur peut demander, et il n'invente pas
une lecture du système qu'il ne sait pas faire.

**D4 — et c'est D3 qui rend le thème éprouvable.** Parce que « clair » et
« sombre » sont des palettes *que nous posons*, un test peut les poser aussi, et
une capture d'écran peut les montrer toutes les deux. Si le thème avait été une
lecture du système, ni l'un ni l'autre ne serait possible sous Qt 6.4.

**Conséquence directe pour #199 : deux captures par écran, et elles sont
atteignables.** La question laissée ouverte par #236 est tranchée ici.

**D5 — une préférence est persistée si l'oublier ennuie, et si s'en souvenir
est sans risque.** Le critère est celui de l'exclusion, parce que c'est celui
qui manque toujours : une préférence est facile à ajouter et impossible à
retirer une fois qu'un utilisateur s'y appuie.

Ce qui passe : géométrie de la fenêtre, état maximisé, largeur des colonnes,
thème. **Plus trois, ajoutées après coup** — la position de la poignée entre la
vidéo et la table et le dernier répertoire ouvert, que #254 porte, puis le côté
de l'insertion, `edit.insert-placement`, que D7 a rendu persistant et que cette
liste ne comptait pas. Le paragraphe ci-dessous dit pourquoi le répertoire passe
là où le dernier *fichier* ne passe pas.

**La fréquence d'image par défaut, elle, ne passe pas — et cette liste l'a
promise pendant toute la phase.** Rien ne la lit : aucun dialogue ne s'en sert,
aucun document n'en hérite. Une préférence dont personne ne se sert est une case
qui ment, et #241 a eu raison de ne pas l'écrire. Le mot est retiré ici plutôt
que le manque comblé, et la question a été instruite puis close : elle **ne
viendra pas** — [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267).

Sept options, donc, et le fichier de configuration en porte sept.

Ce qui ne passe pas, et pourquoi : **le dernier fichier ouvert.** S'en souvenir
veut dire rouvrir au lancement un document que l'utilisateur n'a pas demandé,
et le mode strict de l'ADR 0012 fait qu'ouvrir n'est pas neutre. La liste des
fichiers récents est autre chose — une fonctionnalité, pas une préférence —, et
elle n'est pas de cette phase.

**Retenir un répertoire n'est pas retenir un fichier**, et c'est la coupure que
le critère fait passer là où on ne l'attend pas : un répertoire *pointe* une
boîte de dialogue, il n'ouvre rien. Les deux moitiés du critère ne tranchent
pas au même endroit, et c'est la seconde — « s'en souvenir est sans risque » —
qu'on oublie de poser.

**Ce que la relecture de #240 a écarté**, et qui vaut d'être écrit pour ne pas
être reproposé : le panneau de diagnostics replié — le replier coûte un clic ;
le volume du lecteur — il n'y a pas de réglage de volume avant la barre de
lecture ; le format d'enregistrement par défaut — il vient du fichier ouvert,
ADR 0018 ; et **tout nombre qui appartient à un document** — une durée de
décalage, deux repères de transformation, vrais d'un fichier et faux du suivant.

Elle a aussi mesuré ce que la conception avait déjà économisé. Les trois quarts
de la centaine d'options de Gaupol sont l'état collant d'un dialogue, dont six
`target` qui disent sur quoi une opération porte. `targetOf()` le déduit de la
sélection, donc aucune n'a d'équivalent ici.

**D6 — `Help ▸ Manual` ouvre le manuel installé, et non une URL.** Gaupol ouvre
une URL ; nous ne le suivons pas ici, et pour une raison précise : **une URL
décrit `main`, pas la version installée.** Un utilisateur qui lit le manuel
d'une version qu'il n'a pas est moins bien servi qu'un utilisateur sans manuel.

Le manuel reste du Markdown — c'est sa forme source et celle que le dépôt
relit. `QTextDocument::setMarkdown` sait le rendre sans outil externe ni
navigateur, donc sans dépendance nouvelle. **À vérifier dans l'issue** : nos
sections usent de tableaux, et le rendu Markdown de Qt a ses limites ; si elles
mordent, l'alternative est un rendu HTML à la construction, avec la dépendance
que cela demande.

**D7 — l'insertion suit la règle de Gaupol, parce qu'elle est déjà au noyau.**
`InsertCommand::blank` implémente sa règle de partage du temps disponible, et
son commentaire le dit. Le geste suit :

| | |
| :--- | :--- |
| combien | demandé, un par défaut |
| où | avant ou après la sélection, au choix, et **ce choix est persisté** |
| l'index | le **dernier** sélectionné, plus un si « après » |
| fichier vide | index zéro, et aucune sélection n'est exigée |
| suppression | la sélection, touche `Del` |

Le dernier sélectionné et non le premier : c'est le point qu'on invente mal si
on ne le lit pas, et Gaupol le fait ainsi depuis vingt ans.

**D8 — le déclencheur de l'ADR 0019 ne se déclenche pas, et c'est écrit ici.**
L'ADR demandait de mesurer avant de construire un `Session` qui annonce un
changement de structure. #236 a établi que **la part de la vue n'est pas
mesurable** dans le harnais de banc — il n'a pas de `QApplication`, et en
ajouter une la ferait tourner sans écran, ce que le commentaire du banc écarte
déjà nommément.

Ce qui reste est le chiffre du modèle : `réinitialisation du modèle après une
ligne retirée`, autour de 10 µs sur quatre mille sous-titres. Une insertion est
un geste humain. **Construire une optimisation dont le bénéfice n'est pas
mesurable est exactement ce que l'ADR demandait d'éviter**, donc on ne la
construit pas.

Le déclencheur est reformulé plutôt qu'abandonné : il se rouvre le jour où
quelqu'un mesure sur une machine avec écran et trouve que ça pique, ou le jour
où un producteur change la structure plus vite qu'un humain ne clique.

## Le noyau

### `core/config/settings.hpp`

Le type de D1. Il porte les valeurs, leurs défauts, et rien d'autre : ni
emplacement par défaut, ni singleton.

```
[[nodiscard]] std::expected<Settings, ConfigError> readSettings(const FileSystem&, const std::filesystem::path&);
[[nodiscard]] std::expected<void, FileError>       writeSettings(const FileSystem&, const std::filesystem::path&, const Settings&);
```

Un fichier absent n'est pas une erreur : c'est `Settings{}`. Ce qui produit un
diagnostic, c'est une valeur qu'on n'a pas su lire — et le diagnostic accompagne
la valeur par défaut plutôt que de la remplacer.

### `core/edit/`

Rien de neuf. `InsertCommand::blank` et `RemoveCommand` existent depuis la
phase 2 et n'ont jamais eu de surface. **Cette phase est leur première preuve de
bout en bout**, ce qui est en soi un résultat : deux commandes du noyau vivaient
jusqu'ici sans qu'aucun utilisateur puisse les déclencher.

### Ce que #154 déplace

`gui/opening.cpp` et `gui/saving.cpp` ne contiennent pas une ligne de Qt. Ils
descendent au noyau, et la fenêtre y gagne de nommer la vraie cause d'un échec
d'ouverture. L'issue porte son propre constat et ce cadrage ne le rejoue pas.

## La fenêtre

| Entrée | Menu | Raccourci |
| :----- | :--- | :-------- |
| `Insert Subtitles…` | Edit | `Ins` |
| `Remove Subtitles` | Edit | `Del` |
| `Preferences…` | Edit | — |
| `Manual` | Help | — |

Les deux premières après `Undo`/`Redo`, séparateur entre les deux groupes :
défaire est ce qu'on fait *à* une édition, insérer et supprimer *sont* des
éditions.

`Preferences…` porte le thème, et lui seul. La géométrie et les largeurs de
colonnes ne s'y montrent pas — elles se règlent en déplaçant la fenêtre, et une
préférence qui a déjà un geste n'a pas besoin d'un champ ; le côté de
l'insertion se règle dans le dialogue qui s'en sert. Reste le thème, la seule
qui n'ait aucun geste ailleurs.

**Le cadrage y ajoutait la fréquence par défaut, et la phase ne l'a pas
livrée** — voir D5 : rien ne la lirait. La ligne est corrigée ici plutôt que
laissée dire ce qui n'existe pas ; le manuel, lui, le disait déjà juste. La
question a été rouverte après la phase et refermée pour de bon : **une seule
préférence a un dialogue, et ce sera le thème.**

## L'installation

Ce que posent les règles `install()`, chemins issus de `GNUInstallDirs` :

| Fichier | Où |
| :------ | :- |
| `subedit-gui`, `subedit-cli` | `${CMAKE_INSTALL_BINDIR}` |
| icône | `${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps` |
| `.desktop` | `${CMAKE_INSTALL_DATADIR}/applications` |
| AppStream | `${CMAKE_INSTALL_DATADIR}/metainfo` |
| manuel | `${CMAKE_INSTALL_DATADIR}/subedit/manual` |
| `subedit-cli.1` | `${CMAKE_INSTALL_MANDIR}/man1` |

**Deux de ces règles existent depuis #239** — les binaires et le manuel — parce
qu'un contrôle d'installation n'a rien à prouver sans elles. Les quatre autres
nomment des fichiers que le dépôt ne contient pas encore : l'icône, le
`.desktop`, les métadonnées AppStream et la page de manuel de `subedit-cli`
viennent avec #244, et leurs règles s'ajoutent dans `cmake/Installation.cmake`.

**Ce qu'on empaquette est ce que produit `make release`, issue #247.** La cible
existe depuis cette phase ; auparavant, pour obtenir un binaire optimisé, on
lançait `make bench` et on l'interrompait. Elle ne construit ni le banc ni le
harnais de bout en bout — ce qui est demandé est ce qu'un utilisateur lance, pas
ce qui l'éprouve.

## Tests

**La configuration s'éprouve sans écrire nulle part**, par `InMemoryFileSystem`
et un emplacement donné. C'est ce que #238 met en place, et c'est ce qui fait
que le contrôle de #226 n'a rien à voir ici : aucun test ne touche à un vrai
emplacement.

**Le thème s'éprouve parce qu'il est posé** — D4. Un test demande « sombre » et
lit la palette obtenue.

**L'installation s'éprouve dans un préfixe temporaire** — #239 — et le préfixe
ne laisse rien derrière lui, ce que le contrôle de #226 verrait. `make
install-check` installe, lance les deux binaires installés, et confronte le
manuel déposé à celui du dépôt — liste **calculée** et non recopiée, sans quoi
un chapitre ajouté manquerait en silence.

Ce qu'elle ne prouve pas est écrit dans son en-tête plutôt que laissé croire
prouvé : rien des paquets natifs, qui n'existent pas encore, et rien de ce
qu'aucune machine Ubuntu ne peut dire d'un `.rpm` — il s'y construit et ne s'y
installe pas.

**Les captures d'écran sont doublées** — D4 encore.

## Mesures

Rien de neuf n'est attendu au banc. La lecture d'une configuration est un
fichier de quelques lignes lu une fois au démarrage ; la mesurer serait mesurer
`readFile`.

Le seul chiffre qui compte pour cette phase existe déjà, et D8 l'a interprété.

## Exigences

| Identifiant | Ce qu'il promet |
| :---------- | :-------------- |
| `GUI-CONFIG-01` | la fenêtre retrouve sa géométrie et ses colonnes d'une session à l'autre |
| `GUI-CONFIG-02` | une valeur illisible laisse le défaut en place, et le dit |
| `GUI-CONFIG-03` | une option restée à son défaut est réécrite commentée, donc un défaut changé prend effet |
| `GUI-THEME-01` | le thème se choisit entre trois valeurs, se retient, et « système » ne pose rien |
| `GUI-THEME-02` | « clair » et « sombre » posent une palette explicite |
| `GUI-INSERT-01` | insérer place les sous-titres après le dernier sélectionné, ou avant au choix |
| `GUI-INSERT-02` | insérer dans un document vide ne demande aucune sélection |
| `GUI-REMOVE-01` | supprimer retire la sélection, et l'opération s'annule |
| `GUI-MANUAL-01` | `Help ▸ Manual` ouvre le manuel installé |

**Neuf, et l'empaquetage n'en porte aucune.** Ce n'est pas un oubli : le
registre est confronté aux **tests** par `check-requirements.sh`, et
l'installation ne se prouve pas par un test mais par une cible qui installe puis
lance — #239. Une exigence qu'aucun test ne peut citer serait une ligne morte au
registre.

**Elles entrent au registre en début d'issue, à l'état `prévue`**, comme la
phase 16 l'a tenu. Elle était la première où la règle servait vraiment ; celle-ci
est la seconde, et c'est là qu'on saura si elle tient hors du cas favorable.

**Neuf, et le registre n'en a longtemps porté que huit.** #241 a fondu
`GUI-THEME-02` dans `GUI-THEME-01` sans que rien ne le dise, ce qui a coûté une
promesse : la ligne restante dit que le thème a trois valeurs et se retient, elle
ne dit pas que « clair » et « sombre » posent une palette. Or c'est précisément
cette palette-là qui tient D3 et D4, et deux cas de `theme_test.cpp` la
prouvaient déjà sans que rien ne les rattache à une promesse. La relecture de fin
de phase rend donc `GUI-THEME-02` au registre et lui donne ses tags, plutôt que
d'entériner la fusion.

**Et elle donne au défaut son contrôle**, parce que c'est la deuxième phase de
suite où il passe — la phase 16 avait retiré `CLI-SNAP-03` sans que sa spec
suive et laissé naître `GUI-GRID-03` sans l'y inscrire.
`check-requirements.sh` confronte désormais **la table de chaque spec aux lignes
du registre qui portent sa phase**, dans les deux sens, avec la phase et le
barré. Il a trouvé quatre écarts de plus au passage, tous antérieurs : trois
`CLI-VERSION` remplacées et une `CLI-OUTPUT-06` née en cours de route, absentes
de la spec de la phase 3, et une `CLI-INSPECT-04` absente de celle de la phase 5.

## Manuel

C'est le livrable le plus lourd de la phase, et le seul qui touche tout ce qui
précède : les six sections d'interface sont de la prose sans filet depuis #116,
et cette phase y ajoute des préférences, un thème, deux opérations et une
installation.

L'ordre importe : **#199 avant l'écriture**, sans quoi les captures se prennent
à la main et le manuel décrit à nouveau une interface sur parole.
`docs/manual/subedit-cli/installation.md` est à reprendre en entier — il dit
aujourd'hui que l'outil se lance depuis l'arbre de construction.

## Découpage en issues

| Issue | Sujet | Dépend de |
| :---- | :---- | :-------- |
| #247 | une cible qui produit les binaires en Release | — |
| #238 | isoler l'emplacement de configuration | — |
| #199 | captures d'écran engendrées | D4 |
| #154 | une seule recette d'ouverture | — |
| #240 | la configuration persistée | #238, D1 |
| #241 | le thème clair et sombre | #240, D3 |
| #254 | la poignée et le dernier répertoire retenus | #240, D5 |
| #242 | insérer et supprimer depuis la fenêtre | D7 |
| #243 | le manuel utilisateur complet | #199, #240, #241, #242, #254 |
| #244 | l'empaquetage : `install()`, `.deb` et `.rpm` | #243, #239, #247 |
| #245 | `Help ▸ Manual` rallumée | #244, D6 |

#154 ne dépend de rien et vient tôt : elle remanie le chemin d'ouverture auquel
#240 s'accroche, et sa moitié visible atterrit avant que #243 la décrive.

#245 est dernière par construction, et c'est une chaîne sans raccourci : le
manuel doit être complet pour être installé, l'empaquetage doit être tranché
pour décider où il vit, et l'entrée ne se rallume qu'une fois les deux faits.

#254 est née de la relecture de #240 plutôt que de ce cadrage, et elle touche le
même type que #241 : les faire ensemble évite un conflit sur `core::Settings`.

## Renvois

| Ce qui est renvoyé | Où |
| :----------------- | :- |
| un conteneur Fedora pour éprouver le `.rpm` | [#266](https://github.com/Guyot-Bertrand/sub-edit/issues/266) |
| un bump de Qt pour lire le schéma de couleurs du système | hors phase ; D3 livre sans lui, et l'ADR 0001 se rouvrirait |
| la liste des fichiers récents | hors phase — une fonctionnalité, pas une préférence, D5 |
| la traduction du manuel | phase 15 |

**Les quatre renvois qui ne désignaient personne ont été instruits, et trois
d'entre eux sont morts de l'être.** C'est le résultat qu'on n'attend pas d'une
mise au propre : une condition sans référent survit parce que personne ne la
regarde, et la regarder suffit souvent à la conclure.

| Ce qui était renvoyé | Ce qui a été décidé |
| :------------------- | :------------------ |
| le déclencheur de l'ADR 0019, reformulé par D8 | **abandonné** — [#264](https://github.com/Guyot-Bertrand/sub-edit/issues/264). D8 se suffit : la part de la vue n'est pas mesurable au banc, celle du modèle est de l'ordre de 10 µs, et une insertion est un geste humain |
| Flatpak et AppImage, différés par l'ADR 0023 | **écartés** — [#265](https://github.com/Guyot-Bertrand/sub-edit/issues/265). `.deb` et `.rpm` suffisent au contour visé ; ailleurs, on construit depuis les sources, et le manuel décrit ce chemin en entier |
| la fréquence d'image par défaut dans les préférences | **écartée** — [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267). Aucun des trois lecteurs possibles n'en avait besoin, et D5 avait donc raison la première fois |
| un conteneur Fedora pour éprouver le `.rpm` | **retenu**, et seul des quatre : c'est le seul écart de l'empaquetage qui reste ouvert |

Trois d'entre eux étaient formulés par une condition — « le jour où quelqu'un
mesure », « quand quelqu'un installera vraiment le paquet », « le jour où quelque
chose la lira ». Une condition sans référent finit par s'oublier ; c'est le
défaut que le déroulé d'une phase décrit, et c'est la deuxième relecture
d'affilée à le trouver. **Trois « peut-être » sont devenus trois « non », et
c'est un gain** : un « non » écrit ne se repropose pas.

## Ce que la relecture de fin de phase a sorti

Sept axes, discutés et retenus, chacun avec son issue. Aucun n'est corrigé
ici — c'est la règle du regard critique.

| Issue | Ce qu'elle porte |
| :---- | :--------------- |
| [#268](https://github.com/Guyot-Bertrand/sub-edit/issues/268) | chaque vérification passe-t-elle par l'outil qui compte ? Le manuel n'est jamais rendu par Qt, la page de manuel jamais par groff |
| [#269](https://github.com/Guyot-Bertrand/sub-edit/issues/269) | un commentaire ajouté au `Makefile` fait passer clang-tidy de 2 fichiers à 223 |
| [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270) | six des treize relevés de la phase sont pris au-dessus du seuil de charge |
| [#271](https://github.com/Guyot-Bertrand/sub-edit/issues/271) | `check-installation.sh` a triplé : une chose, ou deux ? |
| [#272](https://github.com/Guyot-Bertrand/sub-edit/issues/272) | `subedit-gui` n'a pas de page de manuel, et Gaupol n'en a qu'une, pour sa fenêtre |
| [#273](https://github.com/Guyot-Bertrand/sub-edit/issues/273) | 75 intitulés de tests en français contre 1188 en anglais, tous nés dans cette phase |
| [#274](https://github.com/Guyot-Bertrand/sub-edit/issues/274) | deux raccourcis que Qt ne pose pas sous X11, et que le manuel promettait |

## Ce que la mesure a tranché

**La phase 16 avait conclu l'inverse, et sa conclusion est fausse.** Elle avait
inscrit, parmi ses trois « affirmations démenties par la mesure » :

> « la garde du slot est inatteignable » → **Qt invoque le slot d'une action
> éteinte**

Qt ne l'invoque pas. `QAction::trigger()` et `QAction::activate(Trigger)` sur
une action éteinte n'émettent rien :

```
activée, trigger()   -> 1
éteinte, trigger()   -> 0
éteinte, activate()  -> 0
```

Le cas qui était censé le trancher — `the entry that is out does nothing when
triggered` — **ne pouvait pas le faire** : ses deux assertions, la barre d'état
inchangée et l'annulation toujours éteinte, sont vraies que le slot soit atteint
ou non. Son commentaire disait pourtant « This case is what says so ».

Le cas compte désormais les émissions plutôt que de les inférer, et il vérifie
aussi le sens positif — la même action rallumée atteint bien le slot. C'est ce
qui distingue « mesuré » de « rien n'a bougé ».

Trois commentaires de cette phase-ci reposaient déjà sur le comportement réel :
c'est pourquoi `une suppression vide n'entre pas dans l'historique` et `le retour
forcé sans historique ne fait rien` rallument l'action à la main pour atteindre
le garde qu'ils éprouvent. `docs/mesures/couverture.md` le dit aussi depuis la
phase 6 — « Qt ne déclenche pas une action désactivée, donc rien ne peut la
parcourir ». Le quatrième, plus ancien, disait le contraire ; il est corrigé.

**La leçon de la phase 16 tient malgré cela, et elle se retourne :** un cas
écrit pour trancher ne tranche que si ses assertions distinguent les deux
réponses. Celui-là n'en distinguait aucune, et il a servi pendant une phase
entière à soutenir la mauvaise.

## Critères de fin

- [x] Les six issues d'implémentation sont fermées, plus #154, #199, #238, #239, #247, #254
- [x] Un tiers installe le paquet et lance l'outil sans l'arbre de construction
- [x] Le manuel décrit ce qui existe, captures comprises, dans les deux thèmes
- [x] `Help ▸ Manual` ouvre le manuel installé
- [x] La relecture de fin de phase est faite — #263
