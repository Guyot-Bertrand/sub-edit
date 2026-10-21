# `split-file`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Bir dosyayı **ikiye** keser: bir baş ve bir kuyruk. Pencerenin `Split Project…` eylemidir — [aynı motor](../subedit-gui/operations.md#split-project),
aynı sayılar, aynı ret — ve **[`append`](append.md) komutunun tam tersidir**: kuyruğu başın sonuna eklemek
başlangıçtaki dosyayı geri verir.

- **`--at N`, kuyruğun ilk altyazısının numarasıdır**; pencerenin tablosunun numaraladığı gibi,
  1'den başlayarak. `--at 3`, 1 ve 2 numaralı altyazıları başa, üçüncüyü ve gerisini
  kuyruğa koyar. **İkinciden sonuncuya kadar** gider: bir kesimin her iki yanında da bir altyazı gerekir.
- **Kuyruk başlangıca geri getirilir**: başta kalan son altyazının bitişi kadar geriye kaydırılır;
  konumları sıralarını ve aralıklarını korur. `split-file` komutunu `append` komutunun tersi yapan budur.
- **Her iki yarı da okunan dosyanın biçimini, kodlamasını, bayt sırası işaretini ve satır sonlarını korur** ve
  geri yazılan her dosyada olduğu gibi yazma sırasında 1'den başlayarak yeniden numaralandırılır.
- **İki çıktı, dolayısıyla kendi hedefi**: `--head` ve `--tail`, **ikisi de zorunlu**. `--output`,
  `--output-dir`, `--in-place` ve `--recursive` `split-file` için yoktur ve bilinmeyen her
  seçenek gibi reddedilir, kod `1`: adlandırılmış iki dosyanın paylaşacağı bir dizin de, üzerine yazılacak bir girdi de yoktur.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>` | evet | **tek** bir yol | — |
| `--at` | evet | bir tam sayı, 2'den dosyanın altyazı sayısına kadar | — |
| `--head`, `--tail` | `--dry-run` dışında **evet, ikisi de** | dosya yolları; dizinleri eksikse oluşturulur | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |
| `--sort` | hayır | bir bayrak: altyazıları **kesmeden önce** başlangıçlarının sırasına koyar; `--at` bu sıraya göre sayar | kapalı |
| `--disjoint` | hayır | bir bayrak: çakışan altyazıları **kesmeden önce** keser; `--at` geriye kalana göre sayar, bkz. [Aralıkları ayrık yapma](#aralıkları-ayrık-yapma) | kapalı |

**`--dry-run` ile hiçbir hedef gerekmez**: hiçbiri verilmeyebilir, ama ikisinden yalnızca biri de verilemez —
`--head` ve `--tail` birlikte gider. Verilenler, `--dry-run` olmadan olduğu gibi değerlendirilir.

**İki hedef ne girdi ne de aynı dosya olabilir**: okunmakta olanın üzerine yazmak
ya da kuyruğu başın üzerine yazmak, hiçbir şey okunmadan önce reddedilir, kod `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

Başta kalan ikinci altyazı 4 sn'de biter: kuyruk 4 sn geriye gider ve 6 sn'de başlayan üçüncü altyazı
2 sn'de başlar.

## Zaman sırasına göre kesme

`--at`, altyazıları **dosyanın verdiği sırayla** numaralar. Sırası bozuk bir dosyada baş, bu yüzden zamanca en erken yarı olmaz.
`--sort`, **kesimi değerlendirmeden önce** altyazıları başlangıçlarının sırasına koyar — [`sort`](sort.md) komutunun yaptığı gibi kararlı olarak —
ve `--at` bundan sonra bu sıraya göre sayar.

Varsayılan olarak asla. Seçenekle, dosya ilk anlatım düzeyinde kaç altyazının yer değiştirdiğini bildirir (`<path>: N subtitles moved`
ya da `<path>: already in order`) ve `--format json` kaydı, **seçenek olmadan bulunmayan** bir `moved` sayısı taşır.

## Aralıkları ayrık yapma

Varsayılan olarak asla. Seçenek, kesimi değerlendirmeden önce, sıralamadan sonra, [`disjoint`](disjoint.md) komutunun yaptığını **varsayılan ayarlarıyla** yapar: `--at`, bunun verdiğine göre sayar. Sıralı bir dosya gerekir: aksi hâlde reddedilir ve önce gelen `--sort` dosyayı önceden sıraya koyar. Dosya bunu ilk anlatım düzeyinde bildirir (`<path>: N subtitles split, M merged`) ve `--format json` kaydı, **seçenek olmadan bulunmayan** `split` ve `merged` sayılarını taşır.

## Kesme reddedildiğinde

**Kesimde çakışan iki yarı kesilmez.** Kuyruğun bir altyazısı, geride kalan son altyazının bitişinden önce
başlıyorsa, kaydırma onu videonun başlangıcından önceye taşırdı — hiçbir biçimin yazmadığı
negatif bir konum. `split-file` **altyazıyı adıyla** reddeder; pencerenin sözleriyle:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

Kod `2`'dir ve **hiçbir şey yazılmaz**. Başka bir yerden kesin. Tam başlangıca düşmeye izin verilir: sıfır bir
konumdur.

**Dosyanın dışındaki `--at` de aynı şekilde reddedilir**, dosya okunduktan sonra: `--at 1` — ilk altyazı
bir kuyruğu başlatamaz, geriye hiçbir şey kalmazdı — ve son altyazıdan sonraki `--at`, ki bu boş bir
kuyruk bırakırdı. Tek altyazılı bir dosya kesilemez.

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan iki dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; `--dry-run` ile `-> …` yerine `(dry run, nothing written)` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<path>: N bytes read, H and T written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile **tek bir nesne** yazılır; `file` kesilen dosyadır.
**`destination` baştır** — girdinin dönüştüğü dosya — ve **`tail` kuyruktur**; bir yol ya da
`destination` gibi `--dry-run` ile `null`. `counts` alanı: `subtitles` (okunan dosyanınki), `head` ve `tail` (her
yarınınki).

## Çıkış kodları

| Kod | Ne zaman |
| :--- | :---- |
| `0` | her iki dosya da yazıldı — ya da `--dry-run` ile hesaplandı |
| `1` | kullanım hatası: `--head` ve `--tail` yok, `split-file` komutunda olmayan bir seçenek, girdi ya da öteki hedef olan bir hedef, pozitif tam sayı olmayan bir `--at` |
| `2` | dosya okunamadı, **kesme reddedildi** (dosyanın dışında ya da çakışma) veya bir hedefe yazılamadı. Kod `3` yoktur: tek bir dosya işlenir |

**Önce baş yazılır**: kuyruğu reddeden bir disk, başı geride bırakır ve kod `2`'dir.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------- | :------ |
| ne `--head` ne de `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| `--dry-run` ile ikisinden yalnızca biri | `--head and --tail go together: give both, or neither with --dry-run` |
| iki hedef aynı dosya | `<path>: named as both the head and the tail` |
| bir hedef girdidir | `<path>: would be written over the input <input>` |
| dosyanın dışında `--at` | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| tek altyazılı bir dosya | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| çakışma | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| eksik, okunamayan ya da bilinmeyen biçimde bir dosya | `<path>: does not exist`… — her yerdeki ile aynı |
