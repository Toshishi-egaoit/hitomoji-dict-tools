#!/bin/bash

STAGE=$1

case "$STAGE" in
g1|g2|g3|g4|g5|g6|gjo|gjin)
	TABLE=y_$STAGE
	;;
*)
	echo "usage: make-p1-sql.sh {g1|g2|g3|g4|g5|g6|gjo|gjin} < stage.tsv" >&2
	exit 1
	;;
esac

awk -F '\t' -v table="$TABLE" '
function quote(s) {
	gsub(/\047/, "\047\047", s)
	return "\047" s "\047"
}
BEGIN {
	print "begin;"
	print "delete from " table ";"
}
$1 == "MISSING_CANDIDATE" {
	print "insert or ignore into " table " (cp, yomi, wd_cnt)"
	print "values ((select cp from k where letter = " quote($2) "), " quote($3) ", " $4 ");"
}
END {
	print "commit;"
}
'
