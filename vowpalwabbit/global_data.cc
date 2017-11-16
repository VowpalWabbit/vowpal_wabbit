/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
 */
#include <stdio.h>
#include <float.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <assert.h>

#include "global_data.h"
#include "gd.h"
#include "vw_exception.h"
#include <boost/foreach.hpp>

using namespace std;

struct global_prediction
{ float p;
  float weight;
};

size_t really_read(int sock, void* in, size_t count)
{ char* buf = (char*)in;
  size_t done = 0;
  int r = 0;
  while (done < count)
  { if ((r =
#ifdef _WIN32
           recv(sock,buf,(unsigned int)(count-done),0)
#else
           read(sock,buf,(unsigned int)(count-done))
#endif
        ) == 0)
      return 0;
    else if (r < 0)
    { THROWERRNO("read(" << sock << "," << count << "-" << done << ")");
    }
    else
    { done += r;
      buf += r;
    }
  }
  return done;
}

void get_prediction(int sock, float& res, float& weight)
{ global_prediction p;
  really_read(sock, &p, sizeof(p));
  res = p.p;
  weight = p.weight;
}

void send_prediction(int sock, global_prediction p)
{ if (
#ifdef _WIN32
    send(sock, reinterpret_cast<const char*>(&p), sizeof(p), 0)
#else
    write(sock, &p, sizeof(p))
#endif
    < (int)sizeof(p))
    THROWERRNO("send_prediction write(" << sock << ")");
}

void binary_print_result(int f, float res, float weight, v_array<char>)
{ if (f >= 0)
  { global_prediction ps = {res, weight};
    send_prediction(f, ps);
  }
}

int print_tag(std::stringstream& ss, v_array<char> tag)
{ if (tag.begin() != tag.end())
  { ss << ' ';
    ss.write(tag.begin(), sizeof(char)*tag.size());
  }
  return tag.begin() != tag.end();
}

void print_result(int f, float res, float, v_array<char> tag)
{ if (f >= 0)
  { char temp[30];
    if (floorf(res) != res)
      sprintf(temp, "%f", res);
    else
      sprintf(temp, "%.0f", res);
    std::stringstream ss;
    ss << temp;
    print_tag(ss, tag);
    ss << '\n';
    ssize_t len = ss.str().size();
    ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
    if (t != len)
    { cerr << "write error: " << strerror(errno) << endl;
    }
  }
}

void print_raw_text(int f, string s, v_array<char> tag)
{ if (f < 0)
    return;

  std::stringstream ss;
  ss << s;
  print_tag (ss, tag);
  ss << '\n';
  ssize_t len = ss.str().size();
  ssize_t t = io_buf::write_file_or_socket(f, ss.str().c_str(), (unsigned int)len);
  if (t != len)
  { cerr << "write error: " << strerror(errno) << endl;
  }
}

void set_mm(shared_data* sd, float label)
{ sd->min_label = min(sd->min_label, label);
  if (label != FLT_MAX)
    sd->max_label = max(sd->max_label, label);
}

void noop_mm(shared_data*, float) {}

void vw::learn(example* ec)
{
	if (ec->test_only || !training)
		l->predict(*ec);
	else
		l->learn(*ec);
}

void compile_gram(vector<string> grams, uint32_t* dest, char* descriptor, bool quiet)
{ for (size_t i = 0; i < grams.size(); i++)
  { string ngram = grams[i];
    if ( isdigit(ngram[0]) )
    { int n = atoi(ngram.c_str());
      if (!quiet)
        cerr << "Generating " << n << "-" << descriptor << " for all namespaces." << endl;
      for (size_t j = 0; j < 256; j++)
        dest[j] = n;
    }
    else if ( ngram.size() == 1)
      cout << "You must specify the namespace index before the n" << endl;
    else
    { int n = atoi(ngram.c_str()+1);
      dest[(uint32_t)(unsigned char)*ngram.c_str()] = n;
      if (!quiet)
        cerr << "Generating " << n << "-" << descriptor << " for " << ngram[0] << " namespaces." << endl;
    }
  }
}

