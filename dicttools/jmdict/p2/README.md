# jmdict/p2

JMdict を使って、KANJIDIC2 由来の読みだけでは不足する読みを人手で追加登録する
フェーズです。

このフェーズでは、`p1` を適用した後の DB を入力にして、使用頻度が低い読みや
踏み外しの可能性が高い読みを抽出します。抽出された候補は人間が確認し、
採用・不採用の判断結果を正式な入力ファイルとして残します。

## 入力

- `../p1/dict-jmdict-p1.db`
- `../jmdict.db`
- JMdict 突合ツール
- 人手で管理する採用・不採用ファイル

## 中間生成物

- `manual-register-candidates.tsv`
- `accepted-yomi.tsv`
- `rejected-yomi.tsv`

## 生成物

- `dict-jmdict-p2.db`

## 方針

- `manual-register-candidates.tsv` は機械生成の候補リストです。
- `accepted-yomi.tsv` と `rejected-yomi.tsv` は人手で管理する判断結果です。
- 現在の `g*.choice` や `g8.list.addendum` に相当する内容は、
  将来的にこのフェーズの人手管理ファイルへ整理します。
- `dict-jmdict-p2.db` は `dict-jmdict-p1.db` に採用済みの読みを反映した中間 DB です。
