# Makefile for KANJIDIC2 base dictionary phase.

GROUPS = gkyo gjo gjin goth
SQLS = $(foreach g,$(GROUPS),$(g)/$(g).xml.k.sql $(g)/$(g).xml.y.sql $(g)/$(g).xml.nanori.sql)

.PHONY: all clean counts

all: dict-kdic2.db

dict-kdic2.db: create.sql $(SQLS)
	rm -f $@
	sqlite3 $@ < create.sql
	{ \
	  echo begin\; ; \
	  for g in $(GROUPS); do \
	    echo ".read $$g/$$g.xml.k.sql" ; \
	    echo ".read $$g/$$g.xml.y.sql" ; \
	    echo ".read $$g/$$g.xml.nanori.sql" ; \
	  done ; \
	  echo commit\; ; \
	} | sqlite3 $@

counts: dict-kdic2.db
	sqlite3 $< \
	  'select count(*) from k;' \
	  'select grade, count(*) from k group by grade order by grade;' \
	  'select jis_level, count(*) from k group by jis_level order by jis_level;' \
	  'select count(*) from y_base;' \
	  'select count(*) from y_nanori;' \
	  'select count(*) from y;'

kdic2.small.xml: ../archive/kanjidic2.xml
	xmlstarlet sel -t \
	  -e "small-kdic2" -n \
	  -m "//character" \
	  -e "character" -n \
	  -c "literal" \
	  -c "codepoint" \
	  -c "radical" \
	  -c "misc" \
	  -c "reading_meaning/rmgroup/reading[@r_type='ja_on' or @r_type='ja_kun']" \
	  -c "reading_meaning/nanori" \
	  -b -n \
	  -b -n \
	  $< > $@

gkyo/gkyo.xml: kdic2.small.xml
	mkdir -p $(@D)
	xmlstarlet sel -t -e "gkyo" -n -m "//character[misc/grade <= 6]" -c . -n -b -n < $< > $@

gjo/gjo.xml: kdic2.small.xml
	mkdir -p $(@D)
	xmlstarlet sel -t -e "gjo" -n -m "//character[misc/grade = 8]" -c . -n -b -n < $< > $@

gjin/gjin.xml: kdic2.small.xml
	mkdir -p $(@D)
	xmlstarlet sel -t -e "gjin" -n -m "//character[misc/grade = 9 or misc/grade = 10]" -c . -n -b -n < $< > $@

goth/goth.xml: kdic2.small.xml
	mkdir -p $(@D)
	xmlstarlet sel -t -e "goth" -n -m "//character[not(misc/grade) or misc/grade > 10]" -c . -n -b -n < $< > $@

%.xml.k.sql %.xml.y.sql %.xml.nanori.sql: %.xml make-kdic2-sql.sh
	cd $(@D) && ../make-kdic2-sql.sh $(notdir $<)

clean:
	rm -f dict-kdic2.db kdic2.small.xml
	rm -rf $(GROUPS)
