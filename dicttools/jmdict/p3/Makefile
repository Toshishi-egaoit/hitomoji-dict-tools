P3_LIMIT ?= 3
P3_FILTER ?= all
DB = dict-jmdict-p3.db
P2DB = ../p2/dict-jmdict-p2.db
JMDB = ../jmdict.db
DICTMATCH = ../dictmatch
P3FILTER = p3filter

# Editor for editing g*.tsv into g*.accept.tsv.
# CAUTION: If you change this editor, update the commands in edit-% too.
VIM = vim

STAGES = g1 g2 g3 g4 g5 g6

.PHONY: all clean clean-tsv counts
.SECONDARY: $(STAGES:%=%.raw.tsv) $(STAGES:%=%.accept.tsv) $(STAGES:%=%.sql)

all: $(DB)

$(DB): $(P2DB) create-p3.sql $(STAGES:%=%.sql)
	rm -f $@
	cp $(P2DB) $@
	sqlite3 $@ < create-p3.sql
	for s in $(STAGES); do sqlite3 $@ < $$s.sql; done

g1.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	bash ../list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 1' > $@

g2.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	bash ../list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 2' > $@

g3.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	bash ../list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 3' > $@

g4.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	bash ../list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 4' > $@

g5.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	bash ../list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 5' > $@

g6.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P2DB)
	bash ../list-missing-yomi.sh -d $(P2DB) -j $(JMDB) -l $(P3_LIMIT) -w 'grade = 6' > $@

$(DICTMATCH):
	$(MAKE) -C .. dictmatch

$(P3FILTER): p3filter.cpp
	g++ $< -o $@ -lsqlite3

define FILTER_RULE
$1.tsv $1.reject.tsv: $1.raw.tsv $$(P3FILTER)
	./$$(P3FILTER) -d $$(P2DB) -r $$(P3_FILTER) --reject $1.reject.tsv < $1.raw.tsv > $1.tsv
endef

$(foreach s,$(STAGES),$(eval $(call FILTER_RULE,$(s))))

%.accept.tsv: %.tsv
	@echo "You MUST edit $< into $@" >&2
	@echo "Run: make edit-$*" >&2
	@false

edit-%:
	@if [ -f $*.accept.tsv ] && { [ ! -f $*.tsv ] || [ $*.accept.tsv -nt $*.tsv ]; }; then \
		$(VIM) $*.accept.tsv; \
	elif [ -f $*.accept.tsv ]; then \
		echo "$*.accept.tsv exists but is older than $*.tsv" >&2; \
		echo "Inspect both files before replacing the accept file." >&2; \
		exit 1; \
	else \
		$(MAKE) $*.tsv; \
		$(VIM) $*.tsv +'saveas $*.accept.tsv'; \
	fi
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
	  'select "y_p3_g6", count(*) from y_p3_g6;'

clean:
	rm -f $(DB)
	rm -f $(STAGES:%=%.sql)
	rm -f $(P3FILTER)

clean-tsv:
	rm -f $(STAGES:%=%.raw.tsv) $(STAGES:%=%.tsv) $(STAGES:%=%.reject.tsv) $(STAGES:%=%.sql)
