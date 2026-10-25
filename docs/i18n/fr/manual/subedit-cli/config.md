# `config`

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Liste, montre et modifie les **réglages d'opération** : la partie du fichier de réglages que la
fenêtre et la ligne de commande partagent. `Adjust Durations…` et `Correct Texts…` y retiennent
leurs valeurs, et [`adjust`](adjust.md) et [`correct`](correct.md) les lisent par défaut — voir
[Le fichier de réglages](invocation.md#le-fichier-de-réglages).

**Ce qu'elle ne fait pas :** elle ne lit ni n'écrit jamais une **préférence d'affichage** (la
géométrie de la fenêtre, les colonnes, le thème, les options de recherche, l'orthographe, la
vidéo…). Celles-là restent l'affaire de la fenêtre, se règlent dans la fenêtre et sont décrites
dans [son manuel](../subedit-gui/preferences.md#le-fichier).
Elle ne touche pas non plus à un fichier de sous-titres.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Sous-commandes

| Sous-commande | Ce qu'elle fait | Fichier |
| :------------ | :-------------- | :------ |
| `config list` | écrit chaque clé d'opération, avec ce à quoi elle sert, ce qu'elle accepte, son défaut et qui s'en sert — **aucune valeur** | n'en lit aucun |
| `config show` | écrit le fichier employé et s'il existe, puis chaque clé d'opération avec sa valeur, d'où vient la valeur, et son défaut | le lit |
| `config set KEY VALUE` | vérifie `VALUE`, puis réécrit le fichier avec `KEY = VALUE`, en gardant tous les autres réglages | le lit et l'écrit |
| `config unset KEY` | remet `KEY` à son défaut, écrit en commentaire comme le fichier écrit toujours un défaut | le lit et l'écrit |

| Option | Requis | Valeurs | Défaut |
| :----- | :----- | :------ | :----- |
| `KEY` | oui, pour `set` et `unset` | une clé d'opération, telle que `config list` la nomme | — |
| `VALUE` | oui, pour `set` | ce que `config list` dit que la clé accepte | — |

`config` n'a aucune option à elle. Les options globales écrites **avant** `config` choisissent le
fichier et la forme de la sortie :

| Option globale | Effet sur `config` |
| :------------- | :----------------- |
| `--config FILE` | `show`, `set` et `unset` agissent sur `FILE` au lieu du fichier de l'utilisateur, même s'il n'existe pas encore ; `list` l'ignore |
| `--no-config` | `list` marche comme d'habitude ; `show` montre les défauts, avec `File: none (--no-config)` ; `set` et `unset` sont refusés : il n'y a pas de fichier où écrire |
| `--format json` | `list` et `show` écrivent un objet JSON par ligne ; `set` et `unset` n'écrivent rien sur la sortie standard |
| `-q`, `--quiet` | `set` et `unset` ne confirment plus ce qu'ils ont écrit |

## Quel fichier

Sans `--config`, le fichier de l'utilisateur :

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

ou `~/.config/subedit/settings.conf` quand `XDG_CONFIG_HOME` est absente, vide ou n'est pas un
chemin absolu. C'est le fichier que la fenêtre lit et écrit ; `config show` en écrit le chemin sur
sa première ligne :

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

Quand ni `XDG_CONFIG_HOME` ni `HOME` ne donnent d'endroit, il n'y a pas de fichier de
l'utilisateur : `show`, `set` et `unset` sont refusés, et `--config FILE` en nomme un.

## `config list`

**Le catalogue des clés d'opération**, dans l'ordre du fichier : chaque clé, puis quatre lignes en
retrait — ce à quoi elle sert, `Accepts:` (l'ensemble fermé, ou la forme et les bornes), `Default:`
et `Used by:` (les commandes de `subedit-cli` et les actions de la fenêtre qui la lisent). Elle ne
lit aucun fichier et n'en a besoin d'aucun, et marche avec `--no-config`.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

Elle liste **41 clés** :

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
41
```

**Vingt et une d'entre elles sont lues par la ligne de commande** ; les vingt autres ne sont lues que par
la fenêtre, et leur ligne `Used by:` le dit, avec l'option de `correct` qui en tient lieu :

| Clé | Lue par |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) et `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` et `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` et `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` et `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) et `Correct Texts…` |
| `correction.remove-blank` | `correct` et `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` et `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) et `Transcribe Audio…` |
| toute autre clé `correction.*` | `Correct Texts…` seulement — `correct` prend `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` et `--disable` sur sa propre ligne |

Les nombres de lignes, `correction.line-break.max-lines` et `correction.line-break.skip-max-lines`,
sont des entiers de 1 à 1000 — les bornes de `correct --max-lines` —, et les longueurs de ligne,
`correction.line-break.max-length` et `correction.line-break.skip-max-length`, des nombres
supérieurs à 0 : `set` refuse le reste, et un fichier modifié à la main qui en porte un garde le
défaut.

`correct` prend les tâches, le code et la longueur de ligne sur sa propre ligne, et c'est voulu :
rien n'est coché d'avance sur une ligne de commande, et la fenêtre mesure les lignes en ems là où la
ligne de commande compte des caractères. `correction.activations`, la liste des cases de motifs que
la fenêtre gère, n'est pas une clé du catalogue : `correct` a `--enable` et `--disable`.

Avec `--format json`, un objet par clé — `key`, `description`, `accepts`, `default`, `used_by`,
et `surface`, `"both"` ou `"window-only"` :

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**Le fichier employé, puis chaque clé d'opération avec sa valeur** : `File: <path> (found)`,
`(not found)` ou `(unreadable: <reason>)`, puis une ligne par clé —
`[file]` et le défaut quand le fichier la règle, `[default]` sinon. Une clé que le fichier porte
avec une valeur qu'il ne sait pas lire montre son défaut. Les préférences d'affichage ne sont pas
listées.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

Un fichier absent n'est pas une erreur : `show` écrit les défauts, et le code est `0` — le fichier
de l'utilisateur comme un `FILE` nommé par `--config`, qu'[`adjust`](adjust.md) et
[`correct`](correct.md), au contraire, refusent (voir
[Le fichier de réglages](invocation.md#le-fichier-de-réglages)). Avec
`--no-config` :

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

Avec `--format json`, un premier objet `{"file", "exists"}` — `file` vaut `null` avec
`--no-config` — puis un objet par clé : `key`, `value`, `source` (`"file"` ou `"default"`),
`default`, `used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` et `config unset`

**`set` vérifie la valeur avec le lecteur même du fichier** — celui dont la fenêtre se sert — et la
refuse quand elle ne se lit pas, avant que rien soit écrit. **`unset`** réécrit le défaut, en
commentaire : la clé suit alors le défaut même si une version ultérieure le change. Remettre au
défaut une clé qui y est déjà réussit et ne change rien.

Les deux **réécrivent le fichier en entier et de façon atomique** : chaque préférence d'affichage
qui s'y trouvait est gardée, le bloc d'opération vient en premier et le bloc d'affichage en second,
chacun sous sa bannière. **Une valeur que le fichier ne savait pas lire, et une clé inconnue, ne
sont pas gardées** — la réécriture est celle de la fenêtre, qui les laisse tomber elle aussi.
**Chaque valeur abandonnée est dite**, sur la sortie d'erreur au niveau par défaut, préférences
d'affichage comprises : `<key>: unreadable value '<value>' replaced by its default`. Le fichier et
son dossier sont créés s'ils manquent.

La confirmation va sur la sortie d'erreur, au niveau par défaut : `KEY = VALUE` pour `set`,
`KEY reset to its default (DEFAULT)` pour `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Codes de retour

| Code | Signification |
| :--- | :------------ |
| `0` | fait — y compris `show` sur un fichier absent et `unset` sur une clé déjà à son défaut |
| `1` | erreur d'usage : pas de sous-commande, un argument manquant, une clé ou une valeur refusée, `--no-config` avec `set` ou `unset`, `--config` avec un nom vide, pas de fichier de l'utilisateur et pas de `--config` ; rien n'est écrit |
| `2` | le fichier ne peut être ni lu ni écrit ; rien n'est écrit |

## Erreurs

| Ce qui la déclenche | Ce qui est écrit, sur la sortie d'erreur |
| :------------------ | :--------------------------------------- |
| une préférence d'affichage | `<key> is a display preference; set it in the window` |
| une clé inconnue | `unknown setting <key>; subedit-cli config list names them` |
| une valeur que la clé n'accepte pas | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` ou `unset` avec `--no-config` | `--no-config has nothing to write to` |
| aucun fichier de l'utilisateur ne peut être situé, et pas de `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` avec `--no-config` | `--config excludes --no-config` |
| `--config` avec un nom vide | `--config needs a file name` |
| un fichier qui existe et ne se lit pas, pour `set` ou `unset` | `<path>: cannot be read; nothing written` |
| un fichier qui ne peut pas être écrit | `<path>: cannot be written`, ou `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
