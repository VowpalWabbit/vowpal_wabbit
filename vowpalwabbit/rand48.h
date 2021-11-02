// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include <cstdint>

float merand48(uint64_t& initial);
float merand48_noadvance(uint64_t v);
float merand48_boxmuller(uint64_t& initial);
