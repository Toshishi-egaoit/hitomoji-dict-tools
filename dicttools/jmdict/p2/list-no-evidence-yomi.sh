#!/bin/bash

DB=../p1/dict-jmdict-p1.db
JMDB=../jmdict.db
WHERE=

while getopts "d:j:w:" opt ; do
	case "$opt" in
	d)
		DB=$OPTARG
		;;
	j)
		JMDB=$OPTARG
		;;
	w)
		WHERE=$OPTARG
		;;
	*)
		echo "usage: list-no-evidence-yomi.sh [-d DB] [-j JMdict-db] -w SQL-where" >&2
		exit 1
		;;
	esac
done

if [ "$WHERE" = "" ] ; then
	echo "usage: list-no-evidence-yomi.sh [-d DB] [-j JMdict-db] -w SQL-where" >&2
	exit 1
fi

CHARS=$(sqlite3 -noheader "$DB" "select group_concat(letter, '') from (select letter from k where $WHERE order by grade, cp)")

if [ "$CHARS" = "" ] ; then
	exit 0
fi

../dictmatch -d "$DB" -j "$JMDB" "$CHARS" 2>&1 >/dev/null |
  awk -F '\t' '$1=="NO_EVIDENCE"'
