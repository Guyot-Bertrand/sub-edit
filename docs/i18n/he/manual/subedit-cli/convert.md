# `convert`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

כותב כל קובץ בפורמט ובצורה המבוקשים. **לעולם אינו משנה את
הקלט**, אלא אם `--in-place` מבקש זאת במפורש.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **כן** | שם מהטבלה שלהלן, ולא דבר אחר | — |
| `--line-endings` | לא | `unix`, `windows` או `mac` | אלה של הקובץ שנקרא |
| `--to-encoding` | לא | כל קידוד ש-ICU יכול לכתוב, מלבד אלה שכותבים סימן משלהם | זה של הקובץ שנקרא |
| `--bom` / `--no-bom` | לא | דגלים, מוציאים זה את זה | מה שהקובץ שנקרא נשא |
| `--sort` | לא | דגל: מסדר את הכתוביות לפי ההתחלה שלהן **לפני הכתיבה**, ראו [מיון בעת הכתיבה](#מיון-בעת-הכתיבה) | כבוי |
| `--join-repeats` | לא | דגל: יוצר כתובית אחת מהחלקים של טקסט אחד הבאים בזה אחר זה, **לפני כל דבר אחר**, ראו [הפיכת הטווחים לזרים](#הפיכת-הטווחים-לזרים) | כבוי |
| `--repair` | לא | דגל: כמו `--join-repeats --sort --disjoint`, בסדר הזה | כבוי |
| `--disjoint` | לא | דגל: חותך את הכתוביות החופפות **לפני הכתיבה**, ראו [הפיכת הטווחים לזרים](#הפיכת-הטווחים-לזרים) | כבוי |
| `--output` / `--output-dir` / `--in-place` | **אחת מהשלוש**, מלבד עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

`mac` נוקב בתו החזרה לתחילת השורה לבדו (`\r`), סוף השורה של Mac OS הקלאסית.

## הפורמטים ש-`--to` מקבלת

| שם | פורמט | סיומת שנכתבת |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**שם, ולא סיומת.** שתי סיומות נוקבות כל אחת בשני פורמטים
— `.sub` היא גם של MicroDVD, `.txt` של MPL2 ושל
TMPlayer — ולכן אפשרות שקיבלה סיומות לא הייתה יודעת איזה מהם
מבוקש. השמות שלמעלה חד-משמעיים, ותואמים את
הסיומת בכל מקום שגם הסיומת חד-משמעית.

**הרשימה סגורה.** זו הרשימה של Gaupol, ותשעתם בה.

## שני הפורמטים שאינם נושאים סוף

**TMPlayer ו-LRC כותבים מיקום אחד בלבד לכל שורה**, את ההתחלה שלה. זה
ניכר משני הצדדים.

**בקריאה**, הסופים נגזרים: סופה של שורה הוא ההתחלה של
הבאה, ולאחרונה ניתנות חמש שניות. כך Gaupol עושה.
אנו אומרים זאת גם באמצעות אבחון ש-`--verbose` מציג בשלוש
תת-הפקודות:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**בכתיבה**, הסופים אינם נכתבים, מחוסר מקום לשים אותם בו.
ההמרה לאחד משני הפורמטים האלה מאבדת אפוא אותם, והלוך ושוב אינו
מוצא אותם שוב — הוא גוזר אותם מחדש.

**LRC מאבד דבר נוסף**: אין בו דרך לשאת מעבר שורה.
שורת דיאלוג בשתי שורות נכתבת בשורה אחת, ושתי השורות מחוברות
ברווח. גם את האובדן הזה אי אפשר לתקן.

לאף אחד מהם אין אוצר מילים לנטוי.

## `--frame-rate`, והפורמט היחיד שסופר בפריימים

**MicroDVD אינו סופר בשניות**: המיקומים בו *הם* מספרי
פריימים, ושום קובץ MicroDVD אינו מצהיר על הקצב שלפיו הם
נספרו. `--frame-rate` נותן אותו, ומשמש בשני הצדדים: זהו
הקצב שלפיו נקרא `.sub` בפריימים, וזה שלפיו המרה
ל-MicroDVD סופרת את הפריימים.

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | לא | קצב בפריימים לשנייה: `25`, `23.976`, `29.97` | ראו להלן |

**בקריאה, בלי `--frame-rate`, הקובץ נקרא ב-23.976** — הערך של Gaupol —
והרמה `-vvv` אומרת זאת:
`counts in frames and states no rate; it was read at`. זהו המקום היחיד בכלי שבו כל מיקום מוצג
נשען על הנחה שהקובץ אינו יכול לאשר.

**בכתיבה, בלי `--frame-rate`, שלושה מקרים ושלוש תשובות:**

| קובץ המוצא | מה קורה |
| :------------------- | :-------------- |
| כבר MicroDVD | הקצב שקרא אותו כותב אותו בחזרה, והפריימים חוזרים זהים |
| מבוסס זמן, ומיקומיו נופלים על רשת | הרשת נלקחת, ו-`-vv` אומר איזו |
| מבוסס זמן, בלי רשת | **סירוב**, קוד 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

הסירוב מכוון. המחווה היחידה שנותרה הייתה להמציא מספר, וכל
שורה בקובץ הייתה זזה במידת הטעות שבו.

## מה ההמרה מאבדת

**המרה לפורמט אחר אינה לעולם בלי אובדן**, ומה שאינו שורד
תלוי בזוג:

| מה שאינו עובר | הסיבה |
| :--------------------- | :------- |
| הכותרת | יש לה משמעות רק בפורמט שלה: `[INFORMATION]` אינה כותרת של WebVTT |
| הנתונים הייחודיים לפורמט | הקואורדינטות של SubRip, המזהה וההגדרות של cue ב-WebVTT אין להם מקבילה במקום אחר |
| הדיוק | SubViewer 2, שני ה-Sub Station Alpha ו-LRC כותבים למאית, MPL2 לעשירית, TMPlayer לשנייה, MicroDVD לפריים; `01:00:00,017` חוזר כ-`01:00:00,020` |
| סוף השורה | ל-TMPlayer ול-LRC אין שדה עבורו; הקריאה הבאה גוזרת אותו מחדש |
| מעברי השורה | LRC בלבד: שורותיו נכנסות בשורה אחת, והשתיים מודבקות חזרה ברווח |
| התגיות שפורמט היעד אינו יכול לכתוב | ל-WebVTT אין `<font color>`, ל-TMPlayer ול-LRC אין דבר כלל |
| הפריסה | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: הם שורדים הלוך ושוב בפורמט שלהם, ונעלמים בהמרה |

**כתיבת קובץ בחזרה בפורמט שלו אינה מאבדת דבר** — זו הערובה
שמחזיקה את כל השאר, ו-`--to srt` על `.srt` מחזיר את אותם הבתים. דבר
אינו מפוענח אפילו: הטקסט נשאר המחרוזת הגולמית שנקראה.

## תגיות מתורגמות, לא מועתקות

**כל פורמט כותב נטוי בדרכו**, והמרה בלי תרגום
הייתה משאירה `{\i1}` בתוך `.srt`: טקסט שהמשתמש רואה, בקובץ
שלעולם לא ידע לפרש אותו. מה שעובר הם **שישה
דברים** — מודגש, נטוי, קו תחתון, צבע, גופן וגודל:

| פורמט | מודגש | נטוי | קו תחתון | צבע | גופן | גודל |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**מקף בטבלה הוא אובדן מוצהר.** המרת `.srt` צבעוני ל-WebVTT מסירה
את הצבע ואומרת זאת; הקובץ שנוצר הוא WebVTT, לא SubRip בהסוואה.

**MicroDVD ו-MPL2 אינם יכולים לעצב חלק משורה.** לתגיות שלהם
אין סוף: `{y:i}` מטה את כל מה שבא אחריו עד סוף השורה, ו-
`{Y:i}` עד סוף הכתובית. שורה שרק חציה
מודגש נכתבת אפוא בלי הדגשה כלל, ולא עם הדגשה שגולשת.

## מה שההמרה מדווחת שאבד

**היא אומרת זאת כשהיא מאבדת, ושותקת כשאינה מאבדת דבר.** דוח
שמופיע בכל קריאה הוא דוח שאיש אינו קורא.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

השורה מופיעה ברמת הפירוט של ברירת המחדל, ובכל אחת מעליה.
הפריטים, בסדר הזה:

| פריט | מה הוא אומר |
| :---- | :----------- |
| `ends are not carried by <format>` | פורמט היעד אינו כותב סוף |
| `line breaks were joined in N subtitles` | LRC בלבד: N כתוביות היו על כמה שורות |
| `N tags dropped` | N תגיות לא ניתן היה לכתוב, פריסה בכלל זה |
| `the header was dropped` | הכותרת אינה חוצה גבול פורמט |
| `the <format> fields of N subtitles were dropped` | N כתוביות נשאו נתונים ייחודיים לפורמט שלהן |
| `positions moved by up to N ms` | המרחק הגדול ביותר שבו מיקום זז |

## שינוי הקידוד

**`--to-encoding` בוחר את הקידוד שנכתב; `--encoding` אומר את זה
שנקרא.** השניים יחד מקודדים קובץ מחדש:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

בלי `--to-encoding`, הקובץ נכתב ב**קידוד שבו נקרא**,
כמו סוף השורה והסימן שלו: קובץ שהומר בלי הוראה
על קידוד מחזיר את אותם התווים, באותם הבתים.

**תו שקידוד היעד אינו יכול לכתוב עוצר את הקובץ**, ודבר
אינו נכתב. ל-`ł` אין מקום ב-Latin-1; החלפתו ב-`?`
הייתה מאבדת טקסט בין הקריאה לכתיבה, בלי שמישהו יראה.

**סימן סדר בתים קיים רק בקידודי Unicode.**
`--bom` עם `--to-encoding` שאינו נושא כזה נדחה ולא מתעלמים ממנו
— ראו טבלת השגיאות שלהלן.

**והקידוד חייב לנקוב בסדר הבתים שלו.** `UTF-16` ו-`UTF-32` הם
שמות ש-ICU מכיר, והממיר שלהם כותב **סימן משלו**, יהיה אשר יהיה מה
שמבקשים: `--no-bom` היה מתקבל אז ולא נשמע לו. שניהם
נדחים אפוא, וההודעה נוקבת בפלט:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` ו-`UTF-32BE` כותבים בדיוק אותם
בתים, הסימן בכלל זה — אך בשליטת `--bom` ו-`--no-bom`. הסירוב
אינו עולה אפוא דבר מלבד שם מדויק יותר, והוא תקף
לקריאה כמו לכתיבה: `--encoding UTF-16` מקבל את אותה תשובה.

## היעד

שלוש האפשרויות וכלליהן משותפים לתת-הפקודות
שכותבות: ראו [הפעלה](invocation.md#היעד). דבר אינו נכתב
בלי אחת מהן.

מה שייחודי ל-`convert`: **`--output-dir` משנה את הסיומת לפי
הפורמט.** `convert --to vtt` על `a.srt` כותב `a.vtt`, לעולם לא `a.srt` שמכיל
WebVTT — קובץ ששמו משקר מכשיל כל כלי אחר.

## המרה במקום נדחית

`--in-place` עם `--to` שמשנה את הפורמט נותן קוד `1` ואינו כותב
דבר: במקום, אין שם שני שיישא את הפורמט החדש, והקובץ
היה נשאר תחת סיומת שתוכנו כבר אינו מצדיק.

עדיין אפשר להשתמש ב-`--in-place` כדי לשנות **את הצורה בלי הפורמט** —
לכתוב `.srt` בחזרה כ-`.srt` עם סוף שורה אחר, למשל.

הסירוב נקבע על **הסיומת בלבד**, לפני כל קריאה: שגיאת
שימוש אסור שתשאיר אצווה כתובה למחצה.

## סוף שורה וסימן סדר בתים

כברירת מחדל, **הקובץ שנכתב ממשיך את מה שהקובץ שנקרא נשא**. המודל
שומר את שניהם בקריאה; כפייתם כברירת מחדל הייתה מאבדת, בכל
המרה, פיסת מידע ששמרו בכוונה.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## מיון בעת הכתיבה

`--sort` כותב את הקובץ **לפי סדר ההתחלות**, יהיה אשר יהיה פורמט היעד. המיון נעשה **על עותק**: הקובץ שנקרא
אין נוגעים בו, ומה שנכתב הוא מה ש-[`sort`](sort.md) היה עושה ממנו — יציב, שתי כתוביות שמתחילות יחד שומרות על סדרן.

**לעולם לא כברירת מחדל**: סדר הקובץ הוא של מחברו כל עוד לא ביקשו דבר אחר. עם האפשרות, כל קובץ אומר זאת, ברמת
הקריינות הראשונה, `<path>: N subtitles moved` או `<path>: already in order`, והרשומה של `--format json` נושאת ספירת
`moved` — **חסרה בלי האפשרות**, כדי לא לשנות את צורת מה שסקריפטים קיימים קוראים.

## הפיכת הטווחים לזרים

`--disjoint` חותך את הכתוביות החופפות כך שאף טווח לא יחפוף עוד טווח אחר, **על אותו עותק כמו המיון**: הקובץ שנקרא אין נוגעים בו.

לעולם לא כברירת מחדל. האפשרות עושה מה ש-[`disjoint`](disjoint.md) עושה, **עם הגדרות ברירת המחדל שלו**, לפני הכתיבה, אחרי המיון. היא צריכה קובץ ממוין: אחרת היא נדחית, ו-`--sort` — שבא קודם — מסדר אותו מראש. הקובץ אומר זאת ברמת הקריינות הראשונה (`<path>: N subtitles split, M merged`), והרשומה של `--format json` נושאת את ספירות `split` ו-`merged`, **חסרות בלי האפשרות**.

**`--join-repeats` בא לפני המיון והחיתוך**: טקסט שנחתך לחלקים הבאים בזה אחר זה נבנה מחדש תחילה (ראו
[`join-repeats`](join-repeats.md)), אחר כך הכתוביות ממוינות, ואחר כך הטווחים נעשים זרים. **`--repair` הוא שלושתם בבת אחת**,
כמו [`repair`](repair.md), עם הגדרות ברירת המחדל של החיתוך. עם `--join-repeats`, כל קובץ אומר
`<path>: N repeated subtitles joined` ברמת הקריינות הראשונה, והרשומה של `--format json` נושאת `counts.joined` — **חסרה בלי האפשרות**.

## פלט

**הפלט הסטנדרטי** — ריק: התוצאה היא הקובץ שנכתב. עם `--dry-run`, גם כן ריק: אין קובץ, ושורת הדיווח אומרת זאת — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

בפלט השגיאה, לפי הרמה:

| רמה | מה מתווסף |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — הקידוד הנקוב הוא זה שנכתב |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא `counts.subtitles`, ואחר כך מה שההמרה איבדה: `lost_ends` ו-`lost_header` (`0` או `1`), `joined_lines`, `lost_tags`, `lost_fields` ו-`furthest_ms` — הפריטים של [משפט האובדן](#מה-שההמרה-מדווחת-שאבד). הטבלה המלאה היא של [הפעלה](invocation.md#מה-כל-תת-פקודה-מציבה-ב-counts).

## קודי יציאה

אלה של הכלי: `0` אם כל הקבצים נכתבו, `2` אם אף אחד, `3` אם
רק חלקם, `1` בשגיאת שימוש.

## שגיאות

| מה מפעיל אותה | הודעה |
| :------------------ | :------ |
| `--bom` עם `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` שמשנה את הפורמט | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` על קידוד שאין לו כזה | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` הנוקב בקידוד לא מוכר | `no encoding is named "<name>"` |
| `--to-encoding` הנוקב בקידוד שכותב סימן משלו | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| תו חסר בקידוד שנכתב | `<path>: holds a character the chosen encoding cannot write` |

**הסירוב ל-`--bom` על קידוד שאינו נושא כזה אינו
קטנוניות.** סימן סדר בתים קיים בקידודי Unicode
ובשום קידוד אחר: לבקש אחד על קובץ ב-Windows-1252 פירושו לבקש
דבר שאינו קיים. הקובץ היה נכתב בלעדיו, ואיש לא היה
יודע שהבקשה לא כובדה.

אלה של היעד משותפות לתת-הפקודות שכותבות:
ראו [הפעלה](invocation.md#היעד).

שגיאות הקריאה הן אלה של [`inspect`](inspect.md): הן נוגעות
לקובץ הקלט ויש להן אותן הודעות.
