# הפעלה

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli [global options] <subcommand> [options] <file>...
```

בלי תת-פקודה, הכלי כותב את העזרה שלו ועוצר.

<!-- example: subedit-cli -->
```console
$ subedit-cli
Read, inspect and retime subtitle files.
Usage: subedit-cli [OPTIONS] [SUBCOMMAND]

Options:
  -h,--help                   Print this help message and exit
  --version                   Display program version information and exit
  -v                          Say more: -v is the default, -vv details, -vvv debugs
  -q,--quiet                  Say nothing but errors
  --encoding NAME             Encoding to read the files in; detected by default
  --format text|json          Form of the result on standard output: text or json
  --config FILE               Read the operation settings from FILE instead of the user's settings file
  --no-config Excludes: --config
                              Ignore any settings file: use the built-in defaults
  --list-encodings            Write the encodings that can be read and written, one a line, and stop

Subcommands:
  inspect                     Report what a subtitle file is made of
  convert                     Write a subtitle file out in another format or shape
  shift                       Move every position of a file by a fixed amount
  transform                   Correct every position from two points known to be right
  framerate                   Re-time a file mastered at one frame rate for another
  snap                        Move every position onto the nearest frame of a frame rate (see framerate)
  hearing-impaired            Remove the sounds described between square brackets
  adjust                      Bring the duration of every subtitle within a reading speed and limits
  replace                     Replace a text in the subtitles, without breaking a tag
  case                        Put the texts in title, sentence, upper or lower case, tags intact
  italics                     Put the texts in italics, or take their italics out
  dialogue-dashes             Put dialogue dashes on the lines, or take them off
  sort                        Put the subtitles in the order of their start, keeping ties as they are
  join-repeats                Join the subtitles that repeat each other, one after the other, into one
  disjoint                    Cut overlapping subtitles so that no range overlaps another, merging what overlaps
  repair                      Join the repeated subtitles, put them in order, then make the ranges disjoint
  correct                     Correct the texts with the patterns of the correction assistant
  append                      Put subtitle files one after another into a single file
  split-file                  Cut a subtitle file in two, a head and a tail
  pair                        Write a translation at the positions of its main file, and say how it lined up
  config                      List, show and change the operation settings
