# עיבוד עץ קבצים

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

כל תת-פקודה שקוראת אצווה של קבצים — כולל `inspect`, ולא [`append`](append.md), [`split-file`](split-file.md) ו-[`pair`](pair.md), שמספר הארגומנטים שלהן הוא זה של זוג או של סרט — מקבלת **תיקייה** במקום קובץ, בצירוף `--recursive` (`-r`).
הדף הזה אומר מה הסריקה כוללת, באיזה סדר, ולאן היא כותבת.

**שום דבר ממה שנאמר להלן אינו משנה את מה שנעשה בקובץ**: כל קובץ נקרא, מעובד ונכתב בנפרד, הכשל של אחד אינו עוצר את האחרים, וארבעת קודי היציאה הם אלה של [הפעלה](invocation.md#קודי-יציאה).
הסריקה מפיקה רשימת נתיבים, והשאר באצווה צורך אותה כאילו הוקלדה ידנית.

## ללא `--recursive` תיקייה נדחית

שגיאת שימוש, קוד `1`, אף קובץ אינו מעובד או נכתב:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## מה הסריקה כוללת

הסריקה היא **רקורסיבית, דטרמיניסטית וללא הפתעות**:

- **הסדר** — הפריטים בכל תיקייה נלקחים לפי סדר שמותיהם, **בית אחר בית** (אותיות רשיות לפני אותיות קטנות, בלי קשר לשפת המחשב), ותת-תיקייה נסרקת במקומה, לעומק תחילה. שתי הרצות נותנות אותם בתים בדיוק, בכל מחשב;
- **קישורים סימליים אינם נעקבים** — לא אל קובץ ולא אל תיקייה: עץ שמכיל קישור אל אחת מתיקיות האב שלו לא היה נגמר;
- **קבצים ותיקיות מוסתרים מתעלמים מהם** — אלה ששמם מתחיל בנקודה;
- **רק הסיומות של הפורמטים המוכרים נשמרות**: `.srt`, `.vtt`, `.ssa`, `.ass`, `.lrc`, `.sub`, בלי קשר לרשיות האותיות. **`.txt` מוחרג**: הסיומת מציינת שני פורמטים, אבל בעיקר כל `LISEZMOI.txt` בעולם, ואצווה שנכשלת על כל אחד מהם לא הייתה שמישה.

**קובץ ששמו נקוב בשורת הפקודה לעולם אינו מסונן.** מי שמנקב אותו רוצה בו: `subedit-cli shift --by 1 --in-place notes.txt` מעבד אותו, יהיו אשר יהיו סיומתו, ומתלונן רק אם תוכנו אינו באף פורמט.

**מה שהושאר בצד נספר**, ונאמר ברמה `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

תיקייה שאין בה דבר לסרוק אומרת זאת ברמת ברירת המחדל (`<directory>: holds no file in a format this tool walks for`) ואין בכך שגיאה.

## לאן הסריקה כותבת

**העץ נשמר תחת `--output-dir`**: נתיבו של קובץ, **יחסית לתיקייה שניתנה**, משוחזר תחת תיקיית הפלט. שם התיקייה שניתנה אינו חלק ממנו — זו הסמנטיקה של `rsync` עם לוכסן סופי, לא של `cp -r`: מנקבים את היעד, ומוסיפים לו את הרמה הרצויה (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

תיקיית הפלט ותיקיות העץ **נוצרות** אם הן חסרות. עם `--in-place`, כל קובץ שנשמר נכתב בחזרה במקומו. סיומת ש-`convert` משנה משתנה גם בעץ: `films/a/x.srt` שהומר ל-WebVTT נכתב `sortie/a/x.vtt`.

**תיקיית הפלט, אם היא בתוך העץ הנסרק, מוחרגת מהסריקה**: הרצה שנייה אינה קוראת שוב את מה שהראשונה כתבה.

## שתי תיקיות, והתנגשויות

הכלל של [היעד](invocation.md#היעד) מכריע: שני קבצים שהיו מגיעים לאותו נתיב נדחים **לפני שמשהו נכתב**, קוד `1`, ושני הקלטים והיעד נקובים בשמם.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## מה זה עולה

האצווה נשארת סדרתית: משכה הוא סכום משכי הקבצים שלה. מבחן הביצועים `make bench` מודד את הסריקה ואת אותה עבודה בקובץ יחיד; הנתונים נמצאים ב[יומן הביצועים](../../../../mesures/performances.md).
