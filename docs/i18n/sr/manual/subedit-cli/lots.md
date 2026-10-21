# Обрада стабла директоријума

<!-- machine-translated -->
> Ова страница је машински превод који још није прегледао читалац језика.

```
subedit-cli <subcommand> --recursive [options] <directory>...
```

Свака подкоманда која чита серију датотека — укључујући `inspect`, а не
[`append`](append.md), [`split-file`](split-file.md) ни [`pair`](pair.md), чија је бројност
пар или један филм — прихвата **директоријум** уместо датотеке, уз опцију `--recursive` (`-r`).
Ова страница каже шта обилазак узима, којим редоследом и где пише.

**Ништа од наведеног не мења оно што се ради са датотеком**: свака се чита,
обрађује и пише независно, неуспех једне не зауставља остале, а
четири излазна кода су она из [покретања](invocation.md#излазни-кодови).
Обилазак даје списак путања, које остатак серије користи као да су
укуцане.

## Без опције `--recursive` директоријум се одбија

Грешка у употреби, код `1`, ниједна датотека није обрађена нити написана:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?" -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; subedit-cli inspect films; echo "code=$?"
films: is a directory: use --recursive
code=1
```

## Шта обилазак узима

Обилазак је **рекурзиван, одређен и без изненађења**:

- **редослед** — ставке сваког директоријума узимају се редом по
  називима, **бајт по бајт** (велика слова пре малих, независно од језика
  машине), а поддиректоријум се обилази на свом месту, у
  дубину. Два покретања дају исте бајтове, на свакој
  машини;
- **симболичке везе се не прате** — ни ка датотеци, ни ка
  директоријуму: стабло које садржи везу ка једном од својих родитеља не би
  имало краја;
- **скривене датотеке и директоријуми се занемарују** — они чији назив почиње
  тачком;
- **задржавају се само наставци познатих формата**: `.srt`, `.vtt`,
  `.ssa`, `.ass`, `.lrc`, `.sub`, независно од величине слова. **`.txt` је изузет**: он
  означава два формата, али пре свега сваки `LISEZMOI.txt` на свету, а серија која
  би пропала на сваком од њих била би неупотребљива.

**Датотека наведена у командној линији никада се не филтрира.** Навести је значи
хтети је: `subedit-cli shift --by 1 --in-place notes.txt` је обрађује, ма какав
јој био наставак, и жали се само ако њен садржај није ни у једном формату.

**Оно што је остављено по страни се броји** и јавља на нивоу `-vv`:

<!-- example: mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1 -->
```console
$ mkdir films; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > films/a.srt; printf 'note\n' > films/LISEZMOI.txt; subedit-cli -vv inspect --recursive films 2>&1 | head -1
films: 1 file left aside, with no extension of a known format
```

Директоријум који не садржи ништа за обилазак то каже на подразумеваном нивоу
(`<directory>: holds no file in a format this tool walks for`) и то није грешка.

## Где обилазак пише

**Стабло се чува под `--output-dir`**: путања датотеке,
**релативна у односу на задати директоријум**, понавља се испод излазног директоријума. Назив
задатог директоријума није њен део — то је семантика програма `rsync` са
завршном косом цртом, а не програма `cp -r`: наводи се одредиште и њему се додаје
ниво који се жели (`--output-dir sortie/films`).

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

Излазни директоријум и директоријуми стабла се **стварају** ако недостају. Уз
`--in-place`, свака задржана датотека се уписује назад на своје место. Наставак који
`convert` мења мења се и у стаблу: `films/a/x.srt` претворена
у WebVTT пише се као `sortie/a/x.vtt`.

**Излазни директоријум, ако је унутар обиласканог стабла, изузет је из обиласка**:
друго покретање поново не чита оно што је прво написало.

## Два директоријума и сударања

Правило о [одредишту](invocation.md#одредиште) то решава: две датотеке
које би завршиле на истој путањи одбијају се **пре било каквог писања**, код `1`, са
наведена оба улаза и одредиште.

<!-- example: mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?" -->
```console
$ mkdir -p un/a deux/a; printf '1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n' > un/a/x.srt; cp un/a/x.srt deux/a/x.srt; subedit-cli shift --by 1 -r --output-dir sortie un deux; echo "code=$?"
sortie/a/x.srt: would be written by both un/a/x.srt and deux/a/x.srt
code=1
```

## Колико кошта

Серија остаје секвенцијална: њено трајање је збир трајања њених датотека.
Мерење `make bench` мери обилазак и исти посао над једном датотеком; бројке
су у [дневнику перформанси](../../../../mesures/performances.md).
