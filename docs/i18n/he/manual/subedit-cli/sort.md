# `sort`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

מחזירה את הכתוביות **לסדר ההתחלות שלהן**. דבר אחר אינו משתנה: לא הטקסטים, לא המיקומים ולא פורמט הקובץ. הכתוביות **ממוספרות מחדש** מ-1 בעת הכתיבה, כמו בכל קובץ שנכתב בחזרה.

**הליבה לעולם אינה ממיינת מעצמה**: קריאת קובץ משאירה את סדרו כפי שהיה, ו-[`inspect`](inspect.md#מה-מדווח-anomalies) מדווחת על שבר בסדר ב-`subtitle N starts before the previous one starts`. `sort` היא **המחווה שנעשית בכוונה**, לקובץ או לאצווה שסדרם שבור — וניתן לבטלה, בחלון, כמו כל פעולה.

- **המיון יציב**: שתי כתוביות שמתחילות באותו רגע **שומרות על הסדר שהקובץ נתן להן**. אף אחת אינה קודמת לאחרת; הזזתן הייתה החלטה שאיש לא ביקש.
- **`sort` אינה מקבלת לא טווח ולא מסמך**: אין מה לבחור, הסדר הוא זה של ההתחלות.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## מה נספר

**המניין הוא של המקומות ששינו כתובית**, כפי שהליבה מדווחת עליו — מה שההיסטוריה של החלון הייתה מכנה הוזז. כתובית אחת שהוצבה שלא במקומה מסיטה כמה אחרות: בדוגמה שלעיל, אף אחד משלושת המקומות אינו שומר על כתוביתו, והמשפט אומר `3 subtitles moved`.

**קובץ שכבר בסדר נכתב בכל זאת**, ואומר זאת: יעד שניתן הוא יעד שנכתב, והפיכת `sort` לחריג הייתה מחייבת סקריפט לדעת אילו תת-פקודות מפיקות לפעמים קובץ ולפעמים לא.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב. עם `--dry-run`, גם אז דבר: אין קובץ, ושורת הדיווח אומרת זאת — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, או `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, שהוחזרו כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריה **כל אבחון קריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא את `counts.subtitles` (כל כתובית בקובץ) ואת `counts.moved` (המקומות ששינו כתובית; `0` בקובץ שכבר בסדר).

## קודי יציאה

אלה של הכלי: `0` אם כל קובץ עובד — קובץ שכבר בסדר נחשב —, `2` אם אף אחד, `3` אם רק חלקם, `1` בשגיאת שימוש.

## שגיאות

אין ייחודיות ל-`sort`: קובץ שאינו ניתן לקריאה נכשל כמו בכל תת-פקודה, ושגיאות היעד משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
