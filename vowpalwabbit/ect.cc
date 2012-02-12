/*
Copyright (c) 2012 Yahoo! Inc.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license

Initial implementation by Hal Daume and John Langford.  

This is not working yet (more to do).
 */

#include <math.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <pthread.h>
#include <time.h>
#include <boost/program_options.hpp>
#include "oaa.h"
#include "parser.h"
#include "hash.h"

using namespace std;

namespace ECT
{
  int k = 1;
  size_t errors = 0;
  struct circuit {
    //At each round, the maximum number participating in each tournament.
    // level 0 = first round, entry 1 = first single elimination tournament.
    v_array< v_array< size_t > > tournament_counts; 
    
    //Derived from tournament_counts, the number of pairs in each tournament.
    v_array< v_array< size_t > > pairs; 
    
    //Derived from pairs, the sum of pairs in all tournaments at that level.
    v_array<size_t> total_pairs;

    //Derived from total_pairs, the sum of total_pairs up to this depth.
    v_array<size_t> cumulative_pairs;
    
    v_array<int> final_rounds; //The final rounds of each tournament. 
    // round number is one larger than the entrants round number and equal to 
    // the exit round number.
    size_t tree_height; //The height of the final tournament.

    v_array<size_t> final_counts;//The number of tournaments at each level in the final tournament.
    v_array<size_t> final_pairs;//The number of pairs at each level in the final tournament.
    v_array<size_t> final_cumulative_pairs;//The cumulative number of pairs at each level in the final tournament.
  };
  circuit c;
  v_array<size_t> tournaments_won;

bool exists(v_array<size_t> db)
{
  for (size_t i = 0; i< db.index();i++)
    if (db[i] != 0)
      return true;
  return false;
}

