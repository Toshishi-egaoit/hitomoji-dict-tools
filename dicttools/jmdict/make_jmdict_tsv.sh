#!/bin/sh

SOURCE=JMdict.xml

if [ ! -f "$SOURCE" ] ; then
	echo "not found: $SOURCE" >&2
	exit 1
fi

# ent_seq, keb
xmlstarlet sel -t \
	-m '/JMdict/entry/k_ele' \
		-v '../ent_seq' -o '	' \
		-v 'keb' -n \
	"$SOURCE" > jm_k.tsv

# ent_seq, reb, re_nokanji
xmlstarlet sel -t \
	-m '/JMdict/entry/r_ele' \
		-v '../ent_seq' -o '	' \
		-v 'reb' -o '	' \
		-v 'count(re_nokanji)' -n \
	"$SOURCE" > jm_r.tsv

# ent_seq, reb, restricted keb
xmlstarlet sel -t \
	-m '/JMdict/entry/r_ele/re_restr' \
		-v '../../ent_seq' -o '	' \
		-v '../reb' -o '	' \
		-v '.' -n \
	"$SOURCE" > jm_restr.tsv

wc -l jm_k.tsv jm_r.tsv jm_restr.tsv
