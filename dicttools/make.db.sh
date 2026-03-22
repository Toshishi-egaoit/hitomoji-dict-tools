#!/bin/sh

# db を再作成
rm -f dict.db
sqlite3 dict.db < create.sql
sqlite3 dict.db << EEOOFF
begin;
.read g8/g8.xml.k.sql
.read g8/g8.xml.y.sql
.read g8/yomi.daku.sql
commit;
EEOOFF

# 独自コーパスがある場合はそれをマージ
# 割合は以下の変数で可変
# ※freq(順位)をそのまま使うと品質が悪いので、その逆数を取って、
#   さらに総レコード数の逆数で割ることで、使用率もどき(Zipf分布)
#   を得ている。
#   一方、my-corpus側は、使用率を正確に出すことで、同じ指標を得る。
#   最終的に得た値を再度逆数を取ることで、順位の合成値を得る
BASE_CORPUS=0.7
MY_CORPUS=0.3

if [ -f my-corpus/corpus.db ] ; then
	echo 'merge with my-corpus.db'
	sqlite3 dict.db << EEOOFF
	attach database 'my-corpus/corpus.db' as my;
	begin ;

	with
	ksum as (select sum(1.0/freq) as s from k),
	tsum as (select sum(cnt) as s from my.t)

	update k
	set freq = 1.0 / (
	    (
		(1.0 / k.freq) / (select s from ksum)
	    ) * $BASE_CORPUS
	    +
	    (
		coalesce(
		    (select cnt * 1.0 / (select s from tsum)
		     from my.t where my.t.cp = k.cp),
		    0
		)
	    ) * $MY_CORPUS
	);
	commit;
EEOOFF
	fi

# 上で得たfreqを用いてGreed法で採色グラフ問題を解く
# これによって、各文字のキー位置(slot)を確定させる
./slot_greed -d dict.db


