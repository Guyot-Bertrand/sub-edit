# `snap`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

מציבה **כל חותמת זמן** — התחלה וסוף — על הפריים הקרוב ביותר בקצב הפריימים שניתן. דבר אינו מתוזמן מחדש: כל מיקום זז בחצי פריים לכל היותר.

## `snap` אינו `framerate`, וכשמחליפים ביניהם זה לא ניכר

לשתי הפקודות אותה צורה — קבצים, יעד, קצב פריימים — והן עושות דברים הפוכים.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| מה היא מחשבת | `t' = t x input rate / output rate` | הפריים הקרוב ביותר, על הרשת של `--rate` |
| בכמה היא מזיזה | ביחס לזמן — **כמה שניות** בסרט באורך מלא | חצי פריים לכל היותר, **בכל מקום** |
| לאיזה קובץ | קובץ ש**תזמונו שגוי** | קובץ ש**תזמונו נכון** ורשתו שגויה |

**מקרה השימוש של `snap`**, בפירוט: סרט בקצב 25 פריימים לשנייה, קובץ כתוביות שנכתב על רשת של 24, שורות שכבר נמצאות פחות או יותר במקומן עד כדי כמה אלפיות שנייה. אין מה לתזמן מחדש — רק להחזיר את חותמות הזמן אל הפריימים הנכונים.

**מקרה השימוש של `framerate`** הוא האחר: קובץ שתוזמן למאסטר ב-25 ורוצים לנגן אותו על עותק ב-23.976, ותזמונו סוטה בכמה שניות עד סוף הסרט.

לקחת אחת במקום השנייה **אינו גורם לשגיאה**: שתיהן מקבלות את הקובץ וכותבות תוצאה. שורת הדיווח היא שמבדילה ביניהן — `snap` תמיד אומרת בכמה הזיזה את המיקומים.

<!-- example: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<file>...` | כן | נתיב אחד או יותר | — |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--rate` | **כן** | קצב פריימים בפריימים לשנייה, נכתב כמו ב-[`framerate`](framerate.md#איך-כותבים-קצב-פריימים) | — |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

`--rate` מקבלת **כל קצב פריימים תקף**, ולא רק את שמונת התקניים: זו בחירת המשתמש, כמו ב-`framerate`. קצב פריימים אפס או שלילי נדחה.

פורמט הקובץ שנקרא **נשמר**: שינוי הפורמט הוא עבודתה של [`convert`](convert.md).

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב. עם `--dry-run`, גם אז דבר: אין קובץ, ושורת הדיווח אומרת זאת — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, שהוחזרו כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריה **כל אבחון קריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

**שני המספרים הם עיקרה של השורה.** חצי פריים ב-25 פריימים לשנייה הוא עשרים אלפיות שנייה, ושום דבר גדול מזה אינו יכול להיכתב שם. מי שרצה את `framerate` והקליד `snap` קורא "by at most 20 ms" במקום שציפה לשניות, ויודע מיד.

## דוגמה

שורה ב-`00:00:01,010`, מיושרת על 25 פריימים לשנייה — כלומר על כפולה של ארבעים אלפיות שנייה:

<!-- example: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## שתי תכונות

**יישור פעמיים אינו משנה דבר בפעם השנייה.** מיקום שכבר על פריים נשאר שם: זו אותה פעולה שמוחלת על אותה תוצאה.

**סדר הכתוביות נשמר.** עיגול לפריים הקרוב ביותר הוא מונוטוני, ולכן שתי התחלות ברווח של פריים לפחות נשארות כך. קרובות יותר, הן יכולות להתמזג — וזו חריגה ש-[`inspect`](inspect.md#מה-מדווח-anomalies) יודעת לנקוב בשמה, ולא היפוך.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא את `counts.subtitles`, את `counts.moved` (המיקומים שהוזזו) ואת `counts.furthest_ms` (ההזזה הגדולה ביותר, באלפיות שנייה). הטבלה המלאה היא זו של [הפעלה](invocation.md#מה-כל-תת-פקודה-מציבה-ב-counts).

## קודי יציאה

אלה של הכלי: `0` אם כל קובץ נכתב, `2` אם אף אחד, `3` אם רק חלקם, `1` בשגיאת שימוש.

## שגיאות

אלה של קצב פריימים, מילה במילה אלה של [`framerate`](framerate.md#שגיאות) — שתיהן קוראות את אותו ערך באותה דרך:

| מה גורם לה | הודעה |
| :------------------ | :------ |
| קצב פריימים אפס או שלילי | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| פסיק עשרוני | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| טקסט שאינו מספר | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| קצב פריימים מופרז | `"…" is not a frame rate: no video runs at that many frames per second` |

**דבר אינו נכתב בלי אחת משלוש אפשרויות היעד**, ושגיאותיהן משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
