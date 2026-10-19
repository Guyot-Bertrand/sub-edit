# subedit — מדריך

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

subedit קורא קובצי כתוביות, בודק אותם ומתזמן אותם מחדש. הוא מורכב משתי
תוכניות החולקות אותו ליבה, ולכן אותם כללים ואותן מילים.

| תוכנית | למה היא משמשת | המדריך שלה |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | לפתוח קובץ, לראות אותו בטבלה, לערוך אותו, לצפות בו מול הסרט | [מדריך החלון](subedit-gui/index.md) |
| `subedit-cli` | לעבד קבצים באצווה, מתוך סקריפט, בלי ממשק | [מדריך שורת הפקודה](subedit-cli/index.md) |

שתיהן קוראות וכותבות **את תשעת הפורמטים** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — ומזהות
את הפורמט לפי התוכן ולא לפי הסיומת. **גם הקידוד מזוהה לפי התוכן**:
קובץ ב-Latin-1, ב-CP1252 או ב-UTF-16 נפתח, נאמר כיצד הוא נקרא, והוא נכתב
בחזרה בקידוד שלו — קובץ שנפתח ונשמר בלי שינוי אינו זז אפילו בבית אחד.

## מאיפה מתחילים

| מה רוצים | לאן הולכים |
| :----------------- | :------- |
| להתקין את הכלי | [התקנה](subedit-cli/installation.md) — היא תקפה לשתי התוכניות |
| לפתוח קובץ ולראות מה יש בו | [הטבלה](subedit-gui/table.md) |
| לתזמן מחדש כתוביות שהוזזו בסכום קבוע | [הפעולות](subedit-gui/operations.md#shift-positions), או [`shift`](subedit-cli/shift.md) |
| לתזמן מחדש קובץ שנכתב לקצב פריימים אחר | [`framerate`](subedit-cli/framerate.md), ו-[`snap`](subedit-cli/snap.md) אם התזמון כבר נכון |
| לדעת מול איזה קצב פריימים נכתב קובץ | [רשת הפריימים](subedit-gui/frame-grid.md), או [`inspect`](subedit-cli/inspect.md) |
| להסיר את ההערות לכבדי שמיעה | [הפעולות](subedit-gui/operations.md#remove-hearing-impaired-mentions), או [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| לתקן הערות, שגיאות נפוצות, רשיות ושבירת שורות במעבר אחד | [`Correct Texts…`](subedit-gui/correct-texts.md), או [`correct`](subedit-cli/correct.md) |
| לבדוק את האיות מילה אחר מילה, עם המילונים של המערכת | [`Check Spelling…`](subedit-gui/check-spelling.md) |
| להאריך כתוביות קצרות מכדי שיהיה אפשר לקרוא אותן | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), או [`adjust`](subedit-cli/adjust.md) |
| להחזיר לסדר קובץ שהכתוביות בו אינן מסודרות | [`sort`](subedit-cli/sort.md) |
| ליצור כתובית אחת מהחלקים של טקסט אחד הבאים בזה אחר זה | [`Join Repeated Subtitles`](subedit-gui/lines.md#מיזוג-כתוביות-חוזרות), או [`join-repeats`](subedit-cli/join-repeats.md) |
| ליישר במכה אחת קובץ שחלוקתו גרועה, שאינו מסודר ושהכתוביות בו חופפות | [`repair`](subedit-cli/repair.md) |
| להסיר את החפיפות בין כתוביות | [`Make Ranges Disjoint…`](subedit-gui/lines.md#הפיכת-הטווחים-לזרים), או [`disjoint`](subedit-cli/disjoint.md) |
| לחבר זה אחר זה את שני חלקי הסרט | [`Append File…`](subedit-gui/operations.md#append-file), או [`append`](subedit-cli/append.md) |
| לחתוך סרט לשני חלקים | [`Split Project…`](subedit-gui/operations.md#split-project), או [`split-file`](subedit-cli/split-file.md) |
| לשנות את הרשיות, את הנטוי או את מקפי הדו־שיח של קובץ שלם | [הפעולות](subedit-gui/operations.md#italic), או [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| לחפש מילה ולהחליף אותה בלי לשבור את התגיות | [חיפוש והחלפה](subedit-gui/find-and-replace.md), או [`replace`](subedit-cli/replace.md) |
| לתרגם לצד הטקסט המקורי | [פתיחת תרגום](subedit-gui/files.md#פתיחת-תרגום), או [`pair`](subedit-cli/pair.md) כדי לתזמן אותו מחדש לפי הקובץ הראשי |
| לעבד מאה קבצים בבת אחת | [עיבוד עץ קבצים](subedit-cli/lots.md), ואחר כך [הפעלה](subedit-cli/invocation.md) |

**בין `framerate` ל-`snap` טעות אינה ניכרת.** אם יש היסוס,
[`snap`](subedit-cli/snap.md#snap-אינו-framerate-וכשמחליפים-ביניהם-זה-לא-ניכר)
נפתח בהשוואה בין השניים.

## מה המדריך הזה מבטיח

**הוא מתאר את הקיים, לעולם לא את המתוכנן.** מה יבוא, ובאיזה סדר, כתוב
ב[מפת הדרכים](../../../feuille-de-route.md).

**המדריך הזה נכתב באנגלית ומתורגם.** תרגומיו עוקבים אחריו דף אחר דף,
בתיקייה `docs/i18n/` של המאגר, עץ אחד לכל שפה. התוכניות מדברות בשפת
הסביבה שלהן; השמות שהן מציגות — תת-פקודות, אפשרויות, תוויות תפריט — מצוטטים כאן כפי שהתוכנית כותבת
אותם באנגלית.

**הדוגמאות והתמונות נוצרות אוטומטית, הפרוזה לא.** בלוקי ה-`console`
מופקים מהרצה ממשית של הפקודה; צילומי המסך, מבנייה של החלון האמיתי
וצילומו. אף אחד מהשניים אינו יכול אפוא לתאר כלי שכבר אינו קיים. מה שנשאר בלי רשת ביטחון
הוא הטקסט שמסביבם, שנכונותו נשענת על הגהה.

**צילומי המסך מראים את שתי הפלטות**, `Light` ו-`Dark` — אלה
ש[ההעדפות](subedit-gui/preferences.md#ערכת-הנושא) מציעות.

## שני הממשקים אומרים את אותן המילים

סיבה אחת לקרוא כל אחד משני המדריכים בעת השימוש בשני: הודעות
השגיאה, שמות הפורמטים, פסקי הרשת ותוויות הביטול נכתבים **פעם אחת**, בליבה, ושתי התוכניות
מצטטות אותם. קובץ שנדחה מניב אותו משפט בשני הצדדים.

מה שמבדיל הוא מה שכל ממשק יכול לעשות:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| כמה קבצים בבת אחת | כן — לשונית אחת לכל מיזם | כן |
| תרגום לצד הקובץ הראשי | כן | כן — `-t` בתת-פקודות הטקסט, ו-[`pair`](subedit-cli/pair.md) כדי לתזמן אותו מחדש |
| הוספת קובץ בסוף, פיצול מיזם | כן | כן — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| ביטול | כן, אלף פריטים | לא רלוונטי — שום דבר אינו משתנה במקום בלי שביקשו |
| עריכת טקסט או מיקום ביד | כן | לא |
| הוספה, מחיקה, מיזוג ופיצול של שורות | כן | לא |
| חיפוש והחלפה | כן | כן — [`replace`](subedit-cli/replace.md) |
| גזירה, העתקה והדבקה של טקסטים | כן | לא |
| התאמת המשכים, הנטוי, הרשיות, המקפים | כן | כן — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| תיקון הטקסטים לפי תבניות | כן — `Correct Texts…` | כן — [`correct`](subedit-cli/correct.md) |
| בדיקת האיות מילה אחר מילה | כן — `Check Spelling…` | לא |
| צפייה בסרט תוך כדי תזמון | כן | לא |
| כתיבה על הקלט | `Save` | `--in-place` |
