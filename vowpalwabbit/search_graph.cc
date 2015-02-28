/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include "search_graph.h"
#include "vw.h"
#include "rand48.h"
#include "gd.h"

/*
example format:

ALL NODES
ALL EDGES
<blank>
ALL NODES
ALL EDGES
<blank>

and so on

node lines look like normal vw examples with unary features:

label:weight |n features
label:weight |n features
...

they are *implicitly* labeled starting at 1. (note the namespace
needn't be called n.) if weight is omitted it is assumed to be 1.0.

edge lines look like:

n1 n2 n3 ... |e features
n1 n2 n3 ... |e features
...

here, n1 n2 n3 are integer node ids, starting at one. technically
these are hyperedges, since they can touch more than two nodes. in the
canonical representation, there will just be n1 and n2.

the only thing that differentiates edges from nodes is that edges have
>1 input.
*/

namespace GraphTask {
  Search::search_task task = { "graph", run, initialize, finish, setup, takedown };

  struct task_data {
    // global data
    size_t num_loops;
    size_t K;  // number of labels, *NOT* including the +1 for 'unlabeled'
    bool   use_structure;
    bool   separate_learners;

    // for adding new features
    size_t mask; // all->reg.weight_mask
    size_t ss;   // all->reg.stride_shift
    
    // per-example data
    uint32_t N;  // number of nodes
    uint32_t E;  // number of edges
    vector< vector<size_t> > adj;  // adj[n] is a vector of *edge example ids* that contain n
    vector<uint32_t> bfs;   // order of nodes to process
    vector<size_t>   pred;  // predictions
    example*cur_node;       // pointer to the current node for add_edge_features_fn
    float* neighbor_predictions;  // prediction on this neighbor for add_edge_features_fn
    weight* weight_vector;
  };

  inline bool example_is_test(polylabel&l) { return l.cs.costs.size() == 0; }
  
  void initialize(Search::search& sch, size_t& num_actions, po::variables_map& vm) {
    task_data * D = new task_data();
    po::options_description sspan_opts("search graphtask options");
    sspan_opts.add_options()("search_graph_num_loops", po::value<size_t>(), "how many loops to run [def: 2]");
    sspan_opts.add_options()("search_graph_no_structure", "turn off edge features");
    sspan_opts.add_options()("search_graph_separate_learners", "use a different learner for each pass");
    sch.add_program_options(vm, sspan_opts);

    D->num_loops = 2;
    D->use_structure = true;
    if (vm.count("search_graph_num_loops"))      D->num_loops = vm["search_graph_num_loops"].as<size_t>();
    if (vm.count("search_graph_no_structure"))   D->use_structure = false;
    if (vm.count("search_graph_separate_learners")) D->separate_learners = true;

    if (D->num_loops <= 1) { D->num_loops = 1; D->separate_learners = false; }
    
    D->K = num_actions;
    D->neighbor_predictions = calloc_or_die<float>(D->K+1);

    if (D->separate_learners) sch.set_num_learners(D->num_loops);
    
    sch.set_task_data<task_data>(D);
    sch.set_options( Search::AUTO_HAMMING_LOSS );
    sch.set_label_parser( COST_SENSITIVE::cs_label, example_is_test );
  }

  void finish(Search::search& sch) {
    task_data * D = sch.get_task_data<task_data>();
    free(D->neighbor_predictions);
    delete D;
  }
  
  inline bool example_is_edge(example*e) { return e->l.cs.costs.size() > 1; }

  void run_bfs(task_data &D, vector<example*>& ec) {
    D.bfs.clear();
    vector<bool> touched;
    for (size_t n=0; n<D.N; n++) touched.push_back(false);

    touched[0] = true;
    D.bfs.push_back(0);

    size_t i = 0;
    while (D.bfs.size() < D.N) {
      while (i < D.bfs.size()) {
        uint32_t n = D.bfs[i];
        for (size_t id : D.adj[n])
          for (size_t j=0; j<ec[id]->l.cs.costs.size(); j++) {
            uint32_t m = ec[id]->l.cs.costs[j].class_index - 1;
            if (!touched[m]) {
              D.bfs.push_back(m);
              touched[m] = true;
            }
          }
        i++;
      }

      if (D.bfs.size() < D.N)
        // we finished a SCC, need to find another
        for (uint32_t n=0; n<D.N; n++)
          if (! touched[n]) {
            touched[n] = true;
            D.bfs.push_back(n);
            break;
          }
    }
  }
  
  void setup(Search::search& sch, vector<example*>& ec) {
    task_data& D = *sch.get_task_data<task_data>();

    D.mask = sch.get_vw_pointer_unsafe().reg.weight_mask;
    D.ss   = sch.get_vw_pointer_unsafe().reg.stride_shift;
    D.weight_vector = sch.get_vw_pointer_unsafe().reg.weight_vector;
    
    D.N = 0;
    D.E = 0;
    for (size_t i=0; i<ec.size(); i++)
      if (example_is_edge(ec[i]))
        D.E++;
      else { // it's a node!
        if (D.E > 0) { cerr << "error: got a node after getting edges!" << endl; throw exception(); }
        D.N++;
      }

    if ((D.N == 0) && (D.E > 0)) { cerr << "error: got edges without any nodes!" << endl; throw exception(); }

    D.adj = vector<vector<size_t>>(D.N, vector<size_t>(0));

    for (size_t i=D.N; i<ec.size(); i++) {
      for (size_t n=0; n<ec[i]->l.cs.costs.size(); n++) {
        if (ec[i]->l.cs.costs[n].class_index > D.N) {
          cerr << "error: edge source points to too large of a node id: " << (ec[i]->l.cs.costs[n].class_index) << " > " << D.N << endl;
          throw exception();
        }
      }
      for (size_t n=0; n<ec[i]->l.cs.costs.size(); n++) {
        size_t nn = ec[i]->l.cs.costs[n].class_index - 1;
        if ((D.adj[nn].size() == 0) || (D.adj[nn][D.adj[nn].size()-1] != i)) // don't allow dups
          D.adj[nn].push_back(i);
      }
    }

    run_bfs(D, ec);

    D.pred.clear();
    for (size_t n=0; n<D.N; n++)
      D.pred.push_back( D.K+1 );
  }

