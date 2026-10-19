# `correct`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

מתקן את הטקסטים לפי **התבניות של Gaupol**, כמו אשף
[`Correct Texts…`](../subedit-gui/correct-texts.md) של החלון — אותן תבניות,
אותו מנוע, אותן ספירות. קיימות כאן שש משימות: **ההערות לכבדי
שמיעה**, **צירוף** ו**פיצול** מילים, **שגיאות נפוצות**,
**רשיות** ו**שבירת שורות**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## מה נבחר ומה לא

**דבר אינו מסומן מראש.** החלון פותח את תיבותיו כפי שהמשתמש השאיר אותן; שורת
פקודה לוקחת את המשימות, הקוד, אורך השורה והשפה משורתה שלה,
לעולם לא מקובץ ההגדרות: לא המשימות שסומנו יום אחד בחלון, ולא תיבות
התבניות שהופעלו שם (`correction.activations`), ולא רשימת ההחלפות של בודק האיות
נקראות, ודבר אינו נכתב בצד ההוא.

**ארבע הגדרות באות מקובץ ההגדרות, כברירת מחדל** — הערכים ש-`Correct Texts…`
זוכר, או ש-[`config set`](config.md) כתב. **אפשרות בשורה מנצחת תמיד**, אחריה
הקובץ, ואחריו ברירת המחדל המובנית; ראו [קובץ ההגדרות](invocation.md#קובץ-ההגדרות)
על מקומו, על `--config` ועל `--no-config`.

| מפתח | מה הוא מספק | אפשרות שדורסת אותו |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | סוגי השגיאות הנפוצות שמוחלים | `--classes` |
| `correction.remove-blank` | האם הכתוביות שהתיקון מרוקן מוסרות | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | מספר השורות המרבי של `line-break` | `--max-lines` |

- **לכל אחת מהארבע יש אפשרות שמבטלת אותה**, יהיה אשר יהיה מה שהקובץ אומר: `--classes human,ocr`
  מחיל את שני הסוגים, `--remove-blank-subtitles` מסיר את הכתוביות שרוקנו, `--max-lines 3`
  מחזיר את שורותיו של Gaupol. `--keep-blank-subtitles` ו-`--remove-blank-subtitles` יחד הן
  שגיאת שימוש.
- **שני הסוגים כבויים בקובץ — מתעלמים מכך, ואומרים זאת** ברמה 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` לא יכולה לכתוב זאת, וזה היה
  משאיר ל-common-errors דבר לעשות. מוחלים במקומם שני הסוגים.
- **מספר שורות מחוץ לטווח 1 עד 1000 אינו נקרא מהקובץ כלל**: הקורא של הקובץ
  דוחה אותו — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, ברמה
  1 — ומשתמשים ב-3 המובנה.

הדוגמאות בדף זה מורצות עם `--no-config`, כדי שיאמרו מה עושות ברירות המחדל
המובנות, יהיו אשר יהיו הגדרותיה של המכונה שמריצה אותן.

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<file>...` | כן | נתיב אחד או יותר | — |
| `--tasks` | **כן** | רשימה מופרדת בפסיקים: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | אין משימה |
| `--code` | **כן** עם משימת תבניות, ו**לא רלוונטי** אחרת | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **כן** עם `join-words` או `split-words`, ו**לא רלוונטי** אחרת | קוד שפה: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | לא | `human`, `ocr` או `human,ocr` | שניהם — או מה שקובץ ההגדרות אומר |
| `--enable`, `--disable` | לא, חוזרת | השם האנגלי של תבנית, או `type:name` | קובצי ה-`.conf` שנשלחים עם הכלי |
| `--max-length` | **כן** עם `line-break` | מספר **תווים** גדול מאפס | **אין** — ראו להלן |
| `--max-lines` | לא | מספר שלם של שורות, גדול מאפס | 3 — או מה שקובץ ההגדרות אומר |
| `--skip-length`, `--skip-lines` | לא | מספר, או `off` | אותם גבולות כמו `--max-length` ו-`--max-lines` |
| `--keep-blank-subtitles` | לא | דגל: הכתוביות שרוקנו נשארות, ריקות | הכתוביות שרוקנו מוסרות — אלא אם קובץ ההגדרות אומר אחרת |
| `--remove-blank-subtitles` | לא | דגל: הכתוביות שרוקנו מוסרות, יהיה אשר יהיה מה שקובץ ההגדרות אומר; נדחה עם `--keep-blank-subtitles` | כבוי |
| `--range` | לא | `N-M`, או `N-` עד הסוף — ראו [`adjust`](adjust.md#--range) | כל הקובץ |
| `--document` | לא | `main` או `translation` — ראו [תרגום](invocation.md#תרגום) | `main` |
| `-t`, `--translation-file` | עם `--document translation` | קובץ תרגום, עבור קלט **אחד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **אחת מהשלוש**, מלבד עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

### המשימות

**סדר הרשימה אינו משנה דבר**: המשימות חלות בסדר של
Gaupol — ההערות, אחר כך צירוף ופיצול מילים, אחר כך השגיאות הנפוצות, אחר כך הרשיות, אחר כך שבירת השורות —, כל אחת על
הטקסט שהקודמת השאירה. כתובית שההערות רוקנו אינה לוקחת
חלק במה שאחר כך.

| משימה | מה היא עושה |
| :---- | :-------------- |
| `mentions` | מסירה את הצלילים שבין סוגריים מרובעים, ובין סוגריים עגולים כש-`Sound in parentheses` מופעל (הסריקה של [`hearing-impaired`](hearing-impaired.md)), ואחר כך את תבניות מילות השיר והדוברים |
| `join-words` | מדביקה בחזרה מילה שנחתכה לשתיים, לפי המילון של `--language` |
| `split-words` | מפצלת שתי מילים דבוקות, לפי אותו מילון |
| `common-errors` | מתקנת את השגיאות הנפוצות: רווחים, פיסוק, ליגטורות, שגיאות OCR |
| `capitalization` | מניחה את האות הגדולה היכן שהשפה דורשת |
| `line-break` | חותכת מחדש את שורות הכתובית כך ששום שורה לא תחרוג מ-`--max-length`, תוך שקילת תבניות השבירה של הקוד |

### קוד התבניות

**`--code` נוקב במפל**: `Zyyy` לכל כתב, ואחר כך הכתב, השפה
והמדינה — `Latn-en-US` קורא את התבניות של `Zyyy`, של `Latn`, של `Latn-en` ושל
`Latn-en-US`. **הוא נדרש**, כי לשורת פקודה אין שפת סביבה:
ברירת מחדל אוניברסלית הייתה מחילה *פחות* ממה שמאמינים, בלי לומר זאת. הוא תקף
לכל משימות אותה הרצה; משימה שרוצה קוד משלה היא הרצה
שנייה. קוד שנכתב לא נכון הוא שגיאת שימוש:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

התבניות שהמשתמש מניח ב-`$XDG_DATA_HOME/subedit/patterns` (או
`~/.local/share/subedit/patterns`) **מתווספות לתבניות שנשלחות עם הכלי**, כמו
בחלון, ותבנית שנשלחת עם הכלי ואי אפשר לקרוא אותה נאמרת ברמה 1: `patterns: <file>, line N (<reason>)`.

### צירוף ופיצול מילים, והמילון

**שתי המשימות האלה אינן קוראות תבנית**: הן שואלות את **מילון המערכת, דרך
Enchant** — תלות של החבילות (`libenchant-2-2` ב-Debian וב-Ubuntu, `enchant2` ב-
Fedora), ש-`subedit-cli` חולק עם החלון. אין להן אפוא לא קוד ולא שם להגדיר:
`--code` חסר משמעות בלי משימת תבניות, ו-`--language` חסר משמעות בלי אחת משתי אלה
— אחת שניתנה בלי השנייה היא שגיאת שימוש, קוד `1`.

- **`join-words`** מדביקה מילה שהמילון אינו מכיר אל זו שלפניה או
  אחריה, **כאשר כיוון אחד בלבד נותן מילה מוכרת**: `hel lo` הופך ל-`hello`; שני
  כיוונים אפשריים, או אף אחד, אינם מכריעים דבר.
- **`split-words`** מפצלת מילה שהמילון אינו מכיר **כאשר בדיוק אחת מהצעותיו
  היא אותה מילה עם רווח**: `hellothere` הופך ל-`hello there`. מילה עם אות
  ראשונה גדולה ואחריה אותיות קטנות נשארת כמות שהיא: לרוב זהו שם.

**בלי מילון לשפה, ההרצה נדחית**, קוד `1`, **לפני שקוראים קובץ**
ובמילות החלון, שמעמעם את הפונקציה ואומר זאת — כאן שורת הפקודה
דוחה ואומרת זאת, ולא שותקת:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**רשימת ההחלפות של החלון אינה נקראת ואינה נכתבת.** היא *מתמלאת* דרך
בחירות המשתמש בבדיקת האיות; לשורת פקודה אין בחירה כזו,
וקריאת הרשימה הייתה גורמת לסקריפט להיות תלוי במה שמחברו הניח בה פעם.

### שבירת שורות

**היא נמדדת בתווים, וזה ההבדל מהחלון.** החלון מודד ב*אמים* —
רוחב שמכויל לפי הגופן, ב-0.55 אם לכל אות —, וברירת המחדל שלו, 24, היא ערך
באמים. לשורת פקודה אין גופן: היא סופרת תווים, אלה של
הטקסט. **העתקת ה-24 כתווים הייתה חותכת כמעט הכול, בלי
לומר זאת**; לכן **אין אורך ברירת מחדל**, ו-`--max-length` נדרשת עם
`line-break`:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` היא 3**, הערך של Gaupol, שהיחידה אינה משנה; שבירת השורות לוקחת
אחת נוספת כששלוש אינן מספיקות. **הדילוג** — כתובית שכבר בתוך הגבולות
נשארת כמות שהיא — לוקח כברירת מחדל **את אותם גבולות** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), כמו ש-Gaupol עושה; `--skip-length off` ו-`--skip-lines off`
מכבים אותו, והטקסט נקרא אז שוב כאחד ונחתך מחדש:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

תבניות השבירה הן של הקוד: `Latn` אינו שובר בין מספר ליחידה שלו ולא
אחרי מקף דו־שיח, `Latn-en` אינו שובר אחרי תווית יידוע, מילת יחס, תואר
או כינוי קניין. **תחת `Zyyy` אין תבנית שבירה**: השורות רק
מאוזנות. **המשימה הזו אינה נדחית לעולם מחוסר תבנית פעילה**: התבניות שלה
שוקלות היכן לשבור, והשבירה מתרחשת גם בלעדיהן. ארבע האפשרויות הגיוניות רק עם
`line-break`; כשהן ניתנות בלעדיה, הן שגיאת שימוש.

### הסוגים, והפעלה לפי שם

**`--classes` מסנן את השגיאות הנפוצות סופית**: ביטול הסימון של `ocr` מוציא את
תבניותיו מההחלה — הליגטורה `ﬁ` נשארת אז `ﬁ`. שלושת סוגי התבניות האחרים
אינם נושאים סוג.

**`--enable` ו-`--disable` קובעים תבנית לפי שמה האנגלי**, כשהשם הוא המפתח
(ADR 0037); שם יכול לציין כמה רשומות, שחולקות אז תיבה
אחת. זה נחוץ, וזה מה שמבדיל מברירות המחדל של החלון: **קובצי ה-`.conf` שנשלחים עם הכלי
מכבים את תבניות ההערות**, ולכן `--tasks mentions` לבדו
לא היה מחיל דבר. שתי התבניות "Sound in brackets" ו-"Sound in parentheses"
מפקדות כל אחת על התוחם שלה — הסוגריים המרובעים, הסוגריים העגולים — ונקבעות באותן
אפשרויות: הפעלת אחת לבדה אינה מסירה את האחרת.

שלושה דברים הם **שגיאות שימוש**, קוד `1`, הנאמרות לפני שקוראים קובץ:

| מה מפעיל אותה | הודעה |
| :------------------ | :------ |
| שם שאינו מציין אף תבנית של המשימות שניתנו, תחת הקוד שניתן | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| שם שמציין תבניות משני סוגים — אז כותבים אותו `type:name`, והסוג הוא `common-error`, `capitalization`, `hearing-impaired` או `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| שם שניתן ל-`--enable` ול-`--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**שגיאת הקלדה נשמעת בקול רם**: החלון מתעלם מהגדרה ישנה כי
הגדרה יכולה לשרוד תבנית שנעלמה; להפעלה אין ניצול כזה.

**משימה בלי אף תבנית פעילה נדחית**, קוד `1` — "אין מה לעשות" היא
תשובה שנאמרת, לא הצלחה שקטה:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

אותו קובץ, עם סריקת הסוגריים המרובעים מופעלת:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## מה נכתב ומה נאמר

הפורמט, הקידוד וסוף השורה של הקובץ שנקרא **נשמרים**. **הכתוביות
שהתיקון מרוקן מוסרות**, כמו תיבת ברירת המחדל של החלון;
`--keep-blank-subtitles` משאיר אותן ריקות, ו-`--remove-blank-subtitles` מסיר אותן
גם כשקובץ ההגדרות שומר אותן. **תרגום שרוקן נשאר ריק**,
ושומר על כתוביתו.

**הספירה היא של החלון** — "Edited N and removed M subtitles": טקסטים
ששונו וכתוביות שהוסרו, **לעולם לא התאמות**. קובץ שדבר בו
אינו משתנה נכתב בכל זאת: יעד שניתן הוא יעד שנכתב.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### תבנית שאי אפשר להחיל

**תבנית שאי אפשר לקרוא, אי אפשר לתרגם או שאינה מסתיימת נקובה בשמה, עם
הכתובית שבה ויתרה**, וכאן שורת הפקודה אומרת יותר
מהחלון: דף האישור נוקב בתבנית פעם אחת בלבד, בלי הטקסט.

| סיבה | משמעות |
| :----- | :------------------- |
| `cannot be translated` | הביטוי שלה משתמש במה שהתרגום ל-ICU דוחה |
| `will not compile` | המנוע דחה את הביטוי שלה |
| `has an invalid replacement` | ההחלפה שלה אינה נקראת כפי ש-Python קורא |
| `timed out` | המנוע ויתר על הטקסט הזה |
| `never settled` | `Repeat` עדיין שינה את הטקסט אחרי מאה מעברים |
| `grew the text too long` | הטקסט חרג ממה שכתובית יכולה להכיל |

ברמה 1, בפלט השגיאה:
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — בלי הסוף כשהתבנית מושמטת בכל טקסט. **המספר הוא
של הקובץ**, יהיו אשר יהיו המשימות שהסירו כתוביות קודם. ב-JSON, זו
אזהרת `pattern-failed` של הקובץ, ש-`detail` שלה נושא את המשפט. **תבנית
שוויתרו עליה אינה קובץ שנכשל**: הקובץ נכתב, וקוד היציאה הוא `0`.

## פלט

**הפלט הסטנדרטי** — ריק, שהרי התוצאה היא הקובץ שנכתב. **עם `--dry-run`, אלה
השינויים המוצעים**, גוש לכל כתובית: מספרה, הטקסט לפני, הטקסט אחרי —
ו-`(removed)` בלי שורת `+` לכתובית שהתיקון היה מסיר; עם
`--keep-blank-subtitles`, היא נשארת, ריקה. ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה מתווסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, התבניות שאי אפשר היה להחיל, קובצי התבניות שאי אפשר לקרוא — ולפניהם, קובץ ההגדרות שנקרא — `settings: read <path>` — ומה ממנו הוזנח |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, ואחריו כל אבחוני הקריאה |

ב-JSON, `counts` נושא את `corrected` (הטקסטים ששונו) ואת `removed` (הכתוביות
שהוסרו), ו-`changes` את הכתוביות שנגעו בהן. עם `-t`, המפתח `alignment` אומר כיצד
התרגום הונח.
`settings` נוקב בקובץ ההגדרות שנקרא, או שהוא `null` כשלא נקרא אחד.

## קודי יציאה

אלה של הכלי: `0` אם כל הקבצים נכתבו, `2` אם אף אחד, `3` אם רק
חלקם, `1` בשגיאת שימוש.

## שגיאות

| מה מפעיל אותה | מה נכתב, בפלט השגיאה |
| :------------------ | :--------------------------------------- |
| קובץ ש-`--config` נוקב בו ואינו קיים — קובץ המשתמש עצמו, כשהוא חסר, נותן את ברירות המחדל | `settings file not found: <FILE>` |
| `--tasks` חסר | `--tasks is required`, ואחריו הפניה ל-`--help` |
| משימה לא מוכרת | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` חסר | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` בלי משימת תבניות | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` חסר עם `join-words` או `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` שאינו קוד שפה | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` בלי אחת משתי המשימות האלה | `--language is for the tasks join-words and split-words, which were not asked for` |
| שפה בלי מילון | `no dictionary for <language>` |
| קוד שנכתב לא נכון | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` חסר עם `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` שאינו מספר גדול מאפס | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` שאינם מספר שלם של שורות | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` או `--skip-lines` שאי אפשר לקרוא | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| אחת מארבע האפשרויות בלי `line-break` | `--max-length is for the task line-break, which was not asked for` |
| סוג לא מוכר | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--keep-blank-subtitles` עם `--remove-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| שם תבנית פגום | ראו הטבלה שלמעלה |
| משימה בלי תבנית פעילה | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, היעד | אלה של [הפעלה](invocation.md#שגיאות) ושל [תרגום](invocation.md#תרגום) |
