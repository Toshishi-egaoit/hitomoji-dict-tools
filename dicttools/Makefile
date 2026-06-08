# makefile for create hitomoji.dic

.PHONY: all clean clean-corpus viewtest build-curation apply-curation

all:
	$(MAKE) -C kdic2
	$(MAKE) -C jmdict
	$(MAKE) -C jmdict/p1
	$(MAKE) -C jmdict/p2
	$(MAKE) -C jmdict/p3
	$(MAKE) -C jmdict/p4
	$(MAKE) -C build

viewtest: hitomoji.dic tools/dictview
	tools/dictview.sh しょう

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

hitomoji.dic:
	$(MAKE) -C build

clean:
	$(MAKE) -C kdic2 clean
	$(MAKE) -C jmdict clean
	$(MAKE) -C jmdict/p1 clean
	$(MAKE) -C jmdict/p2 clean
	$(MAKE) -C jmdict/p3 clean
	$(MAKE) -C jmdict/p4 clean
	$(MAKE) -C build clean
	rm -f tools/dictview

clean-corpus:
	$(MAKE) -C my-corpus clean

tools/dictview: tools/dictview.cpp
	g++ $< -o $@
