# `inspect`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

הפקודה מדווחת ממה עשוי כל קובץ. **היא אינה משנה דבר ואינה כותבת שום קובץ.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## ארגומנטים

| ארגומנט | חובה | ערך | ברירת מחדל |
| :------- | :----- | :----- | :----- |
| `<file>...` | כן | נתיב אחד או יותר של קובצי כתוביות | — |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--detail` | לא | דגל: נותן שם לכל חריגה, במקום לספור אותן — ראו [מה מדווח `anomalies`](#מה-מדווח-anomalies) | כבוי |

היעדר נתיב הוא שגיאת שימוש, ולכן קוד `1`.

## אפשרויות

| אפשרות | חובה | ערך | ברירת מחדל |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | לא | קצב בפריימים לשנייה: `25`, `23.976`, `29.97` | 23.976 |

**היא משמשת רק קבצים הנספרים בפריימים**, כלומר MicroDVD, היחיד מבין תשעת הפורמטים. קובץ MicroDVD אינו נושא דבר מלבד מספרי פריימים; הקצב שבו הם נקראים קובע **כל מיקום בדוח**, החל ב-`span`. כשהיא ניתנת במקום אחר, היא אינה משנה דבר ואינה שגיאה.

ערך שאינו מציין דבר הוא שגיאת שימוש, ולכן קוד `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

זו אותה אפשרות כמו ב-[`convert`](convert.md), מילה במילה, כי זה אותו קצב. `convert` משתמשת בה גם כדי **לכתוב**.

## תרגום: `-t`

| אפשרות | חובה | ערך | ברירת מחדל |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | לא | קובץ תרגום, עבור קלט **אחד** | — |
| `--align-method` | לא, ורק עם `-t` | `position` או `number` | `position` |

