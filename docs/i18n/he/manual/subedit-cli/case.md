# `case`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

מעביר את הטקסטים ל**אות גדולה בכל מילה, לאות גדולה בתחילת משפט, לרשיות או לקטנות**, בלי לגעת
בתגית. זוהי `כלים ▸ רשיות/קטנות` של ה[חלון](../subedit-gui/operations.md), עם כלליה.

| `--to` | השפעה | התוצאה עבור `bonjour marie` |
| :----- | :---- | :---------------------- |
| `title` | כל מילה מקבלת אות גדולה, השאר יורד | `Bonjour Marie` |
| `sentence` | האות הראשונה של הטקסט, והיא בלבד; השאר יורד | `Bonjour marie` |
| `upper` | הכול באותיות רשיות | `BONJOUR MARIE` |
| `lower` | הכול באותיות קטנות | `bonjour marie` |

- **מה שבא לפני האות הראשונה או הספרה הראשונה אינו נוגעים בו**: מקף דו־שיח,
  מרכאה פותחת או סוגריים נשמרים (`- bonjour` נותן `- Bonjour`).
- **תגיות אינן טקסט**: הן אינן משנות רשיות — `<I>` נשאר `<I>` — ואינן
  משנות מקום: תגית שחותכת מילה ממשיכה לחתוך אותה גם אחר כך.
- **ICU קובע היכן מילה מתחילה**: `l'été sera chaud` נותן `L'été Sera Chaud`, והדגשים
  נשמרים (`éléonore` נותן `Éléonore`).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<file>...` | כן | נתיב אחד או יותר | — |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--to` | **כן** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | לא | `N-M`, או `N-` עד הסוף — ראו [`adjust`](adjust.md#--range) | כל הקובץ |
| `--document` | לא | `main` או `translation` — ראו [תרגום](invocation.md#תרגום) | `main` |
| `-t`, `--translation-file` | עם `--document translation` | קובץ תרגום, עבור קלט **אחד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **אחת מהשלוש**, מלבד עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

**הטקסט הראשי כברירת מחדל; התרגום עם `--document translation -t FILE`.** אז
התרגום הוא שמשתנה ונכתב, והוא בלבד — ראו [תרגום](invocation.md#תרגום).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

קובץ ש**אף טקסט בו אינו משתנה** — הכול כבר ברשיות המבוקשת — נכתב כמות שהוא
ואומר זאת:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## פלט

**הפלט הסטנדרטי** — ריק: התוצאה היא הקובץ שנכתב. עם `--dry-run`, רשימת
השינויים, המתוארת ב[לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה מתווסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, או `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, מוחזרים כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

`N` הוא מספר הכתוביות **שהטקסט שלהן השתנה**, כפי שנקרא מהפעולה עצמה.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא
את `counts.changed` — ה-`N` של המשפט — ואת `changes`, רשימת הכתוביות ששונו
(`{"subtitle", "document", "before", "after"}`, **ריקה** כשדבר לא השתנה).

## קודי יציאה

אלה של הכלי: `0` אם כל הקבצים עובדו — וקובץ שדבר בו אינו משתנה נחשב מעובד —,
`2` אם אף אחד, `3` אם רק חלקם, `1` בשגיאת שימוש.

## שגיאות

| מה מפעיל אותה | הודעה |
| :------------------ | :------ |
| `--to` חסר | `--to is required` |
| ערך שאינו אחד מארבעת | `--to: … not in {title,sentence,upper,lower}` |
| טווח שנכתב לא נכון או מחוץ לקובץ | ראו [`adjust`](adjust.md#--range) |

שגיאות היעד משותפות לתת-הפקודות שכותבות:
ראו [הפעלה](invocation.md#היעד).
