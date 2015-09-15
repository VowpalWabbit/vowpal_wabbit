/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif
#include <sys/timeb.h>
#include "parse_args.h"
#include "accumulate.h"
#include "best_constant.h"
#include "vw_exception.h"
// #include <boost/filesystem.hpp>
#include <fstream>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#else
#include <wordexp.h>
#endif

// see http://stackoverflow.com/questions/1706551/parse-string-into-argv-argc
char **split_commandline(const char *cmdline, int *argc)
{
	int i;
	char **argv = NULL;
	assert(argc);

	if (!cmdline)
	{
		return NULL;
	}

	// Posix.
#ifndef _WIN32
	{
		wordexp_t p;

		// Note! This expands shell variables.
		if (wordexp(cmdline, &p, 0))
		{
			return NULL;
		}

		*argc = p.we_wordc;

		if (!(argv = (char**)calloc(*argc, sizeof(char *))))
		{
			goto fail;
		}

		for (i = 0; i < (int)p.we_wordc; i++)
		{
			if (!(argv[i] = strdup(p.we_wordv[i])))
			{
				goto fail;
			}
		}

		wordfree(&p);

		return argv;
	fail:
		wordfree(&p);
	}
#else // WIN32
	{
		wchar_t **wargs = NULL;
		size_t needed = 0;
		wchar_t *cmdlinew = NULL;
		size_t len = strlen(cmdline) + 1;

		if (!(cmdlinew = (wchar_t*)calloc(len, sizeof(wchar_t))))
			goto fail;

		if (!MultiByteToWideChar(CP_ACP, 0, cmdline, -1, cmdlinew, (int)len))
			goto fail;

		if (!(wargs = CommandLineToArgvW(cmdlinew, argc)))
			goto fail;

		if (!(argv = (char**)calloc(*argc, sizeof(char *))))
			goto fail;

		// Convert from wchar_t * to ANSI char *
		for (i = 0; i < *argc; i++)
		{
			// Get the size needed for the target buffer.
			// CP_ACP = Ansi Codepage.
			needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1,
				NULL, 0, NULL, NULL);

			if (!(argv[i] = (char*)malloc(needed)))
				goto fail;

			// Do the conversion.
			needed = WideCharToMultiByte(CP_ACP, 0, wargs[i], -1,
				argv[i], (int)needed, NULL, NULL);
		}

		if (wargs) LocalFree(wargs);
		if (cmdlinew) free(cmdlinew);
		return argv;

	fail:
		if (wargs) LocalFree(wargs);
		if (cmdlinew) free(cmdlinew);
	}
#endif // WIN32

	if (argv)
	{
		for (i = 0; i < *argc; i++)
		{
			if (argv[i])
			{
				free(argv[i]);
			}
		}

		free(argv);
	}

	return NULL;
}

vw& setup(int argc, char *argv[])
{
	vw& all = parse_args(argc, argv);
	io_buf model;
	parse_regressor_args(all, model);
	parse_modules(all, model);
	parse_sources(all, model);

	all.vw_is_main = true;

	if (!all.quiet && !all.bfgs && !all.searchstr)
	{
		std::cerr << std::left
			<< std::setw(shared_data::col_avg_loss) << std::left << "average"
			<< " "
			<< std::setw(shared_data::col_since_last) << std::left << "since"
			<< " "
			<< std::right
			<< std::setw(shared_data::col_example_counter) << "example"
			<< " "
			<< std::setw(shared_data::col_example_weight) << "example"
			<< " "
			<< std::setw(shared_data::col_current_label) << "current"
			<< " "
			<< std::setw(shared_data::col_current_predict) << "current"
			<< " "
			<< std::setw(shared_data::col_current_features) << "current"
			<< std::endl;
		std::cerr << std::left
			<< std::setw(shared_data::col_avg_loss) << std::left << "loss"
			<< " "
			<< std::setw(shared_data::col_since_last) << std::left << "last"
			<< " "
			<< std::right
			<< std::setw(shared_data::col_example_counter) << "counter"
			<< " "
			<< std::setw(shared_data::col_example_weight) << "weight"
			<< " "
			<< std::setw(shared_data::col_current_label) << "label"
			<< " "
			<< std::setw(shared_data::col_current_predict) << "predict"
			<< " "
			<< std::setw(shared_data::col_current_features) << "features"
			<< std::endl;
	}
	return all;
}

int main(int argc, char *argv[])
{
  try {
	  // support multiple vw instances for training of the same datafile for the same instance
	  vector<vw*> alls;
	  if (argc == 3 && !strcmp(argv[1], "--args"))
	  {
		  std::fstream arg_file(argv[2]);

		  int line_count = 1;
		  std::string line;
		  while (std::getline(arg_file, line))
		  {
			  std::stringstream sstr;
			  sstr << line << " -f model." << (line_count++);

			  std::cout << sstr.str() << endl;
			  auto str = sstr.str();
			  const char* new_args = str.c_str();

			  int l_argc;
			  char** l_argv = split_commandline(new_args, &l_argc);

			  alls.push_back(&setup(l_argc, l_argv));
		  }
	  }
	  else
	  {
		  alls.push_back(&setup(argc, argv));
	  }

	  vw& all = *alls[0];
	  struct timeb t_start, t_end;
	  ftime(&t_start);

	  VW::start_parser(all);
	  if (alls.size() == 1)
		  LEARNER::generic_driver(all);
	  else
		  LEARNER::generic_driver(alls);

	  VW::end_parser(all);

	  ftime(&t_end);
	  double net_time = (int)(1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm));
	  if (!all.quiet && all.all_reduce != nullptr)
		  cerr << "Net time taken by process = " << net_time / (double)(1000) << " seconds\n";

	  for (auto v : alls) {
		  VW::sync_stats(*v);
		  VW::finish(*v);
	  }
  } catch (VW::vw_exception& e) {
    cerr << "vw (" << e.Filename() << ":" << e.LineNumber() << "): " << e.what() << endl;
  } catch (exception& e) {
    // vw is implemented as a library, so we use 'throw runtime_error()'
    // error 'handling' everywhere.  To reduce stderr pollution
    // everything gets caught here & the error message is printed
    // sans the excess exception noise, and core dump.
    cerr << "vw: " << e.what() << endl;
    exit(1);
  }
  return 0;

