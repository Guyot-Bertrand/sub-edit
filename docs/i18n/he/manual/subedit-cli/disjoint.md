# `disjoint`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

הפקודה חותכת כתוביות חופפות כך ש**אף טווח `[start, end]` אינו חופף עוד טווח אחר**. זוהי `Make Ranges Disjoint…` של החלון — [אותו מנוע](../subedit-gui/lines.md#הפיכת-הטווחים-לזרים).

**דבר אינו מתקצר ואינו מתארך**: בכל רגע, הקובץ מציג אותם טקסטים כמו קודם. מה שמשתנה הוא האופן שבו ציר הזמן חתוך:

- החיתוך נעשה ב**כל התחלה וכל סוף** של כתוביות בקבוצה חופפת;
- מקטע שמכוסה על ידי כתובית **אחת** נשאר אותה כתובית;
- מקטע שמכוסות בו **כמה** כתוביות הופך לכתובית **אחת** ששורותיה הן שלהן, **לפי סדר ההתחלות שלהן** — המוקדמת ביותר בשורה 1, הבאה בשורה 2;
- כתובית שאינה חופפת דבר אינה נוגעת בה, וסדר השאר נשמר.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<file>...` | כן | נתיב אחד או יותר | — |
| `--minimum` | לא | משך: `0.08`, `1.5`, `00:00:00,080`. אפס משאיר כל מקטע | `0.08` (כשני פריימים) |
| `--max-lines` | לא | מספר שלם: מספר השורות המרבי של כתובית ממוזגת; `0` מבטל את המגבלה | `2` |
| `--dashes` | לא | דגל: מקף לפני הטקסט שכל כתובית תורמת לכתובית ממוזגת | כבוי |
| `--sort` | לא | דגל: ממיין את הכתוביות לפי סדר ההתחלה **קודם** — ראו [נדרש קובץ ממוין](#נדרש-קובץ-ממוין) | כבוי |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, פרט ל-`--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ומדווח, לא כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` ו-`B [3, 8]` נותנות שלוש כתוביות זרות: `A` לבדה, `A` ו-`B` יחד, `B` לבדה. אף רגע אינו אובד ואף רגע אינו נוסף.

## מה הפעולה מחליטה

**שורה אינה משוכפלת לעולם.** שתי כתוביות בעלות אותו טקסט נותנות שורה אחת במקום שבו הן חופפות, ושתי חתיכות שכנות שמציגות אותו דבר הופכות לכתובית אחת בלבד. טקסט של כתובית שנחתכה לכמה חתיכות **חוזר בכל אחת מהן**, אחרת: זה מה שמופיע על המסך באותו רגע.

**מקטע קצר מאוד נבלע בידי שכן.** מתחת ל-`--minimum` המקטע נעלם: השכן **מרחיב את הטווח שלו כדי לכסות אותו**, כך שאף רגע אינו נשאר ריק ודבר אינו זז ביותר מהסף הזה. השכן שנבחר הוא זה ששורותיו **כבר מכילות** את שורות המקטע, ואחרת הארוך ביותר. טקסט המקטע אובד רק אם אף שכן אינו נושא אותו, ואז לפחות מהסף. **כתובית שלמה הקצרה מהסף אינה נבלעת לעולם**: היא נשארת, ונספרת.

**יותר שורות מהמגבלה: חלונות.** כתובית ממוזגת שהייתה מכילה יותר מ-`--max-lines` שורות מוצגת **בחלונות עוקבים** בגודל הזה, בחלקים שווים של המשך. שלוש שורות על פני שלוש שניות:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

ארבע שורות נותנות שלושה חלונות, וכן הלאה: החלון גולש שורה אחת בכל פעם. דבר אינו נקטע ואינו נחתך שוב.

**התרגום עוקב אחר אותם חיתוכים**, שכן כתובית נושאת את שני הטקסטים. **מה שהפורמט נושא נוסף על כך** — סגנון, מיקום — הוא של הכתובית הראשונה שמכסה; האובדן נספר כשהכתוביות הממוזגות שונות זו מזו.

**כתובית שסופה לפני תחילתה** אינה מנוחשת: היא נשארת במקומה, ונספרת.

## נדרש קובץ ממוין

הקבוצות הן רצפים של שכנים: בקובץ שאינו ממוין, השכנים אינם אלה שחופפים. לכן קובץ שהתחלותיו אינן בסדר **נדחה**, וההודעה אומרת זאת:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` ממיינת קודם — כמו [`sort`](sort.md) —, ואומרת זאת. הליבה אינה ממיינת מעצמה לעולם.

מעבר שני אינו משנה דבר: משהטווחים זרים, אין מה לחתוך עוד, והמשפט אומר `already disjoint`.

## פלט

**פלט סטנדרטי** — ריק: התוצאה היא הקובץ שנכתב.

| רמה | מה נוסף בפלט השגיאות |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, ואחריו אחרי `;` מה שלא ניתן היה לעשות במלואו: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — או `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** |

`N subtitles split` סופר את הכתוביות המקוריות שנחתכו לכמה חתיכות, `M merged` את החתיכות שמציגות כמה כתוביות יחד.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), ה-`counts` של כל קובץ:

| ספירה | מה היא אומרת |
| :----- | :----------- |
| `subtitles` | הכתוביות של הקובץ שנכתב |
| `split`, `merged` | כמו במשפט |
| `windowed` | החתיכות המוצגות בחלונות |
| `absorbed` | המקטעים שנבלעו בידי שכן |
| `too_short` | הכתוביות הקצרות מהסף, שנשארו שלמות |
| `ending_before_start` | הכתוביות שסופן לפני תחילתן, שנשארו כפי שהן |
| `extras_dropped` | החתיכות הממוזגות שהסגנון או המיקום שלהן שונים |
| `moved` | עם `--sort` בלבד: המקומות ששונו |

## קודי יציאה

אלו של הכלי: `0` אם כל הקבצים עובדו — קובץ בלי חפיפות נחשב מעובד —, `2` אם אף אחד לא עובד, `3` אם רק חלקם עובדו, `1` בשגיאת שימוש.

## שגיאות

| מה גורם להן | הודעה |
| :------------------- | :------ |
| קובץ שאינו ממוין בלי `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` אינו משך, או שלילי | `--minimum: …` |
| `--max-lines` אינו מספר שלם | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

קובץ שאי אפשר לקרוא נכשל כמו בכל תת-פקודה; השגיאות של היעד משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).

## בפקודות האחרות

[`convert`](convert.md#הפיכת-הטווחים-לזרים), [`append`](append.md#הפיכת-הטווחים-לזרים) ו-[`split-file`](split-file.md#הפיכת-הטווחים-לזרים) מקבלות את `--disjoint`, שעושה אותו דבר עם ההגדרות המוגדרות כברירת מחדל, לפני הכתיבה.

בקובץ שחולק רע — טקסט שנחתך לחלקים העוקבים זה את זה, ונמסר שלא בסדר —, מיון וחיתוך בלבד נותנים שורות שמתהפכות מחתיכה לחתיכה: קודם [`join-repeats`](join-repeats.md), או [`repair`](repair.md), שמשרשרת את שלושתם.
