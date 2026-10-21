# `case`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Metinleri, hiçbir etikete dokunmadan **başlık düzenine, cümle düzenine, büyük harfe veya küçük harfe** getirir.
[Pencerenin](../subedit-gui/operations.md) kurallarıyla birlikte `Araçlar ▸ Büyük/Küçük Harf` işlemidir.

| `--to` | Etki | `bonjour marie` şuna dönüşür |
| :----- | :---- | :---------------------- |
| `title` | her sözcük büyük harf alır, gerisi küçülür | `Bonjour Marie` |
| `sentence` | yalnızca metnin ilk harfi büyür; gerisi küçülür | `Bonjour marie` |
| `upper` | hepsi büyük harf | `BONJOUR MARIE` |
| `lower` | hepsi küçük harf | `bonjour marie` |

- **İlk harften veya ilk rakamdan önce gelene dokunulmaz**: bir diyalog
  çizgisi, bir açılış tırnağı veya bir parantez korunur (`- bonjour` sonucunda `- Bonjour` elde edilir).
- **Etiketler metin değildir**: büyük/küçük harfleri değişmez — `<I>`, `<I>` olarak kalır — ve
  yer değiştirmezler: bir sözcüğü kesen etiket sonrasında da keser.
- **Bir sözcüğün nerede başladığına ICU karar verir**: `l'été sera chaud` sonucunda `L'été Sera Chaud` elde edilir ve aksanlar
  korunur (`éléonore` sonucunda `Éléonore` elde edilir).

<!-- example: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--to` | **evet** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | hayır | `N-M` veya sona kadar `N-` — bkz. [`adjust`](adjust.md#--range) | dosyanın tamamı |
| `--document` | hayır | `main` veya `translation` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `main` |
| `-t`, `--translation-file` | `--document translation` ile | bir çeviri dosyası, **tek bir** girdi için | — |
| `--align-method` | hayır, ve yalnızca `-t` ile | `position` veya `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` hariç | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

**Varsayılan olarak ana metin; çeviri ise `--document translation -t FILE` ile.** O zaman
değiştirilen ve yazılan çeviridir, yalnızca o — bkz. [Bir çeviri](invocation.md#bir-çeviri).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

**Hiçbir metnin değişmediği** — her şeyin zaten istenen düzende olduğu — bir dosya olduğu gibi yazılır ve bunu
söyler:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır. `--dry-run` ile, değişikliklerin listesi;
[Yazmadan önce görme](invocation.md#yazmadan-önce-görme) bölümünde anlatılmıştır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, veya `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konur |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılaması** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

`N`, **metni değişen** altyazıların sayısıdır; işlemin kendisinden okunur.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi
`counts.changed` — cümledeki `N` — ve `changes`, yani değişen altyazıların listesini taşır
(`{"subtitle", "document", "before", "after"}`, hiçbir şey değişmediğinde **boş**).

## Çıkış kodları

Aracın genel kodları: her dosya işlendiyse `0` — hiçbir şeyin değişmediği bir dosya da işlenmiş sayılır —,
hiçbiri işlenmediyse `2`, yalnızca bazıları işlendiyse `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| `--to` eksik | `--to is required` |
| dört değerden biri olmayan bir değer | `--to: … not in {title,sentence,upper,lower}` |
| kötü yazılmış veya dosyanın dışında kalan aralık | bkz. [`adjust`](adjust.md#--range) |

Hedef hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
