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
format ne reconnaît le contenu : **la fenêtre s'ouvre quand même, vide**, et la
raison est écrite sur la sortie d'erreur.

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: nothing to open
```

Ouvrir vide plutôt que refuser de démarrer : l'outil sert encore à quelque
chose, et l'échec d'un argument n'est pas une raison de ne pas se lancer.

**Les diagnostics d'une lecture qui a réussi ne sont pas encore montrés.** Un
fichier mal formé mais lisible s'ouvre en silence ; `subedit-cli inspect` les
rapporte en attendant.

## Codes de retour

| Code | Quand |
| :--- | :---- |
| `0` | la fenêtre s'est ouverte puis a été fermée normalement, ou `--version` a répondu |
| `2` | une erreur inattendue avant l'ouverture — Qt absent, écran indisponible |

L'échec d'ouverture d'un fichier **ne change pas le code de retour** : la
fenêtre s'est ouverte, et c'est ce que le code rapporte.

## Sans écran

`QT_QPA_PLATFORM=offscreen` fait tourner la fenêtre là où il n'y a pas de
serveur graphique — c'est ce que font l'intégration continue et les tests.
