/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */

#include "reductions.h"
#include "interactions.h"
#include "parse_args.h"
#include "vw.h"
using namespace std;
struct audit_regressor_data
{ vw* all;
  size_t increment;
  size_t cur_class;
  size_t total_class_cnt;
  vector<string>* ns_pre;
  io_buf* out_file;
  size_t loaded_regressor_values;
  size_t values_audited;
};

inline void audit_regressor_interaction(audit_regressor_data& dat, const audit_strings* f)
{ // same as audit_interaction in gd.cc
  if (f == nullptr)
  { dat.ns_pre->pop_back();
    return;
  }

  string ns_pre;
  if (!dat.ns_pre->empty())
    ns_pre += '*';

  if (f->first != "" && ((f->first) != " "))
  { ns_pre.append(f->first);
    ns_pre += '^';
  }
  if (f->second != "")
  { ns_pre.append(f->second);
    dat.ns_pre->push_back(ns_pre);
  }
}

inline void audit_regressor_feature(audit_regressor_data& dat, const float, const uint64_t ft_idx)
{
  parameters& weights = dat.all->weights;
  if (weights[ft_idx] != 0)
    ++dat.values_audited;
  else return;
  
  string ns_pre;
  for (vector<string>::const_iterator s = dat.ns_pre->begin(); s != dat.ns_pre->end(); ++s) ns_pre += *s;
  
  ostringstream tempstream;
  tempstream << ':' << ((ft_idx & weights.mask()) >> weights.stride_shift()) << ':' << weights[ft_idx];
  
  string temp = ns_pre + tempstream.str() + '\n';
  if (dat.total_class_cnt > 1) // add class prefix for multiclass problems
    temp = to_string(dat.cur_class) + ':' + temp;
  
  bin_write_fixed(*dat.out_file, temp.c_str(), (uint32_t)temp.size());
  
  weights[ft_idx] = 0.; //mark value audited
}

void audit_regressor_lda(audit_regressor_data& rd, LEARNER::base_learner& base, example& ec)
{
	vw& all = *rd.all;
	
	ostringstream tempstream;
	parameters& weights = rd.all->weights;
	for (unsigned char* i = ec.indices.begin(); i != ec.indices.end(); i++)
	{
		features& fs = ec.feature_space[*i];
		for (size_t j = 0; j < fs.size(); ++j)
		{
			tempstream << '\t' << fs.space_names[j].get()->first << '^' << fs.space_names[j].get()->second << ':' << ((fs.indicies[j] >> weights.stride_shift()) & all.parse_mask);
			for (size_t k = 0; k < all.lda; k++)
			{
				weight& w = weights[(fs.indicies[j] + k)];
				tempstream << ':' << w;
				w = 0.;
			}
			tempstream << endl;
		}
	}

	bin_write_fixed(*rd.out_file, tempstream.str().c_str(), (uint32_t)tempstream.str().size());
}



// This is a learner which does nothing with examples.
//void learn(audit_regressor_data&, LEARNER::base_learner&, example&) {}

void audit_regressor(audit_regressor_data& rd, LEARNER::base_learner& base, example& ec)
{
  vw& all = *rd.all;
  
  if (all.lda > 0)
    audit_regressor_lda(rd, base, ec);
  else
    {
      
      rd.cur_class = 0;
      uint64_t old_offset = ec.ft_offset;
      
      while ( rd.cur_class < rd.total_class_cnt )
	{
	  
	  for (unsigned char* i = ec.indices.begin(); i != ec.indices.end(); ++i)
	    { features& fs = ec.feature_space[(size_t)*i];
	      if (fs.space_names.size() > 0)
		for (size_t j = 0; j < fs.size(); ++j)
		  { audit_regressor_interaction(rd, fs.space_names[j].get());
		    audit_regressor_feature(rd, fs.values[j], (uint32_t)fs.indicies[j] + ec.ft_offset);
		    audit_regressor_interaction(rd, NULL);
		  }
	      else
		for (size_t j = 0; j < fs.size(); ++j)
		  audit_regressor_feature(rd, fs.values[j], (uint32_t)fs.indicies[j] + ec.ft_offset);
	    }
	  
	  INTERACTIONS::generate_interactions<audit_regressor_data, const uint64_t, audit_regressor_feature, true, audit_regressor_interaction >(*rd.all, ec, rd);
	  
	  ec.ft_offset += rd.increment;
	  ++rd.cur_class;
	}
      
      ec.ft_offset = old_offset; // make sure example is not changed.
    }
}
void end_examples(audit_regressor_data& d)
{ d.out_file->flush(); // close_file() should do this for me ...
  d.out_file->close_file();
  delete (d.out_file);
  d.out_file = NULL;
  delete d.ns_pre;
  d.ns_pre =  NULL;
}

