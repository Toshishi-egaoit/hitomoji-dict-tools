# jmdict/p4

JMdict を使って、常用漢字と人名用漢字について KANJIDIC2 由来の読みだけでは
不足する読みを人手で追加登録するフェーズです。

このフェーズでは、`p3` で教育漢字の人手追加を反映した DB を入力にします。
教育漢字側で採用済みの読みを使って JMdict の読み合わせを行うため、常用漢字と
人名用漢字の候補ノイズを減らすことを狙います。

## 入力

- `../p3/dict-jmdict-p3.db`
- `../jmdict.db`
- JMdict 突合ツール
- 人手で管理する採用・不採用ファイル

## 中間生成物

- `gjo.raw.tsv`, `gjin.raw.tsv`
  各段階の `dictmatch` 生出力。
- `gjo.tsv`, `gjin.tsv`
  `../p3/p3filter` 適用後の、人手レビュー対象の候補リスト。
- `gjo.reject.tsv`, `gjin.reject.tsv`
  `../p3/p3filter` が明らかな誤爆と判断して除外した候補。
- `gjo.accept.tsv`, `gjin.accept.tsv`
  人手で採用すると判断した候補。
- `gjo.sql`, `gjin.sql`
  採用 TSV を各 `y_p4_g*` テーブルへ反映する SQL。

## 生成物

- `dict-jmdict-p4.db`

## 方針

- `g*.raw.tsv` は `dictmatch` の機械生成候補リストです。
- `g*.tsv` は `../p3/p3filter` で直前文字の読み漏れや不自然な語頭読みを除外した候補リストです。
- `g*.reject.tsv` には、除外した候補とその理由を残します。
- `g*.tsv` には、候補判断のために `dictmatch` の詳細出力も含めます。
- `g*.accept.tsv` は人手で管理する判断結果です。
- `g*.accept.tsv` は自動生成しません。対応する `g*.tsv` を確認し、採用する行だけを残して作成します。
- SQL 生成時は `MISSING_CANDIDATE` 行だけを採用対象として処理します。
- `dict-jmdict-p4.db` は `dict-jmdict-p3.db` に採用済みの読みを反映した中間 DB です。
