.PHONY: all clean

all: corpus.db

corpus.db: source.txt create-freq.sql make.corpus.db.sh
	./make.corpus.db.sh

clean:
	rm -f corpus.db
	rm -f corpus.base corpus.freq corpus.m4 freq.m4 corpus.sql
