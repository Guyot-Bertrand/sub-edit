# Přidružené video

<!-- machine-translated -->
> Tato stránka je strojovým překladem a dosud ji nezkontroloval žádný čtenář tohoto jazyka.

Nabídka **Video** nese nejprve tři příkazy a pak úkony, které ovládají film
— viz [Přehrávač](player.md#pohyb-filmem).

| Příkaz | Zkratka | Co dělá |
| :------ | :------- | :----------- |
| Vybrat video… | žádná | zvolí soubor videa, ke kterému dokument patří |
| Přepsat zvuk… | žádná | přepíše zvuk filmu na titulky, které nahradí titulky projektu — viz [Přepis zvuku](#přepis-zvuku) |
| Přehrát / pozastavit | `Ctrl+P` | přehraje nebo zastaví video — viz [Přehrávač](player.md#přehrávání-a-zastavení) |

Přidružené video je **vlastností otevřeného dokumentu**, nikoli nastavením
aplikace: otevření jiného souboru titulků začíná od nuly. Nezapisuje
se do žádného souboru a zmizí spolu s oknem.

## Zvuková stopa

Film může nést několik zvukových stop — původní znění, dabing, komentář.
Podnabídka **`Video ▸ Zvuk ▸ Jazyk`** je vypíše, **jednu položku na stopu**, a **označí tu, která hraje**:

| Co soubor uvádí | Co říká položka |
| :------------------ | :------------------ |
| jazyk a název | `1: fra — Original` |
| jen jazyk | `2: eng` |
| jen název | `3: Commentary` |
| ani jedno | `1` |

Číslo je **pořadí stopy ve filmu**, jedna po druhé, v pořadí souboru: dvě stopy
mohou říkat `eng` a soubor, který o svých označeních mlčí, má přesto první a druhou.

**Volba položky přehraje tuto stopu.** Označení je odpovědí přehrávače, nikoli kliknutí: říká, která
stopa hraje.

- **Podnabídka je nedostupná** bez videa nebo bez zvukové stopy. S jedinou stopou zůstane dostupná a
  ukáže ji zaškrtnutou: říká jazyk filmu. Oba úkony hlasitosti, které jsou vedle ní, zůstávají
  dostupné v každém případě: viz [Hlasitost](player.md#hlasitost).
- **Stopa se nepamatuje** od jednoho videa k dalšímu ani od jedné relace k další: číslo stopy patří
  jejímu souboru a jeho přenesení na další film by vybralo cokoli, co tam nese toto číslo.
  Návrat na kartu znovu otevře její film na stopě, kterou si přehrávač zvolí sám.
- Žádná klávesová zkratka: jazyk se mění zřídka a nabídka stačí.

## Volba videa

Dialog se otevře v adresáři souboru titulků a filtruje podle
rozpoznaných přípon videa:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

Nabízí také zobrazení všech souborů. Zrušení dialogu nic
nezmění.

Zvolený soubor se **ihned otevře**: obraz se objeví v okně,
pozastavený na svém prvním okamžiku. Viz [Přehrávač](player.md).

**Přetažení souboru videa do okna udělá totéž** — viz
[Přetažení souborů do okna](files.md#přetažení-souborů-do-okna).

**Video patří projektu aktuální karty.** Každá karta si drží
své vlastní; změna karty změní film. **Návrat na kartu obnoví její
film tam, kde jste ho opustili**, pozastavený, jako každé otevření. Jiný film
zvolený pro tento projekt mezitím naopak začne od začátku.

## Automatický návrh

Když se otevře soubor titulků a **dosud nebylo zvoleno žádné video**,
okno ho hledá ve stejném adresáři: soubor videa, jehož název
bez přípony je předponou našeho, a to až po tečku.

| Soubor titulků | Nalezené video | Proč |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` je předponou `film.fr` |
| `film.en.forced.srt` | `film.mkv` | stejně, bez ohledu na počet segmentů |
| `film.fr.srt` | `film.fr.mkv`, nikoli `film.mkv` | vyhrává nejdelší název |
| `film.fr.srt` | žádné, jsou-li tam `film.mkv` **i** `film.mp4` | dva kandidáty nelze rozlišit |
| `film.fr.srt` | žádné, je-li tam jen `fil.mkv` | `fil` nekončí na tečce |

**Návrh není volba.** Nechá se nahradit jiným
návrhem — otevřením jiného souboru, uložením pod jiným názvem. Výslovná
volba se naopak nikdy nepřepíše: jakmile bylo použito `Select Video…`,
automatický návrh se už za ní nevrátí.

Právě to činí odhad přijatelným: lze ho opravit a jeho
oprava platí.

## Co ukazuje stavový řádek

Vpravo ve stavovém řádku říká jeden řádek, k čemu dokument patří.
Je posledním ze tří: [kódování](files.md#kódování-ve-stavovém-řádku)
přečteného souboru je první, [mřížka snímků](frame-grid.md) odvozená z
pozic následuje.

| Situace | Co se zapíše |
| :-------- | :-------------- |
| video je přidruženo | `Video: film.mkv` |
| … a `ffprobe` uvádí jeho snímkovou frekvenci | `Video: film.mkv, 24000/1001 fps` |
| žádné | `No video` |

**Snímková frekvence jde s filmem, nikoli vedle něj.** Jde o jediný údaj — k čemu
tento dokument patří — a třetí zmínka by ji postavila na stejnou úroveň jako
[mřížku odvozenou z pozic](frame-grid.md), která je údajem jiným.

## Přepis zvuku

**`Transcribe Audio…`** zapíše řeč filmu jako titulky. Je vypnutý, dokud není otevřen film: jde o
zvuk **filmu projektu**, nikdy o soubor, který byste vybírali — k přepisu zvukového
souboru použijte v příkazovém řádku [`transcribe`](../subedit-cli/transcribe.md).

Dialog se ptá na tři věci a říká, zda může začít:

- **zvukovou stopu**, uvedenou tak, jak ji uvádí nabídka [`Audio`](#zvuková-stopa), s jazykem
  a názvem, které soubor uvádí;
- **jazyk**, dvě písmena, například `en` nebo `fr` (`fra` se čte jako `fr`). Začíná jazykem zvolené
  stopy, když ho soubor uvádí, sleduje stopu, dokud jste žádný nezadali, a
  **řádek pod polem říká, odkud se vzal**. Když soubor pro stopu žádný jazyk neuvádí,
  řádek to říká a varuje: model ho pak zjistí z prvních třiceti sekund zvuku, což umí jen vícejazyčný model, a
  **film, který začíná hudbou nebo tichem, může být považován za jiný jazyk a jeho řeč pak přeložena do něj**.
  Zadejte jazyk, pokud ho znáte. U modelu pouze pro angličtinu je pole pevně nastaveno na `en` a nelze ho změnit,
  a řádek říká, že film vyjde anglicky, ať se v něm mluví jakýmkoli jazykem;
- **část filmu**, od jednoho času k druhému — viz [Nejprve zkusit část](#nejprve-zkusit-část). Obě pole
  prázdná znamenají celý film.

**Říká také, co chybí.** Tři řádky říkají, zda nástroj, `ffmpeg` a model jsou k dispozici, a
kde; když něco chybí, `OK` zůstane nedostupné a dialog to oznámí s odkazem na
[Instalace nástrojů pro přepis](../subedit-cli/install-transcription.md). Tlačítko `Preferences…` otevře
[předvolby](preferences.md#přepis), kde se nastavují program a model — jsou to skutečnosti
o počítači, ne volby, které by se měly opakovat při každém přepisu — a dialog se po jejich zavření podívá znovu.

Jazyk se uchová pro příště. Samotný nástroj a meze titulků jsou
[nastavení](preferences.md) (`transcription.*`).

**Když projekt už obsahuje titulky, okno se před jejich nahrazením zeptá** a řekne,
kolik jich zmizí. Ptá se před prací, aby odmítnutí nestálo nic. Když je projekt prázdný,
není se na co ptát.

Práce probíhá **na pozadí**, s oknem a tlačítkem `Cancel`; okno zůstává kolem použitelné. **Okno
říká, v jakém stavu práce je**, abyste věděli, že žije a jak dlouho potrvá:

- **krok**: nejprve “Extrahuje se zvuk…” — okno je pak zaneprázdněné, bez čísla, protože extrakce neříká nic o tom, jak daleko
  došla —, pak “Přepis zvuku…”, s ukazatelem;
- **pozice**: “0:12:30 z 0:50:00”, kam ve filmu nástroj došel, jakmile vytvoří první text;
- **zbývající čas**, “zbývá asi 38 min”: zpočátku “odhaduje se zbývající čas…”, nahradí se, jakmile je hotovo dost práce pro extrapolaci, a počítá se z
  rychlosti poslední minuty; je to přesnost na několik minut, ne příslib;
- **uplynulý čas**, “uplynulo: 0:05:12”, který běží, i když se nic jiného nehýbe;
- **poslední získaný text**, abyste viděli, že práce dává smysl.

U dlouhého filmu s velkým modelem na procesoru mohou první minuty trvat dlouho: nástroj načítá model a pracuje na prvních
oknech po třiceti sekundách. Pozice a poslední text se hýbou dřív než vlastní procento nástroje, které přichází jen v krocích
po 5 % filmu. Když skončí, titulky projektu se nahradí
přepisem **jako jedna operace**: jedno `Undo` vrátí, co tam bylo, včetně textů
překladu. Přepis, který selže nebo který zrušíte, nic nezmění — důvod se
sdělí v okně, kromě zrušení, které ve stavovém řádku jen oznámí, že bylo zrušeno.

**Když je většina toho, co nástroj zapsal, tvořena poznámkami v hranatých závorkách** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —, okno
to po práci oznámí: použitý jazyk pravděpodobně není ten, kterým se mluví, a nástroj mohl řeč přeložit. Spusťte přepis znovu se
zadaným jazykem. Titulky jsou přesto v projektu a jedno `Undo` vrátí, co tam bylo.

### Nejprve zkusit část

Rozsah změní přepis na **zkoušku**, abyste posoudili model nebo jazyk na dvou minutách, než se zavážete na
dvě hodiny. Pole “Část filmu” berou začátek a konec, každý zapsaný jako `2:00`, `1:02:03`, `2:00.5` nebo počet
sekund, `90`; jedno samotné je povoleno (odtud do konce, nebo od začátku potud). `OK` zůstane nedostupné, dokud některé pole
není čas nebo konec nenásleduje po začátku, a řádek pod poli řekne, které.

- **Titulky zachovají čas filmu**: co se říká v 2:10, je ve výsledku na `0:02:10`.
- **Nic se nenahradí a na nic se neptá.** Výsledek se otevře jako **samostatný projekt**, na nové kartě, bez
  souboru; projekt, z něhož vznikl, a jeho titulky zůstanou, jak byly. Uložte jej, nebo zavřete.

**Co je potřeba, se nedodává**: nástroj (whisper.cpp), model a `ffmpeg` k extrakci zvuku.
Viz [Instalace nástrojů pro přepis](../subedit-cli/install-transcription.md).

## Když není film

Okno ukáže tlačítko **`Select Video…`** tam, kde by byl obraz, místo
prázdného místa. Je to tentýž příkaz jako v nabídce `Video`, na dosah
myši: nepřítomnost, se kterou nelze nic udělat, se těžko odlišuje od
poruchy.

Tlačítko zmizí, jakmile se otevře film, a obraz zabere **celé** jeho
místo: výšku, kterou mu tabulka nad sebou nechává — tu, kterou si
[`window.table-share`](preferences.md) pamatuje od jedné relace k další —, a
nikdy méně než minimální výšku obrazu, kterou tabulka v případě potřeby uvolní.
Když film zmizí, tlačítko zaujme totéž místo.

**Název souboru, nikdy jeho cesta.** Cesta o dvou stech znacích
by z lišty vytlačila všechno ostatní; celou cestu ukázal
dialog.

## `ffmpeg` není nutný

`subedit-gui` se ptá programu `ffprobe` — dodávaného s `ffmpeg` — na jedinou
věc: **snímkovou frekvenci, kterou uvádí kontejner**. Čte se jednou
za film, když se přidruží.

| S `ffprobe` | Bez něj |
| :------------- | :------ |
| stavový řádek zapíše snímkovou frekvenci vedle názvu filmu | zapíše jen název |
| dialog převodu navrhne ve spodním poli snímkovou frekvenci filmu | nenavrhne nic a otevře se jako dříve |
| dialog zarovnání se na ní otevře | otevře se na snímkové frekvenci projektu |

**Nic jiného na něm nezávisí a žádná operace neodmítne.** Přehrávač
naopak `ffprobe` vůbec nepotřebuje: opírá se o `libmpv`, který je
závislostí sestavení. Doba trvání filmu, ta, podle které se hlásí, co
přesahuje konec, pochází z přehrávače, a ne z `ffprobe`.

**Přepis je jediná věc, která `ffmpeg` potřebuje**: vyextrahuje z něj zvuk filmu. Viz
[Instalace nástrojů pro přepis](../subedit-cli/install-transcription.md).
Nejde tedy o omezený režim, ale o **běžný provoz** počítače,
na kterém nikdy nebyl nainstalován `ffmpeg`: údaj pochází od uživatele a `ffprobe`
ho navrhne, když je k dispozici.

Přečtená snímková frekvence je **přesné racionální číslo** — `24000/1001`, nikoli `23.976`.
Proto se `ffprobe` pro tuto odpověď zachovává:
přehrávač ví totéž jako číslo s plovoucí desetinnou čárkou a přibližná snímková frekvence
by celý soubor přečasovala mírně vedle.
