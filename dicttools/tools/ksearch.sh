#!/bin/bash

ROOT=$(cd "$(dirname "$0")/.." && pwd)
KDIC2_XML="$ROOT/kdic2/kdic2.small.xml"

if [ "$1" == "" ] ; then
	echo 'search.sh <KANJI>'
	exit 1;
fi

if [ "$1" == "-u" ] ; then
	cp=$2;
	xmlstarlet sel -t -m "//character/codepoint[cp_value='$cp']" -c . "$KDIC2_XML"
	exit $?;
fi

xmlstarlet sel -t -m "//character[literal='$1']" -c . "$KDIC2_XML"