```

## שפה

שורת הפקודה מדברת בשפת הסביבה שלה: **העזרה שלה, הדיווח שלה והשגיאות שלה** עוקבים אחר השפה המבוקשת. הדוגמאות במדריך זה מורצות באנגלית — הבלוקים נוצרים על ידי `make manual`, ולעולם אינם מועתקים ביד.

**איך נבחרת השפה.** הראשון מבין `LC_ALL`, `LC_MESSAGES` ו-`LANG` שהוגדר ואינו ריק הוא האזור (locale); `LANGUAGE`, כשהוא מוגדר, מונה אז את השפות הרצויות, מופרדות בנקודתיים (`LANGUAGE=pt_BR:fr`), והוא נקרא ראשון. זה הסדר ש-gettext עוקב אחריו, והתוכנית עצמה קוראת אותו: אין צורך שהאזור ייווצר במחשב.

| כלל | השפעה |
| :--- | :----- |
| קידוד ומתאם | מתעלמים: `fr_FR.UTF-8@euro` מחפש את `fr_FR`, ואחר כך את `fr` |
| `LC_ALL=C`, אזור `C` או `POSIX`, או היעדר אזור | אנגלית, ו-`LANGUAGE` אינו נקרא |
| פריט הנותן שם לאנגלית (`en`, `en_GB`) | אנגלית: מה שבא אחריו ב-`LANGUAGE` אינו נקרא |
| שפה שהקטלוג שלה אינו מותקן | הבאה שביקשו, ואנגלית אחרי האחרונה, **בלי מילה** |
| הודעה שהקטלוג אינו מתרגם | מוצגת באנגלית |

**שם של תבנית תיקון** ש-`correct` אומרת שלא יכלה להחיל עוקב אחר השפה גם הוא, כשהקטלוג מכיר אותו — התבניות שמגיעות עם התוכנית; תבנית משלכם נקראת בשם שהקובץ שלה כותב. קובצי התבניות עצמם אינם נכתבים מחדש לעולם.

הקטלוגים מותקנים תחת `<prefix>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo` ונמצאים מתיקייה של קובץ ההרצה עצמו. אנגלית מובנית. הגרסה הזו מתקינה את `ca`, `cs`, `de`, `es`, `fi`, `fr`, `gl`, `he`, `hu`, `ie`, `is`, `it_IT`, `nl`, `pl`, `pt`, `pt_BR`, `ru`, `sr`, `tr` ו-`zh_CN` — קטלאנית, צ׳כית, גרמנית, ספרדית, פינית, צרפתית, גליסית, עברית, הונגרית, אינטרלינגה, איסלנדית, איטלקית, הולנדית, פולנית, פורטוגזית, פורטוגזית ברזילאית, רוסית, סרבית, טורקית וסינית מפושטת — מתורגמות במלואן. הצרפתית נבדקה על ידי דובר השפה; הקטלאנית, הצ׳כית, הגרמנית, הספרדית, הפינית, הגליסית, העברית, ההונגרית, האינטרלינגה, האיסלנדית, האיטלקית, ההולנדית, הפולנית, הפורטוגזית, הפורטוגזית הברזילאית, הרוסית, הסרבית, הטורקית והסינית המפושטת הן תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.
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
<!-- machine-translated: pl -->
<!-- machine-translated: pt -->
<!-- machine-translated: pt_BR -->
<!-- machine-translated: ru -->
<!-- machine-translated: sr -->
<!-- machine-translated: tr -->
<!-- machine-translated: zh_CN -->
תחת `LANG=fr_FR.UTF-8`, `subedit-cli --help` מתחילה ב"Lire, inspecter et recaler des fichiers de sous-titres." סקריפט שקורא את מילות פלט הטקסט מגדיר `LANGUAGE=en` (או `LC_ALL=C`) — או קורא `--format json`, שאינו משתנה לעולם.

**מה שאינו עוקב אחר השפה לעולם** — מה שסקריפט קורא:

| לעולם אינו מתורגם | למה |
| :--------------- | :-- |
| פלט `--format json`: מפתחות, ערכים, וה-`message` וה-`detail` של כישלונות ואזהרות | אותם בתים בכל שפה; המילים באנגלית |
| קודי יציאה | הארבעה של [קודי יציאה](#קודי-יציאה), בכל שפה |
| השמות שמקלידים: תת-פקודות, אפשרויות, הערכים שלהן (`off`, `srt`), משתני מטא (`FILE`, `TEXT`, `OPTIONS`) | שורת פקודה שנכתבה פעם אחת עובדת בכל שפה |
| שמות תבניות התיקון ש-`correct --enable` ו-`--disable` מקבלות | תבנית נקראת בשם האנגלי שהקובץ שלה מכיל |
| שמות הפורמטים והקידודים | פורמט נקרא בשמו |
| האבחונים של CLI11 עצמה — ארגומנט חובה חסר, אפשרות לא מוכרת, ערך שאפשרות אינה מקבלת — ו-`Run with --help for more information.` שלה | מנוסחים בתוך המנתח שעליו שורת הפקודה בנויה; הם נשארים באנגלית |

בקשת אנגלית, יהיו אשר יהיו תנאי הסביבה:

<!-- example: LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?" -->
```console
$ LC_ALL=C LANGUAGE=fr subedit-cli shift --output-dir out a.srt; echo "code=$?"
shift needs --by, or --to-grid to work the amount out from the positions
code=1
```

## אפשרויות גלובליות

**הן נכתבות לפני שם תת-הפקודה, ורק שם.** `subedit-cli --encoding cp1252 inspect film.srt` היא הצורה; `subedit-cli inspect --encoding cp1252 film.srt` נדחית, עם `The following argument was not expected: --encoding`. כך גם `--version`, `-v` ו-`--quiet`.

**`--help` היא החריג, ואינה באמת חריג**: `subedit-cli inspect --help` מתקבלת מפני שכל תת-פקודה נושאת עזרה משלה, והיא עזרת תת-הפקודה שנכתבת, לא עזרת הכלי.

| אפשרות | השפעה |
| :----- | :---- |
| `-h`, `--help` | כותבת את העזרה ועוצרת; של תת-הפקודה אם ניתן שמה |
| `--version` | כותבת `subedit <version>`, את הספריות שהכלי רץ מולן ואת השפות המותקנות, ועוצרת — ראו [`--version`](#--version) |
| `-q`, `--quiet` | רמה 0 — בלי שום דיווח |
| `-v`, `-vv`, `-vvv` | רמות 1 עד 3; רמה 1 היא ברירת המחדל |
| `--encoding NAME` | קוראת את הקבצים בקידוד הזה, במקום לנחש אותו |
| `--list-encodings` | כותבת את הקידודים שהכלי יכול לקרוא ולכתוב, אחד בשורה, ו**עוצרת** (קוד `0`) — ראו [`--list-encodings`](#--list-encodings) |
| `--format text\|json` | צורת התוצאה בפלט הסטנדרטי: הטקסט, או אובייקט JSON אחד לכל קובץ — ראו [פלט שסקריפט יכול לקרוא](#פלט-שסקריפט-יכול-לקרוא) |
| `--config FILE` | קוראת את הגדרות הפעולות מ-`FILE` במקום מקובץ ההגדרות של המשתמש — ראו [קובץ ההגדרות](#קובץ-ההגדרות) |
| `--no-config` | אינה קוראת שום קובץ הגדרות: כל הגדרת פעולה שלא ניתנה בשורה לוקחת את ברירת המחדל המובנית שלה — ראו [קובץ ההגדרות](#קובץ-ההגדרות) |

`--quiet` ו-`-v` באותה הפעלה נדחות: שתי כוונות מנוגדות אינן מוכרעות לטובת האחרונה שנכתבה.

`--config` ו-`--no-config` באותה הפעלה נדחות אף הן, עם `--config excludes --no-config`, וכך גם `--config` עם שם ריק, עם `--config needs a file name`: שם ריק אינו נותן שם לשום קובץ, וקובץ המשתמש עצמו אינו נקרא במקומו.

### קובץ ההגדרות

**[`adjust`](adjust.md) ו-[`correct`](correct.md) קוראות את הגדרות הפעולות מקובץ ההגדרות כברירת מחדל** — הערכים ש-`Adjust Durations…` ו-`Correct Texts…` זוכרות בחלון, או ש-[`config set`](config.md) כתבה. שום תת-פקודה אחרת אינה קוראת אותו. הקובץ הוא:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

או `~/.config/subedit/settings.conf` כש-`XDG_CONFIG_HOME` אינו מוגדר, ריק או אינו נתיב מוחלט — הקובץ של החלון, המתואר ב[מדריך שלו](../subedit-gui/preferences.md#הקובץ).

**עדיפות, מהחזקה ביותר: האפשרות בשורת הפקודה, אחר כך הקובץ, אחר כך ברירת המחדל המובנית.** אפשרות מנצחת תמיד: `--speed off` מכבה מהירות שהקובץ קובע. נקרא רק **בלוק הפעולות** של הקובץ; העדפות התצוגה (גיאומטריה, עמודות, ערכת נושא…) לעולם אינן נקראות. [`config list`](config.md#config-list) נותנת שם למפתחות, ולאילו מהם שורת הפקודה קוראת: תשעת המפתחות `duration-adjust.*`, `correction.human`, `correction.ocr`, `correction.remove-blank` ו-`correction.line-break.max-lines`. **לכל אחד מהם יש אפשרות שדורסת אותו להפעלה אחת**, יהא אשר יהא האופן שבו הקובץ קובע אותו — הטבלאות של [`adjust`](adjust.md#מה-מספק-קובץ-ההגדרות) ושל [`correct`](correct.md#מה-נבחר-ומה-לא) נותנות להן שם.

**קריאת קובץ המשתמש לעולם אינה מכשילה פקודה.** קובץ חסר נותן את ברירות המחדל; קובץ שאי אפשר לקרוא נותן את ברירות המחדל ואומר זאת; ערך שאי אפשר לקרוא שומר על ברירת המחדל שלו ונקרא בשמו; מפתח לא מוכר מתעלמים ממנו. הקובץ אינו נכתב לעולם על ידי `adjust` או `correct`.

**מה שנקרא נאמר**, ברמה 1 המוגדרת כברירת מחדל, בשגיאה הסטנדרטית, לפני הקובץ הראשון:

| מקרה | שורה |
| :--- | :--- |
| הקובץ נקרא | `settings: read <path>` |
| ערך בו לא ניתן היה לקרוא | `settings: <key>: cannot read '<value>', default kept` |
| קובץ המשתמש אינו קיים | `settings: <path> not found, defaults used` |
| הקובץ אינו ניתן לקריאה | `settings: <path> unreadable, defaults used` |
| `--no-config` | דבר |

עם [`--format json`](#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ של `adjust` ו-`correct` נושא את `settings`: הנתיב שנקרא, או `null` כשלא נקרא אף אחד.


**`--no-config` הופכת הרצה לניתנת לשחזור**: אותה שורה נותנת אותה תוצאה בכל מחשב, יהא אשר יהא מה שהמשתמש קבע בחלון. השתמשו בה בסקריפטים, בבדיקות ובדיווחי תקלות — או ב-`--config FILE` עם קובץ שמצורף אליהם.

**`--config FILE` נותנת שם לקובץ אחר**, הנקרא באותו אופן — **אלא שהוא חייב להתקיים**. שם שניתן בשורת הפקודה אינו בקשה לברירות המחדל: כש-`FILE` אינו קיים, `adjust` ו-`correct` עוצרות לפני שנקרא כל קלט, כותבות `settings file not found: <FILE>` בשגיאה הסטנדרטית, ויוצאות בקוד השימוש `1`. `FILE` שקיים ואי אפשר לקרוא אותו נותן את ברירות המחדל ואומר זאת, כמו קובץ המשתמש. [`config show`](config.md#config-show) עדיין מציגה `FILE` חסר כ-`(not found)`, ו-[`config set`](config.md#config-set-ו-config-unset) עדיין יוצרת אותו.

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine.conf adjust --dry-run a.srt; echo "code=$?"
settings file not found: mine.conf
code=1
```

### `--version`

**גרסת התוכנית, לבדה בשורה הראשונה** — `subedit <version>`, מה שהפלט היה עד 0.15.38 ומה שסקריפט קורא. אחר כך **שורה אחת לכל ספרייה שהכלי רץ מולה**, אחר כך שפות הממשק המותקנות, בפלט הסטנדרטי; הכלי עוצר, בקוד `0`. תת-פקודה שניתנה גם היא אינה רצה, ו-`-q` ו-`-v` אינם משנים דבר.

<!-- example: subedit-cli --version | head -n 1 -->
```console
$ subedit-cli --version | head -n 1
subedit 1.0.2
```

הפלט במלואו, במחשב עם ICU 74.2 ו-Enchant 2.3.3 — המספרים הם של המחשב:

```console
$ subedit-cli --version
subedit <version>
  ICU      74.2 (built with 74.2)
  Enchant  2.3.3 (built with 2.3.3)
  CLI11    2.4.1 (built with 2.4.1)
  Languages: en, fr
```

| ספרייה | למה הכלי משתמש בה |
| :------ | :------------------------ |
| `ICU` | קריאה וכתיבה של הקידודים, זיהויים, שינוי רשיות האותיות |
| `Enchant` | בודק האיות של `correct` |
| `CLI11` | קריאת שורת הפקודה הזו |