void compile_limits(vector<string> limits, uint32_t* dest, bool quiet)
{ for (size_t i = 0; i < limits.size(); i++)
  { string limit = limits[i];
    if ( isdigit(limit[0]) )
    { int n = atoi(limit.c_str());
      if (!quiet)
        cerr << "limiting to " << n << "features for each namespace." << endl;
      for (size_t j = 0; j < 256; j++)
        dest[j] = n;
    }
    else if ( limit.size() == 1)
      cout << "You must specify the namespace index before the n" << endl;
    else
    { int n = atoi(limit.c_str()+1);
      dest[(uint32_t)limit[0]] = n;
      if (!quiet)
        cerr << "limiting to " << n << " for namespaces " << limit[0] << endl;
    }
  }
}

std::vector<std::string> opts_to_args(const std::vector<boost::program_options::option>& opts)
{
	std::vector<std::string> args;

	BOOST_FOREACH(const boost::program_options::option& option, opts)
	{
		if (option.unregistered)
		{
			args.insert(args.end(), option.original_tokens.begin(), option.original_tokens.end());
			continue;
		}

		if (option.value.empty())
		{
			args.push_back("--" + option.string_key);
			continue;
		}

		BOOST_FOREACH(const std::string& value, option.value)
		{
			if (option.string_key.length() > 0)
				args.push_back("--" + option.string_key);
			args.push_back(value);
		}
	}

	return args;
}

