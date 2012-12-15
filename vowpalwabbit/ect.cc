/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
/*
  Initial implementation by Hal Daume and John Langford.  Reimplementation 
  by John Langford.
*/

#include <math.h>
#include <iostream>
#include <fstream>
#include <float.h>
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
  size_t k = 1;
  size_t errors = 0;

  struct direction { 
    size_t id; //unique id for node
    size_t tournament; //unique id for node
    size_t winner; //up traversal, winner
    size_t loser; //up traversal, loser
    size_t left; //down traversal, left
    size_t right; //down traversal, right
    bool last;
  };

  v_array<direction> directions;//The nodes of the tournament datastructure

  v_array<v_array<v_array<size_t > > > all_levels;

  v_array<size_t> final_nodes; //The final nodes of each tournament. 

  v_array<size_t> up_directions; //On edge e, which node n is in the up direction?
  v_array<size_t> down_directions;//On edge e, which node n is in the down direction?

  size_t tree_height = 0; //The height of the final tournament.
  
  size_t last_pair = 0;
  
  size_t increment = 0;

  v_array<bool> tournaments_won;
  
  bool exists(v_array<size_t> db)
  {
    for (size_t i = 0; i< db.size();i++)
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
 
  bool not_empty(v_array<v_array<size_t > > tournaments)
  {
    for (size_t i = 0; i < tournaments.size(); i++)
    {
      if (tournaments[i].size() > 0)
        return true;
    }
    return false;
  }

  void print_level(v_array<v_array<size_t> > level)
  {
    for (size_t t = 0; t < level.size(); t++)
      {
	for (size_t i = 0; i < level[t].size(); i++)
	  cout << " " << level[t][i];
	cout << " | ";
      }
    cout << endl;
  }

  void print_state()
  { 
    cout << "all_levels = " << endl;
    for (size_t l = 0; l < all_levels.size(); l++)
      print_level(all_levels[l]);
    
    cout << "directions = " << endl;
    for (size_t i = 0; i < directions.size(); i++)
      cout << " | " << directions[i].id << " t" << directions[i].tournament << " " << directions[i].winner << " " << directions[i].loser << " " << directions[i].left << " " << directions[i].right << " " << directions[i].last;
    cout << endl;
  }

  void create_circuit(vw& all, size_t max_label, size_t eliminations)
  {
    if (max_label == 1)
      return;

    v_array<v_array<size_t > > tournaments;

    v_array<size_t> t;

    for (size_t i = 0; i < max_label; i++)
      {
	t.push_back(i);	
	direction d = {i,0,0,0,0,0, false};
	directions.push_back(d);
      }

    tournaments.push_back(t);

    for (size_t i = 0; i < eliminations-1; i++)
      tournaments.push_back(v_array<size_t>());
    
    all_levels.push_back(tournaments);
    
    size_t level = 0;

    size_t node = directions.size();

    while (not_empty(all_levels[level]))
      {
	v_array<v_array<size_t > > new_tournaments;
	tournaments = all_levels[level];

	for (size_t t = 0; t < tournaments.size(); t++)
	  {
	    v_array<size_t> empty;
	    new_tournaments.push_back(empty);
	  }

	for (size_t t = 0; t < tournaments.size(); t++)
	  {
	    for (size_t j = 0; j < tournaments[t].size()/2; j++)
	      {
		size_t id = node++;
		size_t left = tournaments[t][2*j];
		size_t right = tournaments[t][2*j+1];
		
		direction d = {id,t,0,0,left,right, false};
		directions.push_back(d);
		size_t direction_index = directions.size()-1;
		if (directions[left].tournament == t)
		  directions[left].winner = direction_index;
		else
		  directions[left].loser = direction_index;
		if (directions[right].tournament == t)
		  directions[right].winner = direction_index;
		else
		  directions[right].loser = direction_index;
		if (directions[left].last == true)
		  directions[left].winner = direction_index;
		
		if (tournaments[t].size() == 2 && (t == 0 || tournaments[t-1].size() == 0))
		  {
		    directions[direction_index].last = true;
		    if (t+1 < tournaments.size())
		      new_tournaments[t+1].push_back(id);
		    else // winner eliminated.
		      directions[direction_index].winner = 0;
		    final_nodes.push_back((size_t)(directions.size()-1));
		  }
		else
		  new_tournaments[t].push_back(id);
		if (t+1 < tournaments.size())
		  new_tournaments[t+1].push_back(id);
		else // loser eliminated.
		  directions[direction_index].loser = 0;
	      }
	    if (tournaments[t].size() % 2 == 1)
	      new_tournaments[t].push_back(tournaments[t].last());
	  }
	all_levels.push_back(new_tournaments);
	level++;
      }

    last_pair = (max_label - 1)*(eliminations);
    
    if ( max_label > 1)
      tree_height = final_depth(eliminations);
    
    if (last_pair > 0) {
      all.base_learner_nb_w *= (last_pair + (eliminations-1));
      increment = all.length() / all.base_learner_nb_w * all.stride;
    }
  }

  void (*base_learner)(void*, example*) = NULL;
  void (*base_finish)(void*) = NULL;
  
  int ect_predict(vw& all, example* ec)
  {
    if (k == (size_t)1)
      return 1;

    size_t finals_winner = 0;
    
    //Binary final elimination tournament first
    label_data simple_temp = {FLT_MAX, 0., 0.};
    ec->ld = & simple_temp;

    for (size_t i = tree_height-1; i != (size_t)0 -1; i--)
      {
        if ((finals_winner | (1 << i)) <= errors)
          {// a real choice exists
            uint32_t offset = 0;
	  
            size_t problem_number = last_pair + (finals_winner | (1 << i)) - 1; //This is unique.
	    offset = problem_number*increment;
	  
            update_example_indicies(all.audit, ec,offset);
            ec->partial_prediction = 0;
	  
            base_learner(&all, ec);
	  
            update_example_indicies(all.audit, ec,-offset);
	    
	    float pred = ec->final_prediction;
	    if (pred > 0.)
              finals_winner = finals_winner | (1 << i);
          }
      }

    size_t id = final_nodes[finals_winner];
    while (id >= k)
      {
	size_t offset = (id-k)*increment;
	
	ec->partial_prediction = 0;
	update_example_indicies(all.audit, ec,offset);
	base_learner(&all, ec);
	float pred = ec->final_prediction;
	update_example_indicies(all.audit, ec,-offset);

	if (pred > 0.)
	  id = directions[id].right;
	else
	  id = directions[id].left;
      }
    return id+1;
  }

  bool member(size_t t, v_array<size_t> ar)
  {
    for (size_t i = 0; i < ar.size(); i++)
      if (ar[i] == t)
        return true;
    return false;
  }

  void ect_train(vw& all, example* ec)
  {
    if (k == 1)//nothing to do
      return;
    OAA::mc_label * mc = (OAA::mc_label*)ec->ld;
  
    label_data simple_temp = {1.,mc->weight,0.};

    tournaments_won.erase();

    size_t id = directions[mc->label-1].winner;
    bool left = directions[id].left == mc->label - 1;
    do
      {
	if (left)
	  simple_temp.label = -1;
	else
	  simple_temp.label = 1;
	
	simple_temp.weight = mc->weight;
	ec->ld = &simple_temp;
	
	size_t offset = (id-k)*increment;
	
	update_example_indicies(all.audit, ec,offset);
	
	ec->partial_prediction = 0;
	base_learner(&all, ec);
	simple_temp.weight = 0.;
	ec->partial_prediction = 0;
	base_learner(&all, ec);//inefficient, we should extract final prediction exactly.
	float pred = ec->final_prediction;
	update_example_indicies(all.audit, ec,-offset);

	bool won = pred*simple_temp.label > 0;

	if (won)
	  {
	    if (!directions[id].last)
	      left = directions[directions[id].winner].left == id;
	    else
	      tournaments_won.push_back(true);
	    id = directions[id].winner;
	  }
	else
	  {
	    if (!directions[id].last)
	      {
		left = directions[directions[id].loser].left == id;
		if (directions[id].loser == 0)
		  tournaments_won.push_back(false);
	      }
	    else
	      tournaments_won.push_back(false);
	    id = directions[id].loser;
	  }
      }
    while(id != 0);
      
    if (tournaments_won.size() < 1)
      cout << "badness!" << endl;

    //tournaments_won is a bit vector determining which tournaments the label won.
    for (size_t i = 0; i < tree_height; i++)
      {
        for (size_t j = 0; j < tournaments_won.size()/2; j++)
          {
            bool left = tournaments_won[j*2];
            bool right = tournaments_won[j*2+1];
            if (left == right)//no query to do
              tournaments_won[j] = left;
            else //query to do
              {
                float label;
                if (left) 
                  label = -1;
                else
                  label = 1;
                simple_temp.label = label;
		simple_temp.weight = (float)(1 << (tree_height -i -1));
                ec->ld = & simple_temp;
	      
                size_t problem_number = last_pair + j*(1 << (i+1)) + (1 << i) -1;
		
                size_t offset = problem_number*increment;
	      
                update_example_indicies(all.audit, ec,offset);
                ec->partial_prediction = 0;
	      
                base_learner(&all, ec);
		
                update_example_indicies(all.audit, ec,-offset);
		
		float pred = ec->final_prediction;
		if (pred > 0.)
                  tournaments_won[j] = right;
                else
                  tournaments_won[j] = left;
              }
            if (tournaments_won.size() %2 == 1)
              tournaments_won[tournaments_won.size()/2] = tournaments_won[tournaments_won.size()-1];
            tournaments_won.end = tournaments_won.begin+(1+tournaments_won.size())/2;
          }
      }
  }

  void learn(void*a, example* ec)
  {
    vw* all = (vw*)a;

    OAA::mc_label* mc = (OAA::mc_label*)ec->ld;
    if (mc->label > k)
      cout << "label > maximum label!  This won't work right." << endl;
    int new_label = ect_predict(*all, ec);
    ec->ld = mc;
    
    if (mc->label != (uint32_t)-1 && all->training)
      ect_train(*all, ec);
    ec->ld = mc;
    
    *(OAA::prediction_t*)&(ec->final_prediction) = new_label;
  }

  void finish(void* all)
  {
    for (size_t l = 0; l < all_levels.size(); l++)
      {
	for (size_t t = 0; t < all_levels[l].size(); t++)
	  if (all_levels[l][t].begin != all_levels[l][t].end)
	    free (all_levels[l][t].begin);
	if (all_levels[l].begin != all_levels[l].end)
	  free(all_levels[l].begin);
      }
    if (final_nodes.begin != final_nodes.end)
      free (final_nodes.begin);

    if (up_directions.begin != up_directions.end)
      free (up_directions.begin);

    if (directions.begin != directions.end)
      free (directions.begin);

    if (down_directions.begin != down_directions.end)
      free (down_directions.begin);

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

  void parse_flags(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
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

    po::parsed_options parsed_file = po::command_line_parser(all.options_from_file_argc, all.options_from_file_argv).
      style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
      options(desc).allow_unregistered().run();
    po::store(parsed_file, vm_file);
    po::notify(vm_file);

    //first parse for number of actions
    k = 0;
    if( vm_file.count("ect") ) {
      k = (int)vm_file["ect"].as<size_t>();
      if( vm.count("ect") && vm["ect"].as<size_t>() != k )
        std::cerr << "warning: you specified a different number of actions through --ect than the one loaded from predictor. Pursuing with loaded value of: " << k << endl;
    }
    else {
      k = (int)vm["ect"].as<size_t>();

      //append ect with nb_actions to options_from_file so it is saved to regressor later
      std::stringstream ss;
      ss << " --ect " << k;
      all.options_from_file.append(ss.str());
    }

    if(vm_file.count("error")) {
      errors = vm_file["error"].as<size_t>();
      if (vm.count("error") && vm["error"].as<size_t>() != errors) {
        cerr << "warning: specified value for --error different than the one loaded from predictor file. Pursuing with loaded value of: " << errors << endl;
      }
    }
    else if (vm.count("error")) {
      errors = vm["error"].as<size_t>();

      //append error flag to options_from_file so it is saved in regressor file later
      stringstream ss;
      ss << " --error " << errors;
      all.options_from_file.append(ss.str());
    } else {
      errors = 0;
    }

    *(all.p->lp) = OAA::mc_label_parser;
    all.driver = drive_ect;
    base_learner = all.learn;
    all.base_learn = all.learn;
    all.learn = learn;

    base_finish = all.finish;
    all.finish = finish;

    create_circuit(all, k, errors+1);
  }

}
