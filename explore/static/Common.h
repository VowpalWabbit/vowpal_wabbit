#pragma once

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <utility>
#include <limits.h>

#ifdef MANAGED_CODE
#define PORTING_INTERFACE public
#else
#define PORTING_INTERFACE private
#endif

// TODO: reference additional headers your program requires here
#include "utility.h"
#include "Interaction.h"
#include "PolicyFunc.h"
