DB = dict-jmdict-p2.db
P1DB = ../p1/dict-jmdict-p1.db
KDB = ../../kdic2/dict-kdic2.db
JMDB = ../jmdict.db
DICTMATCH = ../dictmatch

STAGES = gkyo gjo

.PHONY: all clean counts
.SECONDARY: $(STAGES:%=ex-%.sql)

all: $(DB)

$(DB): $(P1DB) $(STAGES:%=ex-%.sql)
	rm -f $@
	cp $(P1DB) $@
	for s in $(STAGES); do sqlite3 $@ < ex-$$s.sql; done

ex-gkyo.tsv: list-no-evidence-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	./list-no-evidence-yomi.sh -d $(KDB) -j $(JMDB) -w 'grade between 1 and 6' > $@

ex-gjo.tsv: list-no-evidence-yomi.sh $(DICTMATCH) $(JMDB) $(KDB)
	./list-no-evidence-yomi.sh -d $(KDB) -j $(JMDB) -w 'grade = 8' > $@

$(DICTMATCH):
	$(MAKE) -C .. dictmatch

ex-%.sql: ex-%.tsv make-p2-sql.sh
	./make-p2-sql.sh $* < $< > $@

counts: $(DB)
	sqlite3 $(DB) \
	  'select "y", count(*) from y;' \
	  'select "y_exclude", count(*) from y_exclude;' \
	  'select coalesce(cast(k.grade as text), "NULL"), count(*) from y_exclude inner join k on k.cp = y_exclude.cp group by k.grade order by k.grade;'

clean:
	rm -f $(DB)
	rm -f ex-*.tsv
	rm -f ex-*.sql
