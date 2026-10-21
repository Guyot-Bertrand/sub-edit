# `convert`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Her dosyayı istenen biçimde ve düzende yazar. `--in-place` açıkça istemedikçe
**girdiyi asla değiştirmez**.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--to` | **evet** | aşağıdaki tablodan bir ad, başka hiçbir şey değil | — |
| `--line-endings` | hayır | `unix`, `windows` veya `mac` | okunan dosyanınkiler |
| `--to-encoding` | hayır | ICU’nun yazabildiği herhangi bir kodlama, kendi işaretini yazanlar hariç | okunan dosyanınki |
| `--bom` / `--no-bom` | hayır | birbirini dışlayan bayraklar | okunan dosyanın taşıdığı |
| `--sort` | hayır | bir bayrak: altyazıları **yazmadan önce** başlangıçlarının sırasına koyar, bkz. [Yazarken sıralama](#yazarken-sıralama) | kapalı |
| `--join-repeats` | hayır | bir bayrak: aynı metnin art arda gelen parçalarından tek bir altyazı yapar, **her şeyden önce**, bkz. [Aralıkları ayrık yapma](#aralıkları-ayrık-yapma) | kapalı |
| `--repair` | hayır | bir bayrak: `--join-repeats --sort --disjoint` ile aynı, bu sırayla | kapalı |
| `--disjoint` | hayır | bir bayrak: çakışan altyazıları **yazmadan önce** keser, bkz. [Aralıkları ayrık yapma](#aralıkları-ayrık-yapma) | kapalı |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` hariç | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

`mac`, tek başına satır başı karakterini (`\r`), yani klasik Mac OS’un satır sonunu adlandırır.

## `--to` seçeneğinin kabul ettiği biçimler

| Ad | Biçim | Yazılan uzantı |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**Bir ad, uzantı değil.** İki uzantı ikişer biçimi adlandırır
— `.sub` aynı zamanda MicroDVD’nin, `.txt` ise MPL2 ve
TMPlayer’ın uzantısıdır — dolayısıyla uzantı alan bir seçenek hangisinin
istendiğini bilemezdi. Yukarıdaki adlar belirsiz değildir ve uzantının da
belirsiz olmadığı her yerde uzantıyla örtüşür.

**Liste kapalıdır.** Gaupol’un listesidir ve dokuzu da içinde yer alır.

## Bitiş taşımayan iki biçim

**TMPlayer ve LRC satır başına yalnızca tek bir konum yazar**, başlangıcı. Bu,
iki tarafta da görünür.

**Okurken** bitişler çıkarılır: bir satırın bitişi sonraki satırın
başlangıcıdır ve sonuncuya beş saniye verilir. Gaupol’un yaptığı budur.
Bunu ayrıca, `--verbose` seçeneğinin üç alt komutta gösterdiği bir tanılama iletisiyle de
söyleriz:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**Yazarken** bitişler yazılmaz, konulacak yer yoktur.
Bu iki biçimden birine dönüştürmek bitişleri bu nedenle kaybettirir ve gidiş dönüş
onları geri bulamaz — yeniden çıkarır.

**LRC bir şey daha kaybettirir**: satır sonu taşımanın yolu yoktur.
İki satıra yazılmış bir diyalog satırı tek satıra yazılır, iki satır
bir boşlukla birleştirilir. Bu kayıp da onarılamaz.

İkisinde de italik için bir sözcük dağarcığı yoktur.

## `--frame-rate` ve karelerle sayan tek biçim

**MicroDVD saniyeyle saymaz**: konumları kare
numaralarının *kendisidir* ve hiçbir MicroDVD dosyası bunların hangi hızda
sayıldığını bildirmez. `--frame-rate` bunu verir ve iki tarafta da işe yarar: karelerle yazılmış bir `.sub`
dosyasının okunduğu hız ve MicroDVD’ye dönüştürmede kareleri saydığı hız budur.

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | hayır | saniyedeki kare cinsinden bir hız: `25`, `23.976`, `29.97` | aşağıya bakın |

