#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <float.h>
#include <math.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include <limits.h>

#ifdef MANAGED_CODE
#define PORTING_INTERFACE public
#define MWT_NAMESPACE namespace NativeMultiWorldTesting
#else
#define PORTING_INTERFACE private
#define MWT_NAMESPACE namespace MultiWorldTesting
#endif

// TODO: reference additional headers your program requires here
#include "utility.h"

using namespace std;
