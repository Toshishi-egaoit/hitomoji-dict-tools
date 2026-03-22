# makefile for create hitomoji.dic

viewtest: hitomoji.dic
	./dictview -d $< しょう

clean :
	rm hitomoji.dic
	rm dist.db
	rm slot_greed
	rm dictgen
	rm dictview
	rm dictedit
	rm g8/g8.xml.k.sql
	rm g8/g8.xml.y.sql
	rm g8/yomi.daku.sql
	rm g8/g8.xml
	rm kdic2.small.xml

dictedit : dictedit.cpp
	g++ $< -o $@

dictview : dictview.cpp
	g++ $< -o $@

# 辞書の作成
hitomoji.dic : dist.db dictgen
	./dictgen -d $< -o $@ 
	
dictgen : dictgen.cpp
	g++ $< -o $@ -lsqlite3

# DBの構築
dist.db : g8/g8.xml.k.sql g8/g8.xml.y.sql g8/yomi.daku.sql create.sql slot_greed
	rm dist.db
	sqlite3 dist.db < create.sql
	sqlite3 dist.db < EEOOFF
	begin;
	g8/g8.xml.k.sql
	g8/g8.xml.y.sql
	g8/yomi.daku.sql
	commit;
	EEOOFF
	./slot_greed -d dist.db

slot_greed : slot_greed.cpp
	g++ $< -o $@ -lsqlite3

g8/yomi.daku.sql : g8/yomi.daku.tsv
	(cd g8 ;../make.daku.sql.sh)

g8/g8.xml.k.sql g8/g8.xml.y.sql : g8/g8.xml make.sql.sh
	(cd g8 ;../make.sql.sh)

g8/g8.xml : kdic2.small.xml 
	xmlstarlet sel -t -e 'g8' -n \
	  -m '//character[misc/grade <=8]' \
	  -c . -n \
	  < $< > $@

kdic2.small.xml : archive/kanjidic2.xml
	xmlstarlet sel -t \
	  -m "//character" \
	  -e "<character>" \
	  -c "literal" \
	  -c "codepoint" \
	  -c "radical" \
	  -c "misc" \
	  -c "reading_meaning/rmgroup/reading[@r_type='ja_on' or @r_type='ja_kun']" \
	  -b \
	  -n \
	  archive/kanjidic2.xml > kdic2.small.xml
