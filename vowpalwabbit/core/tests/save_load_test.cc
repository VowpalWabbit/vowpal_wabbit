// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/config/options_cli.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/test_common/test_common.h"
#include "vw/text_parser/parse_example_text.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

using namespace ::testing;

#include <string>

TEST(SaveLoad, SaveResumeBehavesAsIfDatasetConcatenated)
{
  std::array<std::string, 10> input_data = {
      "0.521144 |T PFF |f t1:-0.0236849 t5:-0.10215 r5:0.727735 t10:-0.0387662 r10:0.911208 t20:-0.00777943 "
      "r20:0.952668 t40:0.014542 r40:0.832479 t60:0.00395449 r60:0.724504 t90:0.0281418 r90:0.784653",
      "0.535251 |T WIP |f t1:-0.00195191 t5:-0.112359 r5:0.0279508 t10:-0.0405403 r10:0.605284 t20:0.0126391 "
      "r20:0.823787 t40:0.0221162 r40:0.890048 t60:0.0235056 r60:0.903205 t90:0.0256024 r90:0.923621",
      "0.549824 |T GCC |f t1:-0.0155661 t5:-0.202743 r5:0.161423 t10:-0.00856594 r10:0.689004 t20:0.0949576 "
      "r20:0.882347 t40:0.0576915 r40:0.689529 t60:0.0271459 r60:0.102717 t90:0.0235522 r90:0.0242844",
      "0.585408 |T AAXJ |f t1:-0.0381144 t5:-0.205393 r5:0.598789 t10:-0.0706081 r10:0.826144 t20:0.0431942 "
      "r20:0.922816 t40:0.0408377 r40:0.899879 t60:0.0411133 r60:0.827428 t90:0.0495845 r90:0.799112",
      "0.593261 |T VWO |f t1:-0.0440486 t5:-0.259063 r5:0.65883 t10:-0.0992946 r10:0.870232 t20:0.0563154 r20:0.926398 "
      "t40:0.0606423 r40:0.948557 t60:0.0561357 r60:0.926893 t90:0.0611374 r90:0.909396",
      "0.268074 |T EEV |f t1:0.0864275 t5:0.534149 r5:-0.648219 t10:0.18622 r10:-0.867711 t20:-0.124613 r20:-0.929735 "
      "t40:-0.135311 r40:-0.946801 t60:-0.123586 r60:-0.922735 t90:-0.137895 r90:-0.896058",
      "0.73251 |T GDX |f t1:-0.0602452 t5:-0.477063 r5:0.501001 t10:-0.238843 r10:0.78945 t20:0.00435946 r20:0.832751 "
      "t40:0.0328047 r40:0.874984 t60:0.0779076 r60:0.904937 t90:0.062806 r90:0.874416",
      "0.557516 |T RTH |f t1:-0.0142269 t5:-0.0118355 r5:0.630517 t10:-0.0250743 r10:0.662658 t20:0.0634855 "
      "r20:0.857961 t40:0.0349201 r40:0.698693 t60:0.035595 r60:0.731773 t90:0.0449791 r90:0.767057",
      "0.639897 |T MXI |f t1:-0.0394954 t5:-0.384378 r5:0.502169 t10:-0.139307 r10:0.8061 t20:0.0815173 r20:0.880044 "
      "t40:0.0470638 r40:0.859053 t60:0.0461571 r60:0.874805 t90:0.0628346 r90:0.872736",
      "0.583484 |T EWU |f t1:-0.0221598 t5:-0.162901 r5:0.429709 t10:-0.0214379 r10:0.745645 t20:0.0748484 "
      "r20:0.880457 t40:0.0230047 r40:0.486719 t60:0.0319379 r60:0.619057 t90:0.0519471 r90:0.732452"};

  auto vw_all_data_single_run = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  for (const auto& item : input_data)
  {
    auto& ex = VW::get_unused_example(vw_all_data_single_run.get());
    VW::parsers::text::read_line(*vw_all_data_single_run, &ex, item.c_str());
    VW::setup_example(*vw_all_data_single_run, &ex);
    vw_all_data_single_run->learn(ex);
    vw_all_data_single_run->finish_example(ex);
  }

  auto vw_first_half = VW::initialize(vwtest::make_args("--no_stdin", "--quiet"));

  for (size_t i = 0; i < 5; i++)
  {
    auto& ex = VW::get_unused_example(vw_first_half.get());
    VW::parsers::text::read_line(*vw_first_half, &ex, input_data[i].c_str());
    VW::setup_example(*vw_first_half, &ex);
    vw_first_half->learn(ex);
    vw_first_half->finish_example(ex);
  }

  auto backing_vector = std::make_shared<std::vector<char>>();
  VW::io_buf io_writer;
  io_writer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*vw_first_half, io_writer);
  io_writer.flush();

  auto vw_second_half_from_loaded =
      VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--preserve_performance_counters"),
          VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()));

  for (size_t i = 5; i < 10; i++)
  {
    auto& ex = VW::get_unused_example(vw_second_half_from_loaded.get());
    VW::parsers::text::read_line(*vw_second_half_from_loaded, &ex, input_data[i].c_str());
    VW::setup_example(*vw_second_half_from_loaded, &ex);
    vw_second_half_from_loaded->learn(ex);
    vw_second_half_from_loaded->finish_example(ex);
  }

  EXPECT_EQ(vw_all_data_single_run->sd->weighted_examples(), vw_second_half_from_loaded->sd->weighted_examples());
  EXPECT_EQ(vw_all_data_single_run->sd->sum_loss, vw_second_half_from_loaded->sd->sum_loss);
}
