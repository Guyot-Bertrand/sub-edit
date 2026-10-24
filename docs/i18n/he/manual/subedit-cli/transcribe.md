# `transcribe`

<!-- machine-translated -->
> דף זה הוא תרגום מכונה שעדיין לא נבדק על ידי דובר השפה.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

כותבת **את הדיבור של סרטון, או של קובץ שמע, כקובץ כתוביות**. השמע מוכן,
נמסר למנוע שמוצא מה נאמר ומתי, והקטעים שהוא חותך ממוזגים לכתוביות
שמכבדות משך ואורך.

**שום דבר ממה שהיא צריכה אינו מצורף לתוכנה**: המנוע (whisper.cpp), מודל ו-`ffmpeg`
מותקנים על ידי המשתמש, שקובע היכן המודל. [התקנת כלי
התמלול](install-transcription.md)
מספר היכן למצוא כל אחד מהם ואיזה מודל לבחור. **היעדרם אינו שובר
דבר**: הפקודה אומרת מה חסר, עם הפניה לדף ההוא, אינה כותבת דבר
ומסתיימת בקוד שאינו אפס.

- **הקלט הוא סרטון או קובץ שמע.** מסרטון, `ffmpeg` מחלץ רצועת שמע אחת; בלי
  `ffmpeg`, קובץ שמע שהמנוע קורא כמות שהוא (`wav`, `mp3`, `flac`, `ogg`) עדיין
  מתומלל, אבל סרטון לא, וכך גם כל קובץ כשמבקשים רצועה.
- **קובץ אחד בכל פעם**, והיעד הוא `--output`: אין `--output-dir` או
  `--in-place`, משום שהתוצאה היא קובץ חדש ולא כתיבה מחדש.
- **השפה מזוהה** אלא אם `--language` כופה אותה, והשפה שנעשה בה שימוש מדווחת. מודל
  שנועד לאנגלית בלבד אינו מזהה דבר ואינו מתמלל דבר אחר: שפה שהוא אינו יכול
  לקבל נדחית *לפני* שכל עבודה מתחילה, תוך ציון מגבלת המודל.
- **הקטעים ממוזגים וחתוכים** לפי המגבלות של ההגדרות (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): שני חלקים של אותו משפט שקרובים
  זה לזה הם כתובית אחת, וחלק ארוך מדי או רחב מדי נחתך במילה. **`--raw`
  כותבת את קטעי המנוע כפי שהגיעו**, אחד לכל כתובית, וזו גם הדרך לראות מה
  המיזוג עושה.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --language CODE             Language of the speech: fr, en; detected by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## ארגומנטים ואפשרויות

| אפשרות | חובה | ערכים | ברירת מחדל |
| :----- | :----- | :------ | :----- |
| `<input>` | כן | סרטון או קובץ שמע | — |
| `--output` | כן | קובץ הכתוביות לכתיבה; התיקייה שלו נוצרת אם היא חסרה | — |
| `--to` | לא | השם הקצר של פורמט: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | הפורמט שהסיומת של `--output` מורה עליו, אחרת `srt` |
| `--audio-track` | לא | מספר שלם החל מ-0: המיקום בין רצועות השמע של הסרטון | `0`, הראשונה |
| `--language` | לא | שתי אותיות קטנות, קוד ISO 639-1: `en`, `fr` | ההגדרה `transcription.language`; ריקה: מזוהה אוטומטית |
| `--model` | לא | קובץ המודל | ההגדרה `transcription.model` |
| `--engine` | לא | שם של מנוע: `whisper.cpp` | ההגדרה `transcription.engine` |
| `--keep-audio` | לא | קובץ לשמירת השמע שחולץ, כ-PCM מונו 16 סיביות ב-16 קילו־הרץ בקובץ WAV; התיקייה שלו נוצרת אם היא חסרה | השמע אינו נשמר בשום מקום |
| `--raw` | לא | דגל: כתיבת קטעי המנוע בלי מיזוג ובלי חיתוך | כבוי |

