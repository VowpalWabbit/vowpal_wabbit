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
  fs.start_ns_extent(1);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 5);
  fs.end_ns_extent();
  fs.push_back(1.f, 7);
  fs.start_ns_extent(2);
  fs.push_back(1.f, 13);
  fs.push_back(1.f, 11);
  fs.push_back(1.f, 12);
  fs.end_ns_extent();

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
  check_collections_exact(fs.namespace_extents, std::vector<VW::namespace_extent>{{2, 4, 1}});

  // Uncapped
  unique_features(fs_copy1);
  check_collections_exact(std::vector<feature_index>(fs_copy1.indicies.begin(), fs_copy1.indicies.end()),
      std::vector<feature_index>{1, 2, 3, 5, 7, 11, 12, 13, 25});
  check_collections_exact(fs_copy1.namespace_extents, std::vector<VW::namespace_extent>{{2, 4, 1}, {5, 8, 2}});

  // Special case at max 1
  unique_features(fs_copy2, 1);
  check_collections_exact(
      std::vector<feature_index>(fs_copy2.indicies.begin(), fs_copy2.indicies.end()), std::vector<feature_index>{1});
  BOOST_REQUIRE(fs_copy2.namespace_extents.empty());

  // Special case for max 0
  unique_features(fs_copy3, 0);
  BOOST_REQUIRE(fs_copy3.empty());
  BOOST_REQUIRE(fs_copy3.namespace_extents.empty());

  // Explicit negative input that isn't -1
  unique_features(fs_copy4, -10);
  check_collections_exact(std::vector<feature_index>(fs_copy4.indicies.begin(), fs_copy4.indicies.end()),
      std::vector<feature_index>{1, 2, 3, 5, 7, 11, 12, 13, 25});
  check_collections_exact(fs_copy4.namespace_extents, std::vector<VW::namespace_extent>{{2, 4, 1}, {5, 8, 2}});

  // Special case for max 0
  features empty_features;
  unique_features(empty_features, 0);
  BOOST_REQUIRE(empty_features.empty());
  BOOST_REQUIRE(empty_features.namespace_extents.empty());

  features fs_size_one;
  fs_size_one.push_back(1.f, 1);
  unique_features(fs_size_one);
  check_collections_exact(std::vector<feature_index>(fs_size_one.indicies.begin(), fs_size_one.indicies.end()),
      std::vector<feature_index>{1});
  BOOST_REQUIRE(fs_size_one.namespace_extents.empty());
}

BOOST_AUTO_TEST_CASE(flatten_then_unflatten_namespace_extents_test)
{
  std::vector<VW::namespace_extent> extents{{0, 1, 1}, {1, 2, 2}};

  auto flat_list = VW::details::flatten_namespace_extents(extents, 2);
  auto unflattened_list = VW::details::unflatten_namespace_extents(flat_list);
  BOOST_REQUIRE(unflattened_list == extents);

  extents = {{1, 2, 123}};

  flat_list = VW::details::flatten_namespace_extents(extents, 2);
  unflattened_list = VW::details::unflatten_namespace_extents(flat_list);
  BOOST_REQUIRE(unflattened_list == extents);

  extents = {{1, 2, 1}, {5, 8, 2}, {8, 12, 3}, {13, 14, 4}};
  flat_list = VW::details::flatten_namespace_extents(extents, 18);
  unflattened_list = VW::details::unflatten_namespace_extents(flat_list);
  BOOST_REQUIRE(unflattened_list == extents);
}

BOOST_AUTO_TEST_CASE(sort_feature_group_test)
{
  features fs;
  fs.push_back(1.f, 1);
  fs.push_back(1.f, 25);
  fs.start_ns_extent(1);
  fs.push_back(1.f, 3);
  fs.push_back(1.f, 5);
  fs.end_ns_extent();
  fs.push_back(1.f, 7);
  fs.start_ns_extent(2);
  fs.push_back(1.f, 13);
  fs.push_back(1.f, 11);
  fs.push_back(1.f, 12);
  fs.end_ns_extent();

  const auto parse_mask = (static_cast<uint64_t>(1) << 18) - 1;
  fs.sort(parse_mask);

  check_collections_exact(std::vector<feature_index>(fs.indicies.begin(), fs.indicies.end()),
      std::vector<feature_index>{1, 3, 5, 7, 11, 12, 13, 25});
  check_collections_exact(fs.namespace_extents, std::vector<VW::namespace_extent>{{1, 3, 1}, {4, 7, 2}});
}


BOOST_AUTO_TEST_CASE(iterate_extents_test)
{
  auto* vw = VW::initialize("--quiet");
  auto* ex = VW::read_example(*vw, "|user_info a b c |user_geo a b c d |other a b c d e |user_info a b");
  auto cleanup = VW::scope_exit([&]()
  {
      VW::finish_example(*vw, *ex);
      VW::finish(*vw);
  });

  {
    auto begin = ex->feature_space['u'].hash_extents_begin(VW::hash_space(*vw, "user_info"));
    const auto end = ex->feature_space['u'].hash_extents_end(VW::hash_space(*vw, "user_info"));
    BOOST_REQUIRE_EQUAL(std::distance(begin, end), 2);
    BOOST_REQUIRE_EQUAL(std::distance((*begin).first, (*begin).second), 3);
    ++begin;
    BOOST_REQUIRE_EQUAL(std::distance((*begin).first, (*begin).second), 2);
  }

  // seek first
  {
    auto begin = ex->feature_space['u'].hash_extents_begin(VW::hash_space(*vw, "user_geo"));
    const auto end = ex->feature_space['u'].hash_extents_end(VW::hash_space(*vw, "user_geo"));
    BOOST_REQUIRE_EQUAL(std::distance(begin, end), 1);
    BOOST_REQUIRE_EQUAL(std::distance((*begin).first, (*begin).second), 4);
  }

  // Different first char
  {
    auto begin = ex->feature_space['o'].hash_extents_begin(VW::hash_space(*vw, "other"));
    const auto end = ex->feature_space['o'].hash_extents_end(VW::hash_space(*vw, "other"));
    BOOST_REQUIRE_EQUAL(std::distance(begin, end), 1);
    BOOST_REQUIRE_EQUAL(std::distance((*begin).first, (*begin).second), 5);
  }
}
