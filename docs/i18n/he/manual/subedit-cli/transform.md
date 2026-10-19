# `transform`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

מתקנת **כל** מיקום בקובץ משתי נקודות שערכן הנכון ידוע. זוהי הפעולה שתזמון נסחף דורש — נכון בהתחלה, שגוי בסוף — במקום שבו [`shift`](shift.md), שמזיזה הכול באותה כמות, אינה יכולה לעשות דבר.

פורמט הקובץ שנקרא **נשמר**: שינוי הפורמט הוא עבודתה של [`convert`](convert.md).

<!-- example: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **כן** | נקודת ייחוס, `<index>=<time>` | — |
| `--last` | **כן** | נקודת ייחוס, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

אין צורה קצרה: שתי האפשרויות האלה נכתבות במלואן בלבד.

## איך כותבים נקודת ייחוס

נקודת ייחוס נוקבת בכתובית ואומרת היכן **תחילתה** צריכה לנחות מעתה:

```
<index>=<time>
```

**האינדקס נספר מ-1**, כפי שהוא מוצג וכפי שקובץ SubRip ממספר אותו. `0` נדחה, וכך גם אינדקס שמעבר לכתובית האחרונה.

**הזמן נכתב כמו זה של [`shift`](shift.md)** — שניות או חותמת זמן, המפריד העשרוני נקודה, הרזולוציה אלפית שנייה:

| צורה | דוגמאות |
| :---- | :------- |
| שניות | `10`, `2.999` |
| חותמת זמן | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` ו-`--first 3=00:00:10.000` אומרים אותו דבר.

`--first` ו-`--last` נוקבות בשתי נקודות, לא בטווח. שמותיהן אומרים את השימוש הצפוי — נקודת ייחוס אחת בתחילת הקובץ, האחרת בסופו — ולא אילוץ: החלפתן נותנת אותה תוצאה, ו**כל מה שמחוץ לשתי נקודות הייחוס עוקב כמו מה שביניהן**.

## מה המיקומים האחרים הופכים להיות

שתי נקודות הייחוס קובעות גורם קנה מידה, וכל מיקום עוקב אחריו:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` הם ההתחלות הנוכחיות של שתי הכתוביות הנקובות, `y₁`, `y₂` הזמנים המבוקשים. הגורם מוחזק כשבר מדויק, ו**העיגול נעשה פעם אחת בלבד**, לאלפית שנייה; זה מה שגורם לשתי נקודות הייחוס לנחות **בדיוק** על הזמנים המבוקשים ולא עד כדי אלפית (ראו [ADR 0013](../../../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

כל כתובית נשארת על המסך **ביחס** למשך שבו נשארה שם: הסוף עוקב אחר אותו קנה מידה כמו ההתחלה. התמרה אינה הזזה, ואין לה תכונתה.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

נקודת הייחוס הראשונה אינה זזה, השלישית עוברת מ-9 שניות ל-10 שניות: הגורם הוא `9/8`, והכתובית השנייה — שאיש לא נקב בה — עוברת מ-`5,001` ל-`5,501`, כלומר `1000 + 4001 × 9/8` שעוגל פעם אחת.

## שתי נקודות ייחוס שאינן תקינות

| מצב | מתי רואים אותו | קוד |
| :-------- | :------------- | :--- |
| `--first` ו-`--last` נוקבות באותו אינדקס | לפני כל קריאה | `1` |
| אינדקס שמעבר לכתובית האחרונה | כשהקובץ נפתח | `2` או `3` |
| שתי הכתוביות הנקובות מתחילות באותו רגע | כשהקובץ נפתח | `2` או `3` |

שלושתן נדחות מאותה סיבה: `x₂ − x₁` הוא אפס, ושתי נקודות מתלכדות אינן מגדירות קו. להחזיר חילוק באפס שהולבש כתוצאה היה גרוע מלדחות.

המקרה הראשון הוא **שגיאת שימוש** — שתי האפשרויות פגומות יהיו הקבצים אשר יהיו, ולכן מגיעה הודעה אחת, לא אחת לכל קובץ. השניים האחרים תלויים בקובץ שנפתח, ולכן נחשבים לכשל עיבוד.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

אינדקס מחוץ לגבולות נדחה **תוך נקיבה בגבול**, שאומר גם מה אפשר היה לכתוב:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## מיקום לפני נקודת האפס נדחה

כמו ב-[`shift`](shift.md), ומאותה סיבה: מיקום שלילי נכתב `-00:00:01,000`, צורה שאף אחד מתשעת הפורמטים אינו מגדיר ואף קורא אינו מקבל. המקרה מתעורר כאן כשכתובית נמצאת **לפני נקודת הייחוס הראשונה** וקנה המידה דוחף אותה מעבר לאפס. דבר אינו נכתב, הקוד הוא `2`, וההודעה נוקבת בכתובית הפגומה.

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב. עם `--dry-run`, גם אז דבר: אין קובץ, ושורת הדיווח אומרת זאת — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, שהוחזרו כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריה **כל אבחון קריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

נקודות הייחוס נכתבות בחזרה עם נקודה עשרונית, יהיה אופן הקלדתן אשר יהיה.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא את `counts.subtitles` (כל כתובית בקובץ). הטבלה המלאה היא זו של [הפעלה](invocation.md#מה-כל-תת-פקודה-מציבה-ב-counts).

## קודי יציאה

אלה של הכלי: `0` אם כל קובץ נכתב, `2` אם אף אחד, `3` אם רק חלקם, `1` בשגיאת שימוש.

## שגיאות

| מה גורם לה | הודעה |
| :------------------ | :------ |
| נקודת ייחוס בלי `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| נקודת ייחוס שחסר בה אחד משני חלקיה | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| אינדקס `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| אינדקס שאינו מספר שלם | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| זמן לא תקף | אלה של [`shift`](shift.md) |
| `--first` ו-`--last` על אותו אינדקס | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| אינדקס מחוץ לגבולות | `<path>: subtitle N is past the end: the file holds M subtitles` |
| שתי כתוביות שמתחילות יחד | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| מיקום לפני נקודת האפס | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**דבר אינו נכתב בלי אחת משלוש אפשרויות היעד**, ושגיאותיהן משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