עם `-t`, התרגום מונח על הקובץ **כפי שהחלון עושה כשהוא פותח אותו** — ראו [תרגום](invocation.md#תרגום) — והדוח אומר **איך שורותיו מצאו את הכתובית שלהן**. כך סקריפט יודע אם תרגום מיושר לפני שהוא מתקן אותו.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

השורה הראשונה שנוספה נותנת שם לקובץ ולשיטה; השנייה היא **המשפט שהחלון אומר** כשהוא פותח תרגום, כל חלק רק כשאינו אפס:

| חלק | מה הוא סופר |
| :----- | :---------------- |
| `N lines attached` | שורות התרגום שמצאו כתובית בקובץ הראשי |
| `N subtitles born of a line` | השורות שלא מצאו כתובית וילדו כתובית משלהן, במיקומים משלהן |
| `N subtitles left without a translation` | כתוביות הקובץ הראשי שאף שורה לא הגיעה אליהן |
| `N lines out of order` | השורות שבאו אחרי שורה מאוחרת יותר בקובץ |

תרגום שאינו נושא אף שורה אומר `translation: the file holds no line`. **הדוח על הקובץ עצמו נשאר דוח על הקובץ בלבד**: כתובית שנולדה משורה אינה נספרת ב-`subtitles`.

השגיאות של `-t` הן אלה של קריאה, הנאמרות עם נתיב התרגום, ועוד זו: `<translation>: the file is already open as the main document`, כשהקובץ שניתן הוא הראשי. `--align-method` בלי `-t` היא שגיאת שימוש, וכך גם `-t` עם כמה קלטים — ראו [תרגום](invocation.md#תרגום).

## פלט

הדוח הולך ל**פלט הסטנדרטי** — הוא התוצאה של הפקודה, והוא נכתב בכל רמה, גם עם `--quiet`. בלוק אחד לכל קובץ, ולפניו הנתיב:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| שדה | מה הוא אומר |
| :---- | :----------- |
| `format` | אחד הפורמטים ש-`--to` מקבלת, כתוב במלואו: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | הקידוד שנקרא, ואחריו **מקור התשובה**: `, from its byte order mark` אם הקובץ מצהיר עליו, `, as asked for` אם `--encoding` כפתה אותו, `, detected` אם הוא נשקל |
| `byte order mark` | `present` או `absent` |
| `line endings` | `LF`, `CRLF` או `CR`, ואחריו `, mixed from line N` אם הקובץ מערבב ביניהם |
| `subtitles` | מספר הכתוביות שנקראו |
| `span` | מההתחלה המוקדמת ביותר ועד הסוף המאוחר ביותר, `HH:MM:SS.mmm` |
| `frame rate grid` | קצב הפריימים שלפיו חושבו המיקומים, **שנגזר** — ראו בהמשך |
| `frame rate` | בקובץ הנספר בפריימים, הקצב שבו הוא נקרא ו**מניין הוא בא** — מחליף את השורה הקודמת, ראו בהמשך |
| `anomalies` | `none` באותה שורה, או מה שאינו תקין: **סוג אחד בשורה עם הספירה שלו**, בהזחה מתחת ל-`anomalies:`; עם `--detail`, **חריגה אחת בשורה**, כתובית אחר כתובית |

**`span` אינו "מן הראשונה עד האחרונה"** אלא מן המוקדמת ביותר עד המאוחרת ביותר: בקובץ שסדרו נשבר, השניים שונים, ורק השני אומר אמת על מה שהקובץ מכסה.

**`anomalies`, עם `--detail`, סופרת את הכתוביות מ-1**, כפי שהקובץ ממספר אותן — ולא את שורות הקובץ. מספר כתובית שורד שינוי; מספר שורה אינו שורד. ראו בהמשך.

## מה מדווח `frame rate grid`

**קובץ כתוביות אינו מצהיר על קצב הפריימים שלו.** ל-SubRip אין כותרת, וזו של WebVTT היא טקסט חופשי. השורה הזו אינה קוראת אפוא דבר: היא **גוזרת**, בכך שהיא מודדת אם המיקומים נופלים על רשת של פריימים.

מה שנמדד הוא **ההתחלות**. בכל מקום שבו קיימת רשת הן עליה ללא יוצא מן הכלל, ואילו הסופים סוטים ממנה לעיתים קרובות — סוף מחושב לעיתים קרובות לפי כלל של מהירות קריאה ולא מוצב על פריים.

השורה הראשונה אומרת תמיד את אותו דבר: הקצב שנבחר, פסק הדין, והריכוז התומך בו.

| פסק דין | מה הוא אומר |
| :------ | :----------------- |
| `clean` | המיקומים על רשת |
| `partial` | חלקם על רשת, והשאר לא |
| `none` | אף מועמד אינו מסביר דבר |

ל-`none` יש **שתי סיבות, והן אינן אותה תשובה**: או שאף רשת אינה מתאימה — השורה נותנת אז את המועמד הטוב ביותר ואת הציון שלו, כדי שיראו בכמה הוא נכשל — או שהקובץ סופר מעט מדי כתוביות כדי שאפשר יהיה לומר עליו משהו. שתי התחלות נראות תמיד מיושרות להפליא; זה אינו מוכיח דבר.

**קובץ סדיר בקצב לא תקני נותן `none`, וזה מכוון.** קבוצת המועמדים סגורה — שמונת הקצבים שההמרה מציעה — ודיווח על הפחות שגוי שבהם היה מתן תשובה שגויה במקום שבו "איני יודע" היא התשובה הנכונה.

### השורות הבאות, כשיש להן מה לומר

| שורה | מתי היא מופיעה |
| :---- | :------------------ |
| `grid offset` | המיקומים על הרשת **פלוס מינוס קבוע**: הקובץ הוזז, והנה בכמה |
| `also fits` | מועמד אחר מתאים באותה מידה, כי הוא כפולה שלמה של זה שנבחר |
| `too short a span to separate` | טווח הקובץ אינו מאפשר להבדיל בין הקצב שנבחר לאלה שנקראו בשמם |
| `off the grid` | כמה התחלות סוטות מהרשת, ובכמה **רצפים** |

**`also fits` אינו היסוס.** רשת של 25 פריימים לשנייה *כלולה* ברשת של 50: קובץ שתוזמן על 25 מתאים אפוא לשתיהן. ההפך אינו נכון — 50 אמיתי קורס על 25 — ולכן הנמוך הוא שנבחר, והאחר נקרא בשמו ולא מושתק. הזוגות היחידים הנוגעים בדבר הם 25 ו-50, 29.97 ו-59.94, 30 ו-60.

**`too short a span to separate` שומרת מפני תשובה בטוחה מדי.** 23.976 ו-24 מתרחקים זה מזה במילישנייה אחת לכל שנייה של סרט: בסרט באורך מלא הם נפרדים ללא ספק, ובעשר שניות אי אפשר להבדיל ביניהם. השורה אומרת איזה מהם נשאר אפשרי.

**`off the grid` מבדילה בין שני סיפורים שהאחוז לבדו מבלבל ביניהם.** הרבה רצפים של התחלה אחת הם מיקומים שתוקנו ביד, אחד אחד. מעט רצפים ארוכים הם קטע שתוזמן מחדש או קובץ שהורכב משניים אחרים. מספר הרצפים הוא שמבדיל ביניהם.

### דוגמה

שתים עשרה שורות שהונחו על רשת של 25 פריימים לשנייה — כלומר במיקומים שהם כפולות של 40 אלפיות שנייה — פרושות על פני כמעט תשע דקות:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

הדוגמה יוצרת קובץ משלה ואינה קוראת קובץ מהמאגר, וזה מכוון: אפשר להריץ אותה כמות שהיא, והלולאה מראה מהי רשת טוב יותר ממשפט — מיקומים שהם כפולות של משך פריים.

## מה מדווח `frame rate`, ולמה זו אינה רשת

**רק אחד מתשעת הפורמטים נספר בפריימים**: MicroDVD, שכל שורה בו נושאת שני מספרי פריימים ודבר מלבדם. בקובץ כזה `inspect` אינה כותבת `frame rate grid` אלא `frame rate`, ושתי השורות אינן מוצגות יחד לעולם.

**הסיבה נכנסת במשפט.** המיקומים של קובץ MicroDVD חושבו *מתוך* הפריימים שלו, בקצב נתון; גזירתם מרשת הייתה מחזירה בדיוק את הקצב הזה, כפי שהוא סופק זה עתה. זה היה נתון שהוצג כאילו הוא מדידה.

מה שנכתב במקום זאת הוא **קצב הקריאה ומניין הוא בא** — באותה צורה כמו השורה `encoding`:

| מה השורה אומרת | מתי |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` כפתה אותו |
| `frame rate: 24000/1001 fps, assumed` | איש לא אמר אותו; זו ברירת המחדל |

**שתי תשובות ולא שלוש**, שם ש-`encoding` יש שלוש: קובץ MicroDVD אינו מצהיר על דבר. זו כל הבעיה — הקצב בא בהכרח מבחוץ, והדוח אומר מי משני המקורות החיצוניים.

**הקצב קובע את כל שאר הדוח.** `span` עובר דרכו, ושתי קריאות של אותו קובץ בשני קצבים נותנות שני טווחים:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## מה מדווח `anomalies`

שלושה דברים יכולים להיות לא תקינים במסמך, וכל אחד מתוקן אחרת. **כברירת מחדל `inspect` סופרת אותם**, שורה אחת לכל סוג, בסדר הטבלה: קובץ שנעשה רע נושא מאות מהם, והידיעה כמה מכל סוג היא שאומרת מה להריץ. **`--detail` נותן להם שם אחד אחד.**

| מה כתוב | עם `--detail` | מה זה אומר |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | הסוף לפני ההתחלה |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | היא חופפת את זו שלפניה |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | היא שוברת את סדר הקובץ — [`sort`](sort.md) מחזירה אותה למקומה |

סוג שאינו קיים אינו נכתב. בעמודה השמאלית `N` הוא מספר הכתוביות; בעמודה הבאה, מספר של כתובית.

**אותה כתובית יכולה להופיע פעמיים**, וזה מכוון: זו שמתחילה לפני שהקודמת התחילה מתחילה גם לפני שהיא הסתיימה. הנקודה הראשונה מוסדרת בהזזת הכתובית, השנייה בשינוי משך.

**הכתובית הנקראת בשמה היא זו שאינה במקומה**, ולא זו שמולה היא אינה במקומה. בהתחלות ב-`0`, `4 s`, `2 s`, `3 s`: השלישית שוברת את הסדר, הרביעית אכן באה אחרי השלישית ולכן אינה נקראת בשמה — אין מה לעשות בה.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

ועם `--detail`, הכתובית הנקראת בשמה:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**שורה אחת לכל חריגה עם `--detail`**, כי מאה ממצאים דחוסים בשורה אחת אי אפשר לקרוא ואי אפשר לחפש בהם. מסמך בלי פגם שומר על שורתו היחידה, `anomalies: none`, שסקריפט מחפש. **רשומת `--format json` מונה תמיד כל חריגה**: סקריפט קורא את הפירוט, ו-`--detail` משנה רק את הטקסט.

**זה אינו אבחון קריאה.** האבחונים אומרים במה נתקלה הקריאה ומצביעים על **שורה בקובץ**; החריגות אומרות מה המסמך *הוא* ומצביעות על **כתובית**. קובץ שסדרו נשבר אינו פגום בשל כך: לקריאה אין מה לומר עליו.

## ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), `inspect` כותבת **אובייקט אחד לכל קובץ** במקום הבלוקים שלמעלה; הדיווח נשאר בפלט השגיאות. המפתחות מתארים את מה שהטקסט אומר, ו**אף אחד מהם אינו נועד להיקרא בחזרה מתוך משפט**:

| מפתח | תוכן |
| :-- | :------ |
| `format` | מזהה הפורמט, זה ש-`convert --to` מקבלת: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` הוא `byte-order-mark`, `asked` או `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` או `cr`; השורה שבה מופיע סוג שני, או `null` |
| `subtitles` | מספר הכתוביות |
| `span_ms` | `{"start", "end"}` — ההתחלה הראשונה והסוף האחרון, באלפיות שנייה |
| `frame_rate` | בקובץ הנספר בפריימים: `{"rate", "origin"}` (`asked` או `assumed`); אחרת `null` |
| `grid` | לשאר: הרשת שנגזרה, או `null` בקובץ הנספר בפריימים |
| `anomalies` | מערך של `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **רק עם `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` הוא `position` או `number`, וארבע הספירות הן אלה של המשפט שלמעלה |
| `warnings` | במה נתקלה הקריאה, בכל רמה |

`grid` נושא את `verdict` (`clean`, `partial` או `silent`), `enough_starts`, `rate` (מחרוזת, או `null` כשדבר אינו נקרא בשמו), `concentration_permille` (באלפיות, `null` בלי מספיק התחלות), `offset_ms`, `also_fits` (הקצב שזה כפולה שלמה שלו, או `null`), `not_separated` (הקצבים שטווח קצר מדי אינו מבדיל ביניהם), `strays` (ההתחלות שמחוץ לרשת) ו-`starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## דיווח

בפלט השגיאות, לפי הרמה המבוקשת:

| רמה | מה נוסף |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, ואחריהם **כל אחד מהם** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

## קודי יציאה

אלו של הכלי: `0` אם כל הקבצים נקראו, `2` אם אף אחד לא נקרא, `3` אם רק חלקם נקראו, `1` בשגיאת שימוש.

## שגיאות

אלו של קריאה — של הקובץ, ושל התרגום כשניתן `-t` —, ואלו של `-t`: ראו למעלה. `inspect` אינה כותבת דבר שאפשר לדחות.

| מה גורם לה | הודעה |
| :------------------ | :------ |
| לא ניתן נתיב | `files is required`, ואחריה הפניה ל-`--help` |
| קובץ חסר | `<path>: does not exist` |
| קובץ שאי אפשר לקרוא | `<path>: cannot be opened: permission denied` |
| בתים שאי אפשר לפענח | `<path>: cannot be decoded in the chosen encoding` |
| פורמט שלא זוהה | `<path>: is in no format this tool knows` |
| אין דבר שנראה ככתובית | `<path>: holds nothing recognisable as a subtitle` |

אלו אותן הודעות לשבע עשרה תת-הפקודות: המתכון לפתיחה כתוב פעם אחת, בליבה.

## דוגמה שנכשלת

קובץ ריק אינו מתאים לאף פורמט. הוא אינו מדווח כריק: הוא נדחה, ונקרא בשמו.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
