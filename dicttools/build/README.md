# build

最終的な辞書生成に向けた統合フェーズの生成物を置くディレクトリです。

このディレクトリでは、各フェーズの DB を統合し、頻度補正、slot 決定、
バイナリ辞書生成へ進みます。手作業による判断結果はここでは作らず、
上流フェーズの入力として管理します。

## 入力

- `../kdic2/dict-kdic2.db`
- `../jmdict/p1/dict-jmdict-p1.db`
- `../jmdict/p2/dict-jmdict-p2.db`
- `../jmdict/ex/dict-jmdict-ex.db`
- `../my-corpus/corpus.db`

## 生成物

- `dict-merged.db`
- `dict-slotted.db`
- `hitomoji.dic`

## ツール

- `dictgen.cpp`
  SQLite3 の辞書 DB から IME が読むバイナリ辞書を生成する。

- `slot_greed.cpp`
  各文字の slot を決定し、結果を SQLite3 の辞書 DB に反映する。

## 方針

- このディレクトリの DB と辞書ファイルは、原則として再生成可能な成果物です。
- `dict-merged.db` は各フェーズの結果を統合した DB です。
- `dict-slotted.db` は `slot_greed` によって slot を決定した後の DB です。
- `hitomoji.dic` は IME が読む最終バイナリ辞書です。
- `../dictgen.cpp` と `../slot_greed.cpp` は、将来的にこのディレクトリへ移動する。
