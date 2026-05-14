# makefile for create hitomoji.dic


.PHONY: clean viewtest jmdict-tsv jmdict-db

# 辞書の作成
hitomoji.dic : dict.db dictgen
	./dictgen $< $@ 
	
viewtest: hitomoji.dic dictview
	./dictview しょう


jmdict/add_jmdict_yomi.sql:
	(cd jmdict ; make add_unreg_yomi.sql)

jmdict/jm_k.tsv jmdict/jm_r.tsv jmdict/jm_restr.tsv: jmdict/JMdict.xml jmdict/make.tsv.sh
	(cd jmdict ; ./make_jmdict_tsv.sh)

jmdict/jmdict.db: jmdict/jm_k.tsv jmdict/jm_r.tsv jmdict/jm_restr.tsv jmdict/create.sql jmdict/make.db.sh
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

# DBの構築
dict.db : g8/g8.xml.k.sql g8/g8.xml.y.sql \
	g8/yomi.daku.sql g8/yomi.han.sql create.sql \
	slot_greed make.db.sh \
	my-corpus/corpus.db \
	jmdict/add_unreg_yomi.sql
	./make.db.sh

my-corpus/corpus.db: my-corpus/source.txt
	(cd my-corpus; ./make.corpus.db.sh)

slot_greed : slot_greed.cpp
	g++ $< -o $@ -lsqlite3

g8/yomi.daku.sql : g8/yomi.daku.tsv
	(cd g8 ;../make.tsv2sql.sh yomi.daku)

g8/yomi.han.sql : g8/yomi.han.tsv
	(cd g8 ;../make.tsv2sql.sh yomi.han)

g8/g8.xml.k.sql g8/g8.xml.y.sql : g8/g8.xml make.sql.sh
	(cd g8 ;../make.sql.sh)

g8/g8.xml : kdic2.small.xml 
	xmlstarlet sel -t \
	  -e 'g8' -n \
	  -m '//character[misc/grade <=8]' \
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
