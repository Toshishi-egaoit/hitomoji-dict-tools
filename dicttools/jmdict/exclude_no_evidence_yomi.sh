#!/bin/sh

# JMdictで使用例が見つからない読みをy_excludeに登録するSQLを生成する。
# usage: exclude_no_evidence_yomi.sh [-n] [-d DB] [-j JMdict-db] "XPATH STRING"

DB=../dict.db
JMDB=jmdict.db
DRYRUN=0

while getopts "nd:j:" opt ; do
	case "$opt" in
	n)
		DRYRUN=1
		;;
	d)
		DB=$OPTARG
		;;
	j)
		JMDB=$OPTARG
		;;
	*)
		echo "usage : exclude_no_evidence_yomi.sh [-n] [-d DB] [-j JMdict-db] XPATH-string" >&2
		exit 1
		;;
	esac
done
shift $((OPTIND - 1))

XPATH=$1
if [ "$XPATH" = "" ] ; then
	echo "usage : exclude_no_evidence_yomi.sh [-n] [-d DB] [-j JMdict-db] XPATH-string" >&2
	exit 1
fi

TMP=${TMPDIR:-/tmp}/no_evidence_yomi.$$
trap 'rm -f "$TMP"' EXIT HUP INT TERM

xmlstarlet sel -t \
    -m "$XPATH" \
    -v ../literal -n \
    < ../kdic2.small.xml |
  while read CHAR; do
    ./dictmatch -d "$DB" -j "$JMDB" "$CHAR" 2>&1 >/dev/null |
    awk -F '\t' '$1=="NO_EVIDENCE"'
  done > "$TMP"

if [ ! -s "$TMP" ] ; then
	if [ "$DRYRUN" = 1 ] ; then
		echo "no NO_EVIDENCE readings"
	else
		echo "-- no NO_EVIDENCE readings"
	fi
	exit 0
fi

if [ "$DRYRUN" = 1 ] ; then
	cat "$TMP"
	exit 0
fi

awk -F '\t' '
function quote(s) {
	gsub(/\047/, "\047\047", s)
	return "\047" s "\047"
}
BEGIN {
	print "begin;"
}
$1 == "NO_EVIDENCE" {
	print "insert or ignore into y_exclude (cp, tp, yomi, reason)"
	print "values ((select cp from k where letter = " quote($2) "), " quote($3) ", " quote($4) ", \047no evidence at JMdict\047);"
}
END {
	print "commit;"
}
' "$TMP"
