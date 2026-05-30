# jmdict/p1

JMdict を使って、KANJIDIC2 由来の読みだけでは不足する読みを自動補充する
フェーズです。

音便変化、濁音化、慣用読み、といった KANJIDIC2 では範囲外となる読みを補います。
具体的には、KANJIDIC2 の基礎情報と JMdict の熟語読みを突合し、不足している読みを
抽出します。

ただし、1 パスですべてを登録すると踏み外しが多数発生するため、このフェーズでは
使用頻度が高く、踏み外しがないという前提を置けるものだけを抽出し、機械登録します。

## 入力

- `../../kdic2/dict-kdic2.db`
- `../jmdict.db`
- JMdict 突合ツール

## 中間生成物

- `auto-register-candidates.tsv`
- `auto-register-yomi.tsv`

## 生成物

- `dict-jmdict-p1.db`

## 方針

- このフェーズは、再現可能な機械処理だけで完結させます。
- 人手による採用・不採用判断は置きません。
- `auto-register-candidates.tsv` は機械抽出された候補リストです。
- `auto-register-yomi.tsv` は機械登録対象に絞り込んだ読みリストです。
- `dict-jmdict-p1.db` は `dict-kdic2.db` に自動登録対象の読みを反映した中間 DB です。
