# `join-repeats`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

הפקודה הופכת לכתובית **אחת** כתוביות שמציגות אותו דבר בזו אחר זו: טקסט שנחתך לחלקים העוקבים זה את זה, כפי שכמה כלי חלוקה משאירים אותו. זוהי `Join Repeated Subtitles` של החלון — [אותו מנוע](../subedit-gui/lines.md#מיזוג-כתוביות-חוזרות).

**כתובית חוזרת** היא כתובית שיש לה **אותו טקסט, אותו תרגום ואותן הגדרות פורמט** (סגנון, מיקום, הגדרות כתובית) כמו כתובית אחרת, **והיא מתחילה לכל המאוחר כשהאחרת מסתיימת**: הן נוגעות זו בזו או חופפות. רצף של כתוביות חוזרות הופך לכתובית **שמתחילה ראשונה**, וסופה מועבר אל הסוף המאוחר ביותר ברצף; האחרות נעלמות.

- **סדר הקובץ אינו משנה דבר ואינו משתנה**: הכתובית שנשמרת היא זו שמתחילה ראשונה, בכל מקום שבו נמצאות האחרות. קובץ ממוין נשאר ממוין.
- **דבר אינו מצורף בטעות**: טקסט אחר, תרגום אחר, הגדרות אחרות, רווח בין שני חלקים, כתובית ללא טקסט או כתובית שסופה לפני תחילתה — בכל המקרים האלה הן נשארות נפרדות.
- **כדאי לעשות זאת לפני השאר**: מיון ו[חיתוך החפיפות](disjoint.md) קוראים טוב יותר רצף נקי. [`repair`](repair.md) משרשרת את שלושתם, ו-[`convert`](convert.md#הפיכת-הטווחים-לזרים) מקבלת אותם כאפשרות (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

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
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, פרט ל-`--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ומדווח, לא כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

שלוש החתיכות של `Alpha`, העוקבות זו את זו מהשנייה הראשונה עד השנייה השישית, הן עכשיו כתובית אחת. `Beta`, שמופרדת מהן ברווח, נשארה. מעבר שני אינו משנה דבר ואומר זאת: `no repeated subtitles`.

## פלט

**פלט סטנדרטי** — ריק: התוצאה היא הקובץ שנכתב.

| רמה | מה נוסף בפלט השגיאות |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, או `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), ה-`counts` של כל קובץ: `subtitles` (אלו של הקובץ שנכתב), `joined` (הכתוביות שהוסרו מפני שכתובית אחרת מכסה אותן) ו-`chains` (הרצפים שצורפו).

## קודי יציאה

אלו של הכלי: `0` אם כל הקבצים עובדו — קובץ ללא כתוביות חוזרות נחשב מעובד —, `2` אם אף אחד לא עובד, `3` אם רק חלקם עובדו, `1` בשגיאת שימוש.

## שגיאות

אין שגיאות ייחודיות ל-`join-repeats`: קובץ שאי אפשר לקרוא נכשל כמו בכל תת-פקודה, והשגיאות של היעד משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
