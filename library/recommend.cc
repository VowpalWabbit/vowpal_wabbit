#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <errno.h>
#include <unistd.h>
#include <assert.h>


#include "../vowpalwabbit/vw.h"

using namespace std;

void usage(char *prog) {
        fprintf(stderr, "usage: %s [-b <2|4|8|...|32>] [-v] <blacklist> <users> <items> <topN> <vwparams>\n", prog);
        exit(EXIT_FAILURE);
}

int b=16;
int topn=10;
int verbose=0;
char * blacklistfilename = NULL;
char * itemfilename = NULL;
char * userfilename = NULL;
char * vwparams = NULL;

unsigned hash_ber(char *in, size_t len) {
        unsigned hashv = 0;
        while (len--)  hashv = ((hashv) * 33) + *in++;
        return hashv;
}
unsigned hash_fnv(char *in, size_t len) {
        unsigned hashv = 2166136261UL;
        while(len--) hashv = (hashv * 16777619) ^ *in++;
        return hashv;
}
#define MASK(u,b) ( u & ((1UL << b) - 1))
#define NUM_HASHES 2
void get_hashv(char *in, size_t len, unsigned *out) {
        assert(NUM_HASHES==2);
        out[0] = MASK(hash_ber(in,len),b);
        out[1] = MASK(hash_fnv(in,len),b);
}

#define BIT_TEST(c,i) (c[i/8] & (1 << (i % 8)))
#define BIT_SET(c,i) (c[i/8] |= (1 << (i % 8)))
#define byte_len(b) (((1UL << b) / 8) + (((1UL << b) % 8) ? 1 : 0))
#define num_bits(b) (1UL << b)
char *bf_new(unsigned b) {
        char *bf = (char*)calloc(1,byte_len(b));
        return bf;
}
void bf_add(char *bf, char *line) {
        unsigned i, hashv[NUM_HASHES];
        get_hashv(line,strlen(line),hashv);
        for(i=0;i<NUM_HASHES;i++) BIT_SET(bf,hashv[i]);
}
void bf_info(char *bf, FILE *f) {
        unsigned i, on=0;
        for(i=0; i<num_bits(b); i++) 
                if (BIT_TEST(bf,i)) on++;

        fprintf(f, "%.2f%% saturation (%lu bits)\n", on*100.0/num_bits(b), num_bits(b));
}
int bf_hit(char *bf, char *line) {
        unsigned i, hashv[NUM_HASHES];
        get_hashv(line,strlen(line),hashv);
        for(i=0;i<NUM_HASHES;i++) {
                if (BIT_TEST(bf,hashv[i])==0) return 0;
        }
        return 1;
}

typedef pair<float, string> scored_example;
vector<scored_example> scored_examples;

struct compare_scored_examples
{
    bool operator()(scored_example const& a, scored_example const& b) const
    {
        return a.first > b.first;
    }
};

priority_queue<scored_example, vector<scored_example>, compare_scored_examples > pr_queue;

int main(int argc, char *argv[])
{
        int opt;

        while ( (opt = getopt(argc, argv, "b:v+N:")) != -1) {
                switch (opt) {
                        case 'N':
                                topn = atoi(optarg);
                                break;
                        case 'b':
                                b = atoi(optarg);
                                break;
                        case 'v':
                                verbose++;
                                break;
                        default:
                                usage(argv[0]);
                                break;
                }
        }

        if (optind < argc) blacklistfilename=argv[optind++];
        if (optind < argc) userfilename=argv[optind++];
        if (optind < argc) itemfilename=argv[optind++];
        if (optind < argc) vwparams=argv[optind++];

        if (!blacklistfilename || !userfilename || !itemfilename || !vwparams) usage(argv[0]);

        FILE * fB;
        FILE * fU;
        FILE * fI;

        if((fB = fopen(blacklistfilename, "r")) == NULL)
        {
                fprintf(stderr,"can't open %s: %s\n", blacklistfilename, strerror(errno));
                usage(argv[0]);
        }
        if((fU = fopen(userfilename, "r")) == NULL )
        {
                fprintf(stderr,"can't open %s: %s\n", userfilename, strerror(errno));
                usage(argv[0]);
        }
        if((fI = fopen(itemfilename, "r")) == NULL )
        {
                fprintf(stderr,"can't open %s: %s\n", itemfilename, strerror(errno));
                usage(argv[0]);
        }

        char * buf = NULL;
        char * u = NULL;
        char * i = NULL;
        size_t len = 0;
        ssize_t read;

        /* make the bloom filter */
        if(verbose>0)
                fprintf(stderr, "loading blacklist into bloom filter...\n");
        char *bf= bf_new(b);

        /* loop over the source file */
        while ((read = getline(&buf,&len,fB)) != -1)
        {
                bf_add(bf,buf);
        }                

        /* print saturation etc */
        if (verbose) bf_info(bf,stderr); 

        // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE
        if(verbose>0)
                fprintf(stderr, "initializing vw...\n");
        vw* model = VW::initialize(vwparams);

        char * estr = NULL;

        if(verbose>0)
                fprintf(stderr, "predicting...\n");
        while ((read = getline(&u, &len, fU)) != -1) 
        {
                u[strlen(u)-1] = 0; // chop
                rewind(fI);
                while ((read = getline(&i, &len, fI)) != -1) 
                {
                        free(estr);
                        estr = strdup((string(u)+string(i)).c_str());

                        if (!bf_hit(bf,estr))
                        {
                                example *ex = VW::read_example(*model, estr);
                                model->learn(ex);

                                const string str(estr);

                                if(pr_queue.size() < topn)
                                {        
                                        pr_queue.push(make_pair(ex->final_prediction, str));
                                }
                                else if(pr_queue.top().first < ex->final_prediction)
                                {
                                        pr_queue.pop();
                                        pr_queue.push(make_pair(ex->final_prediction, str));
                                }

                                VW::finish_example(*model, ex);
                        }
                        else
                        {
                                if(verbose>=2)
                                        fprintf(stderr,"skipping:\t%s\n", buf);
                        }

                }

                while(!pr_queue.empty())
                {
                        cout << pr_queue.top().first << "\t" << pr_queue.top().second;
                        pr_queue.pop();
                }
        }

        VW::finish(*model);
        fclose(fI);
        fclose(fU);
        exit(EXIT_SUCCESS);
}





