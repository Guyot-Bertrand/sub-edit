# `italics`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

הפקודה מעבירה את הטקסטים ל**נטוי**, או **מסירה מהם את הנטוי**, בתגיות של פורמט הקובץ. זוהי `Italic` של [החלון](../subedit-gui/operations.md).

**שתי מילים ולא מתג**: כפתור החלון שואל את הליבה לאיזה כיוון לפעול; כאן אומרים זאת במפורש. `--on` ו-`--off` מוציאות זו את זו, ואחת מהן נחוצה — השמטה של שתיהן, או מסירה של שתיהן, היא שגיאת שימוש (קוד `1`).

- **`--on`** עוטפת כל טקסט בזוג תגיות אחד, **אחרי שהיא מסירה את אלה שהיו בו**: טקסט שכבר נטוי אינו נעטף פעמיים, ונטוי שנחתך לשניים נעשה מחדש בחתיכה אחת. על פני כמה שורות, זוג אחד מכסה את הכול.
- **`--off`** מסירה את תגיות הנטוי; **שאר התגית נשמר** (`{\b1\i1}` נותן `{\b1}`).
- הטקסט נכתב **כפי שפורמט הקובץ כותב נטוי**: `<i>…</i>` ב-SubRip, ב-WebVTT וב-SubViewer 2, `{\i1}…{\i0}` בשני ה-Sub Station Alpha, `{Y:i}` ב-MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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
| `--on` / `--off` | **אחת מן השתיים** | דגלים | — |
| `--range` | לא | `N-M`, או `N-` עד הסוף — ראו [`adjust`](adjust.md#--range) | כל הקובץ |
| `--document` | לא | `main` או `translation` — ראו [תרגום](invocation.md#תרגום) | `main` |
| `-t`, `--translation-file` | עם `--document translation` | קובץ תרגום, עבור קלט **אחד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, פרט ל-`--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ומדווח, לא כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

**ברירת המחדל היא הטקסט הראשי; התרגום נבחר עם `--document translation -t FILE`.** אז התרגום הוא שמשתנה ונכתב, והוא בלבד — ראו [תרגום](invocation.md#תרגום).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## פורמט בלי סגנון מסרב

**TMPlayer ו-LRC אינם כותבים סגנון**: אין נטוי להוסיף ואין נטוי להסיר. החלון מאפיר את הפריט במקום להסתיר אותו; שורת הפקודה **מסרבת, קובץ אחר קובץ, ומציינת את הסיבה**, ושאר קבצי האצווה ממשיכים. מה שקובע הוא מה שהפורמט יכול לשאת, ולא האופן שבו הוא כותב אותו: SubRip ו-WebVTT כותבים נטוי באותו אופן, ואינם אומרים אותו דבר על צבע.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

ב-JSON, הכישלון נושא את המזהה **`no-style`**.

## פלט

**פלט סטנדרטי** — ריק: התוצאה היא הקובץ שנכתב. עם `--dry-run`, רשימת השינויים, המתוארת ב[לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה נוסף בפלט השגיאות |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, או `N subtitles taken out of italics`, או `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, מוחזרים כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

`N` הוא מספר הכתוביות **שהטקסט שלהן השתנה**, כפי שנקרא מהפעולה עצמה.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא את `counts.changed` — ה-`N` שבמשפט — ואת `changes`, רשימת הכתוביות ששונו (`{"subtitle", "document", "before", "after"}`, **ריקה** כשדבר לא השתנה).

## קודי יציאה

אלו של הכלי: `0` אם כל הקבצים עובדו — קובץ שבו דבר אינו משתנה נחשב מעובד —, `2` אם אף אחד לא עובד, `3` אם רק חלקם עובדו, `1` בשגיאת שימוש.

## שגיאות

| מה גורם לה | הודעה |
| :------------------ | :------ |
| לא `--on` ולא `--off` | `italics needs --on to put the texts in italics, or --off` |
| שתיהן | `--on and --off say opposite things; give one of them` |
| פורמט בלי סגנון | `<path>: TMPlayer writes no style: there are no italics to put on` (או `take out`) |
| טווח שנכתב לא נכון או מחוץ לקובץ | ראו [`adjust`](adjust.md#--range) |

השגיאות של היעד משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
