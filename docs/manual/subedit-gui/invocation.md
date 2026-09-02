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

Les formats lus sont **SubRip** (`.srt`) et **WebVTT** (`.vtt`), en UTF-8. Le
format est reconnu au contenu et non à l'extension.

## Quand l'ouverture échoue

Un fichier absent, illisible, qui n'est pas de l'UTF-8 valide, ou dont aucun
format ne reconnaît le contenu : **la fenêtre s'ouvre quand même, vide**, et
l'échec est écrit sur la sortie d'erreur.

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
en chemin s'affiche sous la table — voir
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

Si elle ne l'est pas, il en pose une lui-même dans un seul cas : une session
Wayland avec un serveur X à côté, où il demande `xcb`. C'est ce qui permet au
lecteur intégré d'afficher le film **dans** la fenêtre — voir
[Le lecteur](lecteur.md). Partout ailleurs, il laisse Qt choisir.

`QT_QPA_PLATFORM=offscreen` fait tourner la fenêtre là où il n'y a pas de
serveur graphique — c'est ce que font l'intégration continue et les tests.
