#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

MATCHER_P(FloatNearPointwise, tol, "Out of range")
{
  return (std::get<0>(arg) > std::get<1>(arg) - tol && std::get<0>(arg) < std::get<1>(arg) + tol);
}
