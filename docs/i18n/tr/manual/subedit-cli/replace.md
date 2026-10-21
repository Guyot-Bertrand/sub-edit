# `replace`

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Altyazılardaki bir metni **hiçbir etiketi bozmadan** değiştirir. Pencerenin [aramasının](../subedit-gui/find-and-replace.md)
`Replace All` işlemidir ve kurallarını taşır:

- **arama görünen metinde yapılır** — izleyicinin okuduğunda. Etiketler buna dahil değildir:
  `<i>` aramak hiçbir şey bulmaz, `Bonjour` aramak ise yarısı italik olan bir “Bonjour” bulur;
- **değiştirme dosyanın kaynak metninde yapılır**, etiketler olduğu yerde
  kalır. Aranan sözcüğü kesen bir etiket, değiştirmenin tamamını kapsar; eşleşmeye
  dokunan bir biçem, değiştirmenin tamamını kaplar; eşleşmenin dokunmadığı şey
  yerinden oynamaz. Durum durum kurallar, pencerenin aramasındakilerdir.

**`find` alt komutu yoktur**: bir desenin nerede olduğunu hiçbir şeyi değiştirmeden görmek için,
ilgili her altyazıyı listeleyen [`--dry-run`](invocation.md#yazmadan-önce-görme) seçeneğidir.

<!-- example: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | evet | görünen metinde aranacak metin; **boş olamaz** | — |
| `<replacement>` | evet | yerine geçecek şey, etiketlere izin verilir; boş olabilir | — |
| `<file>...` | evet | bir veya daha fazla yol | — |
| `--recursive`, `-r` | hayır | bir bayrak | kapalı — bkz. [Dizin ağacını işleme](lots.md) |
| `--regex` | hayır | bir bayrak: desen bir ICU düzenli ifadesidir | düz metin |
| `--case-sensitive` | hayır | bir bayrak: büyük ve küçük harfleri ayırt eder | yok sayılırlar |
| `--range` | hayır | `N-M` ya da sona kadar `N-` — bkz. [`adjust`](adjust.md#--range) | dosyanın tamamı |
| `--document` | hayır | `main` veya `translation` — bkz. [Bir çeviri](invocation.md#bir-çeviri) | `main` |
| `-t`, `--translation-file` | `--document translation` ile | bir çeviri dosyası, **tek** bir girdi için | — |
| `--align-method` | hayır, ve yalnızca `-t` ile | `position` veya `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | `--dry-run` dışında **üçünden biri** | bkz. [Çalıştırma](invocation.md#hedef) | — |
| `--dry-run` | hayır | bir bayrak: hesaplar ve bildirir, hiçbir şey yazmaz — bkz. [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) | kapalı |

**Tireyle başlayan bir desen** `--` işaretinden sonra yazılır: `subedit-cli replace -- -- "—" film.srt`.

**Varsayılan olarak ana metin; `--document translation -t FILE` ile çeviri.** Bu durumda
değiştirilen ve yazılan yalnızca çeviridir — bkz. [Bir çeviri](invocation.md#bir-çeviri).

### Düz metin ya da ifade

Varsayılan olarak desen **olduğu gibi okunur**: `a.b`, bir `a`, bir nokta, bir `b` arar. Büyük/küçük harf,
pencerede olduğu kadar ince biçimde yok sayılır — `É` ve `é` tek bir harftir.

`--regex` ile desen bir **ICU** düzenli ifadesidir ve Gaupol'un okuduğu gibi okunur: `.` bir satır sonunu
aşar, `^` ve `$` bir altyazının her satırında geçerlidir. Yalnızca değiştirme metninde
şunlar kullanılır:

| Yazılan | Söylenecek |
| :----- | :-------- |
| `$1` ile `$9` arası | eşleşmenin grupları |
| `$0` | eşleşmenin tamamı |
| `\n` | bir satır sonu |
| `\$`, `\\` | bir `$`, bir ters bölü |

Kabuğun tırnaklarına dikkat edin: `'$1 EUR'` yazın, kabuğun `$1` ifadesini okumasına izin verecek `"$1 EUR"` yazmayın.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Etiketler

Aranan sözcüğü bir etiketin kestiği bir metin:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

İlk altyazıda etiket “Bonjour” sözcüğünü kesiyordu: şimdi “Salut” sözcüğünü kapsıyor.
İkincisinde ona dokunulmaz ve olduğu yerde kalır. **Açık bırakılmış bir etiket
açık kalır**: sağlam olmayan işaretlemeyi onarmak başka bir işlem olurdu.

Değiştirme metni **dosyanın biçiminin etiketleriyle** okunur: SubRip için `<i>…</i>`,
Advanced SSA için `{\i1}…`. Yazılanı biçimlendirmenin tek yolu budur.

## Bulunan bir şey yok, değiştirilecek bir şey yok

Başarısızlık olmayan iki yanıt — çıkış kodu `0`'dır ve **dosya yine de yazılır**,
çünkü verilen bir hedef, yazılmış bir hedeftir:

| Durum | Cümle |
| :-------- | :----- |
| desen hiçbir yerde yok | `"zzz" not found` |
| var, ama her eşleşme kendisiyle değiştirilir | `nothing to change` |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

Standart çıktı, **metni değişecek her altyazıyı**, öncesi ve sonrasıyla, dosyadaki
hâliyle — etiketler dahil — listeler:

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

Listenin biçimi [Yazmadan önce görme](invocation.md#yazmadan-önce-görme) bölümünde anlatılır;
JSON'da `changes` alanıdır.

## Çıktı

**Standart çıktı** — hiçbir şey: sonuç, yazılan dosyadır. `--dry-run` ile yukarıdaki
değişiklik listesi.

| Düzey | Hata çıktısına eklenenler |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, ya da `"<pattern>" not found`, ya da `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — okunan dosyanın biçimi, kodlaması, işareti ve satır sonları, olduğu gibi geri konarak |
| 3 | `<path>: N bytes read, M written`, ardından **her okuma tanılama iletisi** — bkz. [Çalıştırma](invocation.md#tanılama-iletilerini-okuma) |

`N`, **metni değişen altyazılarda değiştirilen eşleşmeleri** sayar — pencerenin de
söylediği sayı budur.

### JSON biçiminde

[`--format json`](invocation.md#bir-betiğin-okuyabileceği-çıktı) ile her dosyanın nesnesi şunları taşır:

| Anahtar | İçerik |
| :-- | :------ |
| `counts.replaced` | metni değişen altyazılarda değiştirilen eşleşmeler: cümledeki `N` |
| `counts.matched` | bulunan eşleşmeler, **değiştirilmeleri metni değiştirsin ya da değiştirmesin** — “hiçbir şey bulunamadı” (`0`) ile “değiştirilecek bir şey yok” (`replaced` `0`, `matched` pozitif) arasındaki farkı söyleyen budur |
| `changes` | değişen altyazılar: `{"subtitle", "document", "before", "after"}`, hiçbir şey değişmediğinde **boş** |

## Çıkış kodları

Aracın kodlarıdır: her dosya işlendiyse `0` — hiçbir şey bulunmayan bir dosya işlenmiş
sayılır —, hiçbiri işlenmediyse `2`, yalnızca bazıları işlendiyse `3`, kullanım hatasında `1`.

## Hatalar

| Neyin tetiklediği | İleti |
| :------------------ | :------ |
| boş desen | `pattern: nothing to look for` |
| okunamayan ifade | `pattern: not a regular expression (<ICU's reason>)` |
| kötü yazılmış ya da dosyanın dışında kalan aralık | bkz. [`adjust`](adjust.md#--range) |

**Desen ilk dosyadan önce okunur**: okunamayan bir ifade bir kullanım hatasıdır
(kod `1`), hiçbir şey okunmaz ya da yazılmaz.

Hedefin hataları, yazan alt komutlar için ortaktır:
bkz. [Çalıştırma](invocation.md#hedef).
