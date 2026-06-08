# curation

人手で監修した辞書データを置くディレクトリです。

このディレクトリのファイルは、公開する辞書の内容を再現するために必要な
手作業の判断結果です。機械生成できる中間生成物ではなく、Git で管理します。

## 内容

- `jmdict/p3/*.tsv`
  教育漢字について、JMdict 由来の候補から採用すると判断した読み。
- `jmdict/p4/*.tsv`
  常用漢字と人名用漢字について、JMdict 由来の候補から採用すると判断した読み。
- `hitomoji.dic`
  公開用の最終辞書ファイル。

## 更新

作業ディレクトリ側の `jmdict/p3/*.accept.tsv`、`jmdict/p4/*.accept.tsv`、
および `hitomoji.dic` を更新した後、次を実行します。

```sh
make build-curation
```

## 復元

clone した環境で、この監修データを作業ディレクトリへ戻すには次を実行します。

```sh
make apply-curation
```

通常のビルド処理は、作業ディレクトリ側の `*.accept.tsv` を入力として使います。
