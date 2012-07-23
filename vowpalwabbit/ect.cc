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
#include "ect.h"
#include "parser.h"
#include "simple_label.h"
#include "parse_args.h"

using namespace std;

namespace ECT
{

  //nonreentrant
  int k = 1;
  size_t errors = 0;

  //At each round, the maximum number participating in each tournament.
  // level 0 = first round, entry 1 = first single elimination tournament.
  v_array< v_array< size_t > > tournament_counts; 
  
  //Derived from tournament_counts, the number of pairs in each tournament.
  v_array< v_array< size_t > > pairs; 
  
  //The sum of pairs in all tournaments up to this depth.
  v_array<size_t> cumulative_pairs;
  size_t last_pair;
  
  v_array<int> final_levels; //The final rounds of each tournament. 
  // round number is one larger than the entrants round number and equal to 
  // the exit round number.
  size_t tree_height = 0; //The height of the final tournament.
  
  size_t increment = 0;
  v_array<bool> tournaments_won;

  bool exists(v_array<size_t> db)
  {
    for (size_t i = 0; i< db.index();i++)
      if (db[i] != 0)
        return true;
    return false;
  }

  size_t final_depth(size_t eliminations)
  {
    cout << "eliminations = " << eliminations << endl;
    eliminations--;
    for (size_t i = 0; i < 32; i++)
      if (eliminations >> i == 0)
	return i;
    cerr << "too many eliminations" << endl;
    return 31;
  }
 

  void create_circuit(vw& all, size_t max_label, size_t eliminations)
  {
    cout << "called create_circuit" << endl;
    v_array<size_t> first_round;
    v_array<size_t> first_pair;
  
    if (max_label > 0)
      push(first_round, max_label);
    else
      push(first_round, (size_t)0);

    for (size_t i = 1; i <= errors; i++)
      push(first_round, (size_t)0);

    push(first_pair, first_round[0]/2);

    push(tournament_counts, first_round);
    push(pairs, first_pair);
    push(cumulative_pairs, first_pair[0]);

    int level = 0; // Counts the level of the tournament.
    while(exists(tournament_counts[level]))
      {
        size_t pair_sum = 0;
        v_array<size_t> new_round;
        v_array<size_t> new_pairs;
        v_array<size_t> old_round = tournament_counts[level];
      
        for (size_t i = 0; i < old_round.index(); i++)
          {
            size_t count = 0;
            size_t prev = 0; 
            if (i != 0)
              prev = old_round[i-1];

            if (prev == 2 && (i == 1 || (i > 1 && old_round[i-2] == 0)))
              //last tournament finished
              count += 2;
            else
              count += prev/2;

            int old = old_round[i];
            if ( (old == 2 && prev == 0) || (old == 1 && prev == 0))
              push(final_levels, level+1);
            else
              count += (old+1)/2;
            push(new_round, count);
            push(new_pairs, new_round[i]/2);
            pair_sum += new_pairs[i];
          }
        push(tournament_counts, new_round);
        push(pairs, new_pairs);
        push(cumulative_pairs, pair_sum + cumulative_pairs[level]);
        level++;
      }
    last_pair = (errors+1)*(k-1); // every single tournament has k-1 pairs.

    if ( k > 1)
      tree_height = final_depth(eliminations);
    if (last_pair > 0)
      increment = all.length() / (last_pair + errors) * all.stride;

    cout << "last_pair = " << last_pair << "\ttree_height = " << tree_height << endl;
    for (size_t i = 0; i < tournament_counts.index(); i++)
      for (size_t j = 0; j < tournament_counts[i].index(); j++)
	cout << "tournament_counts[" << i << "][" << j << "] = " << tournament_counts[i][j] << endl;
    for (size_t i = 0; i < final_levels.index(); i++)
      cout << "final_levels["<< i << "] = " << final_levels[i] << endl;
  }

  struct node {
    size_t label;// From leaves, starts as actual label
    size_t tournament;// Starts at 0
    size_t level;// Starts at 0 at the leaves
  };

  bool bye_to_leaf(node& current)
  {
    cout << "current.level = " << current.level << "\tcurrent.tournament = " << current.tournament << endl;
    if ((current.label == tournament_counts[current.level][current.tournament]
	 && tournament_counts[current.level-1][current.tournament] % 2 == 1)
	|| (current.label == 1 && tournament_counts[current.level-1][current.tournament] == 1))
      {
	cout << "executing bye" << endl;
        current.label = tournament_counts[current.level-1][current.tournament];
        current.level--;
        return true;
      }
    else
      return false;
  }

