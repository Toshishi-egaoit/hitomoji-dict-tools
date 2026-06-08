# build

最終的な辞書生成に向けた統合フェーズです。

このディレクトリでは、`kdic2` と `jmdict/p4` までの成果を入力にし、
頻度補正、slot 決定、バイナリ辞書生成へ進みます。手作業による判断結果はここでは
作らず、上流フェーズの入力として管理します。

## 入力

- `../kdic2/dict-kdic2.db`
- `../jmdict/p4/dict-jmdict-p4.db`
- `../my-corpus/corpus.db`

## 生成物

- `dict-merged.db`
- `dict-slotted.db`
- `../hitomoji.dic`

## ツール

- `dictgen.cpp`
  SQLite3 の辞書 DB から IME が読むバイナリ辞書を生成する。

- `slot_greed.cpp`
  各文字の slot を決定し、結果を SQLite3 の辞書 DB に反映する。

## 方針

- このディレクトリの DB と辞書ファイルは再生成可能な成果物です。
- `dict-merged.db` は `jmdict/p4/dict-jmdict-p4.db` をコピーした統合 DB です。
- 現時点では `grade between 1 and 8` の漢字だけを最終辞書対象にします。
- 人名用漢字まで含める場合は `KANJI_WHERE='grade between 1 and 10'` にできますが、
  現行の `MAX_SLOT=120` では一部の読みが溢れるため、slot 方針の追加整理が必要です。
- `dict-slotted.db` は `slot_greed` によって slot を決定した後の DB です。
- `../hitomoji.dic` は IME が読む最終バイナリ辞書です。
- `dictgen.cpp` と `slot_greed.cpp` は、このディレクトリで管理する。
