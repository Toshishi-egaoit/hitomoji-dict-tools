.PHONY: all clean

all: jmdict.db dictmatch

jmdict.db: jm_k.tsv jm_restr.tsv jm_r.tsv make_jmdict_db.sh create.sql
	./make_jmdict_db.sh

jm_k.tsv jm_restr.tsv jm_r.tsv: ../archive/JMdict.xml make_jmdict_tsv.sh
	./make_jmdict_tsv.sh

dictmatch: dictmatch.cpp
	g++ $< -o $@ -lsqlite3

clean:
	rm -f dictmatch
	rm -f jmdict.db jm_k.tsv jm_restr.tsv jm_r.tsv
