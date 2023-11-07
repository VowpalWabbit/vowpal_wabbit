// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw/core/constant.h"
#include "vw/core/example.h"

#include <string>
#include <vector>

namespace VW
{
class kskip_ngram_transformer
{
public:
  static kskip_ngram_transformer build(
      const std::vector<std::string>& grams, const std::vector<std::string>& skips, bool quiet, VW::io::logger& logger);

  /**
   * This function adds k-skip-n-grams to the feature vector.
   * Definition of k-skip-n-grams:
   * Consider a feature vector - a, b, c, d, e, f
   * 2-skip-2-grams would be - ab, ac, ad, bc, bd, be, cd, ce, cf, de, df, ef
   * 1-skip-3-grams would be - abc, abd, acd, ace, bcd, bce, bde, bdf, cde, cdf, cef, def
   * Note that for a n-gram, (n-1)-grams, (n-2)-grams... 2-grams are also appended
   * The k-skip-n-grams are appended to the feature vector.
   * Hash is evaluated using the principle h(a, b) = h(a)*X + h(b), where X is a random no.
   * 32 random nos. are maintained in an array and are used in the hashing.
   */
  void generate_grams(example* ex);

  std::vector<std::string> get_initial_ngram_definitions() const { return initial_ngram_definitions; }
  std::vector<std::string> get_initial_skip_definitions() const { return initial_skip_definitions; }

  kskip_ngram_transformer(const kskip_ngram_transformer& other) = default;
  kskip_ngram_transformer& operator=(const kskip_ngram_transformer& other) = default;
  kskip_ngram_transformer(kskip_ngram_transformer&& other) = default;
  kskip_ngram_transformer& operator=(kskip_ngram_transformer&& other) = default;

private:
  kskip_ngram_transformer(std::vector<std::string> grams, std::vector<std::string> skips);

  std::vector<size_t> gram_mask;
  std::array<uint32_t, NUM_NAMESPACES> ngram_definition;
  std::array<uint32_t, NUM_NAMESPACES> skip_definition;
  std::vector<std::string> initial_ngram_definitions;
  std::vector<std::string> initial_skip_definitions;
};
}  // namespace VW