  void takedown(Search::search& sch, vector<example*>& ec) {
    task_data& D = *sch.get_task_data<task_data>();
    D.bfs.clear();
    D.pred.clear();
    D.adj.clear();
  }

  void add_edge_features_group_fn(task_data&D, float fv, uint32_t fx) {
    example*node = D.cur_node;
    for (size_t k=0; k<=D.K; k++) {
      if (D.neighbor_predictions[k] == 0.) continue;
      feature f = { fv * D.neighbor_predictions[k], (uint32_t) ((( ((fx & D.mask) >> D.ss) + 348919043 * k ) << D.ss) & D.mask) };
      node->atomics[neighbor_namespace].push_back(f);
      node->sum_feat_sq[neighbor_namespace] += f.x * f.x;
    }
    // TODO: audit
  }

  void add_edge_features_single_fn(task_data&D, float fv, uint32_t fx) {
    example*node = D.cur_node;
    size_t k = (size_t) D.neighbor_predictions[0];
    feature f = { fv, (uint32_t) (( ((fx & D.mask) >> D.ss) + 348919043 * k ) << D.ss) };
    node->atomics[neighbor_namespace].push_back(f);
    node->sum_feat_sq[neighbor_namespace] += f.x * f.x;
    // TODO: audit
  }
  
  void add_edge_features(Search::search&sch, task_data&D, uint32_t n, vector<example*>&ec) {
    D.cur_node = ec[n];

    for (size_t i : D.adj[n]) {
      for (size_t k=0; k<D.K+1; k++) D.neighbor_predictions[k] = 0.;

      float pred_total = 0.;
      uint32_t last_pred = 0;
      for (size_t j=0; j<ec[i]->l.cs.costs.size(); j++) {
        size_t m = ec[i]->l.cs.costs[j].class_index - 1;
        if (m == n) continue;
        D.neighbor_predictions[ D.pred[m]-1 ] += 1.;
        pred_total += 1.;
        last_pred = D.pred[m]-1;
      }

      if (pred_total == 0.) continue;
      //for (size_t k=0; k<D.K+1; k++) D.neighbor_predictions[k] /= pred_total;
      example&edge = *ec[i];
      if (pred_total <= 1.) {  // single edge
        D.neighbor_predictions[0] = (float)last_pred;
        GD::foreach_feature<task_data,uint32_t,add_edge_features_single_fn>(sch.get_vw_pointer_unsafe(), edge, D);
      } else // lots of edges
        GD::foreach_feature<task_data,uint32_t,add_edge_features_group_fn>(sch.get_vw_pointer_unsafe(), edge, D);
    }
    ec[n]->indices.push_back(neighbor_namespace);
    ec[n]->total_sum_feat_sq += ec[n]->sum_feat_sq[neighbor_namespace];
    ec[n]->num_features += ec[n]->atomics[neighbor_namespace].size();
  }
  
  void del_edge_features(task_data&D, uint32_t n, vector<example*>&ec) {
    ec[n]->indices.pop();
    ec[n]->total_sum_feat_sq -= ec[n]->sum_feat_sq[neighbor_namespace];
    ec[n]->num_features -= ec[n]->atomics[neighbor_namespace].size();
    ec[n]->atomics[neighbor_namespace].erase();
    ec[n]->sum_feat_sq[neighbor_namespace] = 0.;
  }
  
  void run(Search::search& sch, vector<example*>& ec) {
    task_data& D = *sch.get_task_data<task_data>();

    for (size_t n=0; n<D.N; n++) D.pred[n] = D.K+1;
    
    for (size_t loop=0; loop<D.num_loops; loop++) {
      int start = 0; int end = D.N; int step = 1;
      if (loop % 2 == 1) { start = D.N-1; end=-1; step = -1; } // go inward on odd loops
      for (int n_id = start; n_id != end; n_id += step) {
        uint32_t n = D.bfs[n_id];

        bool add_features = D.use_structure && sch.predictNeedsExample();

        if (add_features) add_edge_features(sch, D, n, ec);
        Search::predictor P = Search::predictor(sch, n+1);
        P.set_input(*ec[n]);
        if (D.separate_learners) P.set_learner_id(loop);
        if (ec[n]->l.cs.costs.size() > 0) // for test examples
          P.set_oracle(ec[n]->l.cs.costs[0].class_index);
        // add all the conditioning
        for (size_t i=0; i<D.adj[n].size(); i++) {
          for (size_t j=0; j<ec[i]->l.cs.costs.size(); j++) {
            uint32_t m = ec[i]->l.cs.costs[j].class_index - 1;
            if (m == n) continue;
            P.add_condition(m+1, 'e');
          }
        }

        // make the prediction
        D.pred[n] = P.predict();
        
        if (add_features) del_edge_features(D, n, ec);
      }
    }

    if (sch.output().good())
      for (uint32_t n=0; n<D.N; n++)
        sch.output() << D.pred[n] << ' ';
  }
}


















