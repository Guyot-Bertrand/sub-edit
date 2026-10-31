# `sync`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**מסנכרנת מחדש קובץ כתוביות לפי השמע של סרט.** המקרה שבשבילו היא קיימת הוא קובץ ה-`.srt` שהורד מהרשת, מוזז
בהתחלה ובסוף, ושהזזה אחת אינה מחזירה למקומו: כל כתובית נחפשת במה שנאמר, ותופסת את המקום שבו היא נאמרת.

היא מאזינה לשמע בעזרת מנוע תמלול — אותו מנוע של [`transcribe`](transcribe.md) — ו**מתאימה את המילים של
כל כתובית למילים ששמע**. שום דבר אינו מתורגם: הכתוביות חייבות להיות בשפה שמדוברת.

**שום דבר ממה שהיא צריכה אינו מצורף לתוכנה**: המנוע, מודל ו-`ffmpeg` הם אלה של
[התקנת כלי התמלול](install-transcription.md), והיעדרם אינו שובר דבר — הפקודה אומרת מה חסר, עם הפניה לדף ההוא,
אינה כותבת דבר ומסתיימת בקוד שאינו אפס.

- **רק ההתחלות והסיומות משתנות.** לעולם לא הטקסט, הסגנון, מספר הכתוביות או סדרן.
- **כתובית שהשמע אינו נותן לה מקום שומרת על הזמנים שלה** — קרדיט שאיש אינו אומר, שיר, שורה בשפה אחרת.
  **שום מיקום אינו ממציאים לעולם, ושום מיקום אינו מנחשים לפי השכנות**: השורה נשארת במקום שבו הקובץ החזיק אותה,
  והפקודה אומרת זאת. זה *אינו* כשל, וקוד היציאה הוא `0`.
- **כתובית נמצאת כשלפחות מחצית מילותיה נשמעו** (`--min-match`, `alignment.min-match`), ולפחות אחת. רישיות, ניקוד,
  פיסוק ותגיות אינם משנים; מילה שאות אחת רחוקה ממילה ארוכה נחשבת כנשמעת.
- **הסיום הוא המקום שבו המילה האחרונה מסתיימת.** המנוע מתזמן מילים בקירוב, כמה מאיות שנייה לפני הסיום של
  הכתובית עצמה, ולכן כתובית עשויה לצאת קצרה מעט מכפי שהייתה.
- **כתובית שהייתה נכנסת בבאה אחריה נחתכת עד המקום שבו הבאה מתחילה.** כתובית שעדיין חופפת את הבאה, מה
  שחיתוך אינו מרפא, נשארת ונאמרת.
