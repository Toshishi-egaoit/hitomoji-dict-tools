#!/bin/bash

DICT_DB=${DICT_DB:-../kdic2/dict-kdic2.db}

if [ ! -f source.txt ]; then
	echo "ERROR: source.txt is missing" >&2
	exit 1
fi

if [ ! -f "$DICT_DB" ]; then
	echo "ERROR: dictionary DB is missing: $DICT_DB" >&2
	echo "Run: make -C ../kdic2" >&2
	exit 1
fi

# 入力ソースから各文字の使用回数を求める 
grep -oP '[\x{4E00}-\x{9FFF}]' < source.txt > corpus.base
sort corpus.base | uniq -c | sort -nr > corpus.freq

# 文字毎の使用ひん度を求めて、DBに投入する
nl corpus.freq | \
sed \
	-e 's/^ */FUNC(/' \
	-e 's/\t */,/' \
	-e 's/ /,/' \
	-e 's/$/)/' > corpus.m4

cat << 'EEOOFF' > freq.m4
define(FUNC, `insert into t (freq, cnt , letter ,cp ) select $1, $2, "$3", dict.k.cp from dict.k where dict.k.letter = "$3" ;')
EEOOFF

m4 freq.m4 corpus.m4  > corpus.sql

# dbを生成する
rm -f corpus.db
sqlite3 corpus.db << EEOOFF
.read create-freq.sql
attach database '$DICT_DB' as dict;
begin;
.read corpus.sql
commit;
EEOOFF
