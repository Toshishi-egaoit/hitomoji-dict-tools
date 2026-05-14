
add_unreg_yomi.sql : g1.sql g2.sql g3.sql g4.sql g5.sql g6.sql
	cat g1.sql g2.sql g3.sql g4.sql g5.sql g6.sql > $@

g1.sql: g1.choice
	./make_unreg_yomi_sql.sh $<

g2.sql: g2.choice
	./make_unreg_yomi_sql.sh $<

g3.sql: g3.choice
	./make_unreg_yomi_sql.sh $<

g4.sql: g4.choice
	./make_unreg_yomi_sql.sh $<

g5.sql: g5.choice
	./make_unreg_yomi_sql.sh $<

g6.sql: g6.choice
	./make_unreg_yomi_sql.sh $<

g8.sql: g8.choice
	./make_unreg_yomi_sql.sh $<

g1.choice: g1.list
	echo "You MUST edit $<  into $@"
	@false

g2.choice: g2.list
	echo "You MUST edit $<  into $@"
	@false

g3.choice: g3.list
	echo "You MUST edit $<  into $@"
	@false

g4.choice: g4.list
	echo "You MUST edit $<  into $@"
	@false

g5.choice: g5.list
	echo "You MUST edit $<  into $@"
	@false

g6.choice: g6.list
	echo "You MUST edit $<  into $@"
	@false

g8.choice: g8.list
	echo "You MUST edit $<  into $@"
	@false

LOPT=$(if $(LIMIT), -l $(LIMIT),)

g1.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 1]' > $@

g2.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 2]' > $@

g3.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 3]' > $@

g4.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 4]' > $@

g5.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 5]' > $@

g6.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 6]' > $@

g8.list : dictmatch jmdict.db get_unreg_yomi_list.sh
	./get_unreg_yomi_list.sh $(LOPT) '//character/misc[grade = 8]' > $@

jmdict.db : jm_k.tsv jm_restr.tsv jm_r.tsv make_jmdict_db.sh
	./make_jmdict_db.sh

jm_k.tsv jm_restr.tsv jm_r.tsv: JMdict.xml make_jmdict_tsv.sh 
	./make_jmdict_tsv.sh

dictmatch : dictmatch.cpp
	g++ $< -o $@ -lsqlite3

clean:
	rm -f dictmatch
	rm -f g*.list
