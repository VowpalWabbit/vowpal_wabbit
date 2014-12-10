all:	sample

time:	explore.cpp
	g++ -Wall -O3 explore.cpp -I static -I ../vowpalwabbit -std=c++0x

sample:	explore_sample.cpp
	g++ -Wall -O3 explore_sample.cpp -I static -I ../vowpalwabbit -std=c++0x