// blackbox wrapping of boost program options to ignore duplicate specification of options allowed only ones, but specified multiple times
// Behavior: only the first occurence is kept
// Strategy: add one argument after each other until we trigger multiple_occurrences exception. Special care has to be taken of arguments to options.
po::variables_map add_options_skip_duplicates(vw& all, po::options_description& opts, bool do_notify)
{
	std::vector<std::string> args(all.args);
	po::variables_map new_vm;

	for (int i = 0;i<2;i++)
	{
		// i = 0: initial parse attempt
		// i = 1: retry attempt after removing dups
		try
		{
			po::parsed_options parsed = po::command_line_parser(args).
				style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
				options(opts).allow_unregistered().run();
			po::store(parsed, new_vm);

			// unique multi elements to avoid infinite growth
			for (auto& it : new_vm)
			{
				if (it.second.value().type() == typeid(vector<string>))
				{
					auto& values = it.second.as<vector<string>>();
					auto end = unique(values.begin(), values.end());
					values.erase(end, values.end());
				}
			}

			if (do_notify)
				po::notify(new_vm);

			// re-create args after unique
			all.args = opts_to_args(parsed.options);
			return new_vm;
		}
		catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::multiple_occurrences>>&)
		{ }

		args.clear();
		bool previous_option_needs_argument = false;
        for (auto&& arg : all.args)
		{
			new_vm.clear();
			args.push_back(arg);
			try
			{
				po::parsed_options parsed = po::command_line_parser(args).
					style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
					options(opts).allow_unregistered().run();
				po::store(parsed, new_vm);

				if (do_notify)
					po::notify(new_vm);

				previous_option_needs_argument = false;
			}
			catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::multiple_occurrences>>&)
			{
				auto ignored = arg;

				args.pop_back();
				if (previous_option_needs_argument)
				{
					auto option = args.back();
					auto duplicate_value = ignored;
					ignored =  option + " " + ignored;
					args.pop_back();

					// check if at least the values are the same

					// reparse arguments so far
					new_vm.clear();
					po::parsed_options parsed_full = po::command_line_parser(args).
						style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
						options(opts).allow_unregistered().run();
					po::store(parsed_full, new_vm);

					// parse just the duplicate option
					vector<string> sub_args;
					sub_args.push_back(option);
					sub_args.push_back(duplicate_value);

					po::variables_map sub_vm;
					po::parsed_options parsed_dup = po::command_line_parser(sub_args).
						style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing).
						options(opts).allow_unregistered().run();
					po::store(parsed_dup, sub_vm);

					// we need to compare the parsed actions to overcome different representation of the same value
					// e.g. --epsilon 0.1 vs --epsilon 0.10000
					auto duplicate_option = sub_vm.begin();
					auto first_option_occurrence = new_vm.find(duplicate_option->first);

					if (first_option_occurrence == new_vm.end() || duplicate_option == sub_vm.end())
						THROW("unable to find duplicate option");

					bool found_disagreement = false;
					if (duplicate_option->second.value().type() == typeid(string))
						found_disagreement = duplicate_option->second.as<string>() != first_option_occurrence->second.as<string>();
					else if (duplicate_option->second.value().type() == typeid(float))
						found_disagreement = duplicate_option->second.as<float>() != first_option_occurrence->second.as<float>();
					else if (duplicate_option->second.value().type() == typeid(double))
						found_disagreement = duplicate_option->second.as<double>() != first_option_occurrence->second.as<double>();
					else if (duplicate_option->second.value().type() == typeid(int))
						found_disagreement = duplicate_option->second.as<int>() != first_option_occurrence->second.as<int>();
					else if (duplicate_option->second.value().type() == typeid(size_t))
						found_disagreement = duplicate_option->second.as<size_t>() != first_option_occurrence->second.as<size_t>();
					else if (duplicate_option->second.value().type() == typeid(uint32_t))
						found_disagreement = duplicate_option->second.as<uint32_t>() != first_option_occurrence->second.as<uint32_t>();
					else
						THROW("Unsupported type for option '" << duplicate_option->first << "'");

					if (found_disagreement)
					{
						// get the original string value
						auto duplicate_option = parsed_dup.options.begin();
						auto first_option_occurrence = std::find_if(parsed_full.options.begin(), parsed_full.options.end(),
							[&duplicate_option](po::option& o) { return duplicate_option->string_key == o.string_key; });

						if (first_option_occurrence == parsed_full.options.end() || duplicate_option == parsed_dup.options.end())
							THROW("unable to find duplicate option");

						auto duplicate_option_value = *duplicate_option->value.begin();
						auto first_option_occurrence_value = *first_option_occurrence->value.begin();

						THROW_EX(VW::vw_argument_disagreement_exception, "Disagreeing option values for '" << option << "': '" << first_option_occurrence_value << "' vs '" << duplicate_option_value << "'");
					}
				}

				all.trace_message << "ignoring duplicate option: '" << ignored << "'" << endl;
			}
			catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::invalid_command_line_syntax>>& e)
			{
				// remember that this option needs an argument to be able to remove the option along with the argument
				// in the next iteration
				if (e.kind() == e.missing_parameter)
					previous_option_needs_argument = true;
			}
			catch (...)
			{
				// ignore anything else
			}
		}

		new_vm.clear();
		// parse ones more to trigger any other exception
	}

	THROW("failed to de-duplicate arguments");
}

void add_options(vw& all, po::options_description& opts)
{ all.opts.add(opts);

  auto new_vm = add_options_skip_duplicates(all, opts, true /* do_notify */);

	for (auto& it : new_vm)
		all.vm.insert(it);
}

void add_options(vw& all)
{ add_options(all, *all.new_opts);
  delete all.new_opts;
}

bool no_new_options(vw& all)
{ //parse local opts once for notifications.
	auto new_vm = add_options_skip_duplicates(all, *all.new_opts, false /* do_notify */);

	all.opts.add(*all.new_opts);

	delete all.new_opts;
	for (auto& it : new_vm)
		all.vm.insert(it);

	if (new_vm.size() == 0) // required are missing;
		return true;
	else
		return false;
}

bool missing_option(vw& all, bool keep, const char* name, const char* description)
{ new_options(all)(name,description);
  if (no_new_options(all))
    return true;
  if (keep)
    *all.file_options << " --" << name;
  return false;
}

void trace_listener_cerr(void*, const std::string& message)
{
	cerr << message;
	cerr.flush();
}

vw_ostream::vw_streambuf::vw_streambuf(vw_ostream& str) : parent(str)
{}

