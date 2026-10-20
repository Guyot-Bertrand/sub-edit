# `adjust`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli adjust [--speed CPS|off] [--shorten | --no-shorten] [--lengthen | --no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

מתאים את **משך** כל כתובית למהירות קריאה ולגבולות: כתובית קצרה מכדי שיהיה אפשר לקרוא אותה
מוארכת, כתובית שיוצאת מהמסך מאוחר מדי אפשר לקצר אותה, ושתי כתוביות אינן חופפות. **רק
הסוף זז**: הזזת ההתחלה הייתה מזיזה את הכתובית, וזו עבודתה של
[`shift`](shift.md). זוהי הפעולה `Adjust Durations…` של
[החלון](../subedit-gui/operations.md#adjust-durations), עם אותן ברירות מחדל.

## ארבעת האילוצים וסדרם

הם חלים **בסדר הזה**, כל אחד קובע את הסוף, והאחרון מנצח:

1. **מהירות הקריאה** — המשך שהטקסט צריך, במספר מסוים של תווים
   לשנייה (תגיות אינן נספרות, מעברי שורה נספרים);
2. **המינימום** — משך הנמוך ממנו מועלה למינימום;
3. **המקסימום** — משך הגבוה ממנו מורד למקסימום;
4. **הרווח** — הזמן שנשאר עד הכתובית הבאה. הוא בא אחרון
   ו-**הוא מנצח**: שתי כתוביות המוצגות יחד הן פגם שהצופה רואה,
   כתובית קצרה במעט אינה כזו.

לכתובית האחרונה בקובץ אין כתובית הבאה, ולכן אין רווח לשמור. הכתובית הבאה
של כתובית היא זו שבקובץ, **בין אם נבחרה ובין אם לא** על ידי `--range`.

| אילוץ | אפשרות | ברירת מחדל | כיבויו |
| :--------- | :----- | :----- | :------- |
| מהירות | `--speed CPS` | **15** תווים לשנייה | `--speed off` |
| כיוון המהירות | `--shorten`, `--no-shorten`, `--lengthen`, `--no-lengthen` | היא **מאריכה** בלבד | — |
| מינימום | `--minimum TIME` | **1.5 שניות** | `--minimum off` |
| מקסימום | `--maximum TIME` | **אין** | `--maximum off` |
| רווח | `--gap TIME` | **0** | `--gap off` |

אלה הגדרותיהם של Gaupol ושל הפתיחה הראשונה של החלון. **`off` מכבה
אילוץ: הוא כבר אינו נבדק.** זה אינו קביעתו לאפס — מינימום של `0`
הוא מינימום של אפס, רווח של `0` אוסר כל חפיפה.

## מה מספק קובץ ההגדרות

**ברירות המחדל שלמעלה הן אלה המובנות.** כברירת מחדל `adjust` קורא את הגדרות הפעולה מתוך
קובץ ההגדרות — הערכים ש-`Adjust Durations…` זוכר בחלון, או ש-
[`config set`](config.md) כתב — והן תופסות את מקום ברירות המחדל המובנות. **אפשרות
שניתנה בשורת הפקודה מנצחת תמיד**, אחריה הקובץ, ואחריו ברירת המחדל המובנית. על מקום הקובץ, על `--config` ועל
`--no-config` ראו [קובץ ההגדרות](invocation.md#קובץ-ההגדרות).

| מפתח | מה הוא מספק | אפשרות שדורסת אותו |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | מהירות הקריאה, בתווים לשנייה | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | לאיזה כיוון המהירות רשאית להזיז סוף; שניהם `false` מכבים את המהירות | `--lengthen`, `--no-lengthen`, `--shorten`, `--no-shorten`, `--speed off`; מתן `--speed CPS` או כיוון מפעיל מחדש מהירות שהקובץ כיבה, כשהיא מאריכה |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | המשך הקצר ביותר, או שאין | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | המשך הארוך ביותר, או שאין | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | הרווח הקטן ביותר, או שאין | `--gap TIME`, `--gap off` |

**לכל מפתח יש אפשרות שמבטלת אותו.** לכל כיוון יש זוג דגלים שקובע אותו
יהיה אשר יהיה מה שהקובץ אומר: `--shorten` ו-`--no-shorten` לקיצור, `--lengthen` ו-
`--no-lengthen` להארכה; דגל שלא ניתן משאיר את כיוון הקובץ כמות שהוא.
`--maximum off` מוריד מקסימום שהקובץ מפעיל. עם `--format json`, `constraints` אומר
את ההגדרות שבהן השתמשו בפועל, ויהיה מקורן אשר יהיה, ו-`settings` את הקובץ שנקרא:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

שאר הדוגמאות בדף זה מורצות עם `--no-config`, כדי שיאמרו מה עושות ברירות המחדל המובנות,
יהיו אשר יהיו הגדרותיה של המכונה שמריצה אותן.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-shorten                Do not let the reading speed bring an end earlier
  --lengthen                  Let the reading speed move an end later
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | לא | מספר תווים לשנייה, **גדול מאפס** (`15`, `12.5`), או `off` | `15` |
| `--shorten` | לא | דגל: המהירות רשאית גם **להקדים** סוף | כבוי |
| `--no-shorten` | לא | דגל: המהירות אינה **מקדימה** עוד סוף, יהיה אשר יהיה מה שקובץ ההגדרות אומר | כבוי |
| `--lengthen` | לא | דגל: המהירות **מאחרת** סוף, יהיה אשר יהיה מה שקובץ ההגדרות אומר | כבוי |
| `--no-lengthen` | לא | דגל: המהירות אינה **מאחרת** עוד סוף | כבוי |
| `--minimum` | לא | משך של אפס או יותר, או `off` | `1.5` |
| `--maximum` | לא | משך **גדול מאפס**, או `off` | אין |
| `--gap` | לא | משך של אפס או יותר, או `off` | `0` |
| `--range` | לא | `N-M`, או `N-` עד הסוף — ראו להלן | כל הקובץ |
| `--output` / `--output-dir` / `--in-place` | **אחת מהשלוש**, מלבד עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

משך נכתב כמו ב-[`shift`](shift.md#איך-כותבים-משך): `2.5`, `00:00:02.500`.

**בלי אילוץ פעיל כלשהו, `adjust` נדחה** (קוד `1`, דבר אינו נכתב): לא היה
מה להתאים אליו, ופקודה שאינה עושה דבר ואומרת שהצליחה היא התשובה הגרועה
ביותר. זה המצב של `--speed off --minimum off --gap off` בלי מקסימום, ושל
`--no-lengthen` לבדו כשהמינימום והרווח כבויים: מהירות שאינה מזיזה
אף סוף אינה מגבילה דבר.

`--shorten`, `--no-shorten`, `--lengthen` ו-`--no-lengthen` אומרים לאיזה כיוון המהירות מזיזה
סוף; יש להם משמעות רק עם מהירות, ו-**`--speed off` דוחה אותם**. דגל שניתן עם
היפוכו — `--shorten --no-shorten`, `--lengthen --no-lengthen` — נדחה גם הוא.

### `--range`

`--range 3-7` מגביל את ההתאמה לכתוביות 3 עד 7, **הנספרות מ-1, הגבולות
נכללים**; `--range 3-` ממשיך עד האחרונה. כתובית אחת נכתבת `4-4`.

- טווח **שמסתיים לפני שהוא מתחיל**, `0`, סימן או מספר בודד נדחים
  כשגיאת שימוש (קוד `1`).
- **הוא תקף לכל קבצי האצווה**, וזה הגיוני רק אם הקבצים
  דומים זה לזה. הוא נשפט **מול כל קובץ**: טווח שחורג מאותו קובץ
  נדחה **לפני שנוגעים בו**, ונקוב בגבול —
  `range 3-9 ends after the last subtitle: the file has 4` —, אותו קובץ נכשל (קוד `2` או `3`, מזהה
  `range-out-of-bounds`) והאחרים ממשיכים.
- הספירות בדוח נוגעות רק לכתוביות שנבחרו.

## מה מקריבים

שני אילוצים יכולים לסתור זה את זה באותה כתובית: מינימום של 1.5 שניות אינו נכנס
ב-0.8 השנייה שהכתובית הבאה משאירה. **הסדר מכריע, והדוח אומר
מה ויתר** — מה ש-Gaupol אינו עושה: הוא שובר אותם בשקט.

כל אילוץ נקוב בשמו רק אם **לפחות כתובית אחת לא קיבלה אותו**, לאחר
שהאחרים הוחלו. למקסימום אין ספירה: הוא מוחל לפני הרווח, שיכול
רק לקצר, ולכן הוא תמיד מתקיים. מינימום הגבוה מהמקסימום מוותר למקסימום
ונספר כמינימום.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

הכתובית השלישית הייתה רוצה להסתיים ב-15.7 שניות כדי להימשך 1.5 שניות, אך הרביעית מתחילה
ב-15.0 שניות: **הרווח מנצח**, היא מסתיימת ב-15.0 שניות, והדוח נוקב במינימום.

כיבוי הרווח נותן למינימום לנצח — ושתי הכתוביות חופפות:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

ושום דבר אינו מוקרב או מותאם כשהקובץ כבר עומד בגבולותיו:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

הקובץ נכתב כמות שהוא: **יעד שניתן הוא יעד שנכתב**.

## פלט

**הפלט הסטנדרטי** — ריק: התוצאה היא הקובץ שנכתב.

| רמה | מה מתווסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, או `<path>: no duration to adjust -> <destination>` — ולפניהם, קובץ ההגדרות שנקרא — `settings: read <path>` — ראו [קובץ ההגדרות](invocation.md#קובץ-ההגדרות) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, מוחזרים כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריו **כל אבחוני הקריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

המשפט הוא של החלון: `N` הוא מספר הכתוביות שסופן זז, והשאר,
חסר כשדבר לא ויתר, מפרט מה לא נשמר — `the reading speed`,
`the minimum duration`, `the gap` — עם מספר הכתוביות הנוגעות בדבר.

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ
נושא, נוסף על המעטפה:

| מפתח | תוכן |
| :-- | :------ |
| `counts.subtitles` | הכתוביות **המכוונות**: כל הקובץ, או הטווח |
| `counts.adjusted` | אלה שסופן זז |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | הכתוביות שעדיין אינן שומרות על האילוץ הזה, לאחר שהאחרים הוחלו, **כולל אלה שסופן לא זז** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — התווים לשנייה **כמחרוזת** (`"15"`, `"12.5"`) — או `null` אם כבוי |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | המשכים שהיו בשימוש, ב**אלפיות שנייה שלמות**, או `null` אם כבוי |
| `settings` | קובץ ההגדרות שנקרא, או `null` כשלא נקרא אחד |

`constraints` אומר **את ההגדרות שבהן השתמשו בפועל**, כולל ברירות המחדל: שני חישובים
על אותן כתוביות מושווים אז בלי לקרוא את שורת הפקודה.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## קודי יציאה

אלה של הכלי: `0` אם כל הקבצים עובדו, `2` אם אף אחד, `3` אם רק
חלקם, `1` בשגיאת שימוש. קובץ שכבר עומד בגבולותיו נחשב מעובד.

## שגיאות

| מה מפעיל אותה | הודעה |
| :------------------ | :------ |
| קובץ ש-`--config` נוקב בו ואינו קיים — קובץ המשתמש עצמו, כשהוא חסר, נותן את ברירות המחדל | `settings file not found: <FILE>` |
| אין אילוץ פעיל | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| מהירות שאינה כזו | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| דגל כיוון עם `--speed off` | `--lengthen, --no-lengthen, --shorten and --no-shorten say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| דגל כיוון עם היפוכו | `--shorten and --no-shorten say opposite things; give one of them` (וכך גם `--lengthen` ו-`--no-lengthen`) |
| משך שאינו כזה | `--minimum: "…" is not a time: …` (וכך גם `--maximum`, `--gap`) |
| משך שלילי | `--minimum: "-1" is not a duration of zero or more` (וכך גם `--gap`) |
| מקסימום אפס או שלילי | `--maximum: "0" is not a duration above zero` |
| טווח שנכתב לא נכון | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| טווח שמסתיים לפני שהוא מתחיל | `--range: "5-3" is not a range: it ends before it starts` |
| טווח מחוץ לקובץ | `<path>: range N-M starts after the last subtitle: the file has K`, או `… ends after …` |

שגיאות היעד משותפות לתת-הפקודות שכותבות:
ראו [הפעלה](invocation.md#היעד).
