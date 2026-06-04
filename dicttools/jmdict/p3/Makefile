P3_LIMIT ?= 3
DB = dict-jmdict-p3.db
P2DB = ../p2/dict-jmdict-p2.db
JMDB = ../jmdict.db
DICTMATCH = ../dictmatch

# Editor for editing g*.tsv into g*.accept.tsv.
# CAUTION: If you change this editor, update the +saveas command in edit-% too.
VIM = vim

STAGES = g1 g2 g3 g4 g5 g6 gjo gjin

.PHONY: all clean clean-tsv counts
.SECONDARY: $(STAGES:%=%.accept.tsv) $(STAGES:%=%.sql)

all: $(DB)

$(DB): $(P2DB) create-p3.sql $(STAGES:%=%.sql)
	rm -f $@
	cp $(P2DB) $@
	sqlite3 $@ < create-p3.sql
	for s in $(STAGES); do sqlite3 $@ < $$s.sql; done

g1.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 1' > $@

g2.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 2' > $@

g3.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 3' > $@

g4.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 4' > $@

g5.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 5' > $@

g6.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 6' > $@

gjo.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 8' > $@

gjin.tsv: list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	./list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade in (9, 10)' > $@

$(DICTMATCH):
	$(MAKE) -C .. dictmatch

%.accept.tsv: %.tsv
	@echo "You MUST edit $< into $@" >&2
	@echo "Run: make edit-$*" >&2
	@false

edit-%: %.tsv
	$(VIM) $< +'saveas $*.accept.tsv'
	test -f $*.accept.tsv

%.sql: %.accept.tsv make-p3-sql.sh
	./make-p3-sql.sh $* < $< > $@

counts: $(DB)
	sqlite3 $(DB) \
	  'select "y", count(*) from y;' \
	  'select "y_p3_g1", count(*) from y_p3_g1;' \
	  'select "y_p3_g2", count(*) from y_p3_g2;' \
	  'select "y_p3_g3", count(*) from y_p3_g3;' \
	  'select "y_p3_g4", count(*) from y_p3_g4;' \
	  'select "y_p3_g5", count(*) from y_p3_g5;' \
	  'select "y_p3_g6", count(*) from y_p3_g6;' \
	  'select "y_p3_gjo", count(*) from y_p3_gjo;' \
	  'select "y_p3_gjin", count(*) from y_p3_gjin;'

clean:
	rm -f $(DB)
	rm -f *.sql

clean-tsv:
	rm -f *.tsv *.sql
