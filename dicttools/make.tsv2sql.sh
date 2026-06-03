#!/bin/bash
# yomi.daku.tsv ファイルをy_dakuonのにinsertするSQLに変換する。
# 使用するツールは以下の通り。
# * sed
# * m4

if [ "$1" == "" ] ; then
	echo 'usage: make.daku.sql.sh TARGET' ;
	exit ;
fi

SOURCE=$1

# その後、sedで以下を行う。
# * 送り仮名の分割
# * 続きを示す"-"の削除
# * m4向けに行頭にFUNC( を埋め込み
# * xmlでのカタカナ表記（音読み）をひらがなに変換

# その後、m4で漢字定義のSQLを作成
# * FREQマクロ,FUNCマクロを定義して、freq値がNullのものを3000に置換
# これをuniqして $SOURCE.k.sqlに展開

# 次に読みのデータのSQLを作成
# * FUNCマクロを定義して、$SOURCE.y.sqlに展開


sed \
	-e 's/\t/,/g' \
	-e 's/^/FUNC(/' \
	-e 's/$/)/' \
	< $SOURCE.tsv > $SOURCE.m4

if [ "$1" == "sed" ] ; then
	exit; 
fi

cat <<'EEOOFF' > macro_d.m4
-- table k insertion (#1=literal, #2=cp, #3=yomi(daku), #4=yomi(sei) )
define(FUNC, insert into y_dakuon ( cp, yomi, yomi_seion ) values ( $2, '$3', '$4');)
EEOOFF

m4 macro_d.m4 $SOURCE.m4  | uniq > $SOURCE.sql
