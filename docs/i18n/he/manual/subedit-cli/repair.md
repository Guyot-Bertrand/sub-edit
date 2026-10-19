# `repair`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

עושה בבת אחת את מה ששלוש פקודות עושות זו אחר זו: [`join-repeats`](join-repeats.md), אחר כך [`sort`](sort.md), ואחר כך [`disjoint`](disjoint.md). **בסדר הזה, שהוא היחיד שעובד**: החיתוך קורא את סדר ההתחלות, ועדיף שהקטעים החוזרים של אותו טקסט ייעלמו לפני שניהם. בקובץ שחולק גרוע, אינו ממוין וחופף, זו הפקודה להריץ.

ההגדרות הן אלה של `disjoint`, שסוגרת את השרשרת; `--sort` אינו קיים כאן, מפני שהמיון תמיד נעשה.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<file>...` | כן | נתיב אחד או יותר | — |
| `--minimum` | לא | משך: `0.08`, `1.5`, `00:00:00,080`. אפס שומר על כל מרווח | `0.08` (בערך שני פריימים) |
| `--max-lines` | לא | מספר שלם; `0` מבטל את המגבלה | `2` |
| `--dashes` | לא | דגל: מקף לפני הטקסט שכל כתובית מביאה לכתובית ממוזגת | כבוי |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, שנחתכה לשני קטעים עוקבים, הופכת שוב לכתובית אחת; אז היא תופסת את אותו טווח כמו `Beta`, והשתיים הופכות לאחת בלבד, בת שתי שורות, המוקדמת ראשונה.

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב. בפלט השגיאה, ברמה הראשונה, **משפט אחד ששורשר משלושת המניינים**, מופרדים בנקודה-פסיק: מה ש-`join-repeats`, `sort` ו-`disjoint` אומרות כל אחת, במילותיה שלה — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), ה-`counts` של כל קובץ: `subtitles`, `joined`, `moved`, ואחריהם אלה של [`disjoint`](disjoint.md#ב-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`, `ending_before_start`, `extras_dropped`).

## קודי יציאה ושגיאות

אלה של הכלי. אין שגיאה ייחודית ל-`repair` מעבר להגדרות, שנשפטות כמו אלה של `disjoint`: `--minimum` שאינו משך או שלילי, `--max-lines` שאינו מספר שלם, קוד `1`.