**Okurken, `--frame-rate` olmadan dosya 23,976 değeriyle okunur** — Gaupol’un
değeri — ve `-vvv` düzeyi bunu söyler:
`counts in frames and states no rate; it was read at`. Araçta, gösterilen her konumun
dosyanın doğrulayamayacağı bir varsayıma dayandığı tek yer burasıdır.

**Yazarken, `--frame-rate` olmadan üç durum ve üç yanıt vardır:**

| Başlangıç dosyası | Ne olur |
| :------------------- | :-------------- |
| zaten MicroDVD | onu okuyan hız onu geri yazar ve kareler özdeş döner |
| zamana dayalı ve konumları bir ızgaraya oturuyor | ızgara alınır ve `-vv` hangisi olduğunu söyler |
| zamana dayalı ve ızgarası yok | **ret**, kod 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

Ret kasıtlıdır. Geriye kalan tek hareket bir sayı uydurmak olurdu ve
dosyanın her satırı, sayının ne kadar yanlış olduğu kadar kayardı.

## Bir dönüştürmenin kaybettikleri

**Başka bir biçime dönüştürmek asla kayıpsız değildir** ve nelerin geçemediği
çifte bağlıdır:

| Neler geçemez | Neden |
| :--------------------- | :------- |
| üstbilgi | yalnızca kendi biçiminde anlamlıdır: `[INFORMATION]` bir WebVTT üstbilgisi değildir |
| biçime özgü veriler | SubRip’in koordinatlarının, bir WebVTT ipucunun tanıtıcısının ve ayarlarının başka yerde karşılığı yoktur |
| duyarlık | SubViewer 2, iki Sub Station Alpha ve LRC yüzde birine, MPL2 ondalık birime, TMPlayer saniyeye, MicroDVD kareye kadar yazar; `01:00:00,017` değeri `01:00:00,020` olarak döner |
| bir satırın bitişi | TMPlayer ve LRC’de bunun için alan yoktur; sonraki okuma yeniden çıkarır |
| satır sonları | yalnızca LRC: satırları tek satıra sığar ve ikisi bir boşlukla yeniden birleştirilir |
| hedef biçimin yazamadığı etiketler | WebVTT’de `<font color>` yoktur, TMPlayer ve LRC’de hiçbir şey yoktur |
| yerleşim | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: kendi biçimlerinde gidiş dönüşte korunurlar, dönüştürmede kaybolurlar |

**Bir dosyayı kendi biçiminde geri yazmak hiçbir şey kaybettirmez** — geri kalan her şeyi
ayakta tutan güvence budur ve bir `.srt` dosyasında `--to srt` aynı baytları geri verir. Hiçbir şey
çözülmez bile: metin, okunan ham dizge olarak kalır.

## Etiketler kopyalanmaz, çevrilir

**Her biçim italiği kendi yoluyla yazar** ve çevirmeden dönüştürmek
bir `.srt` dosyasında `{\i1}` bırakırdı: kullanıcının gördüğü metin, bunu
yorumlamayı asla bilmeyecek bir dosyada. Geçenler **altı
şeydir** — kalın, italik, altı çizili, renk, yazı tipi ve boyut:

| Biçim | Kalın | İtalik | Altı çizili | Renk | Yazı tipi | Boyut |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**Bir çizgi, bildirilen bir kayıptır.** Renkli bir `.srt` dosyasını WebVTT’ye dönüştürmek
rengi kaldırır ve bunu söyler; üretilen dosya WebVTT’dir, kılık değiştirmiş SubRip değil.

**MicroDVD ve MPL2 bir satırın yalnızca bir kısmını biçimlendiremez.** Etiketlerinin
sonu yoktur: `{y:i}` satırın sonuna kadar olan her şeyi eğer ve
`{Y:i}` altyazının sonuna kadar. Yalnızca yarısı kalın olan bir satır
bu nedenle taşan bir kalınla değil, hiç kalın olmadan yazılır.

## Dönüştürmenin kaybettiğini bildirdikleri

**Kaybettiğinde söyler, hiçbir şey kaybetmediğinde susar.** Her çağrıda
görünen bir rapor, kimsenin okumadığı bir rapordur.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

Satır varsayılan ayrıntı düzeyinde ve üstündeki her düzeyde görünür.
Öğeler, bu sırayla:

