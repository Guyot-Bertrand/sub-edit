# `italics`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Metinleri **italik** yapar ya da **italiklerini kaldırır**; bunu dosya biçiminin etiketleriyle yapar.
[Pencerenin](../subedit-gui/operations.md) `Italic` işlemidir.

**İki sözcük var, bir geçiş düğmesi yok**: pencerenin düğmesi hangi yöne işleyeceğini çekirdeğe
sorar; burada ise bunu kullanıcı söyler. `--on` ve `--off` birbirini dışlar ve biri gereklidir — ikisini de vermemek
ya da ikisini birden vermek bir kullanım hatasıdır (kod `1`).

- **`--on`** her metni, içindeki etiketleri **kaldırdıktan sonra**, tek bir etiket çiftiyle
  sarar: zaten italik olan bir metin iki kez sarılmaz, ikiye bölünmüş italikler
  tek parça olarak yeniden yapılır. Birden çok satırda, tek bir çift tamamını kapsar.
- **`--off`** italik etiketlerini kaldırır; **bir etiketin geri kalanı korunur** (`{\b1\i1}`
  `{\b1}` olur).
- Metin, **dosyanın biçiminin italiği yazdığı gibi** yazılır: SubRip, WebVTT ve
  SubViewer 2 için `<i>…</i>`, iki Sub Station Alpha için `{\i1}…{\i0}`, MicroDVD için `{Y:i}`.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
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

## Bağımsız değişkenler ve seçenekler

| Seçenek | Zorunlu | Değerler | Varsayılan |
| :----- | :----- | :------ | :----- |
| `<file>...` | evet | bir ya da daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--on` / `--off` | **ikisinden biri** | bayraklar | — |
| `--range` | hayır | `N-M` ya da sona kadar `N-` — bkz. [`adjust`](adjust.md#--range) | dosyanın tamamı |
| `--document` | hayır | `main` veya `translation` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `main` |
| `-t`, `--translation-file` | `--document translation` ile | bir çeviri dosyası, **tek bir** girdi için | — |
| `--align-method` | hayır, yalnızca `-t` ile | `position` veya `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` dışında | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

**Varsayılan olarak ana metin; çeviri için `--document translation -t FILE`.** O durumda
değiştirilen ve yazılan çeviridir, yalnızca o — bkz. [Bir çeviri](invocation.md#bir-çeviri).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## Stili olmayan bir biçim reddeder

**TMPlayer ve LRC hiçbir stil yazmaz**: ne italik yapılacak ne de kaldırılacak bir şey vardır. Pencere
bu öğeyi gizlemek yerine soluk gösterir; komut satırı ise **dosya dosya, nedenini söyleyerek
reddeder** ve toplu işin diğer dosyaları sürer. Önemli olan, biçimin neyi taşıyabildiğidir,
onu nasıl yazdığı değil: SubRip ve WebVTT italiği aynı şekilde yazar, ama renk konusunda aynı şeyi
söylemezler.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

JSON biçiminde, başarısızlık **`no-style`** tanımlayıcısını taşır.

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır. `--dry-run` ile, değişikliklerin listesi;
[Yazmadan önce görme](invocation.md#yazmadan-önce-görme) bölümünde anlatılır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>` ya da `N subtitles taken out of italics` ya da `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konur |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

`N`, **metni değişen** altyazıların sayısıdır; işlemin kendisinden okunur.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi
`counts.changed` — cümledeki `N` — ve `changes`, yani değişen altyazıların listesini taşır
(`{"subtitle", "document", "before", "after"}`; hiçbir şey değişmediğinde **boş**).

## Çıkış kodları

Aracın kodları: her dosya işlendiyse `0` — hiçbir şeyin değişmediği bir dosya da işlenmiş sayılır —,
hiçbiri işlenmediyse `2`, yalnızca bazıları işlendiyse `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| ne `--on` ne `--off` | `italics needs --on to put the texts in italics, or --off` |
| ikisi birden | `--on and --off say opposite things; give one of them` |
| stili olmayan bir biçim | `<path>: TMPlayer writes no style: there are no italics to put on` (ya da `take out`) |
| yanlış yazılmış ya da dosyanın dışında kalan aralık | bkz. [`adjust`](adjust.md#--range) |

Hedefin hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
