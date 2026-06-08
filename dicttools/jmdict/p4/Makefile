P4_LIMIT ?= 3
P4_FILTER ?= all
DB = dict-jmdict-p4.db
P3DB = ../p3/dict-jmdict-p3.db
JMDB = ../jmdict.db
DICTMATCH = ../dictmatch
P3FILTER = ../p3/p3filter

# Editor for editing g*.tsv into g*.accept.tsv.
# CAUTION: If you change this editor, update the commands in edit-% too.
VIM = vim

STAGES = gjo gjin

.PHONY: all clean clean-tsv counts
.SECONDARY: $(STAGES:%=%.raw.tsv) $(STAGES:%=%.accept.tsv) $(STAGES:%=%.sql)

all: $(DB)

$(DB): $(P3DB) create-p4.sql $(STAGES:%=%.sql)
	rm -f $@
	cp $(P3DB) $@
	sqlite3 $@ < create-p4.sql
	for s in $(STAGES); do sqlite3 $@ < $$s.sql; done

gjo.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P3DB)
	bash ../list-missing-yomi.sh -d $(P3DB) -j $(JMDB) -l $(P4_LIMIT) -w 'grade = 8' > $@

gjin.raw.tsv: ../list-missing-yomi.sh $(DICTMATCH) $(JMDB) $(P3DB)
	bash ../list-missing-yomi.sh -d $(P3DB) -j $(JMDB) -l $(P4_LIMIT) -w 'grade in (9, 10)' > $@

$(DICTMATCH):
	$(MAKE) -C .. dictmatch

$(P3DB):
	$(MAKE) -C ../p3 dict-jmdict-p3.db

$(P3FILTER):
	$(MAKE) -C ../p3 p3filter

define FILTER_RULE
$1.tsv $1.reject.tsv: $1.raw.tsv $$(P3FILTER)
	$$(P3FILTER) -d $$(P3DB) -r $$(P4_FILTER) --reject $1.reject.tsv < $1.raw.tsv > $1.tsv
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

%.sql: %.accept.tsv make-p4-sql.sh
	bash ./make-p4-sql.sh $* < $< > $@

counts: $(DB)
	sqlite3 $(DB) \
	  'select "y", count(*) from y;' \
	  'select "y_p4_gjo", count(*) from y_p4_gjo;' \
	  'select "y_p4_gjin", count(*) from y_p4_gjin;'

clean:
	rm -f $(DB)
	rm -f $(STAGES:%=%.sql)

clean-tsv:
	rm -f $(STAGES:%=%.raw.tsv) $(STAGES:%=%.tsv) $(STAGES:%=%.reject.tsv) $(STAGES:%=%.sql)
