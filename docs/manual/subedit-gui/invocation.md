# Invocation

```
subedit-gui [--version] [<fichier>]
```

| Argument | Requis | Valeur | Défaut |
| :------- | :----- | :----- | :----- |
| `<fichier>` | non | le chemin d'un fichier de sous-titres à ouvrir | aucun — la fenêtre s'ouvre vide |
| `--version` | non | — | — |

**Un seul fichier.** Les arguments suivants sont ignorés : la fenêtre ne tient
qu'un projet à la fois.

## `--version`

Écrit `subedit <version>` sur la sortie standard et rend la main aussitôt, sans
ouvrir de fenêtre. Code de retour `0`.

```console
$ subedit-gui --version
subedit <version>
```

Le numéro n'est pas recopié ici : ce manuel n'a pas de bloc engendré qui le
tiendrait à jour, et un numéro figé serait faux dès le prochain incrément.

## Ouvrir un fichier

```console
$ subedit-gui film.srt
```

Les formats lus sont **les neuf** que le filtre d'ouverture énumère — voir
[Ouvrir et enregistrer](fichiers.md#ouvrir). Le format est reconnu au contenu et
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
[les diagnostics d'une lecture](fichiers.md#les-diagnostics-dune-lecture).

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
[Le lecteur](lecteur.md).

`QT_QPA_PLATFORM=offscreen` fait tourner la fenêtre là où il n'y a pas de
serveur graphique — c'est ce que font l'intégration continue et les tests.

## Language

The window speaks the language of its environment, **chosen once, when it starts**: changing the
variables of a running window changes nothing until the next launch. The variables and the rules
are those of the command line — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` for
English, English **without a word** when the language asked for is not installed — described in
[the command line's manual](../subedit-cli/invocation.md#language). The catalogues are the same
files, installed under `<prefix>/share/subedit/locale`.

| What | Follows the language |
| :--- | :------------------- |
| the menus and their entries, with their underlined letters; the toolbar; the tool tips | yes |
| the columns of the table and of `View ▸ Columns`, the tabs, the status line, the play bar | yes |
| the standard buttons Qt draws itself (`OK`, `Cancel`) | yes, when Qt's own catalogue of that language is installed (`qtbase_<language>.qm`); English otherwise |
| the dialogs and the messages they show | their own labels no, English; a sentence they share with the command line — the account of an operation, the reason of a failure — yes |
| the keyboard shortcuts | never: `Ctrl+S` is `Ctrl+S` in every language |
| a message the catalogue does not translate | shown in English |

This version installs `fr`, whose translation is still to be done: until it is, its messages are
shown in English. The captures of this manual are taken in English.
