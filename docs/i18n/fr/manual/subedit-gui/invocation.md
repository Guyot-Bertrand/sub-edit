# Invocation

```
subedit-gui [--version] [<file>]
```

| Argument | Requis | Valeur | Défaut |
| :------- | :----- | :----- | :----- |
| `<file>` | non | le chemin d'un fichier de sous-titres à ouvrir | aucun — la fenêtre s'ouvre vide |
| `--version` | non | — | — |

**Un seul fichier.** Les arguments suivants sont ignorés : la fenêtre ne tient
qu'un projet à la fois.

## `--version`

Écrit sur la sortie standard le bloc qu'écrit le `--version` de la ligne de
commande, et rend la main aussitôt, sans ouvrir de fenêtre. Code de retour `0`.

**`subedit <version>` seul sur la première ligne**, puis une ligne par
bibliothèque avec laquelle la fenêtre tourne, avec la version trouvée à
l'exécution et, entre parenthèses, celle avec laquelle elle a été compilée ; puis
les langues d'interface installées, par code. Sur une machine qui a Qt 6.4.2 :

```console
$ subedit-gui --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  Qt       6.4.2 (built with 6.4.2)
  libmpv   2.2 (built with 2.2)
  Languages: en, fr
```

Les numéros ne sont pas recopiés ici : ce sont ceux de la machine, et ce manuel
n'a pas de bloc engendré qui tiendrait à jour celui du programme. Les
bibliothèques sont celles de la ligne de commande, ICU et Enchant, et les deux
que seule la fenêtre lie — Qt, et libmpv, le lecteur, dite par la version de son
API cliente, celle qu'annonce `mpv.pc`. Le reste est décrit dans
[le manuel de la ligne de commande](../subedit-cli/invocation.md#--version). Les
mots `built with` et `Languages` suivent la langue de la fenêtre ; rien d'autre.

`Aide ▸ À propos de subedit` montre les mêmes bibliothèques, de la même liste,
dans un tableau — le nom, la version chargée, la version de compilation, une
ligne chacune — puis les langues dessous ; ses en-têtes suivent la langue de la
fenêtre.

## Ouvrir un fichier

```console
$ subedit-gui film.srt
```

Les formats lus sont **les neuf** que le filtre d'ouverture énumère — voir
[Ouvrir et enregistrer](files.md#ouvrir). Le format est reconnu au contenu et
non à l'extension, et **l'encodage l'est aussi** : un BOM
si le fichier en porte un, sinon l'encodage que ses octets proposent.

## Quand l'ouverture échoue

Un fichier absent, illisible, dont les octets ne se décodent dans aucun
encodage, ou dont aucun format ne reconnaît le contenu : **la fenêtre s'ouvre
quand même, vide**, et l'échec est écrit sur la sortie d'erreur.

**Le message nomme la cause**, dans les mêmes mots que la ligne de commande :

| Message | Ce qui s'est passé |
| :------ | :----------------- |
| `does not exist` | le chemin ne désigne aucun fichier |
| `cannot be opened: permission denied` | le système refuse de l'ouvrir |
| `cannot be read` | le système a refusé pour une autre raison |
| `cannot be decoded in the chosen encoding` | les octets ne se lisent pas dans l'encodage retenu |
| `is in no format this tool knows` | aucun format ne reconnaît le contenu |
| `holds nothing recognisable as a subtitle` | le format est reconnu, mais rien n'y est un sous-titre |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Ouvrir vide plutôt que refuser de démarrer : l'outil sert encore à quelque
chose, et l'échec d'un argument n'est pas une raison de ne pas se lancer.

**Un fichier mal formé mais lisible s'ouvre**, et ce que la lecture a rattrapé
en chemin se lit dans un bouton de la barre d'état — voir
[les diagnostics d'une lecture](files.md#les-diagnostics-dune-lecture).

## Codes de retour

| Code | Quand |
| :--- | :---- |
| `0` | la fenêtre s'est ouverte puis a été fermée normalement, ou `--version` a répondu |
| `2` | une erreur inattendue avant l'ouverture — Qt absent, écran indisponible |

L'échec d'ouverture d'un fichier **ne change pas le code de retour** : la
fenêtre s'est ouverte, et c'est ce que le code rapporte.

## La page de manuel

Une installation dépose `subedit-gui(1)` sous `share/man/man1` :

```console
$ man subedit-gui
```

Elle dit la forme d'appel, `--version`, les messages d'échec d'ouverture, les
codes de retour, les deux variables d'environnement et le fichier de réglages.
**Elle ne décrit pas la fenêtre** — c'est ce manuel-ci qui le fait, et la page
le renvoie ici.

## La plateforme Qt

`QT_QPA_PLATFORM` choisit sur quoi la fenêtre tourne. **Si la variable est
posée, `subedit-gui` la respecte sans discuter.**

Si elle ne l'est pas, il laisse Qt choisir : le lecteur intégré dessine l'image dans
la fenêtre elle-même, et ne demande rien à la plateforme — voir
[Le lecteur](player.md).

`QT_QPA_PLATFORM=offscreen` fait tourner la fenêtre là où il n'y a pas de
serveur graphique — c'est ce que font l'intégration continue et les tests.

## Langue

La fenêtre parle la langue de son environnement, **choisie une fois, au démarrage** : changer les
variables d'une fenêtre en marche ne change rien avant le lancement suivant. Les variables et les
règles sont celles de la ligne de commande — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C`
pour l'anglais, l'anglais **sans un mot** quand la langue demandée n'est pas installée — décrites
dans [le manuel de la ligne de commande](../subedit-cli/invocation.md#langue). Les catalogues sont
les mêmes fichiers, installés sous `<prefix>/share/subedit/locale`.

**Une langue choisie dans `Édition ▸ Préférences…` passe en premier**, et l'environnement n'est lu
que lorsqu'elle vaut `System default`, ce qui est le cas tant qu'aucune n'est choisie : une langue
installée choisie là l'emporte sur toutes les variables, `LC_ALL=C` compris. Ce choix aussi
s'applique au lancement suivant. Voir
[La langue de l'interface](preferences.md#la-langue-de-linterface).

| Quoi | Suit la langue |
| :--- | :------------- |
| les menus et leurs entrées, avec leurs lettres soulignées ; la barre d'outils ; les infobulles | oui |
| les colonnes de la table et de `Affichage ▸ Colonnes`, les onglets, la ligne d'état, la barre de lecture | oui |
| les boutons standard que Qt dessine lui-même (`OK`, `Cancel`) | oui, quand le catalogue de Qt lui-même pour cette langue est installé (`qtbase_<language>.qm`) ; l'anglais sinon |
| les dialogues, les pages de `Correct Texts…`, les titres et les filtres des boîtes de fichiers, les boîtes de message et ce qu'elles disent | oui |
| les lignes que la fenêtre écrit sur sa sortie d'erreur, `subedit-gui: …` | oui, le nom du programme excepté |
| les noms des motifs de correction dans `Correct Texts…`, et leurs descriptions dans les infobulles | oui, pour les motifs livrés avec le programme ; un motif à vous est montré comme son fichier l'écrit |
| les noms des encodages, des fins de ligne et des langues d'orthographe (`UTF-8`, `CRLF`, `en_US`) | jamais : ce sont des noms, pas des mots |
| les raccourcis clavier | jamais : `Ctrl+S` est `Ctrl+S` dans toutes les langues |
| un message que le catalogue ne traduit pas | montré en anglais |

Cette version installe `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT` et `nl` — le catalan, le tchèque,
l'allemand, l'espagnol, le finnois, le français, le galicien, l'hébreu, le hongrois, l'interlingue, l'islandais, l'italien et le néerlandais — traduits en entier. Le français a
été relu par un lecteur de la langue ; le catalan, le tchèque, l'allemand, l'espagnol, le finnois,
le galicien, l'hébreu, le hongrois, l'interlingue, l'islandais, l'italien et le néerlandais sont traduits par machine et pas encore relus par un lecteur de la langue.
<!-- machine-translated: ca -->
<!-- machine-translated: cs -->
<!-- machine-translated: de -->
<!-- machine-translated: es -->
<!-- machine-translated: fi -->
<!-- machine-translated: gl -->
<!-- machine-translated: he -->
<!-- machine-translated: hu -->
<!-- machine-translated: ie -->
<!-- machine-translated: is -->
<!-- machine-translated: it_IT -->
<!-- machine-translated: nl -->
Sur un bureau français, la barre de
menus porte `&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. Les
captures de ce manuel-ci sont prises en français.
