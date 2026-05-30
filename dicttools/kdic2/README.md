# kdic2

KANJIDIC2 由来の基礎辞書データを扱うフェーズです。

このディレクトリでは、KANJIDIC2 から機械的に抽出できる情報だけを扱います。
JMdict による読みの補完、人手による採用判断、独自コーパスによる頻度補正、
slot の決定はこのフェーズには含めません。

## 入力

- `../archive/kanjidic2.xml`

## 中間生成物

- `kdic2.small.xml`
- `gkyo/`
  教育漢字に属する漢字に関するデータ。KANJIDIC2 の `grade <= 6`。
- `gjo/`
  常用漢字に属する漢字に関するデータ。KANJIDIC2 の `grade = 8`。
- `gjin/`
  人名用漢字に属する漢字に関するデータ。KANJIDIC2 の `grade = 9` と
  `grade = 10`。
- `goth/`
  上記以外の全ての漢字に関するデータ。KANJIDIC2 の grade がない文字、
  または `grade > 10`。


## 生成物

- `dict-kdic2.db`

## テーブル

- `k`
  KANJIDIC2 由来の漢字情報。
  `radical` には `rad_type="classical"` の部首番号を入れる。
  `strokes` には `misc/stroke_count` の先頭値を入れる。
  `jis_level` には JIS 漢字水準を入れる。

- `y_base`
  KANJIDIC2 の `ja_on`, `ja_kun` 由来の読み。

- `y_nanori`
  KANJIDIC2 の `nanori` 由来の名前読み。
  将来検証できるように DB へ保存するが、通常の `y` view には含めない。

## 実行方法

```
make -C kdic2
```

件数確認は次のように行う。

```
make -C kdic2 counts
```

## 方針

- このフェーズの生成物は、入力 XML と変換スクリプトから再生成できる状態にします。
- 人手で判断した読みや補正値は置きません。
- `gkyo`, `gjo`, `gjin`, `goth` は互いに重ならない区分として扱います。
- `gjin` は人名用漢字として `grade = 9` と `grade = 10` をまとめて扱います。
- `goth` には KANJIDIC2 の grade がない文字も含めます。その場合、DB 上の `grade`
  は `0` として扱います。
- `jis_level` は、`jis208` の 16-01 から 47-51 を第1水準、それ以外の
  `jis208` を第2水準、`jis213` 第1面を第3水準、`jis213` 第2面を第4水準として
  扱います。判定できない文字は `NULL` とします。
- `nanori` は通常変換の候補を増やしすぎる可能性があるため、抽出だけ行い、
  通常の辞書生成には使いません。
- 現行のトップレベル生成経路は当面残し、このディレクトリの生成経路と並走させます。
