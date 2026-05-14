#!/bin/sh

DB=jmdict.db

for f in jm_k.tsv jm_r.tsv jm_restr.tsv ; do
	if [ ! -f "$f" ] ; then
		echo "not found: $f" >&2
		echo "run ./make.tsv.sh first" >&2
		exit 1
	fi
done

rm -f "$DB"

sqlite3 "$DB" <<'EEOOFF'
.read create.sql
.mode tabs
.import jm_k.tsv jm_k
.import jm_r.tsv jm_r
.import jm_restr.tsv jm_restr
EEOOFF

sqlite3 "$DB" <<'EEOOFF'
select 'jm_k', count(*) from jm_k;
select 'jm_r', count(*) from jm_r;
select 'jm_restr', count(*) from jm_restr;
select 'jm_pair', count(*) from jm_pair;
EEOOFF