- **כל שורה נותנת שתי גרסאות**: זו שנמצאה בזמן ריצה, ראשונה, וזו שהתוכנית קומפלה מולה, בסוגריים. הן שונות כשספריית המחשב שודרגה לאחר בניית התוכנית — הדבר הראשון למסור בדיווח תקלה.
- **הכותרות של Enchant אינן נושאות גרסה**: זו שבסוגריים היא זו ש-`pkg-config` דיווח עליה כשהבנייה הוגדרה.
- **CLI11 מקומפלת לתוך התוכנית**, כשהכותרות שלה הן כל מה שיש ממנה: שתי גרסאותיה זהות תמיד.
- **לא Qt ולא libmpv**: שורת הפקודה אינה מקשרת אף אחת מהן, ומונה רק את מה שהיא רצה מולו. החלון מונה אותן — ראו [המדריך שלו](../subedit-gui/invocation.md#--version).
- **`Languages`** מונה את `en`, שפת המקור, שאינה זקוקה לקטלוג, ואחריה כל קטלוג המותקן תחת `<prefix>/share/subedit/locale` וניתן לקריאה, לפי קוד — השפות ש-`עריכה ▸ העדפות…` מציעה.
- **מה שעוקב אחר השפה**: `built with` ו-`Languages`. השורה הראשונה, השמות, המספרים וקודי השפות לעולם אינם עוקבים.
- **עם `--format json`**, לפני `--version` או אחריה, **אובייקט אחד בשורה אחת**: `schema` (`1`), `command` (`"version"`), `version`, `libraries` — מערך של `{"name":"ICU","loaded":"74.2","built":"74.2"}`, בסדר הטקסט — ו-`languages`, מערך של קודים. דבר בו אינו מתורגם.

<!-- example: subedit-cli --version --format json | grep -o '"name":"[^"]*"' -->
```console
$ subedit-cli --version --format json | grep -o '"name":"[^"]*"'
"name":"ICU"
"name":"Enchant"
"name":"CLI11"
```

### `--list-encodings`

**הקידודים שהכלי יכול לקרוא ולכתוב, אחד בשורה, ממוינים לפי שם**, בפלט הסטנדרטי — ואז הכלי עוצר, בקוד `0`, כמו `--version`: זו התשובה, לא שלב. תת-פקודה שניתנה גם היא אינה רצה.

<!-- example: subedit-cli --list-encodings | grep '^UTF' | head -4 -->
```console
$ subedit-cli --list-encodings | grep '^UTF' | head -4
UTF-16BE
UTF-16LE
UTF-32BE
UTF-32LE
```

- **אלה השמות ש-`--encoding` מקבלת**, הקנוניים; גם הכינויים שלהם (`cp1252`) מתקבלים, בלי שהם מופיעים ברשימה.
- **הכמות תלויה ב-ICU המותקן**, ושום שם אינו מובטח מעבר לאלה שיש בכל ICU (`UTF-8`, `UTF-16LE`, `windows-1252`…). לכן המדריך אינו נותן כמות.
- **`UTF-16` ו-`UTF-32` אינם בה**: ICU מכירה אותם, אבל המתמיר שלהם כותב סימן משלו, והמודל דוחה אותם — ראו בהמשך. רשימה לבחור ממנה אינה מציעה את מה שהשדה הבא היה דוחה.
- **עם `--format json`**, **אובייקט אחד לכל קידוד**: `{"schema":1,"command":"list-encodings","name":"UTF-8"}`. זה אינו דוח על קובץ: האובייקטים האלה **אינם נושאים לא `file` ולא `ok`**, החריג היחיד למעטפת של [המבנה](#המבנה). הם עדיין אובייקט אחד בשורה, אחד לכל פריט.
- **רמת הדיווח אינה משנה דבר**: רשימה היא תוצאה, ו-`-q` אינה לוקחת אותה.

### `--encoding`, ומה שאינה יכולה לכפות

**היא חלה על כל תת-פקודה**, כי כולן קוראות: קובץ שקידודו נוחש לא נכון נשאר לא נכון יהא אשר יהא מה שעושים בו, ואפשרות ש-`inspect` בלבד נושאת הייתה משאירה אי-התאמה בלי תרופה.

**השם הוא של ICU, וכל הכינויים שלו מתקבלים**: `cp1252`, `windows-1252` ו-`WINDOWS 1252` נותנים שם לאותו קידוד. הקבוצה אינה כתובה בשום מקום: היא זו ש-ICU של המחשב יכולה להמיר, ו-[`--list-encodings`](#--list-encodings) מונה אותה. שם שהיא אינה יכולה להמיר נדחה לפני שנקרא קובץ אחד:

```console
$ subedit-cli --encoding klingon-1 inspect film.srt
no encoding is named "klingon-1"
```

**חריג אחד, ורק אחד: הקידוד חייב לנקוב בסדר הבתים.** `UTF-16` ו-`UTF-32` הם שמות ש-ICU מכירה והמתמיר שלהם כותב **סימן משלו**. קידוד כזה אינו יכול לחיות במודל הזה — הסימן חדל להיות בחירה, ו-`--no-bom` הייתה מתקבלת ולא נשמעת לה — ולכן שניהם נדחים, בקריאה כמו בכתיבה:

```console
$ subedit-cli --encoding UTF-16 inspect film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE` ו-`UTF-16BE` קוראים וכותבים אותם בתים, כולל הסימן; מה ששם מדויק מוסיף הוא שיודעים איזה מהם.

**סימן סדר בתים גובר על האפשרות.** זה הדבר היחיד שקובץ כתוביות מצהיר על עצמו, וקריאתו אחרת ממה שהוא מצהיר הייתה ציות לקורא נגד הקובץ. אי-ההתאמה **נאמרת**, כאבחון:

```console
$ subedit-cli -vvv --encoding cp1252 inspect avec-bom.srt
avec-bom.srt: a byte order mark that contradicts the encoding asked for ("UTF-16LE"), settled by the reader
```

זה מה ש-Gaupol עושה — היא מתחילה את הקריאה מחדש בקידוד של הסימן — אבל בלי לומר זאת; כאן הפער בין מה שבוקש למה שנעשה אינו שתיקה.

## תת-פקודות

| תת-פקודה | מה היא עושה | האם היא כותבת? |
| :------------ | :-------------- | :----------- |
| [`inspect`](inspect.md) | מדווחת ממה עשוי קובץ, בלי לשנות דבר | לא |
| [`convert`](convert.md) | כותבת קובץ בפורמט אחר, או בצורה אחרת | כן |
| [`shift`](shift.md) | מזיזה כל מיקום באותה כמות | כן |
| [`transform`](transform.md) | מתקנת כל מיקום לפי שתי נקודות שידוע שהן נכונות | כן |
| [`framerate`](framerate.md) | מתזמנת מחדש קובץ מקצב פריימים אחד לאחר | כן |
| [`snap`](snap.md) | מזיזה כל חותמת זמן אל הפריים הקרוב ביותר | כן |
| [`hearing-impaired`](hearing-impaired.md) | מסירה את ההערות לכבדי שמיעה | כן |
| [`adjust`](adjust.md) | מתאימה את המשכים למהירות קריאה ולמגבלות | כן |
| [`replace`](replace.md) | מחליפה טקסט בכתוביות, בלי לשבור תגית | כן |
| [`case`](case.md) | מעבירה את הטקסטים לאות גדולה בכל מילה, בתחילת משפט, לאותיות רשיות או קטנות | כן |
| [`italics`](italics.md) | מעבירה את הטקסטים לנטוי, או מסירה מהם את הנטוי | כן |
| [`dialogue-dashes`](dialogue-dashes.md) | מציבה מקפי דו־שיח, או מסירה אותם | כן |
| [`sort`](sort.md) | מחזירה את הכתוביות לסדר ההתחלה שלהן | כן |
| [`join-repeats`](join-repeats.md) | הופכת לכתובית אחת את חלקי הטקסט האחד העוקבים זה את זה | כן |
| [`disjoint`](disjoint.md) | חותכת את הכתוביות החופפות כך שאף טווח אינו חופף עוד טווח אחר | כן |
| [`repair`](repair.md) | משרשרת את `join-repeats`, את `sort` ואת `disjoint` | כן |
| [`correct`](correct.md) | מתקנת את הטקסטים בתבניות של Gaupol: הערות, צירוף ופיצול מילים, שגיאות נפוצות, רשיות, שבירת שורות | כן |
| [`append`](append.md) | מוסיפה קבצים אחרי הראשון, ל**קובץ אחד** | כן |
| [`split-file`](split-file.md) | חותכת קובץ ל**שניים**: ראש וזנב | כן |
| [`pair`](pair.md) | כותבת תרגום במיקומי הקובץ הראשי שלו, ואומרת איך שורותיו התיישרו | כן |
| [`config`](config.md) | מונה, מציגה ומשנה את הגדרות הפעולות של קובץ ההגדרות | קובץ ההגדרות בלבד |

כולן נמצאות כאן, עשרים ואחת; עזרת הכלי מונה אותן באותו סדר.

**רק אחת אינה נוגעת בדבר**, והיא העמודה הימנית: `inspect` קוראת ומדווחת. תשע עשרה האחרות כותבות, ומה שבא בהמשך תקף לשבע עשרה מהן: ל-[`append`](append.md) ול-[`split-file`](split-file.md) יש לכל אחת מבנה קלט ופלט משלה — N קבצים ל**תוצאה אחת**, קובץ אחד ל**שתיים** — ולכן יעד משלה, שהן מתארות בדף שלהן. [`config`](config.md) אינה כותבת קובץ כתוביות: היא קוראת ומשנה את קובץ ההגדרות, ושום דבר ממה שבא בהמשך אינו חל עליה.

## היעד

שבע עשרה תת-הפקודות שכותבות — [`convert`](convert.md), [`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md), [`hearing-impaired`](hearing-impaired.md), [`adjust`](adjust.md), [`replace`](replace.md), [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md), [`sort`](sort.md), [`join-repeats`](join-repeats.md), [`disjoint`](disjoint.md), [`repair`](repair.md), [`correct`](correct.md), [`pair`](pair.md) — מקבלות את היעד שלהן באותו אופן. (ל-[`append`](append.md) ול-[`split-file`](split-file.md) יש יעד משלהן, בדף שלהן.) [`inspect`](inspect.md) אינה כותבת קובץ ואינה מקבלת אף אחת מהאפשרויות האלה.

**יעד שכבר קיים נדרס**, בלי שאלה ובלי אפשרות להימנע מכך: הכתיבה אטומית — הקובץ אינו נכתב לעולם בחצי, וכתיבה שהופסקה משאירה את הישן שלם —, אבל התוכן הקודם אובד. זה אינו פגם שהשארנו: זה הכלל, והרצת פקודה שוב על אותה תיקיית פלט מריצה אותה שוב על עצמה.

**דבר אינו נכתב לעולם בלי יעד מפורש.** שלוש הדרכים לתת אותו מוציאות זו את זו, אחת נחוצה: בלעדיה דבר אינו נכתב וקוד היציאה הוא `1` — פרט ל-[`--dry-run`](#לראות-לפני-שכותבים), שאינה כותבת דבר ולכן אינה דורשת יעד.

| אפשרות | לאן התוצאה הולכת |
| :----- | :---------------- |
| `--output FILE` | אל הקובץ הזה, בשם המדויק הזה; קלט אחד |
| `--output-dir DIR` | אל התיקייה הזו, בשם הבסיס של הקלט |
| `--in-place` | על הקלט, בכתיבה אטומית |

כלי שדורס את הקלט שלו מפני שלא אמרו לו דבר הוא כלי שמפסיקים להשתמש בו בפעם השנייה. הסירוב עולה שורה וחוסך את הקובץ.

`--output` עם כמה קלטים נדחית: הקובץ האחרון היה נכתב על הקודמים. באצווה, `--output-dir` היא היחידה מבין השלוש שיש בה היגיון, יחד עם `--in-place`.

**תיקיית הפלט נוצרת** אם היא חסרה, כולל ההורים — עבור `--output-dir`, ועבור התיקייה ש-`--output` כותבת אליה. זה נעשה לאחר שנבדקה שורת הפקודה ו**לפני הקובץ הראשון**. אם אי אפשר, נאמר זאת פעם אחת, לא נקרא שום קובץ, וקוד היציאה הוא `2`.

**האצווה נשפטת כולה לפני הכתיבה.** יעד כל קלט מחושב קודם, ומכך נובעים שני סירובים — שגיאות שימוש, קוד `1`, **אף קובץ אינו נכתב** ואף תיקייה אינה נוצרת:

- **שני קלטים שהיו מגיעים לאותו קובץ** — `a/film.srt` ו-`b/film.srt` עם `--output-dir`, או `a/film.srt` ו-`b/film.vtt` שהומרו שניהם ל-WebVTT, שהיעד הסופי שלו שינה סיומת. ההודעה נותנת שם ליעד ולשני הקלטים; שינוי שם השני או מתן לאחרון לנצח היה מאבד קובץ בלי לומר זאת, והסירוב עולה שורה;
- **יעד שהוא אחד הקלטים**, בלי `--in-place` — כולל דרך `--output-dir .`, דרך `--output` או דרך קישור סמלי. כתיבה על הקלט היא מחווה שנותנים לה שם.

שני נתיבים נותנים שם לאותו קובץ כשהמערכת אומרת כך, לא כשהם כתובים אותו דבר: `in/../in/film.srt` ו-`in/film.srt` הם קובץ אחד.

**הסיומת עוקבת אחר הפורמט שנכתב.** היא משתנה רק ב-[`convert`](convert.md), תת-הפקודה היחידה ששינתה פורמט; האחרות שומרות על הפורמט של הקובץ שנקרא, ולכן על הסיומת שלו.

| מה גורם לה | הודעה |
| :------------------ | :------ |
| אין יעד, בלי `--dry-run` | `no destination given: use --output, --output-dir or --in-place` |
| שני יעדים | `--output, --output-dir and --in-place exclude one another` |
| `--output` על אצווה | `--output names one file but several were given: use --output-dir instead` |
| שני קלטים, יעד אחד | `<destination>: would be written by both <input> and <input>` |
| קלט היה נדרס | `<destination>: would be written over the input <input>` — אותו משפט עבור [`append`](append.md) ו-[`split-file`](split-file.md) |
| תיקיית פלט שאי אפשר ליצור | `<directory>: cannot be created: permission denied`, או `: cannot be created` |
| יעד שאי אפשר לכתוב | `<path>: <destination>: cannot be written: permission denied`, או `: cannot be written` |
| תו שחסר בקידוד הנכתב | `<path>: holds a character the chosen encoding cannot write` |

**האחרונה ראויה למשפט.** קובץ נכתב בחזרה ב**קידוד שבו נקרא**, כמו עם סוף השורה והסימן שלו. אם שינוי הכניס לתוכו תו שהקידוד הזה אינו מכיר — `ł` ב-Latin-1 —, הכתיבה נכשלת ו**דבר אינו נכתב**. החלפתו ב-`?` הייתה אובדן טקסט לעיני מי שכתב אותו זה עתה.

## לראות לפני שכותבים

**`--dry-run`** מתקבלת בכל אחת משש עשרה תת-הפקודות שכותבות, ופירושה אותו דבר בכל מקום: **לקרוא, לחשב, לדווח, לא לכתוב קובץ**. מוסיפים אותה לשורה שעמדו להריץ; ליישום מריצים שוב את אותה שורה בלעדיה. החישוב דטרמיניסטי: מה ש-`--dry-run` הראתה הוא מה שההרצה הבאה תכתוב, כל עוד הקובץ לא השתנה.

- **היא אינה כותבת קובץ ואינה יוצרת תיקייה**, כולל עם `--in-place`: קובץ הקלט נשאר כפי שהיה.
- **היא אינה דורשת יעד.** אפשר לתת אחד, ו**הוא נבדק אז כמו בלי `--dry-run`**: שני קלטים לאותו יעד, קלט שהיה נדרס, `--output` על אצווה, שתי דרכים לומר לאן — שורת פקודה שנדחית בלי `--dry-run` נדחית גם איתה, באותו משפט ובאותו קוד `1`.
- **קוד היציאה הוא של הרצה אמיתית**: `0` כשהכול חושב, `2` או `3` כשלא ניתן היה לקרוא או לעבד קבצים, `1` לשימוש. תו שקידוד הקובץ לא יכול היה לכתוב מכשיל הרצת ניסיון כפי שהוא מכשיל הרצה אמיתית. **העובדה שיש שינויים אינה משנה את הקוד**: ארבעת הקודים אומרים אם הכלי הצליח, לא אם הקובץ תקין.
- **הדיווח אומר זאת.** במקום שבו הרצה אמיתית כותבת `-> <destination>`, הרצת ניסיון כותבת `(dry run, nothing written)`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by 1 --dry-run a.srt; ls -- *.srt
a.srt: 1 subtitle shifted by 1.000 s (dry run, nothing written)
a.srt
```

**בתת-פקודה ששינתה טקסטים** — [`hearing-impaired`](hearing-impaired.md), [`replace`](replace.md), [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) ו-[`correct`](correct.md) —, **הפלט הסטנדרטי נושא את השינויים המוצעים**. בלוק אחד לכל כתובית ששונתה, בסדר הקובץ:

```
<path>: subtitle <number>
- <the text before, line by line>
+ <the text after, line by line>
```

- ה**מספר** הוא זה שיש לכתובית בקובץ שנקרא, נספר מאחת;
- כל שורה של הטקסט שלפני נושאת `- ` לפניה, כל שורה של הטקסט שאחרי `+ `: טקסט בכמה שורות נקרא כמות שהוא, ושורה שמתחילה בעצמה במקף דו־שיח נותנת `- - `;
- **כתובית שהפעולה הייתה מסירה** אומרת זאת בשורה הראשונה שלה, `<path>: subtitle <number> (removed)`, ואין לה שורות `+`;
- כתובית של התרגום, עם `--document translation`, מכריזה על עצמה `(translation)` באותו מקום — ראו [תרגום](#תרגום);
- השורה הראשונה של כל בלוק מתחילה בנתיב, כך ש-`grep '^film.srt: '` מוצא את הבלוקים של קובץ אחד באצווה;
- **אין בלוק** כשדבר לא ישתנה.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\n- [Grincement]\n- Qui est là ?\n\n' > mentions.srt; subedit-cli hearing-impaired --dry-run mentions.srt
mentions.srt: 2 subtitles cleaned, 1 removed (dry run, nothing written)
mentions.srt: subtitle 1 (removed)
- [Bruit de pas]
mentions.srt: subtitle 2
- Attends [il tousse] Marie.
+ Attends Marie.
mentions.srt: subtitle 3
- - [Grincement]
- - Qui est là ?
+ Qui est là ?
```

**הרשימה זהה ב-JSON**: אובייקט הקובץ נושא מערך `changes`, ראו [המבנה](#המבנה), עם `after` שווה `null` עבור כתובית שהוסרה. **היא מחושבת רק כש-`--dry-run` או `--format json` מבקשות אותה**: אצווה של אלפי כתוביות אינה בונה אותה לחינם.

ב**תת-הפקודות האחרות** — [`convert`](convert.md), [`shift`](shift.md), [`transform`](transform.md), [`framerate`](framerate.md), [`snap`](snap.md), [`adjust`](adjust.md), [`sort`](sort.md), [`append`](append.md), [`split-file`](split-file.md), [`pair`](pair.md) —, הפלט הסטנדרטי נושא את מה שהפלט של הרצה אמיתית היה נושא, כלומר דבר בטקסט; ב-JSON, אובייקט כל קובץ עם `"dry_run":true`, `"destination":null` ואותם `counts`.

## שני פלטים, שני תפקידים

| פלט | מה הוא נושא |
| :----- | :--------------- |
| סטנדרטי | **התוצאה, והתוצאה בלבד** — הדוח של `inspect`, השינויים המוצעים של טקסט ב-[`--dry-run`](#לראות-לפני-שכותבים); עם `--format json`, אובייקט אחד לכל קובץ — אחד לכל הרצה עבור [`append`](append.md), [`split-file`](split-file.md) ו-[`pair`](pair.md) |
| שגיאה | **כל השאר** — הדיווח, האזהרות, השגיאות |

הפיצול הזה הוא שמאפשר להפנות את התוצאה בלי לקבל את הסיפור בתוכה:

```console
$ subedit-cli inspect *.srt > rapport.txt
```

הדוח הולך לקובץ, הדיווח נשאר על המסך.

## פלט שסקריפט יכול לקרוא

**`--format json`** מחליפה את הטקסט של הפלט הסטנדרטי ב**אובייקט JSON אחד לכל קובץ קלט**, בפורמט JSON Lines: אובייקט קומפקטי אחד בשורה, ב-UTF-8 בלי סימן, וסוף השורה הוא `\n`. `text` היא ברירת המחדל ואינה משתנה. זו אפשרות גלובלית, ולכן נכתבת לפני תת-הפקודה; ערך שאינו `text` או `json` הוא שגיאת שימוש, קוד `1`.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nUn.\n\n' > a.srt; subedit-cli --format json shift --by 1 --output-dir sortie a.srt absent.srt; echo "code=$?"
a.srt: 1 subtitle shifted by 1.000 s -> sortie/a.srt
{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":false,"destination":"sortie/a.srt","counts":{"subtitles":1,"shifted_by_ms":1000},"warnings":[]}
absent.srt: does not exist
{"schema":1,"command":"shift","file":"absent.srt","ok":false,"error":{"kind":"not-found","message":"absent.srt: does not exist"}}
1 of 2 files shifted, 1 failed
code=3
```

**אובייקט אחד לכל קלט, אף פעם אפס, אף פעם שניים**, בין שהקלט הצליח ובין שלא, בסדר שבו ניתנו הקלטים — או נמצאו, עם [`--recursive`](lots.md). סופרים שורות, מוצאים קבצים. **שלוש תת-פקודות הן החריג, כי מבנה הקלט והפלט שלהן אינו של אצווה: [`append`](append.md) (N קלטים, פלט אחד), [`split-file`](split-file.md) (קלט אחד, שני פלטים) ו-[`pair`](pair.md) (שני קבצים, פלט אחד) כותבות **אובייקט אחד לכל ההרצה**, שנותן שם לקובץ שהדף שלהן אומר.** **תת-הפקודות שכותבות מחדש כותבות אפוא גם בפלט הסטנדרטי**, מה שהחוזה שלהן לא עשה בטקסט: הקובץ שלהן נשאר התוצאה שלהן, והאובייקט הוא הדוח עליו.

**הדיווח אינו משתנה.** `-q`, `-v`, `-vv` ו-`-vvv` פועלות על פלט השגיאות, בטקסט, כמו בלי `--format`; **הפלט הסטנדרטי אינו משתנה בהן**. אבחוני הקריאה, שהטקסט שומר לרמה 3, הם נתונים ו**תמיד** ב-`warnings`. שגיאת שימוש אינה כותבת אובייקט: היא מתגלה לפני שנגעו בקובץ כלשהו, ולכן אין אליו מה לצרף אותה; היא הולכת לפלט השגיאות עם קוד `1`, והפלט הסטנדרטי נשאר ריק.

### המבנה

| מפתח | תוכן | נוכח |
| :-- | :------ | :------- |
| `schema` | מספר שלם, גרסת המבנה הזה — `1` | תמיד |
| `command` | שם תת-הפקודה | תמיד |
| `file` | הנתיב **כפי שניתן**, או כפי שהסריקה של תיקייה חיברה אותו | תמיד |
| `ok` | בוליאני | תמיד |
| `error` | `{"kind", "message"}` | אם `ok` שקר |
| `dry_run` | בוליאני: אמת עבור [`--dry-run`](#לראות-לפני-שכותבים) | אם `ok` אמת, בתת-פקודה שכותבת |
| `destination` | הנתיב שנכתב, או `null` כש-`dry_run` אמת: דבר לא נכתב | כנ"ל |
| `counts` | אובייקט של **מספרים שלמים**, ייחודי לתת-הפקודה | כנ"ל |
| `changes` | מערך של `{"subtitle", "document", "before", "after"}`: הטקסטים ששונו, ראו בהמשך | בתת-פקודה ששינתה טקסטים |
| `warnings` | מערך של `{"kind", "line"?, "detail"?, "settled"?}` | אם `ok` אמת |
| `alignment` | איך תרגום הונח, ראו [תרגום](#תרגום) | עם `-t`, ועבור `pair` |
| `constraints` | ההגדרות ששימשו | `adjust` |
| `settings` | קובץ ההגדרות שנקרא, או `null` כשלא נקרא אף אחד — ראו [קובץ ההגדרות](#קובץ-ההגדרות) | `adjust`, `correct`, אם `ok` אמת |
| `inputs`, `tail` | רכיב אחד לכל קובץ שהתווסף; נתיב הזנב | `append`; `split-file` |

`line` חסר באבחון על הקובץ כולו; `detail` חסר כשאין מה להוסיף; `settled` הוא `true` כשהקריאה **הכריעה** בדבר — מספור שנוצר מחדש —, וחסר כשהשאירה את הדברים כפי שהם.

**`changes`** מונה, בסדר הקובץ, כל כתובית שהטקסט שלה משתנה: `subtitle` הוא מספרה בקובץ שנקרא, נספר מאחת; `document` הוא `"main"` או `"translation"`; `before` הוא הטקסט שלפני; `after` הטקסט שאחרי, או **`null` כשהכתובית מוסרת**. מעברי השורה בטקסט הם `\n`. הוא נכתב להרצה אמיתית כמו ל-[`--dry-run`](#לראות-לפני-שכותבים), ו**ריק (`[]`) כשדבר לא משתנה**: נוכחותו אומרת שתת-הפקודה מונה את שינוייה, היעדרו שאין לה כאלה.

**אין מספר נקודה צפה, בשום מקום.** מיקום או משך הם מספר שלם של **אלפיות שנייה** (`…_ms`); ספירה היא מספר שלם; קצב פריימים הוא **מחרוזת** (`"25"`, `"24000/1001"`) מפני שהוא מדויק, מה שנקודה צפה רציונלית אינה יכולה להכיל; ריכוז רשת הוא מספר שלם ב**אלפיות** — `1000` הוא רשת מושלמת. שני פלטים אז משתווים בית אחר בית, בלי עיגול.

**נתיב שאינו UTF-8** — לינוקס מתירה זאת — אינו יכול להיות מיוצג ב-JSON: הוא נכתב עם U+FFFD במקום הבתים הלא תקינים, והאובייקט נושא את האזהרה `path-not-utf8`. הנאמנות לנתיב כזה אינה מובטחת, וזה נאמר.

### מה כל תת-פקודה מציבה ב-`counts`

| תת-פקודה | מפתחות `counts` |
| :------------ | :--------------- |
| [`shift`](shift.md) | `subtitles`, `shifted_by_ms` (עם סימן) |
| [`transform`](transform.md) | `subtitles` |
| [`framerate`](framerate.md) | `subtitles` |
| [`snap`](snap.md) | `subtitles`, `moved` (מיקומים שהוזזו), `furthest_ms` (ההזזה הגדולה ביותר) |
| [`hearing-impaired`](hearing-impaired.md) | `cleaned` (טקסטים שנכתבו מחדש), `removed` (כתוביות שהוסרו) |
| [`replace`](replace.md) | `replaced` (התאמות שהוחלפו בכתוביות ששונו), `matched` (התאמות שנמצאו) |
| [`case`](case.md), [`italics`](italics.md), [`dialogue-dashes`](dialogue-dashes.md) | `changed` (כתוביות שהטקסט שלהן השתנה) |
| [`correct`](correct.md) | `corrected` (טקסטים ששונו), `removed` (כתוביות שהוסרו) — לעולם לא התאמות |
| [`sort`](sort.md) | `subtitles`, `moved` (המקומות שבהם כתובית התחלפה) |
| [`adjust`](adjust.md) | `subtitles` (הכתוביות שאליהן כיוונו), `adjusted` (אלה שסופן הוזז), ואחר כך קבוצת `sacrificed`: `speed`, `minimum`, `gap`; ומחוץ ל-`counts`, `constraints` — ההגדרות ששימשו |
| [`convert`](convert.md) | `subtitles`, ואחר כך מה שההמרה איבדה: `lost_ends` ו-`lost_header` (0 או 1), `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` |
| [`append`](append.md) | `files`, `subtitles`, `appended`, ואחר כך `lost_ends`, `lost_header`, `joined_lines`, `lost_tags`, `lost_fields`, `furthest_ms` מסוכמים; ומחוץ ל-`counts`, `inputs` — רכיב אחד לכל קובץ שהתווסף |
| [`split-file`](split-file.md) | `subtitles`, `head`, `tail`; ומחוץ ל-`counts`, `tail` — נתיב הזנב (`destination` הוא הראש) |
| [`pair`](pair.md) | `subtitles` (אלה של הקובץ שנכתב); ומחוץ ל-`counts`, `alignment` |
| [`inspect`](inspect.md) | אין `counts`: תיאור הקובץ, ראו הדף שלה |

### מזהי שגיאה

`error.kind` הוא אחד מאלה; **קורא מתייחס למזהה לא מוכר כ"אחר"**.

| `kind` | מה זה |
| :----- | :----------- |
| `not-found`, `permission-denied`, `io` | המערכת סירבה לקרוא — או לכתוב — את הקובץ |
| `unknown-format`, `no-subtitle-found`, `undecodable` | הקובץ נקרא ואינו מניח לקרוא אותו |
| `unencodable` | תו שהקידוד שנבחר אינו יכול לכתוב |
| `no-byte-order-mark`, `no-frame-rate` | מה ש-`convert` אינה יכולה לכתוב בלי שיאמרו לה |
| `before-the-origin`, `beyond-the-end`, `no-transform`, `no-grid` | פעולה שאינה יכולה לחול על הקובץ הזה |
| `range-out-of-bounds` | `--range` שהקובץ הזה אינו מכיל |
| `no-style` | פורמט שאינו כותב סגנון (TMPlayer, LRC), עבור `italics` |
| `refused` | כל סירוב אחר של פעולה |

### מה מובטח ומה לא

**מובטח כל עוד `schema` הוא `1`**: שמות המפתחות, הטיפוס שלהם והיחידה שלהם; משמעות `ok`, `file` ו-`destination`; **אובייקט אחד לכל קלט, בסדר הקלטים** — אחד לכל הרצה עבור שלוש תת-הפקודות בעלות מבנה קלט ופלט משלהן; מזהי ה-`kind` הקיימים; דטרמיניזם — אותם קלטים, אותם ארגומנטים, אותם בתים.

**לא מובטח**: **נוסח ההודעות** (`message`, `detail`), אנגלית עבור אדם, שמתוקנת כשצריך — סקריפט שמשווה אותו שבור מראש; **סדר המפתחות** באובייקט, קבוע אבל לא מובטח על ידי JSON; נוכחות **מפתחות שטרם נוספו**.

**קורא מתעלם אפוא ממפתח לא מוכר**, ומתייחס ל-`kind` לא מוכר כ"אחר": הוספת מפתח או מזהה אינה משנה את `schema`. הסרה או שינוי שם של מפתח, שינוי טיפוס או יחידה, הסרת מזהה מגדילים אותה — זו שבירה, המוכרזת ב-CHANGELOG וכאן.

## רמות הדיווח

כל רמה **מכילה את הקודמת**: עלייה בדרגה מוסיפה, לעולם אינה מחליפה.

| רמה | איך | מה פלט השגיאות נושא |
| :----- | :------ | :------------------------------ |
| 0 | `--quiet` | דבר, **פרט לשגיאות** |
| 1 | כברירת מחדל, או `-v` | שורה אחת לכל קובץ שעובד, וסיכום ברגע שיש כמה |
| 2 | `-vv` | ומה שזוהה: פורמט, קידוד, BOM, סוף שורה |
| 3 | `-vvv` | ועקבות איתור התקלות: בתים שנקראו ונכתבו, ו**כל אבחון קריאה** |

**שגיאות אינן מושתקות לעולם, גם עם `--quiet`.** פקודה שנכשלת בשקט לא הייתה משאירה דבר מלבד קוד היציאה שלה.

הסיכום מופיע רק מקבצים שניים: בקלט יחיד הוא היה חוזר על השורה שלפניו.

## אבחוני קריאה

פורמטי כתוביות נקראים **כמיטב היכולת**: מול חריגה, הקורא אינו עוצר — הוא מכריע, או משאיר את הדברים כפי שהם, ואומר זאת. מה שנתקלו בו מוצג ב**רמה 3**, שורה אחת לכל אבחון, בפלט השגיאות:

```
a.srt: 2 diagnostics while reading
a.srt: line 6: SubRip numbers that do not follow ("7"), settled by the reader
a.srt: line 9: a line that fits nowhere, left as it stands
```

כל שבע עשרה תת-הפקודות מדווחות עליהם, לא רק [`inspect`](inspect.md): קובץ שנקרא כמיטב היכולת ואחר כך נכתב בחזרה עבר את אותן החלטות, ושתיקה עליהן הייתה גורמת להאמין שדבר לא קרה.

**אבחון אינו כישלון לעולם.** הקובץ נקרא, הפקודה הצליחה, קוד היציאה הוא `0`. לכן הם חיים ברמה המפורטת ביותר: נמוך יותר, הם היו קוברים את השורה שאומרת מה נעשה באמת.

### מה כל שורה נושאת

| חלק | מה הוא אומר |
| :----- | :------------- |
| `line N` | היכן, נספר מ-1, כפי שעורך מציג; **חסר** במה שנוגע לקובץ כולו |
| המשפט | במה נתקלו, מבין שתים עשרה הקטגוריות שלהלן |
| `("…")` | הטקסט הפגום של הקובץ, כשהקטגוריה אינה מספיקה; קטוע ל-80 בתים |
| הסוף | מה נעשה בנדון: `settled by the reader`, או `left as it stands` |

**לשתי קטגוריות אין שורה**, והן השתיים שעוסקות בקידוד: הבתים נשקלו, או שהסימן שלהם נקרא, לפני שהייתה שורה אחת של הקובץ. השורה שלהן מתחילה אפוא במשפט, בלי `line N` לפניו — אין מקום לתת לו שם.

**סוף השורה הוא החשוב ביותר.** `settled by the reader` פירושו שהקורא הכריע ושהקובץ שנכתב נושא את החלטתו — מספור חסר נוצר מחדש. `left as it stands` פירושו שלא נגע בדבר מפני ש**רק אתם יכולים להכריע**: כתובית שמסתיימת לפני שהיא מתחילה נשארת כפי שהיא.

### מספר השורה של בלוק

חריגה על **בלוק שלם** מעוגנת בשורת התזמון שלו, ולא בשורה המדויקת שמעוררת אותה. מספור לא עקבי שנכתב בשורה 5 מדווח אפוא בשורה 6, זו של שורת התזמון שבאה אחריו — המספר הפגום נמצא ב-`("…")`. רק החריגות על **שורה** — שורת תזמון שאי אפשר לקרוא, טקסט לפני הראשונה — מדווחות על עצמן.

### שתים עשרה הקטגוריות

| משפט | מה גורם לו |
| :----- | :------------------ |
| `a line that fits nowhere` | שורה שאינה נכנסת לאף בלוק |
| `a timing line that could not be read` | שורת תזמון שאי אפשר לקרוא |
| `a SubRip block without its number` | מספור חסר, שנוצר מחדש בעת הכתיבה |
| `SubRip numbers that do not follow` | מספור שקופץ |
| `text before the first timing line` | טקסט לפני שורת התזמון הראשונה |
| `a WebVTT block of an unknown kind` | בלוק WebVTT שלא זוהה |
| `declares an event column this tool cannot fill` | עמודת אירוע של Sub Station Alpha שדבר כאן אינו ממלא |
| `counts in frames and states no rate; it was read at` | קובץ בפריימים, שאינו מצהיר על קצב פריימים — זה ששימש הוא הפירוט |
| `carries no end times; each one was taken from the next start` | פורמט בלי סופים, TMPlayer או LRC: **אף סוף בטבלה אינו בא מהקובץ** |
| `more than one kind of line ending` | סופי שורה מעורבבים |
| `an encoding nothing declared` | הקידוד הוצע בשקילת הבתים, והוא אינו UTF-8 |
| `a byte order mark that contradicts the encoding asked for` | הקובץ נושא סימן, ו-`--encoding` נתנה שם לאחר — הסימן מנצח |

**שלושת האמצעיים הם של שלב 9**, ושני האחרונים מבין השלושה אינם מדברים על שורה אלא על הקובץ כולו: הם אומרים שערך שהוצג לא נקרא. עמודת `End` שמולאה ב-LRC, מיקום ב-MicroDVD — הקובץ לא נשא דבר מזה.

**פעולה יכולה להוסיף אבחונים משלה**, שאינם אבחוני קריאה: [`correct`](correct.md#תבנית-שאי-אפשר-להחיל) אומרת, עם הכתובית, את התבנית שוויתרה. היא נאמרת ברמה 1 ונושאת, באובייקט הקובץ, את המזהה `pattern-failed` — לעולם לא כישלון של הקובץ.

## תרגום

קובץ תרגום מעובד **כמו כל קובץ כתוביות**: `subedit-cli case --to sentence film.fr.srt` משנה אותו בלי שום חידוש. מה שההתאמה מוסיפה, ומה ששלוש אפשרויות אומרות, צר: **לדעת איך שורות תרגום נופלות על כתוביות הקובץ הראשי** — זו `inspect -t` —, ו**לשנות את התרגום תוך שמירה עליו מונח על הקובץ הראשי שלו**, באותם כללים כמו בחלון.

| אפשרות | היכן | ערך | ברירת מחדל |
| :----- | :---- | :----- | :----- |
| `-t`, `--translation-file` | `inspect`, `hearing-impaired`, `replace`, `case`, `italics`, `dialogue-dashes`, `correct`, `pair` | קובץ התרגום | — |
| `--document` | שש תת-הפקודות של הטקסט, לא `inspect` ולא `pair` | `main` או `translation` | `main` |
| `--align-method` | כמו `-t` | `position` או `number` | `position` |

- **`position`** משווה את אמצע כל שורה לגבולות הכתוביות: שורה חסרה אינה מזיזה את אלה שאחריה. **`number`** שולחת שורה *n* אל כתובית *n*, בלי להסתכל במיקומים. המיקום הוא ברירת המחדל, מהסיבה שיש לחלון: שורה חסרה באמצע מזיזה את כל שאר התרגום שהותאם לפי מספר.
- **`--document translation` דורשת `-t`, ו-`-t` דורשת `--document translation`** בתת-פקודה של טקסט — האחת בלי השנייה היא שגיאת שימוש, קוד `1`, מפני שקובץ שנקרא ושום מחווה אינה משתמשת בו הוא השמטה ולא העדפה. ל-`-t` ב-`inspect` אין `--document`: היא מדווחת. **גם `--align-method` בלי `-t` היא שגיאת שימוש.**
- **`-t` נותנת שם לקובץ, ולכן יש בה היגיון רק עבור קלט יחיד**: עם כמה קלטים, או עם תיקייה שנלקחה ב-`--recursive`, היא נדחית כמו `--output`, **לפני שנקרא קובץ**. באצווה, מריצים הפעלה אחת לכל זוג.
- **קובץ תרגום זהה לקובץ הראשי נדחה** — איך שלא נכתב הנתיב שלו: `<translation>: the file is already open as the main document`, קוד `2`.
- **רק המסמך שאליו כיוונו נכתב**, ב**נתיב שלו ובפורמט שלו**: הפורמט, הקידוד וסופי השורה של קובץ התרגום. `--output` נותנת שם לקובץ התרגום, `--output-dir` מניחה שם את שם הבסיס שלו, `--in-place` כותבת אותו מחדש; **לעולם לא נוגעים בקובץ הראשי**. הנתיבים של הדוח הם של התרגום.
- **הכתוביות שההתאמה מעוררת לחיים** — שורה שדבר לא הגיע אליה — נכתבות לתוך התרגום עם **המיקומים שלהן**, יהא אשר יהא האופן; **כתובית שאיש לא תרגם נכתבת כבלוק בלי טקסט**, שנקרא בלי חריגה.
- **`--encoding` ו-`--frame-rate` חלות על שני הקבצים.**
- תת-הפקודות של **המיקום** — `shift`, `transform`, `framerate`, `snap`, `adjust` — אין להן את האפשרויות האלה: נותנים להן את שני הקבצים, כמו לאצווה, `subedit-cli shift --by 2 film.srt film.fr.srt --output-dir out/`.
- **כתיבת התרגום מתוזמן מחדש על הקובץ הראשי** — הדבר היחיד שההתאמה עושה וקובץ לבדו אינו עושה — היא עבודתה של [`pair`](pair.md), שאין לה לא `--document` ולא `--recursive`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\n[Un oiseau] Bonjour.\n\n2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n' > film.fr.srt; subedit-cli hearing-impaired film.srt -t film.fr.srt --document translation --output propre.fr.srt; cat propre.fr.srt
film.fr.srt: 1 subtitle cleaned, 0 removed -> propre.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

עם `--format json`, אובייקט כל קובץ נושא גם את המפתח `alignment`, שאומר איך התרגום הונח — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}`, כמו המפתח `translation` של [`inspect`](inspect.md#ב-json) — ו**עם `--dry-run`**, `changes` מונה את כתוביות התרגום, כל אחת עם `"document":"translation"`.

## כמה קבצים

כל תת-פקודה מקבלת כמה נתיבים, פרט ל-[`append`](append.md), ל-[`split-file`](split-file.md) ול-[`pair`](pair.md), שמבנה הקלט והפלט שלהן הוא זה שבדף שלהן. כל אחד מעובד באופן עצמאי: כישלון של אחד אינו מפסיק את האחרים, והכישלונות מדווחים בציון הקובץ והסיבה.

**תיקייה מייצגת את הקבצים שהיא מכילה, עם `--recursive` (`-r`)**: בלי האפשרות הזו, מתן תיקייה היא שגיאת שימוש, קוד `1`, ודבר אינו מעובד — עדיף לסרב מאשר לא לעבד דבר ולהחזיר `0`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

הסריקה, העץ שנשמר תחת `--output-dir` ומה שנשאר בצד מתוארים ב[עיבוד עץ קבצים](lots.md).

## דפי ה-man

התקנה מציבה **דף man אחד לכל תת-פקודה** לצד `subedit-cli(1)`: `subedit-cli-shift(1)`, `subedit-cli-convert(1)` והאחרים, אחד לכל שורה של [טבלת תת-הפקודות](#תת-פקודות). `subedit-cli(1)` מכיל את הפרק הזה — האפשרויות הגלובליות, הפלטים, קודי היציאה — וכל דף אחר מכיל את הפרק של תת-הפקודה שלו, כולל הדוגמאות. הדף של החלון, `subedit-gui(1)`, לצדם.

```console
$ man subedit-cli
$ man subedit-cli-shift
```

**הדפים אינם נכתבים בנפרד מהמדריך הזה**: הם נוצרים ממנו בעת בניית התוכנית, ולכן הם אומרים מה שהוא אומר, ואינם יכולים להיסחף ממנו. הסעיף FILES של כל דף נותן שם לתיקייה שבה המדריך מותקן.

| דפים | היכן הם מותקנים |
| :---- | :----------------------- |
| אנגלית | `share/man/man1` |
| כל שפה מותקנת — קטלאנית, צ׳כית, גרמנית, ספרדית, פינית, צרפתית, גליסית, עברית, הונגרית, אינטרלינגה, איסלנדית, איטלקית, הולנדית, פולנית, פורטוגזית, פורטוגזית ברזילאית, רוסית, סרבית, טורקית, סינית מפושטת | `share/man/<language>/man1` — `share/man/fr/man1` |

**`man` בוחר את השפה לפי האזור (locale)**, כמו התוכנית: תחת אזור צרפתי, `man subedit-cli` מציג את הדף הצרפתי, ושפה שדפיה אינם מותקנים מקבלת את האנגליים. `-L` מבקשת שפה יהא אשר יהא האזור:

```console
$ man -L fr subedit-cli-shift
```

## קודי יציאה

| קוד | משמעות |
| :--- | :------------ |
| `0` | הכול הצליח |
| `1` | שגיאת שימוש — אפשרות לא מוכרת, ערך לא תקין, שילוב אסור |
| `2` | אף קובץ לא ניתן היה לעבד |
| `3` | חלק מהקבצים עובדו, אחרים לא |

`2` ו-`3` מובחנים כדי שסקריפט ידע איך לפעול בלי לקרוא שוב את הפלט: "דבר לא עבד" ו"אחד חסר" אינם קוראים לאותה תגובה.

שגיאת שימוש מתגלה **לפני כל עיבוד**: היא אינה משאירה לעולם אצווה מעובדת בחצי.

## שגיאות

| מה גורם לה | מה נכתב, בפלט השגיאות |
| :------------------ | :--------------------------------------- |
| אפשרות או תת-פקודה לא מוכרת | השם הפגום, והפניה ל-`--help` |
| `--quiet` עם `-v` | `--quiet and -v ask for opposite things; give one or the other` |
| `--config` עם `--no-config` | `--config excludes --no-config` |
| `--config` עם שם ריק | `--config needs a file name` |
| קובץ חסר | `<path>: does not exist` |
| קובץ שאי אפשר לקרוא | `<path>: cannot be opened: permission denied` |
| קידוד שכותב סימן משלו | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| בתים שאי אפשר לפענח | `<path>: cannot be decoded in the chosen encoding` |
| פורמט שלא זוהה | `<path>: is in no format this tool knows` |
| אין דבר שנראה ככתובית | `<path>: holds nothing recognisable as a subtitle` |
| `--document translation` בלי `-t` | `--document translation needs the translation file: use -t` |
| `-t` בלי `--document translation` | `-t names a translation to change: use --document translation` |
| `--align-method` בלי `-t` | `--align-method needs a translation file: use -t` |
| `-t` עם כמה קלטים, או תיקייה | `-t names one file but several inputs were given: use one invocation per pair` |
| תרגום זהה לקובץ הראשי | `<translation>: the file is already open as the main document` |
