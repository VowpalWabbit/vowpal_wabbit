#include "vw/config/cli_help_formatter.h"
#include "vw/config/option_builder.h"
#include "vw/config/option_group_definition.h"
#include "vw/config/options_cli.h"
#include "vw/core/crossplat_compat.h"
#include "vw/core/memory.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/errno_handling.h"

#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

int pairs = 0;
int users = 0;
int items = 0;
int recs = 0;
int skipped = 0;
int banned = 0;
int show = 1;

int bits = 16;
int topk = 10;
int verbose = 0;

std::string blacklistfilename;
std::string itemfilename;
std::string userfilename;
std::string vwparams;

void progress()
{
  fprintf(stderr, "%12d %8d %8d %8d %12d %s %s\n", pairs, users, items, recs, skipped, userfilename.c_str(),
      itemfilename.c_str());
}

#define MASK(u, b) (u & ((1UL << b) - 1))
#define NUM_HASHES 2
void get_hashv(char* in, size_t len, unsigned* out)
{
  assert(NUM_HASHES == 2);
  out[0] = MASK(VW::uniform_hash(in, len, 1), bits);
  out[1] = MASK(VW::uniform_hash(in, len, 2), bits);
}

#define BIT_TEST(c, i) (c[i / 8] & (1 << (i % 8)))
#define BIT_SET(c, i) (c[i / 8] |= (1 << (i % 8)))
#define BYTE_LEN(b) (((1UL << b) / 8) + (((1UL << b) % 8) ? 1 : 0))
#define NUM_BITS(b) (1UL << b)

char* bf_new(unsigned b)
{
  char* bf = (char*)calloc(1, BYTE_LEN(b));
  return bf;
}

void bf_add(char* bf, char* line)
{
  unsigned i, hashv[NUM_HASHES];
  get_hashv(line, strlen(line), hashv);
  for (i = 0; i < NUM_HASHES; i++) { BIT_SET(bf, hashv[i]); }
}

void bf_info(char* bf, FILE* f)
{
  unsigned i, on = 0;
  for (i = 0; i < NUM_BITS(bits); i++)
  {
    if (BIT_TEST(bf, i)) { on++; }
  }

  fprintf(f, "%.2f%% saturation\n%lu bf bit size\n", on * 100.0 / NUM_BITS(bits), NUM_BITS(bits));
}

int bf_hit(char* bf, char* line)
{
  unsigned i, hashv[NUM_HASHES];
  get_hashv(line, strlen(line), hashv);
  for (i = 0; i < NUM_HASHES; i++)
  {
    if (BIT_TEST(bf, hashv[i]) == 0) { return 0; }
  }
  return 1;
}

using scored_example = std::pair<float, std::string>;
std::vector<scored_example> scored_examples;

struct compare_scored_examples
{
  bool operator()(scored_example const& lhs, scored_example const& rhs) const { return lhs.first > rhs.first; }
};

std::priority_queue<scored_example, std::vector<scored_example>, compare_scored_examples> pr_queue;

