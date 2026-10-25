# `transcribe`

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--suppress-notes | --keep-notes]
                       [--model FILE]
                       [--vad-model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>

subedit-cli transcribe --list-languages [--model FILE] [--engine NAME]
```

Skrifar **tal myndskeiðs, eða hljóðskrár, sem skjátextaskrá**. Hljóðið er undirbúið,
afhent vél sem finnur hvað er sagt og hvenær, og bútarnir sem hún sker eru sameinaðir í
skjátexta sem virða tiltekna lengd og breidd.

**Ekkert af því sem hún þarf fylgir forritinu**: vélin (whisper.cpp), líkan og `ffmpeg`
eru sett upp af notandanum, sem segir hvar líkanið er. [Uppsetning
umritunartækjanna](install-transcription.md) segir hvar hvert þeirra er að finna og hvaða líkan á að velja. **Skortur á þeim
bilar ekkert**: skipunin segir hvað vantar, með tilvísun á þá síðu, skrifar ekkert
og lýkur með kóða sem er ekki núll.

- **Inntakið er myndskeið eða hljóðskrá.** Úr myndskeiði dregur `ffmpeg` út eina hljóðrás; án
  `ffmpeg` er hljóðskrá sem vélin les eins og hún er (`wav`, `mp3`, `flac`, `ogg`) samt
  umrituð, en myndskeið ekki, og ekki heldur nein skrá þegar beðið er um hljóðrás.
- **Aðeins ein skrá í einu**, og áfangastaðurinn er `--output`: það er enginn `--output-dir` eða
  `--in-place`, þar sem niðurstaðan er ný skrá en ekki endurskrif.
- **Tungumálið er greint** nema `--language` þvingi það fram, og tungumálið sem var notað er tilkynnt.
  Líkan sem er gert fyrir ensku eingöngu greinir ekkert og umritar ekkert annað: tungumál sem það getur ekki
  tekið við er hafnað *áður* en nokkur vinna hefst, með því að nefna takmörk líkansins.
- **Tungumál er skrifað með tveimur stöfum — `fr`, `en`, `de` —, eða með þriggja stafa kóðanum sem skrá gefur upp, `fra`,
  `eng`, sem er lesinn sem sá tveggja stafa.** Allt annað er hafnað áður en nokkur vinna hefst. **Greiningin
  hlustar aðeins á fyrstu þrjátíu sekúndurnar**: kvikmynd sem byrjar á tónlist eða þögn getur verið tekin fyrir
  annað tungumál, og vélin skrifar þá talmálið *þýtt* yfir á það tungumál — frönsk kvikmynd sem er tekin fyrir
  enska kemur út á ensku, eða sem `[SPEAKING FRENCH]`. Þegar þú þekkir tungumálið skaltu gefa það upp.
- **Hægt er að umrita bara hluta kvikmyndarinnar**, með `--start` og `--end` — sjá
  [Að umrita hluta](#að-umrita-hluta). Þannig er líkan prófað á tveimur mínútum áður en það er keyrt á
  tveimur klukkustundum.
- **Hægt er að telja upp tungumálin sem vél býður**, með `--list-languages` — sjá
  [Að telja upp tungumálin](#að-telja-upp-tungumálin).
- **Hljóð eru sjálfgefið ekki skrifuð sem athugasemdir**: vélinni er sagt að gera það ekki (`--suppress-notes`; hennar eigin valkostur er
  `--suppress-nst`), svo að `(Sonnerie de téléphone)` eða `[MUSIC PLAYING]` koma ekki fram í skjátextunum, hvorki í sviga né í hornklofum.
  `--keep-notes` skrifar þau, fyrir skjátexta fyrir heyrnarskerta. Þegar athugasemdir eru bannaðar skrifar vélin `...` fyrir kafla án tals, og **eftirvinnslan fellir niður bútana sem hafa hvorki staf né tölustaf**
  — `...`, `…`, `♪` — áður en hún sameinar og klippir; `--raw` heldur þeim, eins og bútar vélarinnar komu.
- **Bútarnir eru sameinaðir og skornir** samkvæmt mörkum stillinganna (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): tveir bútar úr einni setningu sem eru
  nálægt hvor öðrum verða einn skjátexti, og bútur sem er of langur eða of breiður er skorinn — fyrst við setningarlok, síðan við kommu, semíkommu eða tvípunkt, og inni í setningu aðeins þegar ekkert greinarmerki er til að klippa við —, og forðast er klippingu sem skilur eftir skjátexta of lítinn til að lesa. Skrefin þrjú gerast í röð, hvert yfir alla umritunina: fella niður, sameina, klippa. Skjátexti sem hefur bút heldur upphafi hans, og sá sem lýkur honum heldur endinum. **`--raw`
  skrifar búta vélarinnar eins og þeir komu**, einn fyrir hvern skjátexta, og það er líka leiðin til að sjá hvað
  sameiningin gerir.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] [input]

Positionals:
  input TEXT                  Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --suppress-notes            Do not write sounds as notes, such as (phone ringing); the transcription.suppress-notes setting by default
  --keep-notes                Write sounds as notes, against the setting
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --list-languages            List the languages the engine offers for the model, and transcribe nothing
  --raw                       Write the segments as the engine cut them
```

## Röksemdir og valkostir

| Valkostur | Nauðsynlegt | Gildi | Sjálfgefið |
| :----- | :----- | :------ | :----- |
| `<input>` | já, nema með `--list-languages` | myndskeið eða hljóðskrá | — |
| `--output` | já, nema með `--list-languages` | skjátextaskráin sem á að skrifa; mappan hennar er búin til ef hana vantar | — |
| `--to` | nei | stutta heiti sniðs: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | sniðið sem skráarending `--output` segir til um, annars `srt` |
| `--audio-track` | nei | heil tala frá 0: staðan meðal hljóðrása myndskeiðsins | `0`, sú fyrsta |
| `--language` | nei | tveir stafir, ISO 639-1 kóði: `en`, `fr`; þriggja stafa kóði eins og `fra` er lesinn sem `fr` | stillingin `transcription.language`; tómt: greint sjálfkrafa |
| `--suppress-notes` | nei | fáni: ekki skrifa hljóð sem athugasemdir; hafnað fyrir vél sem ekki er hægt að segja það | stillingin `transcription.suppress-notes` |
| `--keep-notes` | nei | fáni: skrifa þau, gegn stillingunni; andstæða `--suppress-notes`, og báðum saman er hafnað | stillingin `transcription.suppress-notes` |
| `--list-languages` | nei | fáni: telja upp tungumálin sem vélin býður fyrir líkanið, og umrita ekkert | slökkt |
| `--start` | nei | tími: `2:00`, `1:02:03`, `2:00.5`, eða fjöldi sekúndna, `90` | upphaf hljóðsins |
| `--end` | nei | tími, skrifaður eins og fyrir `--start`; hann verður að koma á eftir `--start` | endir hljóðsins |
| `--model` | nei | líkanaskráin | stillingin `transcription.model` |
| `--vad-model` | nei | raddvirknilíkan (Silero-líkan fyrir whisper.cpp) sem lætur vélina sleppa þögnum áður en hún afkóðar: hlutar hennar fylgja þá talinu og sá fyrsti byrjar þar sem talið byrjar í stað 0. Skráin verður að vera til, og vél sem getur ekki notað hana hafnar valkostinum með nafni | stillingin `transcription.vad-model` |
| `--engine` | nei | heiti vélar: `whisper.cpp` | stillingin `transcription.engine` |
| `--keep-audio` | nei | skrá til að geyma útdregna hljóðið í, sem mónó 16 bita PCM í WAV-skrá, á þeirri sýnatökutíðni sem vélin biður um (16 kHz fyrir whisper.cpp); mappan hennar er búin til ef hana vantar, og nafnið má bera `{rate}`, sem er skipt út fyrir þá tíðni í riðum (`film.{rate}.wav` er skrifað `film.16000.wav`) | hljóðið er hvergi geymt |
| `--raw` | nei | rofi: skrifa búta vélarinnar án sameiningar og án skurðar | slökkt |

**Stillingarnar** eru lesnar úr stillingaskránni eins og stillingar [`adjust`](adjust.md) — sjá
[Stillingaskráin](invocation.md#stillingaskráin) — og hver þeirra á sér hér sinn valkost:

| Lykill | Merking | Sjálfgefið |
| :-- | :------ | :------ |
| `transcription.engine` | vélin | `whisper.cpp` |
| `transcription.executable` | forrit vélarinnar, altæk slóð; tómt: leitað að því í `PATH` | tómt |
| `transcription.model` | líkanaskráin, altæk slóð | tómt |
| `transcription.vad-model` | raddvirknilíkanið, algild slóð; tómt: ekkert, og þögnum er ekki sleppt. Stilling sem vél getur ekki notað er felld niður þegjandi | tómt |
| `transcription.language` | tungumál hljóðsins, tveir stafir; tómt: greint sjálfkrafa | tómt |
| `transcription.suppress-notes` | ekki skrifa hljóð sem athugasemdir | `true` |
| `transcription.merge-gap-ms` | bútar úr einni setningu sem eru nær hver öðrum en þetta margar millisekúndur eru sameinaðir | `400` |
| `transcription.max-duration-ms` | lengsti tími sem skjátexti er á skjánum, í millisekúndum | `7000` |
| `transcription.max-characters` | flestir stafir sem skjátexti rúmar, línuskil ekki talin með | `84` |

## Að velja hljóðið

Myndskeið getur borið margar hljóðrásir — upprunalegu, skýringar, annað tungumál. `--audio-track`
tekur **stöðuna meðal hljóðrása** skrárinnar, frá 0, í þeirri röð sem skráin telur þær upp.
Rás sem myndskeiðið hefur ekki er hafnað, **ásamt þeim rásum sem það hefur**, svo að næsta
kall verði rétt:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Að umrita hluta

`--start` og `--end` takmarka vinnuna við þann hluta kvikmyndarinnar sem er á milli þeirra. Nóg er að gefa upp annað:
`--start` eitt og sér nær til enda, `--end` eitt og sér byrjar í upphafi.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **Tími er skrifaður `M:SS` eða `H:MM:SS`, með aukastöfum á eftir punkti ef vill (`2:00.5`), eða sem fjöldi
  sekúndna (`90`, `90.5`)**. Kommu er hafnað, eins og alls staðar í tólinu.
- **Skjátextarnir halda tíma kvikmyndarinnar**: setningin sem sögð er á 2:10 í kvikmyndinni er skrifuð á `00:02:10`, ekki á
  `00:00:10`. Skrána má leggja óbreytta yfir alla kvikmyndina.
- **Jaðrarnir eru áætlaðir**: vélin vinnur í þrjátíu sekúndna gluggum, svo það sem hún skilar er skorið
  við bilið eftir á; setning sem nær yfir jaðar er skorin þar.
- **Ekkert byrjar ef ekki er hægt að nota bilið**: mörk sem eru ekki tími, eða `--end` sem er ekki á eftir
  `--start`, er notkunarvilla sem nefnir vandann, skrifar ekkert og ræsir hvorki `ffmpeg` né vélina.

Hljóð allrar kvikmyndarinnar er samt dregið út: það sem bilið sparar er umritunin, sem er mun lengri.

## Að telja upp tungumálin

`--list-languages` skrifar tungumálin sem vélin býður **fyrir líkanið**, eitt í hverri línu: kóðann sem `--language` tekur, tab-staf, og heiti
tungumálsins á tungumáli viðmótsins. Hún þarf hvorki inntak né úttak, og umritar ekkert.

```bash
subedit-cli transcribe --list-languages --model ggml-small.bin
```

```text
en	English
zh	Chinese
de	German
…
```

Listinn fer eftir vélinni og líkaninu: um hundrað tungumál fyrir fjöltyngt whisper.cpp-líkan, aðeins `en` fyrir líkan fyrir ensku
eingöngu. Vél sem getur ekki talið upp tungumál sín telur ekkert upp. Líkan sem er ekki gefið upp eða er ekki til staðar er villa, með tilvísun á leiðbeiningarnar; aðeins
textaformið, ekkert er skrifað með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið).

## Úttak

**Hefðbundið úttak** — ekkert: niðurstaðan er skráin sem var skrifuð.

| Stig | Hvað bætist við á villuúttakið |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | skrefið og framvindan, ein lína fyrir hvert prósent: `extracting the audio…`, síðan `transcribing…`, síðan `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — prósentan, staðan sem náðst hefur í hljóðinu, tíminn sem eftir er og tíminn sem liðinn er; **línan er sögð aftur á tíu sekúndna fresti þegar ekkert nýtt hefur borist**, svo að löng bið sé ekki tekin fyrir frost |
| 3 | og hver nýr texti sem vélin fær, `last text: …` |

**Prósentan er staða vélarinnar í hljóðinu, ekki talning á hennar eigin gluggum**: vélin skrifar það sem hún hefur
umritað jafnóðum, og skipunin les hvar hún er stödd, svo talan hreyfist frá fyrstu mínútu. (Framvinda vélarinnar
sjálfrar kemur í skrefum upp á 5 % af hljóðinu, eftir að þrjátíu sekúndna glugga er lokið: ein og sér getur hún
staðið í 0 % í stundarfjórðung á langri kvikmynd með stóru líkani á örgjörva.) **Tíminn sem eftir er þegir í upphafi**: hann er gefinn
þegar nóg hefur verið unnið til að framreikna, og hann fylgir hraða síðustu mínútu, ekki meðaltali allrar
keyrslunnar.

### Á JSON-sniði

Með [`--format json`](invocation.md#úttak-sem-skriftur-geta-lesið) (altæki valkosturinn, skrifaður á undan
undirskipuninni) ber hlutur skrárinnar tungumálið sem var notað í `language`, `counts.subtitles` (skjátextarnir
sem voru skrifaðir) og `counts.segments` (bútar vélarinnar sem þeir voru gerðir úr), og `audio` þegar
`--keep-audio` geymdi hljóðið.

## Tilkynningar

Ein tilkynning er sögð á fyrsta stigi, áður en vinnan hefst, því hún breytir því sem búast má við:

- **Með líkani fyrir ensku eingöngu**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` Líkan
  þar sem skráarnafnið endar á `.en` er slíkt.

## Skilakóðar

Þeir sömu og hjá tólinu: `0` ef skráin var skrifuð, `2` ef það var ekki hægt — tól eða líkan vantar, tungumál
sem líkanið getur ekki tekið við, misheppnuð útdráttur eða umritun —, `1` við notkunarvillu, eins og vél
sem enginn skráði, eða bil sem ekki er hægt að nota.

## Villur

| Orsök | Skilaboð |
| :---- | :------ |
| ekkert vélarforrit fannst, eða það sem var stillt er ekki til | `the transcription engine was not found`, svo tilvísunin í leiðbeiningarnar |
| ekkert líkan gefið, eða það er ekki til | `no model was given, or the model file is not there`, svo tilvísunin í leiðbeiningarnar |
| `ffmpeg` fannst ekki fyrir myndskeið | `ffmpeg was not found, so the audio of a video cannot be extracted`, svo tilvísunin |
| rás sem myndskeiðið hefur ekki | `the video has no audio track N`, svo rásirnar sem það hefur |
| tungumál sem líkanið getur ekki umritað | `the model cannot transcribe the language xx` |
| ekkert tungumál, og líkan sem getur ekki greint eitt | `the model cannot detect the language: give one` |
| vél sem enginn skráði | `no transcription engine is called NAME` |
| `<input>` eða `--output` vantar | `input is required` eða `--output is required` |
| `--suppress-notes` með `--keep-notes` | `--suppress-notes and --keep-notes say opposite things; give one of them` |
| `--suppress-notes` fyrir vél sem ekki er hægt að segja það | `the engine NAME cannot be told not to write sounds as notes` |
| tungumál sem er ekki kóði | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| mörk bilsins sem eru ekki tími | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` ekki á eftir `--start` | `the end of the range must come after its start` |
| önnur bilun | `the transcription failed`, með því sem vélin sagði |

Tilvísunin í leiðbeiningarnar er `See the manual page "Installing the transcription tools"
(install-transcription).`
