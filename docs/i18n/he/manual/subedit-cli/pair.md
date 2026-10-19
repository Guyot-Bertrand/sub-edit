# `pair`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

כותבת **את התרגום מתוזמן מחדש על מיקומי הקובץ הראשי**, ואומרת איך השורות שלו מצאו את הכתובית שלהן. זוהי הפעולה `Open Translation…` של החלון — [אותה התאמה](../subedit-gui/files.md#פתיחת-תרגום), אותו משפט — ואחריה כתיבת התוצאה, שהחלון מבצע באמצעות `Save Translation`.

**זה הדבר היחיד שההתאמה עושה ושקובץ המעובד לבדו אינו עושה.** שאר תת-הפקודות שמקבלות `-t` משנות את התרגום שהן קוראות — [`case`](case.md), [`replace`](replace.md)… —; `pair` אינה משנה דבר בטקסט: היא **מניחה** את התרגום על הקובץ הראשי וכותבת את מה שההנחה הפיקה. הקובץ הראשי נקרא כדי לקבל את השורות, **ואינו נכתב לעולם**.

- **מה שנכתב הוא התרגום**, ב**נתיבו שלו ובפורמט שלו**: הפורמט, הקידוד וסוף השורה של קובץ התרגום. הכתוביות שלו נמצאות **במיקומי הקובץ הראשי**: שורה שנפלה לצד היעד מקבלת בחזרה את גבולות הכתובית שהיא מתרגמת.
- **`-t` הוא חובה.** ל-`pair` בלי תרגום אין מה להניח: זו שגיאת שימוש, קוד `1`.
- **קובץ ראשי אחד בלבד**, וללא `--recursive`: זוג הוא שני קבצים, ולעץ אין דרך לומר איזה תרגום שייך לאיזה קובץ ראשי. לאצווה, הרצה אחת לכל זוג.
- **`--output` מנקב את קובץ התרגום הנכתב, `--output-dir` מציב שם את שם הבסיס שלו, `--in-place` כותב אותו מחדש**, כמו בכל תת-פקודה שמקבלת `-t` — ראו [תרגום](invocation.md#תרגום).
- **`--document` אינו קיים**: יש רק מסמך אחד שמכוונים אליו, התרגום.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
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
| `<main>` | כן | נתיב **יחיד** | — |
| `-t`, `--translation-file` | **כן** | קובץ התרגום | — |
| `--align-method` | לא | `position` או `number` — ראו [תרגום](invocation.md#תרגום) | `position` |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

התרגום איחר ב-0.3 שניות: כל שורה מצאה את הכתובית שאמצעה חופף לה, והקובץ שנכתב נושא את מיקומי הקובץ הראשי.

## מה ההתאמה אומרת

המשפט הוא **זה שהחלון אומר** כשהוא פותח תרגום, וכל חלק בו מופיע רק אם אינו אפס — ראו [`inspect -t`](inspect.md#תרגום--t) לטבלת חלקיו. **הוא נכתב כבר מרמה 1**, ולא ברמה 2 כמו בשאר תת-הפקודות: כאן ההתאמה אינה פרט של העיבוד, היא התוצאה.

**מה ההתאמה עושה לקובץ שנכתב**:

- **כתובית שאיש לא תרגם נכתבת כבלוק בלי טקסט**, שנקרא בלי שום חריגה;
- **כתובית שנולדה משורה** — שורה ששום דבר לא הגיע אליה — נכתבת עם **המיקומים שלה עצמה**, בכל שיטה שתהיה: הקובץ שנכתב יכול אפוא למנות יותר כתוביות מהקובץ הראשי;
- **כל מה שהמסמך נשא כתרגום מתרוקן** לפני ההנחה, מה שאינו חל בשורת הפקודה: הקובץ הראשי נקרא לבדו.

**שתי השיטות** נותנות אותו קובץ כשכל שורה נופלת על הכתובית שלה, ואינן נותנות כשחסרה שורה או כשהתרגום מוסט — ולכן השיטה היא בחירה, ולכן המיקום הוא ברירת המחדל. שמונת המקרים של `src/test/data/paires/` מונים אותם, עם הקובץ הצפוי לכל שיטה.

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב.

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **הנתיב הוא של התרגום**, שהוא הקובץ שנכתב; עם `--dry-run`, `(dry run, nothing written)` במקום `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה **של התרגום**, שהוחזרו כפי שהיו |
| 3 | `<translation>: N bytes read, M written`, ואחריה **כל אבחון קריאה** של הקובץ הראשי ושל התרגום — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), **אובייקט יחיד**, ש-`file` בו הוא התרגום ו-`destination` הוא הקובץ שנכתב. `counts.subtitles` מונה **את הכתוביות של הקובץ שנכתב**, כולל אלה ששורה החיתה. המפתח `alignment` — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — נושא את אותם מניינים כמו המשפט, כמו המפתח `translation` של [`inspect`](inspect.md#ב-json).

## קודי יציאה

אלה של הכלי: `0` אם התרגום נכתב — או חושב, עם `--dry-run` —, `2` אם אחד משני הקבצים לא היה ניתן לקריאה או אם לא ניתן היה לכתוב את היעד, `1` בשגיאת שימוש. **תרגום שהותאם גרוע אינו כשל**: הוא נכתב, והמשפט אומר מה לא מצא את מקומו.

## שגיאות

| מה גורם להן | הודעה |
| :------------------- | :------ |
| אין `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — הוא אינו קיים ב-`pair` |
| יותר מקובץ ראשי אחד | `The following argument was not expected: <path>` |
| התרגום הוא הקובץ הראשי | `<translation>: the file is already open as the main document`, קוד `2` |
| אחד הקבצים חסר, אינו ניתן לקריאה או בפורמט לא מוכר | אותה הודעה כמו בכל מקום, עם נתיב הקובץ הפגום |
