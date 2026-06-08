DB_MERGED = dict-merged.db
DB_SLOTTED = dict-slotted.db
DIC = ../hitomoji.dic

KDIC2_DB = ../kdic2/dict-kdic2.db
JMDICT_DB = ../jmdict/p4/dict-jmdict-p4.db
CORPUS_DB = ../my-corpus/corpus.db

DICTGEN = dictgen
SLOT_GREED = slot_greed

BASE_CORPUS ?= 0.7
MY_CORPUS ?= 0.3
KANJI_WHERE ?= grade between 1 and 8

.PHONY: all clean counts

all: $(DIC)

$(DIC): $(DB_SLOTTED) $(DICTGEN)
	./$(DICTGEN) $(DB_SLOTTED) $@

$(DB_SLOTTED): $(DB_MERGED) $(SLOT_GREED)
	rm -f $@
	cp $(DB_MERGED) $@
	@if [ -f $(CORPUS_DB) ]; then \
		echo 'merge with my-corpus.db'; \
		sqlite3 $@ "attach database '$(CORPUS_DB)' as my; \
			begin; \
			with \
			ksum as (select sum(1.0/freq) as s from k), \
			tsum as (select sum(cnt) as s from my.t) \
			update k \
			set freq = 1.0 / ( \
			    ((1.0 / k.freq) / (select s from ksum)) * $(BASE_CORPUS) \
			    + \
			    coalesce((select cnt * 1.0 / (select s from tsum) from my.t where my.t.cp = k.cp), 0) * $(MY_CORPUS) \
			); \
			commit;"; \
	fi
	./$(SLOT_GREED) -d $@

$(DB_MERGED): $(KDIC2_DB) $(JMDICT_DB)
	rm -f $@
	cp $(JMDICT_DB) $@
	sqlite3 $@ "delete from k where coalesce(($(KANJI_WHERE)), 0) = 0;"

$(KDIC2_DB):
	$(MAKE) -C ../kdic2 dict-kdic2.db

$(JMDICT_DB):
	$(MAKE) -C ../jmdict/p4 dict-jmdict-p4.db

$(DICTGEN): dictgen.cpp
	g++ $< -o $@ -lsqlite3

$(SLOT_GREED): slot_greed.cpp
	g++ $< -o $@ -lsqlite3

counts: $(DB_SLOTTED)
	sqlite3 $(DB_SLOTTED) \
	  'select "k", count(*) from k;' \
	  'select "y", count(*) from y;' \
	  'select "slot-null", count(*) from k where slot is null;' \
	  'select "max-slot", max(slot) from k;'

clean:
	rm -f $(DB_MERGED) $(DB_SLOTTED) $(DIC)
	rm -f $(DICTGEN) $(SLOT_GREED)
