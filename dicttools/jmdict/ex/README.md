# jmdict/ex

KANJIDIC2 に含まれているが、JMdict では使用例が確認できない読みを除外する
フェーズです。

KANJIDIC2 には、現代語ではほぼ利用しない読みが多数含まれています。このフェーズでは
JMdict の読みと突合し、使用例のない読みを辞書対象から外します。

## 入力

- `../p2/dict-jmdict-p2.db`
- `../jmdict.db`
- JMdict 突合ツール

## 中間生成物

- `exclude-candidates.tsv`
- `excluded-yomi.tsv`

## 生成物

- `dict-jmdict-ex.db`

## 方針

- `exclude-candidates.tsv` は、JMdict に使用例が見つからない読み候補です。
- `excluded-yomi.tsv` は実際に除外する読みの一覧です。
- 現在の `ex-g*.sql` や `exclude-all.sql` に相当する内容は、
  将来的にこのフェーズの生成物へ整理します。
- `dict-jmdict-ex.db` は `dict-jmdict-p2.db` に除外対象の読みを反映した中間 DB です。