int vw_ostream::vw_streambuf::sync()
{
	int ret = std::stringbuf::sync();
	if (ret)
		return ret;

	parent.trace_listener(parent.trace_context, str());
	str("");
	return 0; // success
}

vw_ostream::vw_ostream() : std::ostream(&buf), buf(*this), trace_context(nullptr)
{
	trace_listener = trace_listener_cerr;
}

vw::vw(const vw &)
{ THROW("Copy constructor not supported");
}

vw::vw()
{ sd = &calloc_or_throw<shared_data>();
  sd->dump_interval = 1.;   // next update progress dump
  sd->contraction = 1.;
  sd->first_observed_label = FLT_MAX;
  sd->is_more_than_two_labels_observed = false;
  sd->max_label = 0;
  sd->min_label = 0;

  p = new_parser();
  p->emptylines_separate_examples = false;
  p->lp = simple_label;
  label_type = label_type::simple;

  l = nullptr;
  scorer = nullptr;
  cost_sensitive = nullptr;
  loss = nullptr;

  reg_mode = 0;
  current_pass = 0;
  reduction_stack=v_init<LEARNER::base_learner* (*)(vw&)>();

  data_filename = "";
  delete_prediction = nullptr;

  file_options = new std::stringstream;

  bfgs = false;
  no_bias = false;
  hessian_on = false;
  active = false;
  num_bits = 18;
  default_bits = true;
  daemon = false;
  num_children = 10;
  save_resume = false;
  preserve_performance_counters = false;

  random_positive_weights = false;

  weights.sparse = false;

  set_minmax = set_mm;

  power_t = 0.5;
  eta = 0.5; //default learning rate for normalized adaptive updates, this is switched to 10 by default for the other updates (see parse_args.cc)
  numpasses = 1;

  final_prediction_sink.begin() = final_prediction_sink.end() = final_prediction_sink.end_array = nullptr;
  raw_prediction = -1;
  print = print_result;
  print_text = print_raw_text;
  lda = 0;
  random_seed = 0;
  random_weights = false;
  normal_weights = false;
  tnormal_weights = false;
  per_feature_regularizer_input = "";
  per_feature_regularizer_output = "";
  per_feature_regularizer_text = "";

#ifdef _WIN32
  stdout_fileno = _fileno(stdout);
#else
  stdout_fileno = fileno(stdout);
#endif

  searchstr = nullptr;

  nonormalize = false;
  l1_lambda = 0.0;
  l2_lambda = 0.0;

  eta_decay_rate = 1.0;
  initial_weight = 0.0;
  initial_constant = 0.0;

  all_reduce = nullptr;

  for (size_t i = 0; i < 256; i++)
  { ngram[i] = 0;
    skips[i] = 0;
    limit[i] = INT_MAX;
    affix_features[i] = 0;
    spelling_features[i] = 0;
  }

  interactions = v_init<v_string>();

  //by default use invariant normalized adaptive updates
  adaptive = true;
  normalized_updates = true;
  invariant_updates = true;

  normalized_idx = 2;

  add_constant = true;
  audit = false;

  pass_length = (size_t)-1;
  passes_complete = 0;

  save_per_pass = false;

  stdin_off = false;
  do_reset_source = false;
  holdout_set_off = true;
  holdout_period = 10;
  holdout_after = 0;
  check_holdout_every_n_passes = 1;
  early_terminate = false;

  max_examples = (size_t)-1;

  hash_inv = false;
  print_invert = false;

  // Set by the '--progress <arg>' option and affect sd->dump_interval
  progress_add = false;   // default is multiplicative progress dumps
  progress_arg = 2.0;     // next update progress dump multiplier

  sd->is_more_than_two_labels_observed = false;
  sd->first_observed_label = FLT_MAX;
  sd->second_observed_label = FLT_MAX;

  sd->report_multiclass_log_loss = false;
  sd->multiclass_log_loss = 0;
  sd->holdout_multiclass_log_loss = 0;
}
