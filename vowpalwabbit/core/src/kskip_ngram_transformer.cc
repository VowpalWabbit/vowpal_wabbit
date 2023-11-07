// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/kskip_ngram_transformer.h"

#include "vw/io/logger.h"

#include <memory>

void add_grams(size_t ngram, size_t skip_gram, VW::features& fs, size_t initial_length, std::vector<size_t>& gram_mask,
    size_t skips)
{
  if (ngram == 0 && gram_mask.back() < initial_length)
  {
    size_t last = initial_length - gram_mask.back();
    for (size_t i = 0; i < last; i++)
    {
      uint64_t new_index = fs.indices[i];
      for (size_t n = 1; n < gram_mask.size(); n++)
      {
        new_index = new_index * VW::details::QUADRATIC_CONSTANT + fs.indices[i + gram_mask[n]];
      }

      fs.push_back(1., new_index);
      if (!fs.space_names.empty())
      {
        std::string feature_name(fs.space_names[i].name);
        for (size_t n = 1; n < gram_mask.size(); n++)
        {
          feature_name += std::string("^");
          feature_name += std::string(fs.space_names[i + gram_mask[n]].name);
        }
        fs.space_names.emplace_back(fs.space_names[i].ns, feature_name);
      }
    }
  }
  if (ngram > 0)
  {
    gram_mask.push_back(gram_mask.back() + 1 + skips);
    add_grams(ngram - 1, skip_gram, fs, initial_length, gram_mask, 0);
    gram_mask.pop_back();
  }
  if (skip_gram > 0 && ngram > 0) { add_grams(ngram, skip_gram - 1, fs, initial_length, gram_mask, skips + 1); }
}

void compile_gram(const std::vector<std::string>& grams, std::array<uint32_t, VW::NUM_NAMESPACES>& dest,
    const std::string& descriptor, bool /*quiet*/, VW::io::logger& logger)
{
  for (const auto& gram : grams)
  {
    if (isdigit(gram[0]) != 0)
    {
      int n = atoi(gram.c_str());
      logger.err_info("Generating {0}-{1} for all namespaces.", n, descriptor);
      for (size_t j = 0; j < VW::NUM_NAMESPACES; j++) { dest[j] = n; }
    }
    else if (gram.size() == 1) { logger.out_error("The namespace index must be specified before the n"); }
    else
    {
      int n = atoi(gram.c_str() + 1);
      dest[static_cast<uint32_t>(static_cast<unsigned char>(*gram.c_str()))] = n;
      logger.err_info("Generating {0}-{1} for {2} namespaces.", n, descriptor, gram[0]);
    }
  }
}

void VW::kskip_ngram_transformer::generate_grams(example* ex)
{
  for (namespace_index index : ex->indices)
  {
    size_t length = ex->feature_space[index].size();
    for (size_t n = 1; n < ngram_definition[index]; n++)
    {
      gram_mask.clear();
      gram_mask.push_back(0);
      add_grams(n, skip_definition[index], ex->feature_space[index], length, gram_mask, 0);
    }
  }
}

VW::kskip_ngram_transformer VW::kskip_ngram_transformer::build(
    const std::vector<std::string>& grams, const std::vector<std::string>& skips, bool quiet, VW::io::logger& logger)
{
  kskip_ngram_transformer transformer(grams, skips);

  compile_gram(grams, transformer.ngram_definition, "grams", quiet, logger);
  compile_gram(skips, transformer.skip_definition, "skips", quiet, logger);
  return transformer;
}

VW::kskip_ngram_transformer::kskip_ngram_transformer(std::vector<std::string> grams, std::vector<std::string> skips)
    : initial_ngram_definitions(std::move(grams)), initial_skip_definitions(std::move(skips))
{
  ngram_definition.fill(0);
  skip_definition.fill(0);
}
