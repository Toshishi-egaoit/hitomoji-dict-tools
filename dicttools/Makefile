# makefile for create hitomoji.dic


.PHONY: clean viewtest jmdict-tsv jmdict-db build-curation apply-curation

# 辞書の作成
hitomoji.dic : dict.db dictgen
	./dictgen $< $@ 
	
viewtest: hitomoji.dic dictview
	./dictview しょう

build-curation:
	mkdir -p curation/jmdict/p3 curation/jmdict/p4
	for f in jmdict/p3/*.accept.tsv; do \
		base=$${f##*/}; \
		cp "$$f" "curation/jmdict/p3/$${base%.accept.tsv}.tsv"; \
	done
	for f in jmdict/p4/*.accept.tsv; do \
		base=$${f##*/}; \
		cp "$$f" "curation/jmdict/p4/$${base%.accept.tsv}.tsv"; \
	done
	cp hitomoji.dic curation/hitomoji.dic

apply-curation:
	mkdir -p jmdict/p3 jmdict/p4
	for f in curation/jmdict/p3/*.tsv; do \
		base=$${f##*/}; \
		cp "$$f" "jmdict/p3/$${base%.tsv}.accept.tsv"; \
	done
	for f in curation/jmdict/p4/*.tsv; do \
		base=$${f##*/}; \
		cp "$$f" "jmdict/p4/$${base%.tsv}.accept.tsv"; \
	done
	cp curation/hitomoji.dic hitomoji.dic


jmdict/add_unreg_yomi.sql:
	(cd jmdict ; make add_unreg_yomi.sql)

jmdict/jm_k.tsv jmdict/jm_r.tsv jmdict/jm_restr.tsv: archive/JMdict.xml jmdict/make_jmdict_tsv.sh
	(cd jmdict ; ./make_jmdict_tsv.sh)

jmdict/jmdict.db: jmdict/jm_k.tsv jmdict/jm_r.tsv jmdict/jm_restr.tsv jmdict/create.sql jmdict/make_jmdict_db.sh
	(cd jmdict ; ./make_jmdict_db.sh)

clean:
	(cd jmdict ; make clean)
	rm -f dictgen dictview dictedit slot_greed
	rm -f hitomoji.dic dict.db
	rm -f g8/*.sql g8/g8.xml g8/*.m4
	rm -f kdic2.small.xml

# バイナリ辞書生成プログラム
dictgen : dictgen.cpp
	g++ $< -o $@ -lsqlite3

# 辞書データ編集プログラム
dictedit : dictedit.cpp
	g++ $< -o $@

# 辞書えつ覧プログラム
dictview : dictview.cpp
	g++ $< -o $@

GRADE=g9
# DBの構築
dict.db : $(GRADE)/$(GRADE).xml.k.sql $(GRADE)/$(GRADE).xml.y.sql \
	create.sql \
	slot_greed make.db.sh \
	my-corpus/corpus.db \
	jmdict/add_unreg_yomi.sql \
	jmdict/dictmatch jmdict/exclude_no_evidence_yomi.sh \
	jmdict/jmdict.db
	./make.db.sh $(GRADE)

jmdict/dictmatch: jmdict/dictmatch.cpp
	(cd jmdict ; make dictmatch)

my-corpus/corpus.db: my-corpus/source.txt
	(cd my-corpus; ./make.corpus.db.sh)

slot_greed : slot_greed.cpp
	g++ $< -o $@ -lsqlite3

$(GRADE)/$(GRADE).xml.k.sql $(GRADE)/$(GRADE).xml.y.sql : $(GRADE)/$(GRADE).xml make.sql.sh
	(cd $(GRADE) ;../make.sql.sh $(GRADE).xml) 

$(GRADE)/$(GRADE).xml : kdic2.small.xml 
	xmlstarlet sel -t \
	  -e '$(GRADE)' -n \
	  -m '//character[misc/grade <=9]' \
	  -c . -n \
	  -b -n \
	  < $< > $@

kdic2.small.xml : archive/kanjidic2.xml
	xmlstarlet sel -t \
	  -e "small-kdic2" -n \
	  -m "//character" \
	  -e "character" -n \
	  -c "literal" \
	  -c "codepoint" \
	  -c "radical" \
	  -c "misc" \
	  -c "reading_meaning/rmgroup/reading[@r_type='ja_on' or @r_type='ja_kun']" \
	  -b -n \
	  -b -n \
	  archive/kanjidic2.xml > kdic2.small.xml
