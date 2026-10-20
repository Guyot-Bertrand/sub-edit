# `split-file`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

חותכת קובץ **לשניים**: ראש וזנב. זוהי הפעולה `Split Project…` של החלון — [אותו מנוע](../subedit-gui/operations.md#split-project), אותם מניינים, אותו סירוב — ו**ההפך המדויק של [`append`](append.md)**: הוספת הזנב לראש מחזירה את קובץ המוצא.

- **`--at N` הוא מספרה של הכתובית הראשונה בזנב**, ממוספרת כפי שטבלת החלון ממספרת אותה: מ-1. `--at 3` מציבה את כתוביות 1 ו-2 בראש, ואת השלישית ואת היתר בזנב. הערך הוא **מהשנייה ועד האחרונה**: חיתוך צריך כתובית בכל צד.
- **הזנב מוחזר לנקודת האפס**: הוא מוזז במינוס סופה של הכתובית האחרונה שנשארה בראש, ומיקומיו שומרים על סדרם ועל המרווחים ביניהם. זה מה שהופך את `split-file` להפך של `append`.
- **שני החצאים שומרים על הפורמט, הקידוד, סימן סדר הבתים וסוף השורה של הקובץ שנקרא**, וממוספרים מחדש מ-1 בעת הכתיבה, כמו כל קובץ שנכתב בחזרה.
- **שני פלטים, ולכן יעד משלהם**: `--head` ו-`--tail`, **שניהם חובה**. `--output`, `--output-dir`, `--in-place` ו-`--recursive` אינם קיימים ב-`split-file` ונדחים כמו כל אפשרות לא מוכרת, קוד `1`: אין תיקייה שאפשר לחלוק בה שני קבצים בעלי שם, ואין קלט לדרוס.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<file>` | כן | נתיב **יחיד** | — |
| `--at` | כן | מספר שלם, מ-2 ועד מספר הכתוביות של הקובץ | — |
| `--head`, `--tail` | **כן, שניהם**, למעט עם `--dry-run` | נתיבי קבצים; תיקייתם נוצרת אם היא חסרה | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |
| `--sort` | לא | דגל: מציב את הכתוביות בסדר ההתחלות שלהן **לפני החיתוך**; `--at` סופר בסדר הזה | כבוי |
| `--disjoint` | לא | דגל: חותך את הכתוביות החופפות **לפני החיתוך**; `--at` סופר במה שנשאר, ראו [הפיכת הטווחים לזרים](#הפיכת-הטווחים-לזרים) | כבוי |

**עם `--dry-run` אין חובה ליעד**: אסור לתת אף אחד, אבל לא רק אחד משניים — `--head` ו-`--tail` הולכים יחד. אלה שניתנו נשפטים כמו בלי `--dry-run`.

**שני היעדים אינם יכולים להיות לא הקלט ולא אותו קובץ**: כתיבה על מה שקוראים, או כתיבת הזנב על הראש, נדחית לפני שמשהו נקרא, קוד `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

הכתובית השנייה, שנשארה בראש, מסתיימת ב-4 שניות: הזנב חוזר אחורה ב-4 שניות, והכתובית השלישית, שהתחילה ב-6 שניות, מתחילה ב-2 שניות.

## חיתוך לפי סדר הזמן

`--at` ממספרת את הכתוביות **כפי שהקובץ נותן אותן**. בקובץ שסדרו שבור, הראש אינו אפוא החצי המוקדם ביותר בזמן. `--sort` מציבה את הכתוביות בסדר ההתחלות שלהן **לפני שיפוט החיתוך** — יציב, כמו ש-[`sort`](sort.md) עושה — ואז `--at` סופרת בסדר הזה.

לעולם לא כברירת מחדל. עם האפשרות, הקובץ אומר ברמת הדיווח הראשונה כמה כתוביות זזו (`<path>: N subtitles moved` או `<path>: already in order`), ורשומת `--format json` נושאת מניין `moved`, **שנעדר בלי האפשרות**.

## הפיכת הטווחים לזרים

לעולם לא כברירת מחדל. האפשרות עושה את מה ש-[`disjoint`](disjoint.md) עושה, **עם הגדרות ברירת המחדל שלה**, לפני שיפוט החיתוך, אחרי המיון: `--at` סופרת במה שזה נותן. היא צריכה קובץ ממוין: אחרת היא נדחית, ו-`--sort` — שבאה קודם — ממיינת אותו מראש. הקובץ אומר זאת ברמת הדיווח הראשונה (`<path>: N subtitles split, M merged`), ורשומת `--format json` נושאת את המניינים `split` ו-`merged`, **שנעדרים בלי האפשרות**.

## כשהחיתוך נדחה

**שני חצאים שחופפים בחיתוך אינם נחתכים.** אילו כתובית בזנב התחילה לפני סופה של הכתובית האחרונה שנשארה מאחור, ההזזה הייתה מובילה אותה לפני תחילת הווידאו — מיקום שלילי, שאף פורמט אינו כותב. `split-file` דוחה **ונוקבת בכתובית**, במילות החלון:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

הקוד הוא `2` ו**דבר אינו נכתב**. יש לחתוך במקום אחר. נחיתה בדיוק על ההתחלה מותרת: אפס הוא מיקום.

**`--at` מחוץ לקובץ נדחה באותו אופן**, אחרי שהקובץ נקרא: `--at 1` — הכתובית הראשונה אינה יכולה לפתוח זנב, לא היה נשאר דבר — ו-`--at` מעבר לכתובית האחרונה, שהיה משאיר זנב ריק. קובץ של כתובית אחת אינו ניתן לחיתוך.

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא שני הקבצים שנכתבו.

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; עם `--dry-run`, `(dry run, nothing written)` במקום `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, שהוחזרו כפי שהיו |
| 3 | `<path>: N bytes read, H and T written`, ואחריה **כל אבחון קריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), **אובייקט יחיד**, ש-`file` בו הוא הקובץ שנחתך. **`destination` הוא הראש** — הקובץ שהקלט הופך להיות — ו-**`tail` הזנב**, נתיב, או `null` עם `--dry-run` כמו `destination`. ה-`counts`: `subtitles` (אלה של הקובץ שנקרא), `head` ו-`tail` (אלה של כל חצי).

## קודי יציאה

| קוד | מתי |
| :--- | :---- |
| `0` | שני הקבצים נכתבו — או חושבו, עם `--dry-run` |
| `1` | שגיאת שימוש: אין `--head` ו-`--tail`, אפשרות שאין ל-`split-file`, יעד שהוא הקלט או היעד האחר, `--at` שאינו מספר שלם חיובי |
| `2` | הקובץ לא היה ניתן לקריאה, **החיתוך נדחה** (מחוץ לקובץ, או חפיפה), או שלא ניתן היה לכתוב יעד. אין קוד `3`: מעובד קובץ יחיד |

**הראש נכתב ראשון**: דיסק שמסרב לזנב משאיר את הראש מאחוריו, והקוד הוא `2`.

## שגיאות

| מה גורם להן | הודעה |
| :------------------- | :------ |
| לא `--head` ולא `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| רק אחד משניהם, עם `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| שני היעדים הם אותו קובץ | `<path>: named as both the head and the tail` |
| יעד הוא הקלט | `<path>: would be written over the input <input>` |
| `--at` מחוץ לקובץ | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| קובץ של כתובית אחת | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| חפיפה | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| קובץ חסר, שאינו ניתן לקריאה או בפורמט לא מוכר | `<path>: does not exist`… — כמו בכל מקום |
