# `join-repeats`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Birbiri ardına aynı şeyi gösteren altyazılardan **tek bir altyazı** yapar: bazı bölümleme araçlarının bıraktığı gibi,
art arda gelen parçalara bölünmüş bir metin. Pencerenin `Join Repeated Subtitles` işlemidir —
[aynı motor](../subedit-gui/lines.md#yinelenen-altyazıları-birleştirme).

**Yineleme**, başka bir altyazıyla **aynı metne, aynı çeviriye ve aynı biçim ayarlarına** (bir
stil, bir konum, ipucu ayarları) sahip olan **ve en geç o altyazı biterken başlayan** altyazıdır: birbirine
dokunur ya da çakışırlar. Bir yineleme dizisi **ilk başlayan altyazı** olur; bitişi dizinin en geç
bitişine taşınır, diğerleri kaybolur.

- **Dosyanın sırası fark yaratmaz ve değiştirilmez**: tutulan altyazı, diğerleri nerede olursa olsun, ilk başlayandır.
  Sıralı bir dosya sıralı kalır.
- **Hiçbir şey yanlışlıkla birleştirilmez**: başka bir metin, başka bir çeviri, başka ayarlar, iki
  parça arasında bir boşluk, metni olmayan ya da bitişi başlangıcından önce gelen bir altyazı — bütün bu durumlarda ayrı kalırlar.
- **Gerisinden önce yapılmalıdır**: sıralama ve [çakışmaları kesme](disjoint.md), temiz bir diziyi daha iyi okur.
  [`repair`](repair.md) üçünü birden zincirler, [`convert`](convert.md#aralıkları-ayrık-yapma) ise bunu bir seçenek olarak kabul eder (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir ya da daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` dışında | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

1 sn ile 6 sn arasında art arda gelen üç `Alpha` parçası artık tek bir altyazı oluşturuyor. Bir boşlukla ayrılan `Beta`
olduğu gibi kaldı. İkinci bir geçiş hiçbir şeyi değiştirmez ve bunu söyler: `no repeated subtitles`.

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>` ya da `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın `counts` nesnesi: `subtitles` (yazılan
dosyanın altyazıları), `joined` (başka bir altyazının kapsadığı için silinen altyazılar) ve `chains` (birleştirilen
diziler).

## Çıkış kodları

Aracın kodları: her dosya işlendiyse `0` — yinelemesi olmayan bir dosya da işlenmiş sayılır —, hiçbiri işlenmediyse `2`,
yalnızca bazıları işlendiyse `3`, kullanım hatasında `1`.

## Hatalar

`join-repeats` alt komutuna özgü hata yoktur: okunamayan bir dosya her alt komutta olduğu gibi başarısız olur, hedefin
hataları da yazan alt komutlar için ortaktır: bkz. [Çalıştırma](invocation.md#hedef).
