# `dialogue-dashes`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

מניח **מקף דו־שיח** בראש השורות, או מסיר אותם. זוהי `Dialogue Dashes` של
[החלון](../subedit-gui/operations.md).

**שתי מילים, כמו ב-[`italics`](italics.md)**: `--add` ו-`--remove` מוציאות זו את זו, ואחת מהן נדרשת.

- **`--add`** מניח מקף לפני כל שורה שאין לה אחד: `Bonjour` נותן `- Bonjour`, ו-
  `Bonjour\nMarie` נותן `- Bonjour\n- Marie`. **מקף שכבר קיים אינו מוכפל.**
- **שלושה מקפים מזוהים** — המקף של המקלדת ושני המקפים הארוכים של הסדר הטיפוגרפי —
  **ומה שנכתב הוא המקף של המקלדת**: `— Bonjour` הופך ל-`- Bonjour`. רווחים עודפים
  אחרי המקף נעלמים.
- המקף בא **לפני הטקסט, לא לפני תגית פותחת**: `<i>Bonjour</i>` נותן
  `<i>- Bonjour</i>`. אף תגית אינה זזה.
- **`--remove`** מסיר את המקפים המובילים, בשלוש הצורות.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **אחת מהשתיים** | דגלים | — |
| `--range` | לא | `N-M`, או `N-` עד הסוף — ראו [`adjust`](adjust.md#--range) | כל הקובץ |
| `--document` | לא | `main` או `translation` — ראו [תרגום](invocation.md#תרגום) | `main` |
| `-t`, `--translation-file` | עם `--document translation` | קובץ תרגום, עבור קלט **אחד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **אחת מהשלוש**, מלבד עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

**הטקסט הראשי כברירת מחדל; התרגום עם `--document translation -t FILE`.** אז
התרגום הוא שמשתנה ונכתב, והוא בלבד — ראו [תרגום](invocation.md#תרגום).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## פלט

**הפלט הסטנדרטי** — ריק: התוצאה היא הקובץ שנכתב. עם `--dry-run`, רשימת
השינויים, המתוארת ב[לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה מתווסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, או `N subtitles undashed`, או `nothing to change` |
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
| לא `--add` ולא `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| שתיהן | `--add and --remove say opposite things; give one of them` |
| טווח שנכתב לא נכון או מחוץ לקובץ | ראו [`adjust`](adjust.md#--range) |

שגיאות היעד משותפות לתת-הפקודות שכותבות:
ראו [הפעלה](invocation.md#היעד).