int main(int argc, char* argv[])
{
  using std::cerr;
  using std::cout;
  using std::endl;

  bool help;
  VW::config::options_cli opts(std::vector<std::string>(argv + 1, argv + argc));
  VW::config::option_group_definition desc("Recommend");
  desc.add(VW::config::make_option("help", help).short_name("h").help("Produce help message"))
      .add(VW::config::make_option("topk", topk).default_value(10).help("Number of items to recommend per user"))
      .add(VW::config::make_option("verbose", verbose).short_name("v").help("Increase verbosity (can be repeated)"))
      .add(VW::config::make_option("bf_bits", bits)
               .default_value(16)
               .short_name("b")
               .help("Number of items to recommend"))
      .add(VW::config::make_option("blacklist", blacklistfilename)
               .short_name("B")
               .help("User item pairs (in vw format) that we should not recommend (have been seen before)"))
      .add(VW::config::make_option("users", userfilename)
               .short_name("U")
               .help("Users portion in vw format to make recs for"))
      .add(
          VW::config::make_option("items", itemfilename).short_name("I").help("Items (in vw format) to recommend from"))
      .add(VW::config::make_option("vwparams", vwparams).help("vw parameters for model instantiation (-i model ...)"));

  opts.add_and_parse(desc);
  // Return value is ignored as option reachability is not relevant here.
  auto warnings = opts.check_unregistered();
  _UNUSED(warnings);

  VW::config::cli_help_formatter help_formatter;
  const auto help_message = help_formatter.format_help(opts.get_all_option_group_definitions());

  if (help)
  {
    VW::config::cli_help_formatter help_formatter;
    std::cout << help_message << std::endl;
    return 1;
  }

  if (blacklistfilename.empty() || userfilename.empty() || itemfilename.empty() || vwparams.empty())
  {
    VW::config::cli_help_formatter help_formatter;
    std::cout << help_message << std::endl;
    exit(2);
  }

  FILE* fB;  // NOLINT
  FILE* fU;  // NOLINT
  FILE* fI;  // NOLINT

  if (VW::file_open(&fB, blacklistfilename.c_str(), "r") != 0)
  {
    fprintf(stderr, "can't open %s: %s\n", blacklistfilename.c_str(), VW::io::strerror_to_string(errno).c_str());
    cerr << help_message << endl;
    exit(2);
  }
  if (VW::file_open(&fU, userfilename.c_str(), "r") != 0)
  {
    fprintf(stderr, "can't open %s: %s\n", userfilename.c_str(), VW::io::strerror_to_string(errno).c_str());
    cerr << help_message << endl;
    exit(2);
  }
  if (VW::file_open(&fI, itemfilename.c_str(), "r") != 0)
  {
    fprintf(stderr, "can't open %s: %s\n", itemfilename.c_str(), VW::io::strerror_to_string(errno).c_str());
    cerr << help_message << endl;
    exit(2);
  }

  char* buf = NULL;
  char* u = NULL;
  char* i = NULL;
  size_t len = 0;
  ssize_t read;

  /* make the bloom filter */
  if (verbose > 0) { fprintf(stderr, "loading blacklist into bloom filter...\n"); }
  char* bf = bf_new(bits);

  /* loop over the source file */
  while ((read = getline(&buf, &len, fB)) != -1)
  {
    bf_add(bf, buf);
    banned++;
  }

  /* print saturation etc */
  if (verbose)
  {
    bf_info(bf, stderr);
    fprintf(stderr, "%d banned pairs\n", banned);
  }

  // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE
  if (verbose > 0) { fprintf(stderr, "initializing vw...\n"); }
  auto model = VW::initialize(VW::make_unique<VW::config::options_cli>(VW::split_command_line(vwparams)));

  char* estr = NULL;

  if (verbose > 0)
  {
    fprintf(stderr, "predicting...\n");
    fprintf(stderr, "%12s %8s %8s %8s %12s %s %s\n", "pair", "user", "item", "rec", "skipped", "userfile", "itemfile");
  }

  while ((read = getline(&u, &len, fU)) != -1)
  {
    users++;
    u[strlen(u) - 1] = 0;  // chop
    rewind(fI);
    items = 0;
    while ((read = getline(&i, &len, fI)) != -1)
    {
      items++;
      pairs++;

      if ((verbose > 0) & (pairs % show == 0))
      {
        progress();
        show *= 2;
      }

      free(estr);
      estr = strdup((std::string(u) + std::string(i)).c_str());

      if (!bf_hit(bf, estr))
      {
        VW::example* ex = VW::read_example(*model, estr);
        model->learn(*ex);

        const std::string str(estr);

        if (pr_queue.size() < (size_t)topk) { pr_queue.push(std::make_pair(ex->pred.scalar, str)); }
        else if (pr_queue.top().first < ex->pred.scalar)
        {
          pr_queue.pop();
          pr_queue.push(std::make_pair(ex->pred.scalar, str));
        }

        VW::finish_example(*model, *ex);
      }
      else
      {
        skipped++;
        if (verbose >= 2) { fprintf(stderr, "skipping:#%s#\n", estr); }
      }
    }

    while (!pr_queue.empty())
    {
      cout << pr_queue.top().first << "\t" << pr_queue.top().second;
      pr_queue.pop();
      recs++;
    }
  }

  if (verbose > 0) { progress(); }

  model->finish();
  fclose(fI);
  fclose(fU);
  return 0;
}
