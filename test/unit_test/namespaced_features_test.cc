#ifndef STATIC_LINK_VW
#  define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "test_common.h"

#include "namespaced_features.h"

BOOST_AUTO_TEST_CASE(namespaced_features_test)
{
  VW::namespaced_features feature_groups;
  BOOST_CHECK(feature_groups.empty());

  auto begin_end = feature_groups.get_namespace_index_groups('a');
  BOOST_CHECK(begin_end.second - begin_end.first == 0);

  feature_groups.get_or_create_feature_group(123, 'a');
  feature_groups.get_or_create_feature_group(1234, 'a');

  begin_end = feature_groups.get_namespace_index_groups('a');
  BOOST_CHECK(begin_end.second - begin_end.first == 2);

  BOOST_REQUIRE_THROW(feature_groups[1], VW::vw_exception);
  BOOST_REQUIRE_NO_THROW(feature_groups[123]);

  check_collections_exact(feature_groups.get_indices(), std::set<namespace_index>{'a'});

  feature_groups.remove_feature_group(123);
  begin_end = feature_groups.get_namespace_index_groups('a');
  BOOST_CHECK(begin_end.second - begin_end.first == 1);

  check_collections_exact(feature_groups.get_indices(), std::set<namespace_index>{'a'});
  feature_groups.remove_feature_group(1234);
  check_collections_exact(feature_groups.get_indices(), std::set<namespace_index>{});
}