| Öğe | Ne söyler |
| :---- | :----------- |
| `ends are not carried by <format>` | hedef biçim bitiş yazmaz |
| `line breaks were joined in N subtitles` | yalnızca LRC: N altyazı birkaç satırdaydı |
| `N tags dropped` | N etiket yazılamadı, yerleşim dahil |
| `the header was dropped` | üstbilgi bir biçim sınırını geçemez |
| `the <format> fields of N subtitles were dropped` | N altyazı kendi biçimine özgü veri taşıyordu |
| `positions moved by up to N ms` | bir konumun kaydığı en büyük uzaklık |

## Kodlamayı değiştirme

**`--to-encoding` yazılan kodlamayı seçer; `--encoding` okunanı söyler.**
İkisi birlikte bir dosyayı yeniden kodlar:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

`--to-encoding` olmadan dosya, satır sonlarında ve işaretinde olduğu gibi,
**okunduğu kodlamayla** yazılır: kodlama hakkında talimat verilmeden dönüştürülen bir dosya
aynı karakterleri, aynı baytlarla geri verir.

**Hedef kodlamanın yazamadığı bir karakter dosyayı durdurur** ve
hiçbir şey yazılmaz. Latin-1’de bir `ł` için yer yoktur; onu bir `?` ile değiştirmek
okuma ile yazma arasında kimse görmeden metin kaybettirirdi.

**Bayt sırası işareti yalnızca Unicode kodlamaları için vardır.**
Hiçbirini taşımayan bir `--to-encoding` ile `--bom` yok sayılmak yerine reddedilir
— aşağıdaki hata tablosuna bakın.

