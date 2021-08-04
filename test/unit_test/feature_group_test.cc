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

  auto fs_copy1 = fs;
  auto fs_copy2 = fs;
  auto fs_copy3 = fs;
  auto fs_copy4 = fs;

  // Cap at 5
  unique_features(fs, 5);
  check_collections_exact(
      std::vector<feature_index>(fs.indicies.begin(), fs.indicies.end()), std::vector<feature_index>{1, 2, 3, 5, 7});

  // Uncapped
  unique_features(fs_copy1);
  check_collections_exact(std::vector<feature_index>(fs_copy1.indicies.begin(), fs_copy1.indicies.end()),
      std::vector<feature_index>{1, 2, 3, 5, 7, 11, 12, 13, 25});

  // Special case at max 1
  unique_features(fs_copy2, 1);
  check_collections_exact(
      std::vector<feature_index>(fs_copy2.indicies.begin(), fs_copy2.indicies.end()), std::vector<feature_index>{1});

  // Special case for max 0
  unique_features(fs_copy3, 0);
  BOOST_REQUIRE(fs_copy3.empty());

  // Explicit negative input that isn't -1
  unique_features(fs_copy4, -10);
  check_collections_exact(std::vector<feature_index>(fs_copy4.indicies.begin(), fs_copy4.indicies.end()),
      std::vector<feature_index>{1, 2, 3, 5, 7, 11, 12, 13, 25});

  // Special case for max 0
  features empty_features;
  unique_features(empty_features, 0);
  BOOST_REQUIRE(empty_features.empty());
}
