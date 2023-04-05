// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/common/string_view.h"
#include "vw/common/text_utils.h"
#include "vw/core/cb_with_observations_label.h"
#include "vw/core/memory.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/parser.h"
#include "vw/io/logger.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

TEST(CbWithObservations, CacheLabel)
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));

  VW::cb_with_observations_label cb_with_obs_label;
  cb_with_obs_label.event.weight = 5.f;
  cb_with_obs_label.is_definitely_bad = true;
  cb_with_obs_label.is_observation = true;

  VW::model_utils::write_model_field(io_writer, cb_with_obs_label, "", false);
  io_writer.flush();

  VW::io_buf io_reader;
  io_reader.add_file(VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  auto uncached_label = VW::make_unique<VW::cb_with_observations_label>();
  uncached_label->reset_to_default();
  VW::model_utils::read_model_field(io_reader, *uncached_label);

  EXPECT_FLOAT_EQ(uncached_label->event.weight, 5.f);
  EXPECT_EQ(uncached_label->is_definitely_bad, true);
  EXPECT_EQ(uncached_label->is_observation, true);
}
