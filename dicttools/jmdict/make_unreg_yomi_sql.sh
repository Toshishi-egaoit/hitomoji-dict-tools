#!/bin/sh

# JMdictを用いて、未登録の読みリストをSQL化するスクリプト
# usage make_sql.sh SOURCE 

if [ "$1" == "" ] ; then
	echo "usage: make_unreg_yomi_sql.sh source(g*.choice)"
	exit 1;
fi

if [ -f "$1.choice" ] ; then
	PREFIX=$1;
elif [ -f "$1" ] ; then
	PREFIX=${1%.choice}
else 
	echo "usage: make_unreg_yomi_sql.sh source(g*.choice)"
	exit 1;
fi

sed -e 's/^/FUNC(/' -e 's/\t/,/g' -e 's/$/)/' < $PREFIX.choice > $PREFIX.m4

cat <<'EEOOFF' > jmdict.m4 
define(FUNC, insert into y_jmdict ( cp, yomi, wd_cnt) values ((select cp from k where letter = "$2"),"$3", $4);)
EEOOFF

m4 jmdict.m4 $PREFIX.m4 > $PREFIX.sql
