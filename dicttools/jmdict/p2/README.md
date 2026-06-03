# jmdict/p2

JMdict を使って、KANJIDIC2 由来の読みのうち、現代語での使用例が確認できない
読みを機械的に除外するフェーズです。

このフェーズは追加フェーズではありません。`p1` を適用した後の DB を入力にして、
JMdict に使用例が見つからない読みを `y_exclude` に登録します。

## 入力

- `../p1/dict-jmdict-p1.db`
- `../jmdict.db`
- JMdict 突合ツール

## 中間生成物

- `ex-gkyo.tsv`, `ex-gjo.tsv`, `ex-gjin.tsv`
  `dictmatch` が出力した `NO_EVIDENCE` の候補。
- `ex-gkyo.sql`, `ex-gjo.sql`, `ex-gjin.sql`
  `y_exclude` に反映する SQL。

## 生成物

- `dict-jmdict-p2.db`

## 方針

- このフェーズは、再現可能な機械処理だけで完結させます。
- 人手による採用・不採用判断は置きません。
- `dict-jmdict-p2.db` は `dict-jmdict-p1.db` に機械的な除外結果を反映した中間 DB です。
- 対象は `gkyo`、`gjo`、`gjin` の3区分に分けます。
  `gkyo` は `grade` 1 から 6、`gjo` は `grade = 8`、`gjin` は `grade` 9 と 10 です。
- 人手での追加判断は `../p3/` で扱います。