**Ve kodlamanın bayt sırasını adlandırması gerekir.** `UTF-16` ve `UTF-32`,
ICU’nun bildiği adlardır; dönüştürücüleri, ne istenirse istensin **kendi işaretini**
yazar: `--no-bom` bu durumda kabul edilip ona uyulmazdı. İkisi de
bu nedenle reddedilir ve ileti çıktıyı adlandırır:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` ve `UTF-32BE` tam olarak aynı baytları yazar,
işaret dahil — ama `--bom` ve `--no-bom` denetiminde. Ret
bu nedenle daha kesin bir addan başka bir şeye mal olmaz ve okuma için de yazma için de
geçerlidir: `--encoding UTF-16` aynı yanıtı alır.

## Hedef

Üç seçenek ve kuralları, yazan alt komutların ortak noktasıdır:
bkz. [Çalıştırma](invocation.md#hedef). Bunlardan biri olmadan
hiçbir şey yazılmaz.

`convert` komutuna özgü olan: **`--output-dir` uzantıyı biçimle birlikte
değiştirir.** `a.srt` dosyasında `convert --to vtt`, `a.vtt` yazar; WebVTT içeren bir `a.srt`
asla yazmaz — adı yalan söyleyen bir dosya diğer her aracı yanıltır.

## Yerinde dönüştürme reddedilir

Biçimi değiştiren bir `--to` ile `--in-place` kod `1` verir ve hiçbir şey
yazmaz: yerinde, yeni biçimi taşıyacak ikinci bir ad yoktur ve
dosya, içeriğinin artık haklı çıkarmadığı bir uzantıda kalırdı.

`--in-place` yine de **biçimi değil, düzeni** değiştirmek için kullanılabilir —
örneğin bir `.srt` dosyasını başka satır sonlarıyla `.srt` olarak geri yazmak.

Ret, herhangi bir okumadan önce **yalnızca uzantıya göre** verilir: bir kullanım
hatası asla yarım yazılmış bir toplu işlem bırakmamalıdır.

## Satır sonları ve bayt sırası işareti

Varsayılan olarak, **yazılan dosya okunan dosyanın taşıdığını devralır**. Model
ikisini de okurken saklar; bunları varsayılan olarak dayatmak, her
dönüştürmede bilerek saklanmış bir bilgiyi kaybettirirdi.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Yazarken sıralama

`--sort`, hedef biçim ne olursa olsun dosyayı **başlangıçların sırasıyla** yazar. Sıralama **bir kopya üzerinde** yapılır: okunan dosyaya
dokunulmaz ve yazılan, [`sort`](sort.md) komutunun ondan yapacağı şeydir — kararlı, birlikte başlayan iki altyazı sırasını korur.

**Varsayılan olarak asla**: başka bir şey istenmedikçe bir dosyanın sırası yazarınındır. Seçenekle her dosya bunu, ilk
anlatım düzeyinde, `<path>: N subtitles moved` veya `<path>: already in order` diyerek söyler ve `--format json` kaydı bir
`moved` sayısı taşır — **seçenek olmadan bulunmaz**; böylece mevcut betiklerin okuduğu şeklin biçimi değişmez.

## Aralıkları ayrık yapma

`--disjoint`, çakışan altyazıları artık hiçbir aralık bir diğeriyle çakışmayacak biçimde keser, **sıralamayla aynı kopya üzerinde**: okunan dosyaya dokunulmaz.

Varsayılan olarak asla. Seçenek, yazmadan önce, sıralamadan sonra, [`disjoint`](disjoint.md) komutunun **varsayılan ayarlarıyla** yaptığını yapar. Sıralı bir dosya gerekir: aksi halde reddedilir ve ondan önce gelen `--sort` onu önceden sıraya sokar. Dosya bunu ilk anlatım düzeyinde bildirir (`<path>: N subtitles split, M merged`); `--format json` kaydı da `split` ve `merged` sayılarını taşır, **seçenek olmadan bulunmaz**.

**`--join-repeats` sıralamadan ve kesmeden önce gelir**: art arda gelen parçalara kesilmiş bir metin önce yeniden kurulur (bkz.
[`join-repeats`](join-repeats.md)), sonra altyazılar sıralanır, sonra aralıklar ayrık yapılır. **`--repair` üçü birden demektir**,
[`repair`](repair.md) gibi, kesmenin varsayılan ayarlarıyla. `--join-repeats` ile her dosya ilk anlatım düzeyinde
`<path>: N repeated subtitles joined` der ve `--format json` kaydı `counts.joined` taşır — **seçenek olmadan bulunmaz**.

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır. `--dry-run` ile de boş: dosya yoktur ve anlatım satırı bunu söyler — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

Hata çıktısında, düzeye göre:

| Düzey | Ne eklenir |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — adı geçen kodlama yazılandır |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılaması** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi `counts.subtitles` alanını, ardından dönüştürmenin kaybettiklerini taşır: `lost_ends` ve `lost_header` (`0` veya `1`), `joined_lines`, `lost_tags`, `lost_fields` ve `furthest_ms` — [kayıp cümlesinin](#dönüştürmenin-kaybettiğini-bildirdikleri) öğeleri. Tablonun tamamı [Çalıştırma](invocation.md#her-alt-komutun-counts-içine-koyduğu) sayfasındadır.

## Çıkış kodları

Aracın genel kodları: her dosya yazıldıysa `0`, hiçbiri yazılmadıysa `2`, yalnızca
bazıları yazıldıysa `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| `--no-bom` ile `--bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| biçimi değiştiren `--in-place` | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| hiçbir işaret taşımayan bir kodlamada `--bom` | `<path>: <encoding> has no byte order mark to write` |
| bilinmeyen bir kodlamayı adlandıran `--to-encoding` | `no encoding is named "<name>"` |
| kendi işaretini yazan bir kodlamayı adlandıran `--to-encoding` | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| yazılan kodlamada bulunmayan karakter | `<path>: holds a character the chosen encoding cannot write` |

**Hiçbir işaret taşımayan bir kodlamada `--bom` seçeneğini reddetmek
titizlik değildir.** Bayt sırası işareti Unicode kodlamaları için vardır,
başkaları için yoktur: Windows-1252 kodlamalı bir dosyada işaret istemek,
var olmayan bir şey istemektir. Dosya işaretsiz yazılırdı ve isteğin yerine getirilmediğini
kimse bilmezdi.

Hedefin hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).

Okuma hataları [`inspect`](inspect.md) komutununkilerdir: girdi dosyasıyla
ilgilidir ve iletileri aynıdır.
