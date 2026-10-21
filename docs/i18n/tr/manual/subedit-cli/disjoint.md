# `disjoint`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Çakışan altyazıları keser; böylece **hiçbir `[start, end]` aralığı artık
bir başkasıyla çakışmaz**. Pencerenin `Make Ranges Disjoint…` işlemidir — [aynı motor](../subedit-gui/lines.md#aralıkları-ayrık-yapma).

**Hiçbir şey kısaltılmaz ya da uzatılmaz**: dosya her an öncekiyle aynı metinleri gösterir. Değişen,
zaman çizelgesinin kesilme biçimidir:

- çakışan bir grubun altyazılarının **her başlangıcında ve her bitişinde** kesilir;
- **tek bir** altyazının kapladığı aralık o altyazı olarak kalır;
- **birden çok** altyazının kapladığı aralık, satırları onların satırları olan **tek bir** altyazı olur;
  satırlar **başlangıçlarının sırasına** göre dizilir — en erken başlayan 1. satırda, sonraki 2. satırda;
- hiçbir şeyle çakışmayan altyazıya dokunulmaz ve geri kalanın sırası korunur.

<!-- example: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir ya da daha fazla yol | — |
| `--minimum` | hayır | bir süre: `0.08`, `1.5`, `00:00:00,080`. Sıfır, her aralığı korur | `0.08` (yaklaşık iki kare) |
| `--max-lines` | hayır | bir tam sayı: birleştirilmiş bir altyazının en çok satır sayısı; `0` sınırı kaldırır | `2` |
| `--dashes` | hayır | bir bayrak: her altyazının birleştirilmiş altyazıya getirdiği metnin önüne bir çizgi koyar | kapalı |
| `--sort` | hayır | bir bayrak: altyazıları **önce** başlangıç sırasına koyar — bkz. [Sıralı bir dosya gerekir](#sıralı-bir-dosya-gerekir) | kapalı |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` dışında | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

<!-- example: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` ve `B [3, 8]` üç ayrık altyazı verir: yalnızca `A`, `A` ile `B` birlikte, yalnızca `B`. Hiçbir
an kaybolmaz ya da eklenmez.

## İşlemin neye karar verdiği

**Bir satır asla çoğaltılmaz.** Aynı metne sahip iki altyazı, çakıştıkları yerde tek bir satır verir
ve aynı şeyi gösteren iki komşu parça yalnızca tek bir altyazı olur. Birkaç parçaya kesilmiş bir
altyazının metni ise aksi halde **her parçada yinelenir**: o anda ekranda olan budur.

**Çok kısa bir aralığı bir komşu devralır.** `--minimum` değerinin altındaki bir aralık kaybolur: komşu,
**aralığını onu kaplayacak kadar genişletir**; böylece hiçbir an boş kalmaz ve hiçbir şey bu eşikten
fazla kaymaz. Seçilen komşu, satırları aralığın satırlarını **zaten içeren** komşudur, değilse
en uzun olandır. Aralığın metni yalnızca hiçbir komşu onu taşımıyorsa ve o zaman da eşikten daha kısa bir süre için kaybolur.
**Eşikten daha kısa tam bir altyazı asla devralınmaz**: olduğu gibi bırakılır ve sayılır.

**Sınırdan fazla satır: pencereler.** `--max-lines` değerinden fazla satırı olacak birleştirilmiş bir altyazı,
bu boyutta **art arda gelen pencereler** halinde, sürenin eşit paylarıyla gösterilir. Üç saniye boyunca
üç satır:

<!-- example: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Dört satır üç pencere verir ve böyle devam eder: pencere her seferinde bir satır kayar. Hiçbir şey
kırpılmaz ya da yeniden kesilmez.

**Çeviri aynı kesimleri izler**, çünkü bir altyazı iki metni de taşır. **Biçimin ayrıca taşıdığı şey**
— bir stil, bir konum — kapsayan ilk altyazınınkidir; birleştirilmiş altyazılar birbirinden farklıysa
kayıp sayılır.

**Bitişi başlangıcından önce gelen bir altyazı** tahmin edilmez: olduğu yerde bırakılır ve sayılır.

## Sıralı bir dosya gerekir

Gruplar komşuların dizileridir: sıra bozuksa komşular çakışanlar değildir. Başlangıçları
sıralı olmayan bir dosya bu yüzden **reddedilir** ve ileti bunu söyler:

<!-- example: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` önce sıralamayı yapar — [`sort`](sort.md) ile aynı biçimde — ve bunu söyler. Çekirdek kendiliğinden asla sıralamaz.

İkinci bir geçiş hiçbir şeyi değiştirmez: aralıklar ayrık olduğunda kesilecek bir şey kalmaz ve
cümle `already disjoint` der.

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, ardından bir `;` sonrasında tam yapılamayanlar: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — ya da `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** |

`N subtitles split`, birkaç parçaya kesilen özgün altyazıları sayar; `M merged` ise birkaç altyazıyı
birlikte gösteren parçaları.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın `counts` nesnesi:

| Sayı | Ne söyler |
| :----- | :----------- |
| `subtitles` | yazılan dosyanın altyazıları |
| `split`, `merged` | cümledeki gibi |
| `windowed` | pencereler halinde gösterilen parçalar |
| `absorbed` | bir komşunun devraldığı aralıklar |
| `too_short` | eşikten kısa olup bütün bırakılan altyazılar |
| `ending_before_start` | bitişi başlangıcından önce gelen ve olduğu gibi bırakılan altyazılar |
| `extras_dropped` | stili ya da konumu farklı olan birleştirilmiş parçalar |
| `moved` | yalnızca `--sort` ile: yeri değişenler |

## Çıkış kodları

Aracın kodları: her dosya işlendiyse `0` — çakışması olmayan bir dosya da işlenmiş sayılır —, hiçbiri
işlenmediyse `2`, yalnızca bazıları işlendiyse `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------- | :------ |
| `--sort` olmadan sırasız bir dosya | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` bir süre değil ya da negatif | `--minimum: …` |
| `--max-lines` bir tam sayı değil | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

Okunamayan bir dosya her alt komutta olduğu gibi başarısız olur; hedefin hataları, yazan
alt komutlar için ortaktır: bkz. [Çalıştırma](invocation.md#hedef).

## Diğer komutlarda

[`convert`](convert.md#aralıkları-ayrık-yapma), [`append`](append.md#aralıkları-ayrık-yapma) ve
[`split-file`](split-file.md#aralıkları-ayrık-yapma) komutları, yazmadan önce aynı işi varsayılan
ayarlarla yapan `--disjoint` seçeneğini kabul eder.

İyi bölümlenmemiş bir dosyada — art arda gelen parçalara bölünmüş, sırasız verilmiş bir metin —, yalnızca sıralamak ve kesmek
bir parçadan ötekine geçip duran satırlar verir: önce [`join-repeats`](join-repeats.md) ya da üçünü birden zincirleyen
[`repair`](repair.md) kullanılmalıdır.
