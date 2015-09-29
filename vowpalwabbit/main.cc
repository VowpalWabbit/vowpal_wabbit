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

using namespace std;

int main(int argc, char *argv[])
{
  try {
    vw& all = parse_args(argc, argv);
	io_buf model;
	parse_regressor_args(all, model);
	parse_modules(all, model);
	parse_sources(all, model);

    all.vw_is_main = true;
    struct timeb t_start, t_end;
    ftime(&t_start);
    
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

    VW::start_parser(all);
    LEARNER::generic_driver(all);
    VW::end_parser(all);

    ftime(&t_end);
    double net_time = (int) (1000.0 * (t_end.time - t_start.time) + (t_end.millitm - t_start.millitm)); 
    if(!all.quiet && all.all_reduce != nullptr)
        cerr<<"Net time taken by process = "<<net_time/(double)(1000)<<" seconds\n";

	VW::sync_stats(all);
    VW::finish(all);
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
}

