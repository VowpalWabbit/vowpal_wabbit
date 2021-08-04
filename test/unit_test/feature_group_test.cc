#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "feature_group.h"
#include "unique_sort.h"


BOOST_AUTO_TEST_CASE(unique_feature_group_test)
{
  features fs;
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 2);
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 25);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 5);
  fs.push_back(1.f, 7);
  fs.push_back(1.f, 13);
  fs.push_back(1.f, 11);
  fs.push_back(1.f, 12);

  const auto parse_mask = (static_cast<uint64_t>(1) << 18) - 1;
  fs.sort(parse_mask);
  unique_features(fs, 5);

  check_collections_exact(std::vector<feature_index>(fs.indicies.begin(), fs.indicies.end()),
      std::vector<feature_index>{1, 3, 5, 7, 11});
}
