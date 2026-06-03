#!/bin/bash

STAGE=$1
REASON='no evidence at JMdict'

case "$STAGE" in
gkyo)
	WHERE='grade between 1 and 6'
	;;
gjo)
	WHERE='grade = 8'
	;;
gjin)
	WHERE='grade in (9, 10)'
	;;
*)
	echo "usage: make-p2-sql.sh {gkyo|gjo|gjin} < ex-stage.tsv" >&2
	exit 1
	;;
esac

awk -F '\t' -v reason="$REASON" -v where="$WHERE" '
function quote(s) {
	gsub(/\047/, "\047\047", s)
	return "\047" s "\047"
}
BEGIN {
	print "begin;"
	print "delete from y_exclude"
	print "where reason = " quote(reason)
	print "  and cp in (select cp from k where " where ");"
}
$1 == "NO_EVIDENCE" {
	print "insert or ignore into y_exclude (cp, tp, yomi, reason)"
	print "values ((select cp from k where letter = " quote($2) "), " quote($3) ", " quote($4) ", " quote(reason) ");"
}
END {
	print "commit;"
}
'
