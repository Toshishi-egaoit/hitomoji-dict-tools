#!/bin/bash

DB=dict-jmdict-p1.db
JMDB=../jmdict.db
LIMIT=5
WHERE=

while getopts "d:j:l:w:" opt ; do
	case "$opt" in
	d)
		DB=$OPTARG
		;;
	j)
		JMDB=$OPTARG
		;;
	l)
		LIMIT=$OPTARG
		;;
	w)
		WHERE=$OPTARG
		;;
	*)
		echo "usage: list-missing-yomi.sh [-d DB] [-j JMdict-db] [-l LIMIT] -w SQL-where" >&2
		exit 1
		;;
	esac
done

case "$LIMIT" in
*[!0-9]*|"")
	echo "LIMIT must be a number: $LIMIT" >&2
	exit 1
	;;
esac

if [ "$WHERE" = "" ] ; then
	echo "usage: list-missing-yomi.sh [-d DB] [-j JMdict-db] [-l LIMIT] -w SQL-where" >&2
	exit 1
fi

CHARS=$(sqlite3 -noheader "$DB" "select group_concat(letter, '') from (select letter from k where $WHERE order by grade, cp)")

if [ "$CHARS" = "" ] ; then
	exit 0
fi

../dictmatch -l "$LIMIT" -d "$DB" -j "$JMDB" "$CHARS" 2>&1 >/dev/null |
  awk -F '\t' -v limit="$LIMIT" '$1=="MISSING_CANDIDATE" && $4 >= limit'