  void root_to_leaf(node& current, bool right)
  {//shift down one level
    cout << "root_to_leaf, right = " << right << endl;
    v_array<size_t> prev = tournament_counts[current.level - 1];
  
    cout << "current.tournament = " << current.tournament << endl;
    if (current.tournament > 0 && current.label < 2 && prev[current.tournament-1] == 1 
        && tournament_counts[current.level][current.tournament-1] == 0)
      {
	cout << "in if" << endl;
        current.tournament--;
      }
    else
      {
	cout << "in else" << endl;
        size_t num_losers = 0;
	if (current.tournament > 0)
	  {
	    num_losers = (prev[current.tournament-1]+1) / 2;
	    if (prev[current.tournament - 1] == 1 && current.tournament > 1 
		&& prev[current.tournament - 2] == 0)
	      num_losers = 2;
	  }
	cout << "num_losers = " << num_losers << endl;

        if (current.label < num_losers)
          {
            current.tournament--;
            current.label = current.label * 2 + (right ? 0 : 1);
          }
        else
          current.label = (current.label - 1 - num_losers) * 2 + (right ? 1 : 0) + 1;
      }
    current.level--;
    cout << "current.label = " << current.label << endl;
  }

  size_t get_bit(size_t label, size_t bitNum)
  {
    size_t retVal = (label >> bitNum) & 1;
    return retVal;
  }

  void (*base_learner)(void*, example*) = NULL;
  void (*base_finish)(void*) = NULL;

  int ect_predict(vw& all, example* ec)
  {
    size_t final_winner = 0;

    //Binary final elimination tournament first
    label_data simple_temp = {FLT_MAX,0.,0.};
    ec->ld = & simple_temp;

    for (size_t i = tree_height-1; i != (size_t)0 -1; i--)
      {
        cout << "i = " << i << endl;
        if ((final_winner | (1 << i)) <= errors)
          {// a real choice exists
            uint32_t offset = 0;
	  
            size_t problem_number = last_pair + (final_winner | (1 << i)) - 1; //This is unique.
	  
            offset = problem_number*increment;
	  
            update_example_indicies(all.audit, ec,offset);
            ec->partial_prediction = 0;
	  
            base_learner(&all, ec);
	  
            update_example_indicies(all.audit, ec,-offset);
	  
            if (ec->final_prediction > 0.)
              final_winner = final_winner | (1 << i);
          }
      }

    cout << "finished, final_winner = " << final_winner << endl;
    cout << " round = " << final_levels[final_winner] << endl;
    node current = {1, final_winner, final_levels[final_winner]};
    while (current.level > 0)
      {
	cout << "in while" << endl;
	if (bye_to_leaf(current))//nothing to do.
	  ;
	else
	  {
	    cout << "in else" << endl;
	    size_t problem_number = 0;
	    problem_number += cumulative_pairs[current.level-1];
	    size_t i = 0;
	    while(i < current.tournament)
	      problem_number += pairs[current.level][i++];
	    problem_number += current.label/2;
	    
	    problem_number--;
	    cout << "problem_number = " << problem_number << endl;
	    size_t offset = problem_number*increment;
	    
	    ec->partial_prediction = 0;
	    update_example_indicies(all.audit, ec,offset);
	    base_learner(&all, ec);
	    float pred = ec->final_prediction;
	    cout << "pred = " << pred << endl;
	    update_example_indicies(all.audit, ec,-offset);
	    root_to_leaf(current, pred > 0.);
	  }
      }
    cout << "returning " << current.label << endl;
    return current.label;
  }

  bool bye_to_root(node& current)
  {
    if (current.label == tournament_counts[current.level][current.tournament] 
        && current.label %2 == 1)
      {
        current.label = tournament_counts[current.level+1][current.tournament];
        current.level++;
	cout << "executing bye_to_root" << endl;
        return true;
      }
    else
      return false;
  }

  void leaf_to_root(node& current, bool right)
  {
    cout << "in leaf_to_root, right = " << right << endl;
    v_array<size_t> round = tournament_counts[current.level];

    bool won = (((current.label % 2) == 0) && right) || (((current.label % 2) == 1) && !right);
    bool last_round = round[current.tournament] == 1 && round[current.tournament-1] == 0;

    if (last_round)
      {
        push(tournaments_won, won);
        current.tournament++;
      }
    else if (won)
      {
        int num_losers = (round[current.tournament-1]+1) / 2;
        if (round[current.tournament - 1] == 1 
            && current.tournament > 1 && round[current.tournament-2] == 0)
          num_losers = 2;
        current.label = num_losers + current.label / 2;
      }
    else 
      {
        push(tournaments_won, false);
        current.tournament++;
        current.label = current.label / 2;
      }
    current.level++;
  }

  bool member(size_t t, v_array<size_t> ar)
  {
    for (size_t i = 0; i < ar.index(); i++)
      if (ar[i] == t)
        return true;
    return false;
  }

