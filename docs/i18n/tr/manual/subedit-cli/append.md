# `append`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Dosyaları **ilkinin ardına**, **tek bir** dosyada toplar: iki parçalı bir film, tek bir
altyazı kümesine dönüştürülür. Pencerenin `Append File…` işlemidir — [aynı motor](../subedit-gui/operations.md#append-file),
aynı sayılar — eklenecek dosya sayısı kadar uygulanır.

- **İlk dosya tabandır.** Sonuca **biçimini, kodlamasını, bayt sırası
  işaretini ve satır sonlarını** verir.
- **İki dosya arasına hiçbir şey eklenmez**: her biri, **kendinden öncekinin son altyazısının bitişi** kadar kaydırılır;
  konumları sıralarını ve aralarındaki boşlukları korur. Üçüncü dosya tabanı değil
  ikinciyi izler: dosyalar verildikleri sırayla birbirini izler.
- **Eklenen her dosya dönüştürmeden geçer**: taban biçime çevrilir; etiketleri çevrilir
  ve o biçimin yazamadığı şey, dosya dosya, pencerenin sözcükleriyle bildirilir.
- **Eklenen dosyaların çevirisi yoktur**: tek başına açılan bir dosyanın verecek bir çevirisi yoktur.
- **Bir dosya birkaç kez verilebilir** veya kendisine eklenebilir: bunu hiçbir şey yasaklamaz.

**`append` bir toplu işlem değildir.** N girdi, **tek** çıktı: diğer alt komutların döngüsü — dosya başına bir sonuç
ve diğerlerinin devam etmesine izin veren bir hata — burada anlamsızdır. Bundan üç sonuç çıkar:

- **hedef yalnızca `--output` seçeneğidir, başka hiçbir şey değil.** `--output-dir` ve `--in-place`
  `append` için yoktur ve herhangi bir bilinmeyen seçenek gibi reddedilir, kod `1`; `--recursive` de aynı şekilde: dosyaların
  sırası verilen sıradır ve dolaşılan bir ağacın sırası yoktur;
- **ya hep ya hiç**: okunamayan ilk dosya çalışmayı durdurur ve **hiçbir şey yazılmaz**.
  Bir parçası eksik üç parçalı bir film, iki parçalı bir film değildir;
- sonuç için çalıştırma başına **tek bir satır** ve JSON’da **tek bir nesne**.

<!-- example: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | evet, **en az iki** | yollar: taban, ardından onun arkasına konacaklar, sırayla | — |
| `--output` | **evet**, `--dry-run` hariç | bir dosya yolu; dizini eksikse oluşturulur | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |
| `--sort` | hayır | bir bayrak: bütünü **her şey eklendikten sonra** başlangıçlarının sırasına koyar, bkz. [Sonucu sıralama](#sonucu-sıralama) | kapalı |
| `--disjoint` | hayır | bir bayrak: çakışan altyazıları **her şey eklenip sıralandıktan sonra** keser, bkz. [Aralıkları ayrık yapma](#aralıkları-ayrık-yapma) | kapalı |

**Hedef, girdilerden biri olamaz**, taban dahil: okunmakta olan bir dosyanın üzerine yazmak,
hiçbir şey okunmadan önce reddedilir, kod `1`. Girdilerden farklı, var olan bir `--output`
her yerde olduğu gibi üzerine yazılır.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

Son altyazısı 4 sn işaretinde biten `partie1.srt` dosyasının ardına eklenen `partie2.srt` dosyasında,
0,5 sn işaretinde başlayan altyazı 4,5 sn işaretine kayar.

## Sonucu sıralama

Her dosya **kendinden öncekinin bitişinden** başlayarak kaydırılır: bozulabilecek sıra bu nedenle dosyaların kendi sırasıdır. `--sort`,
sonucu başlangıçların sırasına koyar, **tek seferde, son eklemeden sonra**, tıpkı [`sort`](sort.md) gibi — kararlı biçimde ve içerikte hiçbir şeyi
değiştirmeden.

Varsayılan olarak asla. Seçenekle, taban bunu ilk anlatım düzeyinde bildirir: `<base>: N subtitles moved` veya `<base>: already in order`;
`--format json` kaydı da bir `moved` sayısı taşır, **seçenek olmadan bulunmaz**.

## Aralıkları ayrık yapma

Varsayılan olarak asla. Seçenek, her şey eklendikten sonra, bütün üzerinde [`disjoint`](disjoint.md) komutunun **varsayılan ayarlarıyla** yaptığını yapar. Sıralı bir dosya gerekir: aksi halde reddedilir ve ondan önce gelen `--sort` onu önceden sıraya sokar. Dosya bunu ilk anlatım düzeyinde bildirir (`<path>: N subtitles split, M merged`); `--format json` kaydı da `split` ve `merged` sayılarını taşır, **seçenek olmadan bulunmaz**.

## Biçimler farklı olduğunda

Sonuç, ötekilerin biçimi ne olursa olsun **tabanın biçimindedir**. Geçişin neye mal olduğu,
dosya dosya, pencerenin sözcükleriyle bildirilir — `Append File…` ile ve
[başka bir biçimden yapıştırma](../subedit-gui/clipboard.md#başka-bir-biçimin-etiketleri-çevrilir) ile aynı cümle:

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

Kaybedilen şey yalnızca eklenen **her** dosyanın biçimine bağlıdır: Advanced SSA biçimindeki bir taban
SubRip dosyasını hiçbir şey kaybetmeden alır, tersini alamaz. Sonucun biçimini seçmek için en başa
zaten o biçimde olan bir dosya koyun veya sonradan [`convert`](convert.md) ile dönüştürün.

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | eklenen her dosya için bir satır — `<path>: appended N subtitles`, biçim bir şeye mal olduysa ardından `; <source format> into <base format>: <what is lost>` — sonra `<base>: N subtitles from M files -> <destination>` (`--dry-run` ile `-> <destination>` yerine `(dry run, nothing written)`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — tabanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konur |
| 3 | `<base>: N bytes read, M written` (tüm dosyalar birlikte), ardından **her okuma tanılaması** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile tüm çalıştırma için **tek bir nesne** yazılır;
`file` alanı **tabandır**. `counts` alanı:

| Sayı | Ne söyler |
| :----- | :----------- |
| `files` | çalıştırmanın dosyaları, taban dahil |
| `subtitles` | sonucun altyazıları |
| `appended` | eklenen dosyalardan gelenler |
| `lost_ends`, `lost_header` | `0` veya `1`: eklenen dosyalardan biri bunu kaybetti |
| `joined_lines`, `lost_tags`, `lost_fields` | eklenen dosyaların kaybettiklerinin toplamı |
| `furthest_ms` | bir konumun en büyük kayması, tüm dosyalar birlikte |

`counts` alanının yanında, onların dışında **bir `inputs` dizisi** bulunur: eklenen her dosya için bir öğe, sırayla,
`{"file", "counts"}` — `counts`, `appended` alanını ve ardından aynı kayıpları taşır, **yalnızca o dosya için**. On dosyalık bir çalıştırmada
hangisinin neye mal olduğunu söyleyen odur. Tüm dosyaların okuma uyarıları
`warnings` içinde toplanır.

Bir hata da bir nesnedir, **tek bir** nesne: okunamayan ilk dosyanınki.

## Çıkış kodları

| Kod | Ne zaman |
| :--- | :---- |
| `0` | sonuç yazıldı — veya `--dry-run` ile hesaplandı |
| `1` | kullanım hatası: ikiden az dosya, `--output` yok, `append` komutunda olmayan bir seçenek, girdilerden biri olan bir hedef |
| `2` | **bir dosya okunamadı** veya hedef yazılamadı: hiçbir şey yazılmaz. Kod `3` yoktur: yalnızca tek bir dosya yazan bir çalıştırma ya başarılıdır ya değildir |

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------- | :------ |
| tek bir dosya verildi | `append needs a base file and at least one file to put after it` |
| ne `--output` ne `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` bir girdiyi adlandırıyor | `<destination>: would be written over the input <input>` |
| eksik, okunamayan veya bilinmeyen biçimde bir dosya | `<path>: does not exist`, `<path>: is in no format this tool knows`… — her yerdekiyle aynı |

**Boş bir dosya eklenmez**: hiçbir biçim en az bir altyazı olmadan okumaz ve dosya,
okunamayan her dosya gibi reddedilir — `<path>: holds nothing recognisable as a subtitle` —; bu
çalışmayı durdurur ve hiçbir şey yazmaz.
