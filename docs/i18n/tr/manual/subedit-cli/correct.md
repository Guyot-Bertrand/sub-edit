# `correct`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles | --remove-blank-subtitles]
                    [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Metinleri **Gaupol’un desenleriyle** düzeltir; pencerenin
[`Correct Texts…`](../subedit-gui/correct-texts.md) sihirbazı gibi — aynı desenler,
aynı motor, aynı sayılar. Burada altı görev vardır: **işitme engelli
notları**, **sözcükleri birleştirme** ve **bölme**, **yaygın hatalar**,
**büyük harf kullanımı** ve **satır bölme**.

<!-- example: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
  --remove-blank-subtitles    Remove the subtitles the correction empties, whatever the settings file says
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Neyin seçildiği ve neyin seçilmediği

**Önceden hiçbir şey işaretlenmez.** Pencere kutularını kullanıcının bıraktığı gibi açar; bir
komut satırı görevlerini, kodunu, satır uzunluğunu ve dilini kendi satırından alır,
asla ayar dosyasından almaz: pencerede bir gün işaretlenen görevler, orada açılıp kapatılan desen
kutuları (`correction.activations`) ve yazım denetleyicisinin değiştirme listesi
okunmaz ve o tarafa hiçbir şey yazılmaz.

**Dört ayar varsayılan olarak ayar dosyasından gelir** — `Correct Texts…` işleminin
hatırladığı veya [`config set`](config.md) komutunun yazdığı değerler. **Satırdaki bir seçenek her zaman kazanır**, sonra
dosya, sonra yerleşik varsayılan; dosyanın nerede olduğu, `--config` ve `--no-config` için bkz.
[Ayar dosyası](invocation.md#ayar-dosyası).

| Anahtar | Ne sağlar | Onu geçersiz kılan seçenek |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | uygulanan yaygın hata sınıfları | `--classes` |
| `correction.remove-blank` | düzeltmenin boşalttığı altyazıların kaldırılıp kaldırılmayacağı | `--keep-blank-subtitles`, `--remove-blank-subtitles` |
| `correction.line-break.max-lines` | `line-break` görevinin en çok satır sayısı | `--max-lines` |

- **Dördünün her birinin onu geri alan bir seçeneği vardır**, dosya ne derse desin: `--classes human,ocr`
  iki sınıfı da uygular, `--remove-blank-subtitles` boşalan altyazıları kaldırır, `--max-lines 3`
  Gaupol’un satırlarını geri verir. `--keep-blank-subtitles` ve `--remove-blank-subtitles` birlikte
  kullanım hatasıdır.
- **Dosyada iki sınıfın da kapalı olması yok sayılır ve bildirilir**, düzey 1’de —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` bunu yazamazdı ve
  common-errors görevine yapacak bir şey bırakmazdı. Bunun yerine iki sınıf da uygulanır.
- **1 ile 1000 arasında olmayan bir satır sayısı dosyadan hiç okunmaz**: dosyanın okuyucusu
  onu reddeder — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, düzey
  1’de — ve yerleşik 3 değeri kullanılır.

Bu sayfadaki örnekler `--no-config` ile çalıştırılır; böylece onları çalıştıran makinenin
ayarları ne olursa olsun yerleşik varsayılanların ne yaptığını söylerler.

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--tasks` | **evet** | virgülle ayrılmış bir liste: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | görev yok |
| `--code` | bir desen görevi varsa **evet**, aksi halde **uygulanmaz** | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | `join-words` veya `split-words` varsa **evet**, aksi halde **uygulanmaz** | bir dil kodu: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | hayır | `human`, `ocr` veya `human,ocr` | ikisi de — veya ayar dosyasının dediği |
| `--enable`, `--disable` | hayır, tekrarlanabilir | bir desenin İngilizce adı veya `type:name` | gönderilen `.conf` dosyaları |
| `--max-length` | `line-break` ile **evet** | sıfırdan büyük **karakter** sayısı | **yok** — aşağıya bakın |
| `--max-lines` | hayır | sıfırdan büyük, tam sayıda satır | 3 — veya ayar dosyasının dediği |
| `--skip-length`, `--skip-lines` | hayır | bir sayı veya `off` | `--max-length` ve `--max-lines` ile aynı sınırlar |
| `--keep-blank-subtitles` | hayır | bir bayrak: boşalan altyazılar boş olarak kalır | boşalan altyazılar kaldırılır — ayar dosyası aksini söylemedikçe |
| `--remove-blank-subtitles` | hayır | bir bayrak: ayar dosyası ne derse desin boşalan altyazılar kaldırılır; `--keep-blank-subtitles` ile reddedilir | kapalı |
| `--range` | hayır | `N-M` veya sona kadar `N-` — bkz. [`adjust`](adjust.md#--range) | dosyanın tamamı |
| `--document` | hayır | `main` veya `translation` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `main` |
| `-t`, `--translation-file` | `--document translation` ile | bir çeviri dosyası, **tek bir** girdi için | — |
| `--align-method` | hayır, ve yalnızca `-t` ile | `position` veya `number` | `position` |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` hariç | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

### Görevler

**Listenin sırası fark yaratmaz**: görevler Gaupol’un sırasıyla uygulanır —
önce notlar, sonra sözcükleri birleştirme ve bölme, sonra yaygın hatalar, sonra büyük harf kullanımı, sonra satır bölme —, her biri
bir öncekinin bıraktığı metin üzerinde. Notların boşalttığı bir altyazı sonrakilerde
rol oynamaz.

| Görev | Ne yapar |
| :---- | :-------------- |
| `mentions` | köşeli parantez içindeki sesleri ve `Sound in parentheses` açıksa parantez içindekileri kaldırır ([`hearing-impaired`](hearing-impaired.md) komutunun taraması), ardından şarkı sözü ve konuşmacı desenlerini |
| `join-words` | ikiye bölünmüş bir sözcüğü `--language` sözlüğünden yararlanarak yeniden birleştirir |
| `split-words` | birbirine yapışmış iki sözcüğü aynı sözlükten yararlanarak ayırır |
| `common-errors` | yaygın hataları düzeltir: boşluklar, noktalama, ligatürler, OCR hataları |
| `capitalization` | büyük harfi dilin istediği yere koyar |
| `line-break` | bir altyazının satırlarını hiçbiri `--max-length` değerini aşmayacak biçimde yeniden keser; kodun satır bölme desenlerini tartar |

### Desen kodu

**`--code` basamaklamayı adlandırır**: herhangi bir betik için `Zyyy`, sonra betik, dil
ve ülke — `Latn-en-US`, `Zyyy`, `Latn`, `Latn-en` ve
`Latn-en-US` desenlerini okur. **Zorunludur**, çünkü bir komut satırının çevresel bir dili yoktur:
evrensel bir varsayılan, söylemeden, sanılandan *daha azını* uygulardı. Aynı çalıştırmanın her görevi için
geçerlidir; kendi koduna ihtiyaç duyan bir görev ikinci bir çalıştırmadır. Kötü yazılmış bir kod
kullanım hatasıdır:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

Kullanıcının `$XDG_DATA_HOME/subedit/patterns` dizinine (veya
`~/.local/share/subedit/patterns` dizinine) koyduğu desenler, pencerede olduğu gibi **gönderilen desenlere eklenir**
ve okunamayan gönderilmiş bir desen düzey 1’de bildirilir: `patterns: <file>, line N (<reason>)`.

### Sözcükleri birleştirme ve bölme, ve sözlük

**Bu iki görev hiçbir desen okumaz**: **sistemin sözlüğünü Enchant üzerinden**
sorgularlar — paketlerin, `subedit-cli` ile pencerenin paylaştığı bir bağımlılığı (Debian ve Ubuntu’da `libenchant-2-2`, Fedora’da `enchant2`).
Bu yüzden ayarlanacak bir kodları da adları da yoktur:
`--code` bir desen görevi olmadan anlamsızdır, `--language` ise bu ikisinden biri olmadan
— biri diğeri olmadan verilirse kullanım hatasıdır, kod `1`.

- **`join-words`**, sözlüğün tanımadığı bir sözcüğü kendinden önceki veya sonraki sözcükle birleştirir,
  **yalnızca tek bir yön bilinen bir sözcük verdiğinde**: `hel lo`, `hello` olur; iki olası
  yön ya da hiçbiri hiçbir şeye karar vermez.
- **`split-words`**, sözlüğün tanımadığı bir sözcüğü, **önerilerinden tam olarak biri o sözcüğün
  bir boşlukla yazılmışı olduğunda** ayırır: `hellothere`, `hello there` olur. Baş harfi büyük, ardından küçük harflerle devam eden bir
  sözcüğe dokunulmaz: çoğu zaman bir addır.

**Dil için sözlük yoksa çalıştırma reddedilir**, kod `1`, **hiçbir dosya okunmadan önce**
ve pencerenin sözcükleriyle; pencere işlevi grileştirir ve bunu söyler — burada komut satırı
sessiz kalmak yerine reddeder ve bunu söyler:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**Pencerenin değiştirme listesi ne okunur ne yazılır.** Yazım denetiminde kullanıcının
seçimleriyle *dolar*; bir komut satırında böyle bir seçim yoktur ve listeyi okumak, bir betiği
yazarının bir zamanlar içine koyduğuna bağımlı kılardı.

### Satır bölme

**Karakterle ölçülür ve pencereden fark budur.** Pencere *em* cinsinden ölçer —
yazı tipine göre ayarlanmış, harf başına 0,55 em genişlik —, ve varsayılan 24 değeri em
cinsinden bir değerdir. Bir komut satırının yazı tipi yoktur: metnin karakterlerini
sayar. **24 değerini karakter olarak kopyalamak hemen her şeyi söylemeden
keserdi**; bu yüzden **varsayılan uzunluk yoktur** ve `line-break` ile
`--max-length` zorunludur:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` 3’tür**, birimin değiştirmediği Gaupol değeri; üç satır yetmediğinde satır bölme
bir tane daha alır. **Atlama** — zaten sınırların içinde olan bir altyazıya
olduğu gibi dokunulmaması — varsayılan olarak **aynı sınırları** alır (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), Gaupol’un yaptığı gibi; `--skip-length off` ve `--skip-lines off`
bunu kapatır ve metin o zaman tek parça olarak yeniden okunur ve yeniden kesilir:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

Bölme desenleri kodun desenleridir: `Latn` bir sayı ile birimi arasında ve bir diyalog çizgisinden
sonra bölmez, `Latn-en` bir tanımlıktan, bir edattan, bir unvandan veya bir iyelik belirleyicisinden sonra bölmez. **`Zyyy` altında hiçbir bölme deseni yoktur**: satırlar
yalnızca dengelenir. **Bu görev etkin bir desen olmadığı için asla reddedilmez**: desenleri
nerede bölüneceğini tartar, bölme onlarsız da gerçekleşir. Dört seçenek yalnızca
`line-break` ile anlamlıdır; onsuz verilirlerse kullanım hatasıdır.

### Sınıflar ve ada göre açıp kapatma

**`--classes` yaygın hataları kalıcı olarak süzer**: `ocr` işaretini kaldırmak desenlerini
uygulamadan çıkarır — `ﬁ` ligatürü o zaman `ﬁ` olarak kalır. Diğer üç desen türü
sınıf taşımaz.

**`--enable` ve `--disable` bir deseni İngilizce adıyla ayarlar**, anahtar ad olduğundan
(ADR 0037); bir ad birkaç kaydı gösterebilir ve bunlar tek bir
kutuyu paylaşır. Gereklidir ve pencerenin varsayılanlarını bir kenara bırakan şey budur: **gönderilen
`.conf` dosyaları not desenlerini kapatır**, bu yüzden tek başına `--tasks mentions`
hiçbir şey uygulamazdı. “Sound in brackets” ve “Sound in parentheses” desenlerinin
her biri kendi sınırlayıcısına — köşeli parantezlere, parantezlere — komuta eder ve aynı
seçeneklerle ayarlanır: yalnızca birini açmak ötekini kaldırmaz.

Üç şey **kullanım hatasıdır**, kod `1`, hiçbir dosya okunmadan önce bildirilir:

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| verilen kod altında, verilen görevlerin hiçbir desenini göstermeyen bir ad | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| iki türün desenlerini gösteren bir ad — o zaman `type:name` yazılır, tür `common-error`, `capitalization`, `hearing-impaired` veya `line-break` olur | `--enable: "<name>" names patterns of several types: write type:name, …` |
| hem `--enable` hem `--disable` seçeneğine verilen bir ad | `"<name>" is given to --enable and to --disable: choose one` |

**Bir yazım hatası yüksek sesle bildirilir**: pencere eskimiş bir ayarı yok sayar, çünkü bir
ayar, ortadan kalkan bir desenden daha uzun yaşayabilir; bir çağrının hayatta kalanı yoktur.

**Etkin deseni olmayan bir görev reddedilir**, kod `1` — “yapacak bir şey yok”
söylenen bir yanıttır, sessiz bir başarı değil:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

Aynı dosya, köşeli parantez taraması açıkken:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## Ne yazılır, ne söylenir

Okunan dosyanın biçimi, kodlaması ve satır sonları **korunur**. **Düzeltmenin
boşalttığı altyazılar kaldırılır**, pencerenin varsayılan kutusu gibi;
`--keep-blank-subtitles` onları boş bırakır ve `--remove-blank-subtitles` ayar dosyası onları tutsa bile
kaldırır. **Boşalmış bir çeviri boşalmış kalır**
ve altyazısını korur.

**Sayı pencerenindir** — “Edited N and removed M subtitles”: değişen
metinler ve kaldırılan altyazılar, **eşleşmeler asla**. Hiçbir şeyin
değişmediği bir dosya yine de yazılır: verilen bir hedef yazılan bir hedeftir.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### Uygulanamayan bir desen

**Okunamayan, çevrilemeyen veya bitmeyen bir desen, vazgeçtiği
altyazıyla birlikte adlandırılır** ve komut satırının pencereden daha çok şey söylediği yer burasıdır:
onay sayfası deseni yalnızca bir kez, metin olmadan adlandırır.

| Neden | Ne anlama gelir |
| :----- | :------------------- |
| `cannot be translated` | ifadesi, ICU’ya çevirinin reddettiği bir şeyi kullanıyor |
| `will not compile` | motor ifadesini reddetti |
| `has an invalid replacement` | değiştirmesi Python’ın okuduğu gibi okunmuyor |
| `timed out` | motor o metinden vazgeçti |
| `never settled` | `Repeat` yüz geçişten sonra da metni değiştirmeye devam ediyordu |
| `grew the text too long` | metin bir altyazının alabileceğinden uzun oldu |

Düzey 1’de, hata çıktısında:
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — desen her metin için bir kenara bırakıldığında sonu olmadan. **Numara
dosyanınkidir**, önce hangi görevler altyazı kaldırmış olursa olsun. JSON’da bu,
dosyanın bir `pattern-failed` uyarısıdır ve `detail` alanı cümleyi taşır. **Vazgeçilen bir desen
başarısız bir dosya değildir**: dosya yazılır ve çıkış kodu `0` olur.

## Çıktı

**Standart çıktı** — boş, sonuç yazılan dosyadır. **`--dry-run` ile önerilen
değişikliklerdir**, altyazı başına bir blok: numarası, önceki metin, sonraki metin —
ve düzeltmenin kaldıracağı bir altyazı için `+` satırı olmadan `(removed)`;
`--keep-blank-subtitles` ile altyazı boş olarak kalır. Bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, uygulanamayan desenler, okunamayan desen dosyaları — ve en başta, okunan ayar dosyası — `settings: read <path>` — ve ondan yok sayılanlar |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, ardından her okuma tanılaması |

JSON’da `counts`, `corrected` (değişen metinler) ve `removed` (kaldırılan
altyazılar) alanlarını, `changes` ise dokunulan altyazıları taşır. `-t` ile `alignment` anahtarı çevirinin
nasıl yerleştirildiğini söyler.
`settings`, okunan ayar dosyasını adlandırır; hiçbiri okunmadıysa `null` olur.

## Çıkış kodları

Aracın genel kodları: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`, yalnızca
bazıları yazıldıysa `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | Hata çıktısına ne yazılır |
| :------------------ | :--------------------------------------- |
| `--config` ile adlandırılan var olmayan bir dosya — kullanıcının kendi dosyası eksikse varsayılanlar kullanılır | `settings file not found: <FILE>` |
| `--tasks` eksik | `--tasks is required`, ardından `--help` işaretçisi |
| bilinmeyen bir görev | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` eksik | `--code is required by the tasks that read patterns: <tasks>` |
| desen görevi olmadan `--code` | `--code is for the tasks that read patterns, and none was asked for` |
| `join-words` veya `split-words` ile `--language` eksik | `--language is required by the tasks that check words: <tasks>` |
| dil kodu olmayan bir `--language` | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| o iki görevden biri olmadan `--language` | `--language is for the tasks join-words and split-words, which were not asked for` |
| sözlüğü olmayan bir dil | `no dictionary for <language>` |
| kötü yazılmış bir kod | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `line-break` ile `--max-length` eksik | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| sıfırdan büyük bir sayı olmayan `--max-length` | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| tam sayıda satır olmayan `--max-lines`, `--skip-lines` | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| okunamayan `--skip-length` veya `--skip-lines` | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| dört seçenekten biri `line-break` olmadan | `--max-length is for the task line-break, which was not asked for` |
| bilinmeyen bir sınıf | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| `--remove-blank-subtitles` ile `--keep-blank-subtitles` | `--keep-blank-subtitles and --remove-blank-subtitles say opposite things; give one of them` |
| hatalı bir desen adı | yukarıdaki tabloya bakın |
| etkin deseni olmayan bir görev | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, hedef | [Çalıştırma](invocation.md#hatalar) ve [Bir çeviri](invocation.md#bir-çeviri) sayfalarındakiler |
