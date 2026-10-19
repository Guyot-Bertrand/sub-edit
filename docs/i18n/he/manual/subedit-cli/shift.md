# `shift`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

מזיזה **כל** מיקום בקובץ באותה כמות, לכיוון זה או לכיוון אחר. אף כתובית אינה נשארת על המסך זמן רב יותר או קצר יותר: זה מה שהופך הזזה להזזה.

<!-- example: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **אחת משתיים** | משך עם סימן, ראו להלן | — |
| `--to-grid` | **אחת משתיים** | דגל: הכמות נמדדת, ראו להלן | — |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

פורמט הקובץ שנקרא **נשמר**: שינוי הפורמט הוא עבודתה של [`convert`](convert.md).

## `--to-grid`: הכמות נמדדת ולא ניתנת

קובץ שמיקומיו נמצאים על רשת של פריימים **פלוס מינוס קבוע** הוזז, ואת הקבוע הזה אפשר למדוד על המיקומים עצמם. `--to-grid` מודדת אותו ומחילה אותו, בלי שמישהו יצטרך לדעת אותו.

הכמות **ייחודית לכל קובץ**: שני קבצים שהוזזו מאותה רשת בשתי כמויות שונות חוזרים כל אחד בכמותו שלו. היא נכתבת בדיווח, יחד עם קצב הפריימים שנמצא.

**הדרך היא הקצרה ביותר.** מעבר לחצי פריים, קו הרשת הקרוב ביותר הוא הבא: התיקון הולך קדימה ולא אחורה בכמעט פריים שלם.

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**קובץ שאינו על שום רשת מוכרת נדחה, ונאמר לו מדוע.** אין מה לחבר מחדש, ופאזה שנמדדה על רעש הייתה מזיזה את הקובץ באקראי. ראו את [`inspect`](inspect.md#מה-מדווח-frame-rate-grid) כדי לדעת מה קובץ נותן לפני שמנסים את הפעולה.

`--by` ו-`--to-grid` שתיהן אומרות בכמה להזיז: לתת את שתיהן יחד היא שגיאת שימוש, וכך גם לא לתת אף אחת.

**`--to-grid` אינה [`snap`](snap.md).** היא מזיזה את הקובץ כולו באותה כמות ושומרת את התזמון היחסי **במדויק**; `snap` מזיזה כל מיקום בנפרד וקולטת בדרך את אלה שתוקנו ביד. בקובץ נקי השתיים נותנות אותה תוצאה; בקובץ חלקי הן אינן נותנות.

## איך כותבים משך

שתי צורות, ורק שתיים:

| צורה | דוגמאות |
| :---- | :------- |
| שניות | `30`, `2.999`, `-7.001`, `+1.5` |
| חותמת זמן | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

שתיהן אומרות אותו דבר: `--by 7.001` ו-`--by 00:00:07.001` מפיקות אותו קובץ.

**המפריד העשרוני הוא הנקודה**, שכן שורת הפקודה מדברת אנגלית. פסיק נדחה ולא מפורש: חצי מהעולם כותב `7,001` לשבעת אלפים ואחד, וקריאתו כשבע שניות הייתה ניחוש, שגוי עבור אותו חצי.

**הרזולוציה היא אלפית שנייה.** עדין מזה נדחה ולא מעוגל בשקט: עיגול שותק היה נותן תוצאה שאינה תואמת דבר ממה שהתבקש.

<!-- example: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## הזזה לפני נקודת האפס נדחית

חזרה אחורה מעבר לאפס נותנת קוד `2` ו**אינה כותבת דבר**. ההודעה נוקבת בכתובית שלא יכלה לשאת את ההזזה, ובכך אומרת גם עד היכן אפשר ללכת.

הסיבה אינה עיקרון: מיקום לפני נקודת האפס נכתב `-00:00:01,000`, צורה שאף אחד מתשעת הפורמטים אינו מגדיר ואף קורא אינו מקבל. הליבה אמנם מתירה מיקומים כאלה — דחייתם הייתה הופכת פעולת עריכה למקרה מיוחד — אבל קובץ הוא דבר אחר ממודל בזיכרון.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב. עם `--dry-run`, גם אז דבר: אין קובץ, ושורת הדיווח אומרת זאת — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים).

| רמה | מה נוסף בפלט השגיאה |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — הפורמט, הקידוד, הסימן וסוף השורה של הקובץ שנקרא, שהוחזרו כפי שהיו |
| 3 | `<path>: N bytes read, M written`, ואחריה **כל אבחון קריאה** — ראו [הפעלה](invocation.md#אבחוני-קריאה) |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של כל קובץ נושא את `counts.subtitles` (כל כתובית בקובץ) ואת `counts.shifted_by_ms` (ההזזה, באלפיות שנייה שלמות, **עם סימן**). הטבלה המלאה היא זו של [הפעלה](invocation.md#מה-כל-תת-פקודה-מציבה-ב-counts).

## קודי יציאה

אלה של הכלי: `0` אם כל קובץ נכתב, `2` אם אף אחד, `3` אם רק חלקם, `1` בשגיאת שימוש.

## שגיאות

| מה גורם לה | הודעה |
| :------------------ | :------ |
| פסיק עשרוני | `"…" is not a time: use a decimal point and not a comma, in every language` |
| עדין מאלפית שנייה | `"…" is not a time: positions are held to the millisecond, no finer` |
| טקסט שאינו משך | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| הזזה לפני נקודת האפס | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**דבר אינו נכתב בלי אחת משלוש אפשרויות היעד**, ושגיאותיהן משותפות לתת-הפקודות שכותבות: ראו [הפעלה](invocation.md#היעד).
