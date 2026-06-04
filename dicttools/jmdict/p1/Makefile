P1_LIMIT ?= 5
P1_FILTER ?= onbin,okuri,renyo,renyo-ru,renyo-ichidan-e,renyo-rendaku
DB = dict-jmdict-p1.db
KDB = ../../kdic2/dict-kdic2.db
JMDB = ../jmdict.db
DICTMATCH = ../dictmatch
P1FILTER = p1filter

STAGES = g1 g2 g3 g4 g5 g6 gjo gjin

.PHONY: all clean clean-tsv counts
.SECONDARY: $(STAGES:%=%.accept.tsv) $(STAGES:%=%.sql)

all: $(DB)

$(DB): $(KDB) create-p1.sql $(STAGES:%=%.sql)
	rm -f $@
	cp $(KDB) $@
	sqlite3 $@ < create-p1.sql
	for s in $(STAGES); do sqlite3 $@ < $$s.sql; done

g1.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 1' > $@

g2.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 2' > $@

g3.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 3' > $@

g4.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 4' > $@

g5.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 5' > $@

g6.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 6' > $@

gjo.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade = 8' > $@

gjin.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	bash ../list-missing-yomi.sh -d $(KDB) -j $(JMDB) -l $(P1_LIMIT) -w 'grade in (9, 10)' > $@

$(P1FILTER): p1filter.cpp
	g++ $< -o $@ -lsqlite3

$(DICTMATCH):
	$(MAKE) -C .. dictmatch

%.accept.tsv: %.tsv $(P1FILTER)
	./$(P1FILTER) -d $(KDB) -r $(P1_FILTER) < $< > $@

%.sql: %.accept.tsv make-p1-sql.sh
	./make-p1-sql.sh $* < $< > $@

counts: $(DB)
	sqlite3 $(DB) \
	  'select "y", count(*) from y;' \
	  'select "y_g1", count(*) from y_g1;' \
	  'select "y_g2", count(*) from y_g2;' \
	  'select "y_g3", count(*) from y_g3;' \
	  'select "y_g4", count(*) from y_g4;' \
	  'select "y_g5", count(*) from y_g5;' \
	  'select "y_g6", count(*) from y_g6;' \
	  'select "y_gjo", count(*) from y_gjo;' \
	  'select "y_gjin", count(*) from y_gjin;'

clean:
	rm -f $(DB)
	rm -f *.sql
	rm -f *.accept.tsv
	rm -f $(P1FILTER)

clean-tsv:
	rm -f *.tsv *.accept.tsv *.sql
