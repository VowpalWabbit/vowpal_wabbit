#include "vowpalwabbit.h"

namespace vw_lib {

	vowpalwabbit::vowpalwabbit(vw_settings *settings) {

	}
	vowpalwabbit::vowpalwabbit(const char * args) : vowpalwabbit(new vw_settings(args))
	{
	}
}
