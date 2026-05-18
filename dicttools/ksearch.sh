#!/bin/sh

if [ "$1" == "" ] ; then
	echo 'search.sh <KANJI>'
	exit 1;
fi

if [ "$1" == "-u" ] ; then
	cp=$2;
	xmlstarlet sel -t -m "//character/codepoint[cp_value='$cp']" -c . kdic2.small.xml
	exit $?;
fi

xmlstarlet sel -t -m "//character[literal='$1']" -c . kdic2.small.xml
