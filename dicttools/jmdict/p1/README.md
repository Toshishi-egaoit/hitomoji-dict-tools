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

- `g1.tsv`, `g2.tsv`, `g3.tsv`, `g4.tsv`, `g5.tsv`, `g6.tsv`, `gjo.tsv`, `gjin.tsv`
  各段階の `dictmatch` 結果。重い処理のキャッシュとして扱う。
- `g1.accept.tsv`, `g2.accept.tsv`, `g3.accept.tsv`, `g4.accept.tsv`, `g5.accept.tsv`,
  `g6.accept.tsv`, `gjo.accept.tsv`, `gjin.accept.tsv`
  p1 の保守的な音便フィルタを通過した候補。
- `g1.sql`, `g2.sql`, `g3.sql`, `g4.sql`, `g5.sql`, `g6.sql`, `gjo.sql`, `gjin.sql`
  TSV を各 `y_g*` テーブルへ反映する SQL。
## 生成物

- `dict-jmdict-p1.db`

## 方針

- このフェーズは、再現可能な機械処理だけで完結させます。
- 人手による採用・不採用判断は置きません。
- `P1_LIMIT` の既定値は `5` とします。
- `P1_FILTER` の既定値は `onbin,okuri,renyo,renyo-ru,renyo-ichidan-e,renyo-rendaku` とします。
- 対象は `g1` から `g6`、`gjo`、`gjin` です。
- 人名用漢字の `gjin` も、p1 フィルタを通過した安全な候補だけを自動登録します。
- 候補抽出と p1 フィルタは、常に `../../kdic2/dict-kdic2.db` を基準に行います。
  p1 で自動追加した読みを、さらに別候補の根拠として使うことはしません。
- 自動登録する候補は、既存読みから標準的な音便として説明できるものに限定します。
  初期ルールでは、末尾の `つ`/`ち`/`く` の促音化、語頭の濁音化、語頭の半濁音化だけを
  採用します。
- `P1_FILTER=okuri` を指定すると、`y_base` の `yomi` と `okuri` を連結した読みだけを
  採用します。たとえば `う.け` から `うけ` を採用できます。
- `P1_FILTER=renyo` を指定すると、`y_base` の送り仮名から導出できる連用形だけを採用します。
  `renyo` では `る` の連用形は扱いません。
- `P1_FILTER=renyo-ru` を指定すると、送り仮名 `る` から導出できる `り` 形だけを採用します。
- `P1_FILTER=renyo-ichidan-e` を指定すると、`え段 + る` で終わる送り仮名から、
  下一段の連用形を採用します。たとえば `う.ける` から `うけ` を採用できます。
- `P1_FILTER=renyo-rendaku` を指定すると、連用形を作った後に語頭の濁音化・半濁音化を
  適用した候補だけを採用します。
- `P1_FILTER=onbin,renyo` や `P1_FILTER=onbin,okuri,renyo,renyo-ru,renyo-ichidan-e,renyo-rendaku` で
  複数のフィルタを有効にできます。`P1_FILTER=all` はすべてのフィルタを有効にします。
- TSV はキャッシュとして扱います。条件を変えて再抽出したい場合は、対象 TSV を削除するか
  `make clean-tsv` を使います。
- `dict-jmdict-p1.db` は `dict-kdic2.db` をコピーしたうえで、各 `g*.sql` をまとめて反映して作ります。
- `dict-jmdict-p1.db` は `dict-kdic2.db` に自動登録対象の読みを反映した中間 DB です。
