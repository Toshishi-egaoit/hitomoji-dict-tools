# jmdict/p3

JMdict を使って、教育漢字について KANJIDIC2 由来の読みだけでは不足する読みを
人手で追加登録するフェーズです。

このフェーズでは、`p2` を適用した後の DB を入力にして、使用頻度が低い読みや
踏み外しの可能性が高い読みを抽出します。抽出された候補は人間が確認し、
採用・不採用の判断結果を正式な入力ファイルとして残します。

## 入力

- `../p2/dict-jmdict-p2.db`
- `../jmdict.db`
- JMdict 突合ツール
- 人手で管理する採用・不採用ファイル

## 中間生成物

- `g1.raw.tsv`, `g2.raw.tsv`, `g3.raw.tsv`, `g4.raw.tsv`, `g5.raw.tsv`, `g6.raw.tsv`
  各段階の `dictmatch` 生出力。
- `g1.tsv`, `g2.tsv`, `g3.tsv`, `g4.tsv`, `g5.tsv`, `g6.tsv`
  `p3filter` 適用後の、人手レビュー対象の候補リスト。
- `g1.reject.tsv`, `g2.reject.tsv`, `g3.reject.tsv`, `g4.reject.tsv`, `g5.reject.tsv`, `g6.reject.tsv`
  `p3filter` が明らかな誤爆と判断して除外した候補。
- `g1.accept.tsv`, `g2.accept.tsv`, `g3.accept.tsv`, `g4.accept.tsv`, `g5.accept.tsv`,
  `g6.accept.tsv`
  人手で採用すると判断した候補。
- `g1.sql`, `g2.sql`, `g3.sql`, `g4.sql`, `g5.sql`, `g6.sql`
  採用 TSV を各 `y_p3_g*` テーブルへ反映する SQL。

## 生成物

- `dict-jmdict-p3.db`

## 方針

- `g*.raw.tsv` は `dictmatch` の機械生成候補リストです。
- `g*.tsv` は `p3filter` で直前文字の読み漏れや不自然な語頭読みを除外した候補リストです。
- `g*.reject.tsv` には、除外した候補とその理由を残します。
- `g*.tsv` には、候補判断のために `dictmatch` の詳細出力も含めます。
- `g*.accept.tsv` は人手で管理する判断結果です。
- `g*.accept.tsv` は自動生成しません。対応する `g*.tsv` を確認し、採用する行だけを残して作成します。
- SQL 生成時は `MISSING_CANDIDATE` 行だけを採用対象として処理します。
- 現在の `g*.choice` や `g8.list.addendum` に相当する内容は、
  将来的にこのフェーズの人手管理ファイルへ整理します。
- `dict-jmdict-p3.db` は `dict-jmdict-p2.db` に採用済みの読みを反映した中間 DB です。
- 常用漢字と人名用漢字の人手追加は、`../p4/` で扱います。
