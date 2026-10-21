# `sort`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Altyazıları **başlangıçlarının sırasına** geri koyar. Başka hiçbir şey değişmez: ne metinler, ne
konumlar, ne de dosyanın biçimi. Altyazılar, geri yazılan her dosyada olduğu gibi, yazma sırasında
1'den başlayarak **yeniden numaralandırılır**.

**Çekirdek kendi başına asla sıralamaz**: bir dosyayı okumak sırasını olduğu gibi bırakır ve
[`inspect`](inspect.md#anomalies-ne-bildirir) sıradaki bir kırılmayı
`subtitle N starts before the previous one starts` ile bildirir. `sort`, sırası bozuk bir dosya ya da grup için
**bilerek yapılan harekettir** — ve pencerede, her işlem gibi, geri alınabilir.

- **Sıralama kararlıdır**: aynı anda başlayan iki altyazı, **dosyanın verdiği sırayı korur**.
  Hiçbiri diğerinden önce gelmez; onları yerinden oynatmak kimsenin istemediği bir
  karar olurdu.
- **`sort` ne bir aralık ne de bir belge alır**: seçilecek bir şey yoktur, sıra başlangıçların sırasıdır.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

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
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## Neyin sayıldığı

**Sayı, altyazısı değişen yerlerin sayısıdır**; çekirdek bunu böyle bildirir — pencerenin
geçmişinin taşınmış diyeceği şey. Yerinden çıkmış tek bir altyazı birkaç yeri kaydırır: yukarıdaki
örnekte üç yerin hiçbiri altyazısını korumaz ve cümle `3 subtitles moved` der.

**Zaten sıralı bir dosya yine de yazılır** ve bunu söyler: verilen bir hedef,
yazılmış bir hedeftir ve `sort` komutunu istisna yapmak, bir betiği hangi alt komutların
bazen dosya ürettiğini, bazen hiçbir şey üretmediğini bilmeye zorlardı.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır. `--dry-run` ile de hiçbir şey yoktur: dosya
yoktur ve anlatım satırı bunu söyler — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme).

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>` ya da `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi
`counts.subtitles` (dosyanın tüm altyazıları) ve `counts.moved` (değişen yerler;
zaten sıralı bir dosya için `0`) alanlarını taşır.

## Çıkış kodları

Aracın kodlarıdır: her dosya işlendiyse `0` — zaten sıralı bir dosya işlenmiş sayılır —, hiçbiri
işlenmediyse `2`, yalnızca bazıları işlendiyse `3`, kullanım hatasında `1`.

## Hatalar

`sort` komutuna özgü hata yoktur: okunamayan bir dosya her alt komutta olduğu gibi başarısız olur ve
hedefin hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
