SHELL=/bin/zsh

.SECONDARY: 

.PHONY: all

ratelimit?=1M

all: paradata10000.vw.train.gz

paradata%.vw.train.gz: makeparadata.py text/AA/wiki_00.shuf.bz2
	python ./makeparadata.py $* 4                 \
	  > >(perl -pe 'BEGIN { srand 69; }; $$_ = rand()."\t$$_";' |	   \
	      sort -t$$'\t' -k1n -S30% | cut -f2- | gzip > paradata$*.vw.train.gz) \
	  2> >(perl -pe 'BEGIN { srand 69; }; $$_ = rand()."\t$$_";' |	   \
	      sort -t$$'\t' -k1n -S30% | cut -f2- | gzip > paradata$*.vw.test.gz) 
	while ! test -s paradata$*.vw.test.gz; do sleep 1; done
	while ! test -s paradata$*.vw.train.gz; do sleep 1; done

enwiki-20170601-pages-articles-multistream.xml.bz2:
	wget --limit-rate=$(ratelimit) http://dumps.wikimedia.your.org/enwiki/20170601/$@

text/AA/wiki_00.bz2: WikiExtractor.py enwiki-20170601-pages-articles-multistream.xml.bz2
	python ./WikiExtractor.py --no-templates --bytes 100G -c <(bzcat enwiki-20170601-pages-articles-multistream.xml.bz2)

text/AA/wiki_00.shuf.bz2: text/AA/wiki_00.bz2
	bzcat $< |							\
	  perl -ne 'BEGIN { srand 69; $$/="</doc>\n"; }; 		\
	            1; $$r = rand (); print "$$r\001$$_\000";' |	\
	  sort -k1n -t$$'\001' -z -S50% --compress-program=lzop | 	\
	  perl -F'\001' -ane 'BEGIN { $$/="\000"; }; 			\
	                      1; chomp($$F[1]); print $$F[1]' |		\
	  bzip2 > $@
