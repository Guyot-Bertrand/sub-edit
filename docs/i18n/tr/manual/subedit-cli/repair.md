# `repair`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Art arda çalıştırılan üç komutun işini tek seferde yapar: önce [`join-repeats`](join-repeats.md), sonra
[`sort`](sort.md), sonra [`disjoint`](disjoint.md). **Bu sıra, işe yarayan tek sıradır**: kesme işlemi,
başlangıçların sırasına bakar ve aynı metnin yinelenen parçalarının her ikisinden de önce ortadan kalkması daha iyidir.
Kötü bölümlenmiş, sırasız ve çakışan bir dosyada çalıştırılacak komut budur.

Ayarlar, zincirin sonundaki `disjoint` komutununkilerdir; sıralama her zaman yapıldığından burada `--sort` yoktur.

<!-- example: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--minimum` | hayır | bir süre: `0.08`, `1.5`, `00:00:00,080`. Sıfır, her aralığı korur | `0.08` (yaklaşık iki kare) |
| `--max-lines` | hayır | bir tam sayı; `0` sınırı kaldırır | `2` |
| `--dashes` | hayır | bir bayrak: birleştirilmiş bir altyazıya her altyazının getirdiği metnin önüne bir çizgi koyar | kapalı |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

İki parçaya bölünmüş ve art arda gelen `Alpha`, yeniden tek bir altyazı olur; sonra `Beta` ile aynı aralığı kaplar
ve ikisi, en erken olan başta olmak üzere iki satırlı tek bir altyazı olur.

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır. Hata çıktısında, ilk düzeyde, **üç sayıyı zincirleyen bir
cümle** yazılır; noktalı virgülle ayrılmış olarak: `join-repeats`, `sort` ve `disjoint` komutlarının her birinin
kendi sözleriyle söylediği — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın `counts` alanı: `subtitles`, `joined`,
`moved`, ardından [`disjoint`](disjoint.md#json-biçiminde) komutunkiler (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Çıkış kodları ve hatalar

Aracın kodlarıdır. `disjoint` komutununkiler gibi değerlendirilen ayarların ötesinde `repair` komutuna özgü hata yoktur:
süre olmayan ya da negatif bir `--minimum` ve tam sayı olmayan bir `--max-lines`, kod `1`.
