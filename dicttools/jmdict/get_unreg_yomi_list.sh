#!/bin/sh

# JMdictを用いて、未登録の読みを検索するスクリプト
# usage get_unreg_yomi_list.sh [-l LIMIT] "XPATH STRING"

LIMIT=4

while getopts "l:" opt ; do
	case "$opt" in
	l)
		LIMIT=$OPTARG
		;;
	*)
		echo "usage : get_unreg_yomi_list.sh [-l LIMIT] XPATH-string";
		echo "        XPATH example: //character/misc[grade = '1']";
		exit 1;
		;;
	esac
done
shift $((OPTIND - 1))

case "$LIMIT" in
*[!0-9]*|"")
	echo "LIMIT must be a number: $LIMIT" >&2
	exit 1;
	;;
esac

XPATH=$1

xmlstarlet sel -t \
    -m "$XPATH" \
    -v ../literal -n \
    < ../kdic2.small.xml |
  while read CHAR; do
    ./dictmatch -d ../dict.db -j jmdict.db "$CHAR" 2>&1 >/dev/null |
    awk -F '\t' -v limit="$LIMIT" '$1=="MISSING_CANDIDATE" && $4 >= limit'
  done

exit 0;
