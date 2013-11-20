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
#include <boost/program_options.hpp>

#include "../vowpalwabbit/vw.h"

using namespace std;
namespace po = boost::program_options;

int pairs = 0;
int users = 0;
int items = 0;
int recs = 0;
int skipped = 0;
int banned = 0;
int show = 1;

int b=16;
int topk=10;
int verbose=0;
string blacklistfilename;
string itemfilename;
string userfilename;
string vwparams;

void progress()
{
        fprintf(stderr, "%12d %8d %8d %8d %12d %s %s\n", pairs, users, items, recs, skipped, userfilename.c_str(), itemfilename.c_str());
}

#define MASK(u,b) ( u & ((1UL << b) - 1))
#define NUM_HASHES 2
void get_hashv(char *in, size_t len, unsigned *out) {
        assert(NUM_HASHES==2);
        out[0] = MASK(uniform_hash(in,len,1),b);
        out[1] = MASK(uniform_hash(in,len,2),b);
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

        fprintf(f, "%.2f%% saturation\n%lu bf bit size\n", on*100.0/num_bits(b), num_bits(b));
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
        po::variables_map vm;
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce help message")
                ("topk", po::value<int>(&topk), "number of items to recommend per user")
                ("verbose,v", po::value<int>(&verbose), "increase verbosity (can be repeated)")
                ("bf_bits,b", po::value<int>(&b), "number of items to recommend")
                ("blacklist,B", po::value<string>(&blacklistfilename), "user item pairs (in vw format) that we should not recommend (have been seen before)")
                ("users,U", po::value<string>(&userfilename), "users portion in vw format to make recs for")
                ("items,I", po::value<string>(&itemfilename), "items (in vw format) to recommend from")
                ("vwparams", po::value<string>(&vwparams), "vw parameters for model instantiation (-i model ...)")
                ;

        try {
                po::store(po::parse_command_line(argc, argv, desc), vm);
                po::notify(vm);    
        }
        catch(exception & e)
        {
                cout << endl << argv[0] << ": " << e.what() << endl << endl << desc << endl;
                exit(2);
        }

        if (vm.count("help")) {
                cout << desc << "\n";
                return 1;
        }

        if (blacklistfilename.empty() || userfilename.empty() || itemfilename.empty() || vwparams.empty())
        {
                cout << desc << "\n";
                exit(2);
        }

        FILE * fB;
        FILE * fU;
        FILE * fI;

        if((fB = fopen(blacklistfilename.c_str(), "r")) == NULL)
        {
                fprintf(stderr,"can't open %s: %s\n", blacklistfilename.c_str(), strerror(errno));
                cerr << desc << endl;
                exit(2);
        }
        if((fU = fopen(userfilename.c_str(), "r")) == NULL )
        {
                fprintf(stderr,"can't open %s: %s\n", userfilename.c_str(), strerror(errno));
                cerr << desc << endl;
                exit(2);
        }
        if((fI = fopen(itemfilename.c_str(), "r")) == NULL )
        {
                fprintf(stderr,"can't open %s: %s\n", itemfilename.c_str(), strerror(errno));
                cerr << desc << endl;
                exit(2);
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
                banned++;
        }                

        /* print saturation etc */
        if (verbose)
        {
                bf_info(bf,stderr); 
                fprintf(stderr, "%d banned pairs\n", banned);
        }

        // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE
        if(verbose>0)
                fprintf(stderr, "initializing vw...\n");
        vw* model = VW::initialize(vwparams);

        char * estr = NULL;

        if(verbose>0)
        {
                fprintf(stderr, "predicting...\n");
                fprintf(stderr, "%12s %8s %8s %8s %12s %s %s\n", "pair", "user", "item", "rec", "skipped", "userfile", "itemfile");
        }
        while ((read = getline(&u, &len, fU)) != -1) 
        {
                users++;
                u[strlen(u)-1] = 0; // chop
                rewind(fI);
                items=0;
                while ((read = getline(&i, &len, fI)) != -1) 
                {
                        items++;
                        pairs++;

                        if((verbose > 0) & (pairs % show == 0))
                        {
                                progress();
                                show *= 2;
                        }

                        free(estr);
                        estr = strdup((string(u)+string(i)).c_str());

                        if (!bf_hit(bf,estr))
                        {
                                example *ex = VW::read_example(*model, estr);
                                model->learn(ex);

                                const string str(estr);

                                if(pr_queue.size() < topk)
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
                                skipped++;
                                if(verbose>=2)
                                        fprintf(stderr,"skipping:#%s#\n", estr);
                        }

                }

                while(!pr_queue.empty())
                {
                        cout << pr_queue.top().first << "\t" << pr_queue.top().second;
                        pr_queue.pop();
                        recs++;
                }
        }
        if(verbose>0)
        {
                progress();
        }

        VW::finish(*model);
        fclose(fI);
        fclose(fU);
        exit(EXIT_SUCCESS);

}





