# `replace`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

מחליפה טקסט בכתוביות **בלי לשבור תגית**. זוהי הפעולה `Replace All` של [החיפוש](../subedit-gui/find-and-replace.md) בחלון, עם כלליה:

- **החיפוש נעשה בטקסט הגלוי** — מה שהצופה קורא. התגיות אינן חלק ממנו: חיפוש של `<i>` אינו מוצא דבר, וחיפוש של `Bonjour` מוצא "Bonjour" שמחציתו נטויה;
- **ההחלפה נעשית בטקסט המקור**, זה של הקובץ, והתגיות נשארות במקומן. תגית שחותכת את המילה המבוקשת סוגרת סביב ההחלפה כולה; סגנון שנוגע בהתאמה מכסה את ההחלפה כולה; מה שההתאמה אינה נוגעת בו אינו זז. הכללים, מקרה אחר מקרה, הם אלה של החיפוש בחלון.

**אין תת-פקודה `find`**: כדי לראות היכן תבנית נמצאת בלי לשנות דבר, [`--dry-run`](invocation.md#לראות-לפני-שכותבים) הוא שמונה כל כתובית רלוונטית.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | כן | הטקסט לחיפוש במה שמוצג; **לא ריק** | — |
| `<replacement>` | כן | מה שמחליף אותו, תגיות מותרות; יכול להיות ריק | — |
| `<file>...` | כן | נתיב אחד או יותר | — |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--regex` | לא | דגל: התבנית היא ביטוי רגולרי של ICU | טקסט רגיל |
| `--case-sensitive` | לא | דגל: מבחין בין אותיות רשיות לקטנות | מתעלמים מהן |
| `--range` | לא | `N-M`, או `N-` עד הסוף — ראו [`adjust`](adjust.md#--range) | הקובץ כולו |
| `--document` | לא | `main` או `translation` — ראו [תרגום](invocation.md#תרגום) | `main` |
| `-t`, `--translation-file` | עם `--document translation` | קובץ תרגום, לקלט **יחיד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

**תבנית שמתחילה במקף** נכתבת אחרי `--`: `subedit-cli replace -- -- "—" film.srt`.

**הטקסט הראשי כברירת מחדל; התרגום עם `--document translation -t FILE`.** אז התרגום הוא שמשתנה ונכתב, והוא בלבד — ראו [תרגום](invocation.md#תרגום).

### טקסט רגיל או ביטוי

כברירת מחדל התבנית **נקראת כמות שהיא**: `a.b` מחפש `a`, נקודה, `b`. רשיות האותיות מתעלמים ממנה באותה דקדקנות כמו בחלון — `É` ו-`é` הן אות אחת.

עם `--regex`, התבנית היא ביטוי רגולרי של **ICU**, הנקרא כפי שגאופול קוראת אותו: `.` חוצה שבירת שורה, `^` ו-`$` תופסים בכל שורה של כתובית. בהחלפה, ורק שם:

| כותבים | כדי לומר |
| :----- | :-------- |
| `$1` עד `$9` | קבוצות ההתאמה |
| `$0` | ההתאמה כולה |
| `\n` | שבירת שורה |
| `\$`, `\\` | `$`, לוכסן הפוך |

יש להיזהר ממרכאות המעטפת: `'$1 EUR'`, ולא `"$1 EUR"`, שהיה מניח למעטפת לקרוא את `$1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## תגיות

טקסט שבו תגית חותכת את המילה המבוקשת:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

בכתובית הראשונה, התגית חתכה את "Bonjour": כעת היא סוגרת סביב "Salut". בשנייה, היא אינה נגועה ונשארת במקומה. **תגית שנשארה פתוחה נשארת פתוחה**: תיקון של סימון רעוע יהיה פעולה אחרת.

ההחלפה נקראת ב**תגיות של פורמט הקובץ**: `<i>…</i>` ל-SubRip, `{\i1}…` ל-Advanced SSA. זו הדרך היחידה לעצב את מה שכותבים.

## לא נמצא דבר, אין מה לשנות

שתי תשובות שאינן כשל — קוד היציאה הוא `0` ו**הקובץ נכתב בכל זאת**, שכן יעד שניתן הוא יעד שנכתב:

| מצב | משפט |
| :-------- | :----- |
| התבנית אינה בשום מקום | `"zzz" not found` |
| היא שם, אבל כל התאמה מוחלפת בעצמה | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

הפלט הסטנדרטי מונה **כל כתובית שטקסטה היה משתנה**, לפני ואחרי, כפי שהם בקובץ — כולל התגיות:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

צורת הרשימה מתוארת ב[לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים); ב-JSON זהו `changes`.

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב. עם `--dry-run`, רשימת השינויים שלעיל.

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, או `"<pattern>" not found`, או `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, שהוחזרו כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריה **כל אבחון קריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

`N` מונה **את ההתאמות שהוחלפו בכתוביות שטקסטן השתנה** — זהו גם המספר שהחלון אומר.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא:

| מפתח | תוכן |
| :-- | :------ |
| `counts.replaced` | ההתאמות שהוחלפו, בכתוביות שטקסטן השתנה: ה-`N` של המשפט |
| `counts.matched` | ההתאמות שנמצאו, **בין אם ההחלפה שינתה את הטקסט ובין אם לא** — זה מה שמבדיל בין "לא נמצא דבר" (`0`) לבין "אין מה לשנות" (`replaced` על `0`, `matched` חיובי) |
| `changes` | הכתוביות ששונו: `{"subtitle", "document", "before", "after"}`, **ריק** כשלא השתנה דבר |

## קודי יציאה

אלה של הכלי: `0` אם כל קובץ עובד — קובץ שלא נמצא בו דבר נחשב —, `2` אם אף אחד, `3` אם רק חלקם, `1` בשגיאת שימוש.

## שגיאות

| מה גורם לה | הודעה |
| :------------------ | :------ |
| תבנית ריקה | `pattern: nothing to look for` |
| ביטוי שאינו ניתן לקריאה | `pattern: not a regular expression (<ICU's reason>)` |
| טווח שנכתב גרוע או מחוץ לקובץ | ראו [`adjust`](adjust.md#--range) |

**התבנית נקראת לפני הקובץ הראשון**: ביטוי שאינו ניתן לקריאה הוא שגיאת שימוש (קוד `1`), דבר אינו נקרא או נכתב.

שגיאות היעד משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