  void ect_train(vw& all, example* ec)
  {
    OAA::mc_label* mc = (OAA::mc_label*)ec->ld;
  
    node current = {mc->label, 0, 0};

    label_data simple_temp = {1.,mc->weight,0.};
    cout << "mc->label = " << mc->label << endl;

    tournaments_won.erase();
    while(current.tournament < (size_t) tournament_counts[0].index())
      {
        if (bye_to_root(current))
          ;
        else
          {
            simple_temp.label = (2 * (current.label-1) % 2) - 1;
            ec->ld = &simple_temp;
	    
            size_t problem_number = 0;
            if (current.level != 0)
              problem_number += cumulative_pairs[current.level-1];
            size_t i = 0;
            while(i < current.tournament)
              problem_number += pairs[current.level][i++];
            problem_number += (current.label-1)/2;
	    
	    cout << "problem_number = " << problem_number << endl;
            size_t offset = problem_number*increment;
	    
            update_example_indicies(all.audit, ec,offset);
	    
            ec->partial_prediction = 0;
            base_learner(&all, ec);
            simple_temp.weight = 0.;
            ec->partial_prediction = 0;
            base_learner(&all, ec);//inefficient, we should extract final prediction exactly.
            float pred = ec->final_prediction;
            update_example_indicies(all.audit, ec,-offset);
            leaf_to_root(current, pred > 0.);
          }
      }
  
    //tournaments_won is a bit vector determining which tournaments the label won.

    for (size_t i = 0; i < tree_height; i++)
      {
        for (size_t j = 0; j < tournaments_won.index()/2; j++)
          {
            bool left = tournaments_won[j*2];
            bool right = tournaments_won[j*2+1];
            if (left == right)//no query to do
              tournaments_won[j] = left;
            else //query to do
              {
                size_t label;
                if (left) 
                  label = -1;
                else
                  label = 1;
                simple_temp.label = label;
                ec->ld = & simple_temp;
	      
                size_t problem_number = last_pair + j*(1 << (i+1)) + (1 << i) -1;
	      
                size_t offset = problem_number*increment;
	      
                update_example_indicies(all.audit, ec,offset);
                ec->partial_prediction = 0;
	      
                base_learner(&all, ec);
	      
                update_example_indicies(all.audit, ec,-offset);
	      
                if (ec->final_prediction > 0.)
                  tournaments_won[j] = right;
                else
                  tournaments_won[j] = left;
              }
            if (tournaments_won.index() %2 == 1)
              tournaments_won[tournaments_won.index()/2] = tournaments_won[tournaments_won.index()-1];
            tournaments_won.end = tournaments_won.begin+(1+tournaments_won.index())/2;
          }
      }
  }

  void learn(void*a, example* ec)
  {
    vw* all = (vw*)a;
    OAA::mc_label* mc = (OAA::mc_label*)ec->ld;
    int new_label = ect_predict(*all, ec);
    ec->ld = mc;

    if (mc->label != (uint32_t)-1 && all->training)
      ect_train(*all, ec);
    ec->ld = mc;
    ec->final_prediction = new_label;
  }

  void finish(void* all)
  {
    for (size_t i = 0; i < tournament_counts.index(); i++)
      if (tournament_counts[i].begin != tournament_counts[i].end)
        free(tournament_counts[i].begin);
    if (tournament_counts.begin != tournament_counts.end)
      free(tournament_counts.begin);

    if (final_levels.begin != final_levels.end)
      free (final_levels.begin);

    for (size_t i = 0; i < pairs.index(); i++)
      if (pairs[i].begin != pairs[i].end)
        free(pairs[i].begin);
    if (pairs.begin != pairs.end)
      free(pairs.begin);

    if (cumulative_pairs.begin != cumulative_pairs.end)
      free(cumulative_pairs.begin);

    if (tournaments_won.begin != tournaments_won.end)
      free(tournaments_won.begin);

    base_finish(all);
  }

  void drive_ect(void* in)
  {
    vw* all = (vw*)in;
    example* ec = NULL;
    while ( true )
      {
        if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
          {
            learn(all, ec);
            OAA::output_example(*all, ec);
	    VW::finish_example(*all, ec);
          }
        else if (parser_done(all->p))
          {
            return;
          }
        else 
          ;
      }
  }

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, size_t s)
  {
    po::options_description desc("ECT options");
    desc.add_options()
      ("error", po::value<size_t>(), "error in ECT");

    po::parsed_options parsed = po::command_line_parser(opts).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    opts = po::collect_unrecognized(parsed.options, po::include_positional);
    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("error")) {
      errors = vm["error"].as<size_t>();
    } else {
      cerr << "error: ect requires --error" << endl;
      exit(-1);
    }

    *(all.p->lp) = OAA::mc_label_parser;
    k = s;
    all.driver = drive_ect;
    base_learner = all.learn;
    all.base_learn = all.learn;
    all.learn = learn;

    base_finish = all.finish;
    all.base_finish = all.finish;
    all.finish = finish;

    create_circuit(all, k, errors+1);
  }

}
