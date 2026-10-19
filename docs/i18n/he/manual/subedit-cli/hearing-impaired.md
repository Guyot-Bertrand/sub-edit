# `hearing-impaired`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

הפקודה מסירה מכל הקובץ את **ההערות לכבדי שמיעה** — הצלילים המתוארים בין סוגריים מרובעים. **סוגריים עגולים נשארים כברירת מחדל**: משתמשים בהם לעיתים קרובות לא פחות לשורה נלחשת מאשר לצליל מתואר, והסרתם הייתה מוחקת דיאלוג. `--parentheses` מסירה גם אותם, עבור קובץ שמתאר בהם את הצלילים שלו.

מה שהיא אינה עושה: מילות שיר בין סולמיות, שם הדובר לפני נקודתיים, החזרת אותיות רשיות, תיקון שגיאות OCR. התבניות האלה הן אלה של אשף [`Correct Texts…`](../subedit-gui/correct-texts.md) בחלון, ששורת הפקודה מציעה תחת השם [`correct`](correct.md); ושום דבר כאן אינו ניתן להגדרה: השינוי הוא קבוע, לא ניתן לתצורה.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | לא | דגל: מסיר גם את ההערות שבין סוגריים עגולים | כבוי |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--document` | לא | `main` או `translation` — ראו [תרגום](invocation.md#תרגום) | `main` |
| `-t`, `--translation-file` | עם `--document translation` | קובץ תרגום, עבור קלט **אחד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, פרט ל-`--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ומדווח, לא כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

הפורמט של הקובץ שנקרא **נשמר**: שינוי פורמט הוא עבודתה של [`convert`](convert.md).

**ברירת המחדל היא הטקסט הראשי; התרגום נבחר עם `--document translation -t FILE`.** אז התרגום הוא שמשתנה ונכתב, והוא בלבד — ראו [תרגום](invocation.md#תרגום).

**הערה שהייתה מרוקנת תרגום כותבת אותו ריק, ושומרת את הכתובית**: הכתובית נשארת בקובץ, בלי טקסט. הטקסט הראשי, לעומת זאת, מאבד כתובית שרוקנה כליל, כמתואר בהמשך.

## מה מוסר ומה נשאר

הערה היא סוגר מרובע — או סוגר עגול, עם `--parentheses` — ומה שהוא סוגר, **כולל מעבר שורה**: בקבצים אמיתיים הערה נחתכת לעיתים קרובות בסוף שורה.

| קלט | פלט |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

ההסרה משאירה **בדיוק רווח אחד בין מה שהקיף אותה, ושום דבר בקצה השורה**. זה אינו ניקוי של רווחי הקובץ: שני רווחים שהטקסט כבר נשא נשארים שני רווחים.

**כשההערה חוצה את מעבר השורה, מעבר השורה הוא שנשאר** — שתי השורות נשארות שתי שורות. המקרה הזה אינו תופעה נדירה: בחמישה עשר תרגומי כתוביות אמיתיים, כל סוגר מרובע שנראה יתום היה הערה שנחתכה כך.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**מה שאינו הערה ואינו זז:**

| מקרה | מה קורה |
| :-- | :-------------- |
| תוכן מספרי בלבד — `[1]`, `(12)`, `[ 156478 ]` | נשאר כפי שהוא, כולל התוחמים: הפניה מעולם לא נתנה שם לצליל |
| סוגר עגול ומה שהוא סוגר, בלי `--parentheses` | נשאר כפי שהוא: `Venez (chut) ici` אינו משתנה |
| סוגר מרובע או עגול שדבר אינו סוגר אותו | נשאר כפי שהוא |
| תגית פורמט — `<i>`, `</i>` | לא מוסרת לעולם |

רווח **בין** הספרות, אות או סימן — `[1 2]`, `[1a]`, `[-1]` — הופכים אותו להערה. כך גם תוכן ריק, `[]` או `( )`: אין ספרה, ולכן זו אינה הפניה; אין מה לומר, ולכן אין מה לשמור.

## מה נעלם כליל

**שורה שההסרה רוקנה נעלמת**, ו**כתובית שרוקנה כליל מוסרת מהקובץ**. הכתוביות שאחריה ממוספרות מחדש.

שורה נחשבת ריקה כשכל מה שנשאר בה הוא רווחים, תגיות פורמט, או מקף דו־שיח בודד — `<i>[Musique]</i>` אינה משאירה `<i></i>` על המסך.

**התגיות הן אלה של פורמט הקובץ.** `{\i1}[Musique]{\i0}` ב-Advanced SSA, `{Y:i}[Musique]` ב-MicroDVD ו-`/[Musique]` ב-MPL2 אינן משאירות דבר אף הן. לעומת זאת, `<i>` בפורמט שאינו כותב תגיות — TMPlayer, LRC — הוא טקסט שהצופה קורא, והשורה שנושאת אותו אינה ריקה.

**דיאלוג שהצטמצם לקול אחד אינו עוד דיאלוג**, והמקף שנשאר נעלם. כל עוד נשארים שני קולות, המקפים נשארים.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## קובץ בלי שום הערה נכתב בכל זאת

הוא מועתק זהה, קוד היציאה הוא `0`, והדיווח אומר שלא היה מה להסיר.

זה הכלל של תת-הפקודות שכותבות: **יעד שניתן הוא יעד שנכתב**. חריג כאן היה מחייב סקריפט לדעת איזו מתת-הפקודות מפיקה לפעמים קובץ ולפעמים כלום.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## פלט

**פלט סטנדרטי** — ריק: התוצאה היא הקובץ שנכתב. **עם `--dry-run`, אלה השינויים המוצעים**: לכל כתובית שהטקסט שלה ישתנה, מספרה, הטקסט לפני והטקסט אחרי — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה נוסף בפלט השגיאות |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, או `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, מוחזרים כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

`N` סופר את הכתוביות שהטקסט שלהן השתנה, ו-`M` את אלה שההסרה רוקנה כליל ולכן עזבו את הקובץ.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא את `counts.cleaned` (הטקסטים שנכתבו מחדש) ואת `counts.removed` (הכתוביות שהוסרו); **עם `--dry-run`**, המפתח `changes` מונה גם את הכתוביות ששונו, כמו הפלט הסטנדרטי בצורת הטקסט. הטבלה המלאה נמצאת ב[הפעלה](invocation.md#מה-כל-תת-פקודה-מציבה-ב-counts).

## קודי יציאה

אלו של הכלי: `0` אם כל הקבצים נכתבו, `2` אם אף אחד לא נכתב, `3` אם רק חלקם נכתבו, `1` בשגיאת שימוש.

## שגיאות

לתת-הפקודה הזו אין שגיאה משלה: היא אינה מקבלת ערך לניתוח, ושום טקסט אינו יכול לגרום לה לדחות קובץ. מה שיכול להיכשל הוא הקריאה או הכתיבה, כמו בשאר.

**דבר אינו נכתב בלי אחת משלוש אפשרויות היעד**, והשגיאות שלהן משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
