# Build

この文書では、公開リポジトリから `hitomoji.dic` を再現する手順を説明します。

## 外部データ

KANJIDIC2 と JMdict は再配布しないため、各環境で用意して `archive/` に置きます。

- `archive/kanjidic2.xml`
- `archive/JMdict.xml`

## トシ監修データを使う場合

公開リポジトリには、手作業で選別したデータを `curation/` に入れています。
このデータを使う場合は、まず作業用ファイルへ反映します。

```sh
make apply-curation
touch jmdict/p3/*.accept.tsv jmdict/p4/*.accept.tsv
make
```

`touch` は意図的に手順として分けています。`*.accept.tsv` は人手で管理する判断結果であり、
Makefile から自動更新すると、作業中のレビュー結果を上書きする危険があるためです。

## 自分で選別する場合

`curation/` を使わず、自分で JMdict 候補を確認する場合は、各フェーズで候補を生成し、
`make edit-*` で `*.accept.tsv` を作成します。

```sh
make -C kdic2
make -C jmdict
make -C jmdict/p1
make -C jmdict/p2
make -C jmdict/p3 g1.tsv
make -C jmdict/p3 edit-g1
```

`p3` は `g1` から `g6`、`p4` は `gjo` と `gjin` を確認します。

## 公開用 curation の更新

監修データや `hitomoji.dic` を更新した後、コミット前に次を実行します。

```sh
make build-curation
```

これにより、作業用の `jmdict/p3/*.accept.tsv`、`jmdict/p4/*.accept.tsv`、
および `hitomoji.dic` が `curation/` に同期されます。
