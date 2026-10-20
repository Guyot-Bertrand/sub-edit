# `config`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

מפרט, מציג ומשנה את **הגדרות הפעולות**: החלק בקובץ ההגדרות שהחלון
ושורת הפקודה חולקים. `Adjust Durations…` ו-`Correct Texts…` זוכרות שם את
ערכיהן, ו-[`adjust`](adjust.md) ו-[`correct`](correct.md) קוראות אותם כברירת מחדל — ראו
[קובץ ההגדרות](invocation.md#קובץ-ההגדרות).

**מה שהיא אינה עושה:** היא לעולם אינה קוראת ואינה כותבת **העדפת תצוגה** (גיאומטריית
החלון, העמודות, ערכת הנושא, אפשרויות החיפוש, בדיקת האיות, הווידאו…). אלה נשארות
עניינו של החלון, נקבעות בחלון ומתוארות ב[מדריכו](../subedit-gui/preferences.md#הקובץ).
היא גם לעולם אינה נוגעת בקובץ כתוביות.

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

## תת-פקודות

| תת-פקודה | מה היא עושה | קובץ |
| :---------- | :----------- | :--- |
| `config list` | כותבת כל מפתח פעולה, עם מה שהוא משמש לו, מה שהוא מקבל, ברירת המחדל שלו ומי משתמש בו — **בלי ערך** | אינה קוראת אותו |
| `config show` | כותבת את הקובץ שבשימוש והאם הוא קיים, ואז כל מפתח פעולה עם ערכו, מקור הערך וברירת המחדל שלו | קוראת אותו |
| `config set KEY VALUE` | בודקת את `VALUE`, ואז כותבת את הקובץ מחדש עם `KEY = VALUE`, ושומרת על כל שאר ההגדרות | קוראת וכותבת אותו |
| `config unset KEY` | מחזירה את `KEY` לברירת המחדל שלו, כתובה כהערה כפי שהקובץ כותב תמיד ברירת מחדל | קוראת וכותבת אותו |

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :------- | :----- | :------ |
| `KEY` | כן, ב-`set` וב-`unset` | מפתח פעולה, כפי ש-`config list` נוקבת בו | — |
| `VALUE` | כן, ב-`set` | מה ש-`config list` אומרת שהמפתח מקבל | — |

ל-`config` אין אפשרות משלה. האפשרויות הגלובליות הנכתבות **לפני** `config` בוחרות את
הקובץ ואת צורת הפלט:

| אפשרות גלובלית | השפעה על `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` ו-`unset` פועלות על `FILE` במקום על קובץ המשתמש, גם כשהוא עדיין אינו קיים; `list` מתעלמת ממנה |
| `--no-config` | `list` פועלת כרגיל; `show` מציגה את ברירות המחדל, עם `File: none (--no-config)`; `set` ו-`unset` נדחות: אין קובץ לכתוב אליו |
| `--format json` | `list` ו-`show` כותבות אובייקט JSON אחד בשורה; `set` ו-`unset` אינן כותבות דבר בפלט הסטנדרטי |
| `-q`, `--quiet` | `set` ו-`unset` אינן מאשרות עוד את מה שכתבו |

## איזה קובץ

בלי `--config`, קובץ המשתמש:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

או `~/.config/subedit/settings.conf` כש-`XDG_CONFIG_HOME` אינו מוגדר, ריק או אינו נתיב
מוחלט. זהו הקובץ שהחלון קורא וכותב; `config show` כותבת את נתיבו בשורה הראשונה שלה:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

כשאף אחד מ-`XDG_CONFIG_HOME` ו-`HOME` אינו נותן מקום, אין קובץ משתמש: `show`, `set`
ו-`unset` נדחות, ו-`--config FILE` נוקב באחד.

## `config list`

**קטלוג מפתחות הפעולה**, בסדר הקובץ: כל מפתח, ואחריו ארבע שורות
מוזחות — למה הוא משמש, `Accepts:` (הקבוצה הסגורה, או הצורה והגבולות), `Default:` ו-
`Used by:` (פקודות `subedit-cli` ופעולות החלון שקוראות אותו). היא אינה קוראת קובץ ואינה
צריכה אחד, ופועלת עם `--no-config`.

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

היא מפרטת **33 מפתחות**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
33
```

**שלושה עשר מהם נקראים בשורת הפקודה**; עשרים האחרים נקראים בחלון בלבד,
ושורת ה-`Used by:` שלהם אומרת זאת, עם האפשרות של `correct` שתופסת את מקומם:

| מפתח | נקרא על ידי |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) ו-`Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` ו-`Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` ו-`Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` ו-`Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) ו-`Correct Texts…` |
| `correction.remove-blank` | `correct` ו-`Correct Texts…` |
| `correction.line-break.max-lines` | `correct` ו-`Correct Texts…` |
| כל מפתח `correction.*` אחר | `Correct Texts…` בלבד — `correct` לוקחת את `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` ו-`--disable` משורתה שלה |

מספר השורות נקבע במפתח `correction.line-break.max-lines`
ובמפתח השני שלצדו, `correction.line-break.skip-max-lines`. ערכם הוא מספר שלם מ-1 עד 1000 — גבולות
`correct --max-lines` —. אורך השורה נקבע במפתח `correction.line-break.max-length`
ובמפתח השני שלצדו, `correction.line-break.skip-max-length`. ערכם הוא מספר גדול מ-0.
`set` דוחה את השאר, וקובץ שנערך ביד ומכיל ערך כזה שומר על ברירת המחדל.

`correct` לוקחת את המשימות, הקוד ואורך השורה משורתה שלה בכוונה: דבר אינו
מסומן מראש בשורת פקודה, והחלון מודד שורות באמים ואילו שורת הפקודה
סופרת תווים. `correction.activations`, רשימת תיבות התבניות שהחלון מנהל, אינה
מפתח בקטלוג: ל-`correct` יש `--enable` ו-`--disable`.

עם `--format json`, אובייקט אחד לכל מפתח — `key`, `description`, `accepts`, `default`, `used_by`,
ו-`surface`, `"both"` או `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**הקובץ שבשימוש, ואז כל מפתח פעולה עם ערכו**: `File: <path> (found)`,
`(not found)` או `(unreadable: <reason>)`, ואז שורה לכל מפתח —
`[file]` וברירת המחדל כשהקובץ קובע אותו, `[default]` כשאינו קובע. מפתח
שהקובץ מכיל עם ערך שאי אפשר לקרוא מציג את ברירת המחדל שלו. העדפות התצוגה אינן מפורטות.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

קובץ חסר אינו שגיאה: `show` כותבת את ברירות המחדל, והקוד הוא `0` — קובץ המשתמש
כמו `FILE` ש-`--config` נוקב בו, ש-[`adjust`](adjust.md) ו-[`correct`](correct.md), לעומת זאת,
דוחות (ראו [קובץ ההגדרות](invocation.md#קובץ-ההגדרות)). עם
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

עם `--format json`, אובייקט ראשון `{"file", "exists"}` — `file` הוא `null` עם `--no-config` —
ואז אובייקט אחד לכל מפתח: `key`, `value`, `source` (`"file"` או `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` ו-`config unset`

**`set` בודקת את הערך עם הקורא של הקובץ עצמו** — הקורא שהחלון משתמש בו — ודוחה
אותו כשאי אפשר לקרוא אותו, לפני שנכתב דבר. **`unset`** כותבת בחזרה את ברירת המחדל,
כהערה: המפתח עוקב אז אחר ברירת המחדל גם אם גרסה מאוחרת משנה אותה. ביטול הגדרה של
מפתח שכבר בברירת המחדל מצליח ואינו משנה דבר.

שתיהן **כותבות את הקובץ מחדש כולו ובאופן אטומי**: כל העדפת תצוגה שכבר נמצאת בו נשמרת,
גוש הפעולות בא ראשון וגוש התצוגה שני, כל אחד תחת כרזתו. **ערך
שהקובץ לא הצליח לקרוא, ומפתח לא מוכר, אינם נשמרים** — הכתיבה מחדש היא של החלון, שמשמיט גם
אותם. **כל ערך שהושמט נאמר**, בשגיאה הסטנדרטית ברמת ברירת המחדל, העדפות
התצוגה בכלל זה: `<key>: unreadable value '<value>' replaced by its default`. הקובץ
ותיקייתו נוצרים כשהם חסרים.

האישור הולך לשגיאה הסטנדרטית, ברמת ברירת המחדל: `KEY = VALUE` ב-`set`,
`KEY reset to its default (DEFAULT)` ב-`unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
```

## קודי יציאה

| קוד | משמעות |
| :--- | :------ |
| `0` | בוצע — כולל `show` על קובץ חסר ו-`unset` על מפתח בברירת המחדל |
| `1` | שגיאת שימוש: אין תת-פקודה, ארגומנט חסר, מפתח או ערך שנדחו, `--no-config` עם `set` או `unset`, `--config` עם שם ריק, אין קובץ משתמש ואין `--config`; דבר אינו נכתב |
| `2` | אי אפשר לקרוא או לכתוב את הקובץ; דבר אינו נכתב |

## שגיאות

| מה מפעיל אותה | מה נכתב, בשגיאה הסטנדרטית |
| :--------------- | :------------------------------------- |
| העדפת תצוגה | `<key> is a display preference; set it in the window` |
| מפתח לא מוכר | `unknown setting <key>; subedit-cli config list names them` |
| ערך שהמפתח אינו מקבל | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` או `unset` עם `--no-config` | `--no-config has nothing to write to` |
| אי אפשר לאתר קובץ משתמש, ואין `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` עם `--no-config` | `--config excludes --no-config` |
| `--config` עם שם ריק | `--config needs a file name` |
| קובץ שקיים ואי אפשר לקרוא, ב-`set` או ב-`unset` | `<path>: cannot be read; nothing written` |
| קובץ שאי אפשר לכתוב | `<path>: cannot be written`, או `<path>: cannot be written: permission denied` |

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
