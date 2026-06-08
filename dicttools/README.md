# ひともじ辞書ツール(hitomoji-dicttools)

このディレクトリ以下には、日本語IME「ひともじ」の辞書(hitomoji.dic)を作成/管理するためのツール群を格納している。

上流辞書データは `archive/` に置く。`archive/` は Git 管理対象外であり、
KANJIDIC2 と JMdict の再配布を避けるため、各環境で個別に用意する。

- `archive/kanjidic2.xml`
- `archive/JMdict.xml`

辞書生成の基本順序は次の通り。

```
make -C kdic2
make -C jmdict
make -C jmdict/p1
```

公開リポジトリから辞書を再現する手順は `BUILD.md` を参照する。

ディレクトリ構成は次のようにしている。詳細は各ディレクトリ内の README.md を参照されたい。


## 1. 辞書生成

### kdic2

KANJIDIC2 を用いてベース辞書を作成する

### jmdict

JMdict を用いて読みの不足や現代語で使用しない読みの排除を行う

### my-corpus

辞書の使用頻度情報のカスタマイズ(使用はオプション)

### build

辞書ファイル(hitomoji.dic)の作成を行う


## 2. ツール類

### tools

辞書閲覧ツール、辞書編集ツールなど