inline void print_ex(vw& all, size_t ex_processed, size_t vals_found, size_t progress)
{ all.trace_message << std::left
            << std::setw(shared_data::col_example_counter) << ex_processed
            << " " << std::right
            << std::setw(9) << vals_found
            << " "  << std::right
            << std::setw(12) << progress << '%'
            << std::endl;
}

void finish_example(vw& all, audit_regressor_data& dd, example& ec)
{ bool printed = false;
  if (ec.example_counter+1 >= all.sd->dump_interval && !all.quiet)
  { print_ex(all, ec.example_counter+1, dd.values_audited, dd.values_audited*100/dd.loaded_regressor_values);
    all.sd->weighted_unlabeled_examples = (double)(ec.example_counter+1); //used in update_dump_interval
    all.sd->update_dump_interval(all.progress_add, all.progress_arg);
    printed = true;
  }

  if (dd.values_audited == dd.loaded_regressor_values)
  { // all regressor values were audited
    if (!printed)
      print_ex(all, ec.example_counter+1, dd.values_audited, 100);
    set_done(all);
  }

  VW::finish_example(all, &ec);
}

void finish(audit_regressor_data& dat)
{ if (dat.values_audited < dat.loaded_regressor_values)
    dat.all->trace_message << "Note: for some reason audit couldn't find all regressor values in dataset (" <<
         dat.values_audited << " of " << dat.loaded_regressor_values << " found)." << endl;
}

template<class T>
void regressor_values(audit_regressor_data& dat, T& w)
{  for (typename T::iterator iter = w.begin(); iter != w.end(); ++iter)
		if (*iter != 0) dat.loaded_regressor_values++;
}

void init_driver(audit_regressor_data& dat)
{ // checks a few settings that might be applied after audit_regressor_setup() is called

  po::variables_map& vm = dat.all->vm;
  if ( (vm.count("cache_file") || vm.count("cache") ) && !vm.count("kill_cache") )
    THROW("audit_regressor is incompatible with a cache file.  Use it in single pass mode only.");

  dat.all->sd->dump_interval = 1.; // regressor could initialize these if saved with --save_resume
  dat.all->sd->example_number = 0;


  dat.increment = dat.all->l->increment/dat.all->l->weights;
  dat.total_class_cnt = dat.all->l->weights;

  if (dat.all->vm.count("csoaa"))
  { size_t n = dat.all->vm["csoaa"].as<size_t>();
    if (n != dat.total_class_cnt)
    { dat.total_class_cnt = n;
      dat.increment = dat.all->l->increment/n;
    }
  }

  // count non-null feature values in regressor
  if (dat.all->weights.sparse)
    regressor_values(dat, dat.all->weights.sparse_weights);
  else
    regressor_values(dat, dat.all->weights.dense_weights);
  
  if (dat.loaded_regressor_values == 0)
    THROW("regressor has no non-zero weights. Nothing to audit.");

  if (!dat.all->quiet)
  { dat.all->trace_message << "Regressor contains " << dat.loaded_regressor_values << " values\n";
    dat.all->trace_message << std::left
              << std::setw(shared_data::col_example_counter) << "example"
              << " "
              << std::setw(shared_data::col_example_weight) << "values"
              << " "
              << std::setw(shared_data::col_current_label) << "total"
              << std::endl;
    dat.all->trace_message << std::left
              << std::setw(shared_data::col_example_counter) << "counter"
              << " "
              << std::setw(shared_data::col_example_weight) << "audited"
              << " "
              << std::setw(shared_data::col_current_label) << "progress"
              << std::endl;
  }

}



LEARNER::base_learner* audit_regressor_setup(vw& all)
{ if (missing_option<string,false>(all, "audit_regressor", "stores feature names and their regressor values. Same dataset must be used for both regressor training and this mode.")) return nullptr;

  po::variables_map& vm = all.vm;

  string out_file = vm["audit_regressor"].as<string>();
  if (out_file.empty())
    THROW("audit_regressor argument (output filename) is missing.");

  if (all.numpasses > 1)
    THROW("audit_regressor can't be used with --passes > 1.");

  all.audit = true;

  audit_regressor_data& dat = calloc_or_throw<audit_regressor_data>();
  dat.all = &all;
  dat.ns_pre = new vector<string>(); // explicitly invoking vector's constructor
  dat.out_file = new io_buf();
  dat.out_file->open_file( out_file.c_str(), all.stdin_off, io_buf::WRITE );

  LEARNER::learner<audit_regressor_data>& ret = LEARNER::init_learner<audit_regressor_data>(&dat, setup_base(all), audit_regressor, audit_regressor, 1);
  ret.set_end_examples(end_examples);
  ret.set_finish_example(finish_example);
  ret.set_finish(finish);
  ret.set_init_driver(init_driver);

  return LEARNER::make_base<audit_regressor_data>(ret);
}
