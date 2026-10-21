# Dizin ağacını işleme

<!-- machine-translated -->
> Bu sayfa makine çevirisidir ve henüz dili bilen bir okuyucu tarafından gözden geçirilmemiştir.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Bir dosya grubunu okuyan her alt komut — `inspect` dahil; bir çift ya da bir film alan
[`append`](append.md), [`split-file`](split-file.md) ve [`pair`](pair.md) hariç — dosya yerine bir **dizin**
kabul eder; bunun için `--recursive` (`-r`) kullanılır.
Bu sayfa, dizin taramasının neleri aldığını, hangi sırayla aldığını ve nereye yazdığını anlatır.

**Aşağıdakilerin hiçbiri bir dosyaya yapılan işlemi değiştirmez**: her dosya ayrı ayrı okunur,
işlenir ve yazılır, birinin başarısız olması diğerlerini durdurmaz ve dört
çıkış kodu [çalıştırma](invocation.md#çıkış-kodları) sayfasındakilerle aynıdır.
Tarama, bir yol listesi üretir; grubun geri kalanı bu listeyi sanki elle
yazılmış gibi kullanır.

## `--recursive` olmadan dizin reddedilir

Kullanım hatası, kod `1`; hiçbir dosya işlenmez ya da yazılmaz:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Taramanın aldıkları

Tarama **özyinelemeli, belirleyici ve sürprizsizdir**:

- **sıra** — her dizinin girdileri adlarının sırasına göre, **bayt bayt**
  alınır (makinenin diline bakılmaksızın büyük harfler küçük harflerden önce gelir) ve bir alt dizin,
  kendi yerinde, önce derinlik esasına göre taranır. İki çalıştırma, her makinede aynı
  baytları verir;
- **sembolik bağlantılar izlenmez** — ne bir dosyaya ne de bir
  dizine: üst dizinlerinden birine bağlantı içeren bir ağacın
  sonu olmazdı;
- **gizli dosyalar ve dizinler yok sayılır** — adı bir
  noktayla başlayanlar;
- **yalnızca bilinen biçimlerin uzantıları tutulur**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`; büyük/küçük harf fark etmez. **`.txt` hariç tutulur**: iki
  biçimin adıdır, ama her şeyden önce dünyadaki her `LISEZMOI.txt` dosyasının uzantısıdır ve her birinde
  başarısız olan bir grup işlemi kullanılamaz olurdu.

**Komut satırında adı verilen bir dosya asla süzülmez.** Adını vermek
istemek demektir: `subedit-cli shift --by 1 --in-place notes.txt` uzantısı ne olursa olsun
dosyayı işler ve yalnızca içeriği hiçbir biçimde değilse şikâyet eder.

**Dışarıda bırakılanlar sayılır** ve `-vv` düzeyinde bildirilir:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

İşlenecek hiçbir şey içermeyen bir dizin bunu varsayılan düzeyde bildirir
(`<directory>: holds no file in a format this tool walks for`) ve bu bir hata değildir.

## Taramanın yazdığı yer

**Ağaç `--output-dir` altında korunur**: bir dosyanın yolu,
**verilen dizine göre göreli olarak**, çıktı dizininin altında yeniden oluşturulur. Verilen
dizinin adı buna dahil değildir — bu, sonunda bölü işareti olan `rsync` anlamıdır,
`cp -r` anlamı değil: hedefin adı verilir ve istenen
düzey ona eklenir (`--output-dir sortie/films`).

<!-- example: mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort -->
```console
$ mkdir -p films/a/b; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/x.srt; cp films/x.srt films/a/x.srt; cp films/x.srt films/a/b/y.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli shift --by 1 --recursive --output-dir sortie films; find sortie -type f | sort
films/a/b/y.srt: 1 subtitle shifted by 1.000 s -> sortie/a/b/y.srt
films/a/x.srt: 1 subtitle shifted by 1.000 s -> sortie/a/x.srt
films/x.srt: 1 subtitle shifted by 1.000 s -> sortie/x.srt
3 of 3 files shifted
sortie/a/b/y.srt
sortie/a/x.srt
sortie/x.srt
```

Çıktı dizini ve ağaçtakiler eksikse **oluşturulur**. `--in-place` ile tutulan
her dosya kendi yerine geri yazılır. `convert` komutunun değiştirdiği bir uzantı
ağaçta da değişir: WebVTT biçimine dönüştürülen `films/a/x.srt`
dosyası `sortie/a/x.vtt` olarak yazılır.

**Çıktı dizini taranan ağacın içindeyse taramanın dışında bırakılır**:
ikinci bir çalıştırma, ilkinin yazdıklarını yeniden okumaz.

## İki dizin ve çakışmalar

[Hedef](invocation.md#hedef) kuralı bunu belirler: aynı yolda sonlanacak iki dosya
**hiçbir şey yazılmadan önce** reddedilir; kod `1`, iki girdi ve hedef
adlarıyla bildirilir.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Maliyeti

Grup sıralı kalır: süresi dosyalarının sürelerinin toplamıdır.
`make bench` kıyaslaması taramayı ve aynı işi tek bir dosyada ölçer; rakamlar
[başarım günlüğünde](../../../../mesures/performances.md) yer alır.
