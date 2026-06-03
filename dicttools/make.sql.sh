#!/bin/bash
# kdic-small.xml からsqlを生成するスクリプト
# 使用するツールは以下の通り。
# * xmlstarlet
# * sed
# * m4


if [ "$1" == "" ] ; then
	echo "usage: make.sql.sh source-XML";
	echo "   ex: make.sql.sh g9.xml"
	exit 1;
fi

SOURCE=$1

# このスクリプトではxmlから以下を抽出
# * literal
# * misc/grade
# * misc/freq
# * codepoint/cp_value[@cp_type="ucs"]
# * reading (jp_on / jp_kunの両方)
# なお、以下の記述を使えば、xmlstarletのみで処理できるが、見通しが悪いので採用していない
#	-v "translate(substring-before(concat(., '.'), '.'), '-','')" -o "," \
#	-v "substring-after(., '.') " \

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


xmlstarlet sel -t \
	-m '//character' \
	-m 'reading' \
		-v "../literal" -o "," \
		-v ../misc/grade  -o "," \
		-v ../misc/freq  -o "," \
		-v '../codepoint/cp_value[@cp_type="ucs"]'  -o "," \
		-v 'substring-after(@r_type,"_")'  -o "," \
		-v "."   -n \
	$SOURCE > $SOURCE.csv
if [ "$1" == "xml" -o "$1" == "xmlstarlet" ] ; then
	exit; 
fi

sed \
	-e 's/^/FUNC(/' \
	-e 's/$/)/' \
	-e 's/\./,/' \
	-e 's/-//' \
	-e 'y/アイウエオカキクケコサシスセソ/あいうえおかきくけこさしすせそ/' \
	-e 'y/タチツテトナニヌネノハヒフヘホ/たちつてとなにぬねのはひふへほ/' \
	-e 'y/マミムメモヤユヨラリルレロ/まみむめもやゆよらりるれろ/' \
	-e 'y/ワヲンッャュョ/わをんっゃゅょ/' \
	-e 'y/ガギグゲゴザジズゼゾダヂヅデド/がぎぐげござじずぜぞだぢづでど/' \
	-e 'y/バビブベボパピプペポ/ばびぶべぼぱぴぷぺぽ/' \
	< $SOURCE.csv > $SOURCE.m4

if [ "$1" == "sed" ] ; then
	exit; 
fi

cat <<'EEOOFF' > macro_k.m4
# table k insertion (#1=literal, #2=grade, #3=freq, #4=codepoint )
define(FREQ, `ifelse($1,,3000 ,$1)')
define(FUNC, insert into k ( letter, grade, freq , cp) values ( '$1', $2, `FREQ($3)', 0x$4);)
EEOOFF

m4 macro_k.m4 $SOURCE.m4  | uniq > $SOURCE.k.sql

cat <<'EEOOFF' > macro_y.m4
# table k insertion ($4=codepoint , $5=type, $6=yomi, $7=okuri)
define(FUNC, insert into y_base ( cp , tp , yomi, okuri ) values ( 0x$4, '$5' , '$6', '$7') ;)
EEOOFF

m4 macro_y.m4 $SOURCE.m4  > $SOURCE.y.sql
