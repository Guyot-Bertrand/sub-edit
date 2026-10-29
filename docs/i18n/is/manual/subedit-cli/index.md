# subedit-cli — handbók

<!-- machine-translated -->
> Þessi síða er vélþýðing sem enginn lesandi tungumálsins hefur enn farið yfir.

Skipanalínuverkfæri subedit: vinna með skjátextaskrár án myndræns
viðmóts, í runum eða úr skriftu. Um hitt forritið og til að vita
hvar á að byrja, sjá
[handbókina](../index.md).

> **Núverandi staða.** Undirskipanirnar sautján í `subedit-cli` eru til: skoða,
> umbreyta, færa, umbreyta staðsetningum, tímasetja rammatíðnina upp á nýtt, smella á rammanet, fjarlægja
> tilvísanir fyrir heyrnarskerta, laga tímalengdir, skipta út texta, breyta stafstöðu,
> skáletri og þankastrikum, koma skjátextunum aftur í röð, leiðrétta
> textana með mynstrum Gaupol, setja skrár hverja á eftir annarri, skipta
> skrá í tvennt og skrifa þýðingu á staðsetningar aðalskrárinnar. Þessi handbók
> lýsir því sem er til, aldrei því sem er fyrirhugað; það sem kemur næst er í
> [vegvísinum](../../../../feuille-de-route.md).

## Kaflar

| Kafli | Efni |
| :------ | :------ |
| [Uppsetning](installation.md) | að smíða og setja upp verkfærið |
| [Ræsing](invocation.md) | altækir valkostir, úttak, skilakóðar |
| [Stillingaskráin](invocation.md#stillingaskráin) | það sem `adjust` og `correct` lesa úr stillingaskrá gluggans, `--config`, `--no-config` |
| [Vinnsla á möpputré](lots.md) | margar skrár, heil mappa, `--recursive` |
| [`inspect`](inspect.md) | tilkynna úr hverju skrá er gerð |
| [`convert`](convert.md) | skrifa skrá á öðru sniði eða í annarri mynd |
| [`shift`](shift.md) | færa hverja staðsetningu um sömu stærð |
| [`transform`](transform.md) | leiðrétta hverja staðsetningu út frá tveimur punktum sem vitað er að eru réttir |
| [`framerate`](framerate.md) | tímasetja skrá upp á nýtt úr einni rammatíðni í aðra |
| [`snap`](snap.md) | setja tímamerkin aftur á ramma rammatíðni |
| [`hearing-impaired`](hearing-impaired.md) | fjarlægja tilvísanir fyrir heyrnarskerta |
| [`adjust`](adjust.md) | laga tímalengdir að lestrarhraða og mörkum |
| [`replace`](replace.md) | skipta út texta án þess að brjóta merki |
| [`case`](case.md) | setja textana í hástafi í hverju orði, hástaf í upphafi setningar, hástafi eða lágstafi |
| [`italics`](italics.md) | setja textana í skáletur eða taka skáletrið af þeim |
| [`dialogue-dashes`](dialogue-dashes.md) | setja þankastrik á eða taka þau af |
| [`sort`](sort.md) | koma skjátextunum aftur í röð eftir upphafi þeirra |
| [`join-repeats`](join-repeats.md) | gera einn skjátexta úr bútum sama texta sem fylgja hver öðrum |
| [`disjoint`](disjoint.md) | skera skjátexta sem skarast þannig að ekkert tímabil skarist lengur við annað |
| [`repair`](repair.md) | tengja saman `join-repeats`, `sort` og `disjoint` á skrá sem er illa skipt, í ólagi og með skörun |
| [`correct`](correct.md) | leiðrétta textana með mynstrum Gaupol: tilvísanir, sameining og skipting orða, algengar villur, hástafir, línuskipting |
| [`append`](append.md) | setja skrár á eftir þeirri fyrstu, í eina skrá |
| [`split-file`](split-file.md) | skipta skrá í tvennt, með halann fluttan aftur að upphafspunkti |
| [`pair`](pair.md) | skrifa þýðingu á staðsetningar aðalskrárinnar og segja hvernig línur hennar röðuðust saman |
| [`transcribe`](transcribe.md) | skrifa tal myndskeiðs eða hljóðskrár sem skjátexta |
| [`sync`](sync.md) | tímastilla skjátexta skráar eftir hljóði myndskeiðs |
| [Uppsetning umritunartækjanna](install-transcription.md) | vélin, líkanið og `ffmpeg`, sem forritið fylgir ekki með |
| [`config`](config.md) | telja upp, sýna og breyta stillingum aðgerða sem eru sameiginlegar með glugganum |
