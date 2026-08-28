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
[l'ADR 0023](../adr/0023-deb-pour-la-premiere-livraison.md).

## Portée

| Dans la phase | Hors de la phase |
| :------------ | :--------------- |
| préférences persistées | les préférences que personne ne réclame |
| thème clair, sombre, système | un thème propre à l'application |
| insertion et suppression depuis la fenêtre | fusionner, scinder — phase 10 |
| manuel utilisateur complet | la traduction du manuel — phase 15 |
| règles `install()` et paquet `.deb` | Flatpak, AppImage — différés, ADR 0023 |
| `Help ▸ Manual` rallumée | une aide contextuelle |
| une seule recette d'ouverture (#154) | |

## Décisions applicables

**D1 — la configuration est un type du noyau, tolérante option par option.**
[ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md). Elle
lit et écrit par `core::FileSystem`, **reçoit** son emplacement, et n'a ni
numéro de version ni migration. Une clé inconnue est ignorée, une valeur
illisible laisse le défaut en place avec un diagnostic, et **une option restée à
son défaut est réécrite commentée**.

**D2 — un paquet `.deb`, après des règles `install()`.**
[ADR 0023](../adr/0023-deb-pour-la-premiere-livraison.md). L'installation
d'abord, le format ensuite ; Flatpak et AppImage différés avec leur raison.

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
thème, fréquence d'image par défaut.

Ce qui ne passe pas, et pourquoi : **le dernier fichier ouvert.** S'en souvenir
veut dire rouvrir au lancement un document que l'utilisateur n'a pas demandé,
et le mode strict de l'ADR 0012 fait qu'ouvrir n'est pas neutre. La liste des
fichiers récents est autre chose — une fonctionnalité, pas une préférence —, et
elle n'est pas de cette phase.

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
| suppression | la sélection, touche `Suppr` |

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
| `Remove Subtitles` | Edit | `Suppr` |
| `Preferences…` | Edit | — |
| `Manual` | Help | — |

Les deux premières après `Undo`/`Redo`, séparateur entre les deux groupes :
défaire est ce qu'on fait *à* une édition, insérer et supprimer *sont* des
éditions.

`Preferences…` porte le thème et la fréquence par défaut. La géométrie et les
largeurs de colonnes ne s'y montrent pas — elles se règlent en déplaçant la
fenêtre, et une préférence qui a déjà un geste n'a pas besoin d'un champ.

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

## Tests

**La configuration s'éprouve sans écrire nulle part**, par `InMemoryFileSystem`
et un emplacement donné. C'est ce que #238 met en place, et c'est ce qui fait
que le contrôle de #226 n'a rien à voir ici : aucun test ne touche à un vrai
emplacement.

**Le thème s'éprouve parce qu'il est posé** — D4. Un test demande « sombre » et
lit la palette obtenue.

**L'installation s'éprouve dans un préfixe temporaire** — #239 — et le préfixe
ne laisse rien derrière lui, ce que le contrôle de #226 verrait.

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
| `GUI-THEME-01` | le thème a trois valeurs, et « système » ne pose aucune palette |
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
| #238 | isoler l'emplacement de configuration | — |
| #199 | captures d'écran engendrées | D4 |
| #154 | une seule recette d'ouverture | — |
| #240 | la configuration persistée | #238, D1 |
| #241 | le thème clair et sombre | #240, D3 |
| #242 | insérer et supprimer depuis la fenêtre | D7 |
| #243 | le manuel utilisateur complet | #199, #240, #241, #242 |
| #244 | l'empaquetage et les règles `install()` | #243, #239 |
| #245 | `Help ▸ Manual` rallumée | #244, D6 |

#154 ne dépend de rien et vient tôt : elle remanie le chemin d'ouverture auquel
#240 s'accroche, et sa moitié visible atterrit avant que #243 la décrive.

#245 est dernière par construction, et c'est une chaîne sans raccourci : le
manuel doit être complet pour être installé, l'empaquetage doit être tranché
pour décider où il vit, et l'entrée ne se rallume qu'une fois les deux faits.

## Renvois

| Ce qui est renvoyé | Où |
| :----------------- | :- |
| Flatpak et AppImage | différés, ADR 0023, sans phase nommée — ce sont des livraisons, pas des finitions |
| le déclencheur de l'ADR 0019 | reformulé par D8 : une mesure sur machine avec écran, ou un producteur plus rapide qu'un humain |
| un bump de Qt pour lire le schéma de couleurs du système | hors phase ; D3 livre sans lui, et l'ADR 0001 se rouvrirait |
| la liste des fichiers récents | hors phase — une fonctionnalité, pas une préférence, D5 |
| la traduction du manuel | phase 15 |

## Critères de fin

- [ ] Les six issues d'implémentation sont fermées, plus #154, #199, #238, #239
- [ ] Un tiers installe le paquet et lance l'outil sans l'arbre de construction
- [ ] Le manuel décrit ce qui existe, captures comprises, dans les deux thèmes
- [ ] `Help ▸ Manual` ouvre le manuel installé
- [ ] La relecture de fin de phase est faite