- **קובץ שאף אחת מכתוביותיו לא נמצאה נדחה ואינו נכתב**: זה כמעט בוודאי השמע הלא נכון או השפה הלא נכונה.
- **אפשר להאזין לחלק מהסרט בלבד**, עם `--start` ו-`--end`, כמו ב-[`transcribe`](transcribe.md#תמלול-חלק-מהסרט): רק
  הכתוביות ש**מתחילות** בטווח נחפשות ונוגעים בהן, השאר נשארות כמות שהן ואינן נספרות.
- **להאזנה לשמע יש מחיר לכל קובץ שניתן.** כדי לסנכרן מחדש כמה קבצים על אותו סרט, צפו להמתין לכל אחד.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :------- | :----- | :------ |
| `<files>` | כן | קובצי הכתוביות לסנכרון מחדש; עם `-r`, תיקיות | — |
| `--recursive`, `-r` | לא | דגל | כבוי — ראו [עיבוד עץ קבצים](lots.md) |
| `--video` | אחת מבין `--video` ו-`--audio` | הסרט שמאזינים לו | — |
| `--audio` | אחת מבין `--video` ו-`--audio` | קובץ שמע להאזנה, במקום סרט | — |
| `--audio-track` | לא | מספר שלם החל מ-0: המיקום בין רצועות השמע של הסרט | `0`, הראשונה |
| `--language` | לא | שתי אותיות, קוד ISO 639-1: `en`, `fr` | ההגדרה `transcription.language`; ריקה: מזוהה אוטומטית |
| `--start`, `--end` | לא | זמן: `2:00`, `1:02:03`, `2:00.5`, או מספר שניות; `--end` חייב לבוא אחרי `--start` | כל השמע |
| `--model` | לא | קובץ המודל | ההגדרה `transcription.model` |
| `--vad-model` | לא | מודל זיהוי פעילות קולית, שגורם למנוע לדלג על השתיקות; איתו המילים עוקבות אחרי הדיבור מקרוב יותר | ההגדרה `transcription.vad-model` |
| `--engine` | לא | שם המנוע שמאזין: `whisper.cpp` | ההגדרה `transcription.engine` |
| `--keep-audio` | לא | קובץ לשמירת השמע שחולץ; התיקייה שלו נוצרת אם היא חסרה | השמע אינו נשמר בשום מקום |
| `--min-match` | לא | מספר שלם מ-1 עד 100: החלק ממילות הכתובית שחייב להישמע, באחוזים | ההגדרה `alignment.min-match`, `50` |
| `--doubt-gap` | לא | מספר שלם של אלפיות שנייה, 0 או יותר: עד כמה ההזזה של כתובית יכולה להיות שונה מזו של שכנותיה לפני שהיא מסומנת | ההגדרה `alignment.doubt-gap`, `2000` |
| `--report` | לא | קובץ לכתיבת הכתוביות שיש לבדוק, אחת בשורה: מספר הכתובית, טאב, והסיבה; קובץ כתוביות אחד בכל פעם | דבר אינו נכתב |
| `--output` / `--output-dir` / `--in-place` | **אחת משלוש**, למעט עם `--dry-run` | ראו [הפעלה](invocation.md#היעד) | — |
| `--dry-run` | לא | דגל: מחשב ואומר, אינו כותב דבר — ראו [לראות לפני שכותבים](invocation.md#לראות-לפני-שכותבים) | כבוי |

היעד — `--output`, `--output-dir`, `--in-place`, `--dry-run` — ו-`-r` הם אלה של הפקודות האחרות שכותבות קובץ
מחדש: ראו [היעד](invocation.md#היעד). הקובץ שומר על הפורמט שלו, על הקידוד שלו ועל סופי השורות שלו.

**ההגדרות** נקראות מקובץ ההגדרות — ראו [קובץ ההגדרות](invocation.md#קובץ-ההגדרות) — ולכל אחת יש כאן אפשרות משלה.
המנוע, המודל ומודל הפעילות הקולית הם אלה של `transcribe`.

| מפתח | משמעות | ברירת מחדל |
| :-- | :------ | :------ |
| `alignment.engine` | מנוע ההתאמה; יש אחד, `transcript-matching`, שמאזין ומתאים | `transcript-matching` |
| `alignment.min-match` | החלק ממילות הכתובית שחייב להישמע כדי שתימצא, באחוזים | `50` |
| `alignment.doubt-gap` | עד כמה, באלפיות שנייה, ההזזה של כתובית יכולה לסטות מזו של שכנותיה לפני שהיא מסומנת | `2000` |

## מה מסומן

הפקודה אומרת אילו כתוביות לבדוק, ולמה. כתובית מסומנת בגלל **הראשון** מאלה שמתקיים:

| סיבה | משמעות |
| :-- | :------ |
| not found in the audio, left as it was | פחות מ-`--min-match` ממילותיה נשמעו: היא שומרת על הזמנים שלה |
| only partly heard | נמצאה, אבל פחות מ-80 % ממילותיה נשמעו |
| moved differently from its neighbours | נמצאה, אבל ההזזה שלה סוטה מההזזה החציונית של שתי הכתוביות בכל צד ביותר מ-`--doubt-gap` |

כתובית יכולה גם להיות **cut back to the start of the next subtitle**, או **still overlap the next subtitle**.

שני מספרים מספרים את סיפורו של הקובץ כולו. **ההזזה החציונית** היא האמצע של מידת ההזזה של הכתוביות: קובץ
שפשוט איחר נקרא כאן, ובשום מקום אחר. **הסחיפה** היא ההזזה החציונית של עשירית הכתוביות האחרונה פחות זו של
העשירית הראשונה: היא קרובה לאפס עבור היסט קבוע, וגדולה עבור קובץ שאיחר יותר ויותר לאורך הסרט.

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב.

| רמה | מה נאמר בפלט השגיאה |
| :---- | :------------------------------- |
| 1 | `<file>: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s -> <destination>` |
| 2 | השלב וההתקדמות, כמו ב-[`transcribe`](transcribe.md#פלט) |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא), האובייקט של הקובץ נושא `counts`: `subtitles`
(שנחפשו), `placed` (שקיבלו מקום), `moved` (שזמניהן השתנו), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` ו-`drift_ms`; ו-`flagged`, רשימה של `{"subtitle": N, "kind": "…"}` שבה המספר הוא
מקומה של הכתובית בקובץ ו-`kind` הוא אחד מ-`not-found`, `partial`, `out-of-line`, `cut-back` ו-`overlaps`.
המזהים מובטחים; המשפטים אינם.

## דוגמה

```bash
subedit-cli sync --video film.mkv --model small.bin --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

## קודי יציאה

אלה של הכלי: `0` אם הקובץ נכתב, עם כתוביות מסומנות או בלעדיהן, `2` אם לא ניתן היה לכתוב אותו — כלי או מודל חסרים,
חילוץ או האזנה שנכשלו, אף כתובית שלא נמצאה —, `1` בשגיאת שימוש, כמו היעדר גם של `--video` וגם של `--audio` (או שניהם
יחד), אפשרות מחוץ לטווח שלה, או טווח שאי אפשר להשתמש בו.

## שגיאות

| סיבה | הודעה |
| :---- | :------ |
| לא `--video` ולא `--audio`, או שניהם | `sync needs the audio to listen to: give --video or --audio, and not both` |
| לא נמצאה תוכנית מנוע, או שזו שהוגדרה אינה קיימת | `the transcription engine was not found`, ואחריה ההפניה למדריך |
| לא ניתן מודל, או שהוא אינו קיים | `no model was given, or the model file is not there`, ואחריה ההפניה למדריך |
| `ffmpeg` לא נמצא עבור סרטון | `ffmpeg was not found, so the audio of a video cannot be extracted`, ואחריה ההפניה |
| רצועה שאין בסרטון | `the video has no audio track N`, ואחריה הרצועות שיש בו |
| שפה שהמודל אינו יכול לתמלל | `the model cannot transcribe the language xx` |
| מנוע שאיש לא רשם | `no transcription engine is called NAME`, או `there is no alignment engine called NAME` |
| אף כתובית לא נמצאה | `none of the N subtitles was found in the audio: is it the right audio, and the right language?` |
| `--min-match` מחוץ ל-1 עד 100, `--doubt-gap` שלילי | שגיאת השימוש של האפשרות |
| `--report` עם יותר מקובץ כתוביות אחד | `--report names one file: give one subtitle file when you use it` |