**ההגדרות** נקראות מקובץ ההגדרות כמו אלה של [`adjust`](adjust.md) — ראו
[קובץ ההגדרות](invocation.md#קובץ-ההגדרות) — ולכל אחת יש כאן אפשרות משלה:

| מפתח | משמעות | ברירת מחדל |
| :-- | :------ | :------ |
| `transcription.engine` | המנוע | `whisper.cpp` |
| `transcription.executable` | תוכנית המנוע, נתיב מוחלט; ריק: נחפשת ב-`PATH` | ריק |
| `transcription.model` | קובץ המודל, נתיב מוחלט | ריק |
| `transcription.language` | שפת השמע, שתי אותיות; ריק: מזוהה אוטומטית | ריק |
| `transcription.merge-gap-ms` | חלקים של אותו משפט שקרובים זה לזה מכמות זו של אלפיות שנייה מחוברים | `400` |
| `transcription.max-duration-ms` | משך ההצגה המרבי של כתובית על המסך, באלפיות שנייה | `7000` |
| `transcription.max-characters` | מספר התווים המרבי בכתובית, בלי לספור מעברי שורה | `84` |

## בחירת השמע

סרטון יכול לשאת כמה רצועות שמע — המקורית, פרשנות, שפה אחרת. `--audio-track`
מקבלת את **המיקום בין רצועות השמע** של הקובץ, החל מ-0, לפי הסדר שבו הקובץ מונה אותן.
רצועה שאין בסרטון נדחית, **יחד עם הרצועות שיש בו**, כך שהקריאה הבאה
תהיה נכונה:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## פלט

**הפלט הסטנדרטי** — דבר: התוצאה היא הקובץ שנכתב.

| רמה | מה נוסף בפלט השגיאה |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | התקדמות המנוע, `transcribing: 25%`, כפי שהוא מדווח עליה — בצעדים של כשלושים שניות שמע |

### ב-JSON

עם [`--format json`](invocation.md#פלט-שסקריפט-יכול-לקרוא) (האפשרות הכללית, הנכתבת לפני
תת־הפקודה), האובייקט של הקובץ נושא את השפה שנעשה בה שימוש ב-`language`, את `counts.subtitles` (הכתוביות
שנכתבו) ואת `counts.segments` (קטעי המנוע שמהם נוצרו), ואת `audio` כאשר
`--keep-audio` שמרה את השמע.

## קודי יציאה

אלה של הכלי: `0` אם הקובץ נכתב, `2` אם לא ניתן היה לכתוב אותו — כלי או מודל חסרים, שפה
שהמודל אינו יכול לקבל, חילוץ או תמלול שנכשלו —, `1` בשגיאת שימוש, כמו מנוע
שאיש לא רשם.

## שגיאות

| סיבה | הודעה |
| :---- | :------ |
| לא נמצאה תוכנית מנוע, או שזו שהוגדרה אינה קיימת | `the transcription engine was not found`, ואחריה ההפניה למדריך |
| לא ניתן מודל, או שהוא אינו קיים | `no model was given, or the model file is not there`, ואחריה ההפניה למדריך |
| `ffmpeg` לא נמצא עבור סרטון | `ffmpeg was not found, so the audio of a video cannot be extracted`, ואחריה ההפניה |
| רצועה שאין בסרטון | `the video has no audio track N`, ואחריה הרצועות שיש בו |
| שפה שהמודל אינו יכול לתמלל | `the model cannot transcribe the language xx` |
| אין שפה, ומודל שאינו יכול לזהות אחת | `the model cannot detect the language: give one` |
| מנוע שאיש לא רשם | `no transcription engine is called NAME` |
| כל כשל אחר | `the transcription failed`, עם מה שהמנוע אמר |

ההפניה למדריך היא `See the manual page "Installing the transcription tools"
(install-transcription).`
