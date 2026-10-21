# `dialogue-dashes`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Satırların başına bir **diyalog çizgisi** koyar veya kaldırır. [Pencerenin](../subedit-gui/operations.md)
`Dialogue Dashes` işleminin karşılığıdır.

**İki sözcük, [`italics`](italics.md) gibi**: `--add` ile `--remove` birbirini dışlar ve biri gereklidir.

- **`--add`**, çizgisi olmayan her satırın önüne bir çizgi koyar: `Bonjour` sonucunda `- Bonjour`,
  `Bonjour\nMarie` sonucunda `- Bonjour\n- Marie` elde edilir. **Zaten var olan bir çizgi ikilenmez.**
- **Üç çizgi tanınır** — klavyenin tiresi ve dizgicinin iki uzun çizgisi —
  **ve yazılan tiredir**: `— Bonjour`, `- Bonjour` olur. Çizgiden sonraki
  fazla boşluklar kaybolur.
- Çizgi **metnin önüne gelir, açılış etiketinin önüne değil**: `<i>Bonjour</i>` sonucunda
  `<i>- Bonjour</i>` elde edilir. Hiçbir etiket yerinden oynamaz.
- **`--remove`**, baştaki çizgileri üç biçimiyle de kaldırır.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **ikisinden biri** | bayraklar | — |
| `--range` | hayır | `N-M` veya sona kadar `N-` — bkz. [`adjust`](adjust.md#--range) | dosyanın tamamı |
| `--document` | hayır | `main` veya `translation` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `main` |
| `-t`, `--translation-file` | `--document translation` ile | bir çeviri dosyası, **tek bir** girdi için | — |
| `--align-method` | hayır, ve yalnızca `-t` ile | `position` veya `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **üçünden biri**, `--dry-run` hariç | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve söyler, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

**Varsayılan olarak ana metin; çeviri ise `--document translation -t FILE` ile.** O zaman
değiştirilen ve yazılan çeviridir, yalnızca o — bkz. [Bir çeviri](invocation.md#bir-çeviri).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Çıktı

**Standart çıktı** — boş: sonuç, yazılan dosyadır. `--dry-run` ile, değişikliklerin listesi;
[Yazmadan önce görme](invocation.md#yazmadan-önce-görme) bölümünde anlatılmıştır.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, veya `N subtitles undashed`, veya `nothing to change` |
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
| ne `--add` ne `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| ikisi birden | `--add and --remove say opposite things; give one of them` |
| kötü yazılmış veya dosyanın dışında kalan aralık | bkz. [`adjust`](adjust.md#--range) |

Hedef hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