  size_t final_depth(size_t eliminations)
  {
    eliminations--;
    for (size_t i = 0; i < 32; i++)
      if (eliminations >> i == 0)
	return i;
    cerr << "too many eliminations" << endl;
    return 31;
  }
  
void create_circuit(size_t max_label, size_t eliminations, circuit& c)
{
  v_array<size_t> first_round;
  v_array<size_t> first_pair;

  if (max_label > 1)
    push(first_round, max_label);
  else
    push(first_round, (size_t)0);

  push(first_pair, first_round[0]/2);

  for (size_t i = 1; i < eliminations; i++)
    {
      push(first_round, (size_t)0);  
      push(first_pair, (size_t)0);
    }
  push(c.tournament_counts, first_round);
  push(c.pairs, first_pair);
  push(c.total_pairs, first_pair[0]);
  push(c.cumulative_pairs, c.total_pairs[0]);

  int depth = 0;
  while(exists(c.tournament_counts[depth]))
    {
      size_t total_pairs = 0;
      v_array<size_t> new_round;
      v_array<size_t> new_pairs;
      v_array<size_t> old_round = c.tournament_counts[depth];
      for (size_t i = 1; i < old_round.index(); i++)
	{
	  size_t count = 0;
	  size_t prev = old_round[i-1];
	  if (prev == 2 && (i == 1 || (i > 1 && old_round[i-2] == 0)))
	    //last tournament finished
	    count += 2;
	  else
	    count += prev/2;
	  int old = old_round[i];
	  if (old == 2 && prev == 0)
	    push(c.final_rounds, depth-1);
	  else
	    count += (old+1)/2;
	  push(new_round, count);
	  push(new_pairs, new_round[i]/2);
	  total_pairs += new_pairs[i];
	}
      push(c.tournament_counts, new_round);
      push(c.pairs, new_pairs);
      push(c.total_pairs, total_pairs);
      push(c.cumulative_pairs, c.total_pairs[depth]);
      depth++;
    }

  c.tree_height = final_depth(eliminations);

  depth=0;
  while (eliminations > 1)
    {
      eliminations = (eliminations+1)/2;
      push(c.final_counts, eliminations);
      push(c.final_pairs, eliminations/2);
    }
}

struct node {
  size_t label;// From leaves, starts as actual label
  size_t eliminations;// Starts at 1
  size_t depth;// Starts at 0 at the leaves
};

void leaf_to_root(node& current, bool right, circuit& c)
{
  v_array<size_t> round = c.tournament_counts[current.depth];

  bool won = (((current.label % 2) == 1) && right) || (((current.label % 2) == 0) && !right);
  bool last_round = round[current.eliminations] == 1 && round[current.eliminations-1] == (size_t)-1;

  if (last_round)
    {
      if (won)
	{
	  push(tournaments_won, current.eliminations);
	}
      current.eliminations++;
    }
  else if (won)
    {
      int num_losers = (round[current.eliminations-1]+1) / 2;
      if (round[current.eliminations - 1] == 1 
	  && current.eliminations > 1 && round[current.eliminations-2] == (size_t)-1)
	num_losers = 2;
      current.label = num_losers + current.label / 2;
    }
  else 
    {
      current.eliminations++;
      current.label = current.label / 2;
    }
  current.depth++;
}

bool bye_to_root(node& current, circuit& c)
{
  if (current.label == c.tournament_counts[current.depth][current.eliminations] 
      && current.label %2 == 0)
    {
      current.label = c.tournament_counts[current.depth+1][current.eliminations];
      current.depth++;
      return true;
    }
  else
    return false;
}

void root_to_leaf(node& current, bool right, circuit& c)
{
  v_array<size_t> prev = c.tournament_counts[current.depth - 1];
  
  if (current.label < 2 && prev[current.eliminations-1] == 1 
      && c.tournament_counts[current.depth][current.eliminations-1] == (size_t)-1)
    {
      current.eliminations--;
    }
  else
    {
      size_t num_losers = (prev[current.eliminations-1]+1) / 2;
      if (prev[current.eliminations - 1] == 1 && current.eliminations > 1 
	  && prev[current.eliminations - 2] == (size_t)-1)
	num_losers = 2;

      if (current.label < num_losers)
	{
	  current.eliminations--;
	  current.label = current.label * 2 + (right ? 0 : 1);
	}
      else
	current.label = (current.label - num_losers) * 2 + (right ? 1 : 0);
    }
  current.depth--;
}

bool bye_to_leaf(node& current, circuit& c)
{
  if (current.label == c.tournament_counts[current.depth][current.eliminations]
      && (c.tournament_counts[current.depth-1][current.eliminations] % 2 == 0))
    {
      current.label = c.tournament_counts[current.depth-1][current.eliminations];
      current.depth--;
      return true;
    }
  else
    return false;
}

size_t get_bit(size_t label, size_t bitNum)
{
  size_t retVal = (label >> bitNum) & 1;
  return retVal;
}

void (*base_learner)(example*) = NULL;
void (*base_finish)() = NULL;

struct final_node {
  int label;
  int level;
};

int ect_predict(example* ec)
{
  size_t new_label = 0;

  //Binary final elimination tournament first
  label_data simple_temp = {FLT_MAX,0.,0.};
  ec->ld = & simple_temp;

  for (size_t i = c.tree_height-1; i >= 0; i--)
    {
      if ((new_label | (1 << i)) <= errors)
	{// a real choice exists
	  uint32_t offset = 0;
	  if (i != c.tree_height-1)
	    {
	      final_node temp = {new_label,i};
	      offset = uniform_hash(&temp,sizeof(temp),0);
	      OAA::update_indicies(ec,offset);
	      ec->partial_prediction = 0;
	    }

	  base_learner(ec);

	  if (i != c.tree_height-1)
	    OAA::update_indicies(ec,-offset);

	  if (ec->final_prediction > 0.)
	    new_label = new_label | (1 << i);
	}
    }
  
  node current = {0, new_label+1, c.final_rounds[new_label]};
  while (current.depth > 0)
    {
      if (bye_to_leaf(current,c))
	;
      else
	{
	  node current2 = current;
	  current2.label = current2.label/2;
	  current2.depth--;
	  uint32_t offset = uniform_hash(&current2,sizeof(current2),0);
	  ec->partial_prediction = 0;
	  OAA::update_indicies(ec,offset);
	  base_learner(ec);
	  float pred = ec->final_prediction;
	  OAA::update_indicies(ec,-offset);
	  root_to_leaf(current, pred > 0., c);
	}
    }

  return current.label;
}

bool member(size_t t, v_array<size_t> ar)
{
  for (size_t i = 0; i < ar.index(); i++)
    if (ar[i] == t)
      return true;
  return false;
}

void ect_train(example* ec)
{
  OAA::mc_label* mc = (OAA::mc_label*)ec->ld;
  
  node current = {mc->label, 1, 0};
  
  tournaments_won.erase();
  while(current.eliminations < (size_t) c.tournament_counts[0].index())
    {
      if (bye_to_root(current,c))
	;
      else
	{
	  label_data simple_temp = {(2 * current.label % 2) - 1, mc->weight, 0.};
	  
	  node current2 = current;
	  current2.label = current2.label/2;
	  uint32_t offset = uniform_hash(&current2,sizeof(current2),0);
	  OAA::update_indicies(ec,offset);

	  ec->partial_prediction = 0;
	  base_learner(ec);
	  simple_temp.weight = 0.;
	  ec->partial_prediction = 0;
	  base_learner(ec);
	  float pred = ec->final_prediction;
	  OAA::update_indicies(ec,-offset);
	  leaf_to_root(current, pred > 0., c);
	}
    }
  
  int depth = 0;
  while (tournaments_won.index() > 0)
    {
      size_t cur_pos = 0;
      int insert_pos = 0;
      float weight = mc->weight;

      while (cur_pos < tournaments_won.index())
	{
	  size_t label = tournaments_won[cur_pos];
	  if ((label | (1 << depth)) <= c.tournament_counts[0][1]) // not a bye
	    {
	      if (get_bit(label,depth) == 1 || ((cur_pos < tournaments_won.index()-1) && (label+1 != tournaments_won[cur_pos+1])))
		{
		  label &= ~(1 << depth);
		  final_node temp = { label, depth };
		  uint32_t offset = uniform_hash(&temp, sizeof(temp), 0);
		  OAA::update_indicies(ec,offset);

		  label_data simple_temp = {(2*get_bit(label,depth) % 2) - 1, weight, 0.};
		  ec->ld = &simple_temp;

		  ec->partial_prediction = 0;
		  base_learner(ec);
		  ec->partial_prediction = 0.;
		  simple_temp.weight = 0.;
		  base_learner(ec);
		  OAA::update_indicies(ec,-offset);
		  if ((ec->final_prediction > 0.) == (get_bit(label, depth) == 1))
		    {
		      tournaments_won[insert_pos] = label;
		      insert_pos++;
		    }
		  
		  if ((cur_pos < tournaments_won.index()-1) && (label+1 != tournaments_won[cur_pos+1]))
		    cur_pos++;
		}
	    }
	  else // a bye
	    {
	      label &= ~(1 << depth);
	      tournaments_won[insert_pos] = label;
	      insert_pos++;
	    } 

	  cur_pos++;
	}

      tournaments_won.end = tournaments_won.begin+insert_pos;
      depth++;
      weight *= 2.;
    }
}

void learn(example* ec)
{
  OAA::mc_label* mc = (OAA::mc_label*)ec->ld;
  int new_label = ect_predict(ec);
  
  if (mc->label != (uint32_t)-1 && global.training)
    ect_train(ec);
  ec->ld = mc;
  ec->final_prediction = new_label;
}

void finish()
{
  base_finish();
}

void drive_ect()
{
  example* ec = NULL;
  while ( true )
    {
      if ((ec = get_example()) != NULL)//semiblocking operation.
	{
	  learn(ec);
	  OAA::output_example(ec);
	  free_example(ec);
	}
      else if (parser_done())
	{
	  finish();
	  return;
	}
      else 
	;
    }
}

void parse_flags(size_t s, size_t e, void (*base_l)(example*), void (*base_f)())
{
  *(global.lp) = OAA::mc_label_parser;
  k = s;
  errors = e;
  global.driver = drive_ect;
  base_learner = base_l;
  base_finish = base_f;

  create_circuit(k, errors+1, c);
}

}
