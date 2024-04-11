
#include "example_data_generator.h"
#include "prototype_example.h"
#include "prototype_example_root.h"
#include "prototype_label.h"
#include "prototype_namespace.h"
#include "prototype_typemappings.h"
#include "vw/common/future_compat.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"
#include "vw/test_common/test_common.h"

template <typename T, typename FB_t = typename vwtest::fb_type<T>::type>
void create_flatbuffer_and_validate(VW::workspace& w, const T& prototype)
{
  flatbuffers::FlatBufferBuilder builder;

  Offset<FB_t> buffer_offset = prototype.create_flatbuffer(builder, w);
  builder.Finish(buffer_offset);

  const FB_t* fb_obj = GetRoot<FB_t>(builder.GetBufferPointer());

  prototype.verify(w, fb_obj);
}

template <>
void create_flatbuffer_and_validate<vwtest::prototype_label_t, void>(
    VW::workspace& w, const vwtest::prototype_label_t& prototype)
{
  if (prototype.label_type == fb::Label_NONE) { return; }  // there is no flatbuffer to create

  flatbuffers::FlatBufferBuilder builder;

  Offset<void> buffer_offset = prototype.create_flatbuffer(builder, w);
  builder.Finish(buffer_offset);

  switch (prototype.label_type)
  {
    case fb::Label_SimpleLabel:
    case fb::Label_CBLabel:
    case fb::Label_ContinuousLabel:
    case fb::Label_Slates_Label:
    {
      prototype.verify(w, prototype.label_type, builder.GetBufferPointer());
      break;
    }
    case fb::Label_NONE:
    {
      break;
    }
    default:
    {
      THROW("Label type not currently supported for create_flatbuffer_and_validate");
      break;
    }
  }
}

TEST(FlatBufferParser, ValidateTestAffordances_NoLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::prototype_label_t label_prototype = vwtest::no_label();
  create_flatbuffer_and_validate(*all, label_prototype);
}

TEST(FlatBufferParser, ValidateTestAffordances_SimpleLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));
  create_flatbuffer_and_validate(*all, vwtest::simple_label(0.5, 1.0));
}

TEST(FlatBufferParser, ValidateTestAffordances_CBLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));
  create_flatbuffer_and_validate(*all, vwtest::cb_label({1.5, 2, 0.25f}));
}

TEST(FlatBufferParser, ValidateTestAffordances_ContinuousLabel)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  std::vector<VW::cb_continuous::continuous_label_elm> probabilities = {{1, 0.5f, 0.25}};

  create_flatbuffer_and_validate(*all, vwtest::continuous_label(probabilities));
}

TEST(FlatBufferParser, ValidateTestAffordances_Slates)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--slates"));

  std::vector<VW::action_score> probabilities = {{1, 0.5f}, {2, 0.25f}};

  VW::slates::example_type types[] = {
      VW::slates::example_type::UNSET,
      VW::slates::example_type::ACTION,
      VW::slates::example_type::SHARED,
      VW::slates::example_type::SLOT,
  };

  for (VW::slates::example_type type : types)
  {
    create_flatbuffer_and_validate(*all, vwtest::slates_label_raw(type, 0.5, true, 0.3, 1, probabilities));
  }
}

TEST(FlatbufferParser, ValidateTestAffordances_Namespace)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::prototype_namespace_t ns_prototype = {"U_a", {{"a", 1.f}, {"b", 2.f}}};
  create_flatbuffer_and_validate(*all, ns_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_Simple)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::prototype_example_t ex_prototype = {{
                                                  {"U_a", {{"a", 1.f}, {"b", 2.f}}},
                                                  {"U_b", {{"a", 3.f}, {"b", 4.f}}},
                                              },
      vwtest::simple_label(0.5, 1.0)};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_Unlabeled)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::prototype_example_t ex_prototype = {{
      {"U_a", {{"a", 1.f}, {"b", 2.f}}},
      {"U_b", {{"a", 3.f}, {"b", 4.f}}},
  }};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_CBShared)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::prototype_example_t ex_prototype = {{
                                                  {"U_a", {{"a", 1.f}, {"b", 2.f}}},
                                                  {"U_b", {{"a", 3.f}, {"b", 4.f}}},
                                              },
      vwtest::cb_label_shared(), "tag1"};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_Example_CB)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::prototype_example_t ex_prototype = {{
                                                  {"T_a", {{"a", 5.f}, {"b", 6.f}}},
                                                  {"T_b", {{"a", 7.f}, {"b", 8.f}}},
                                              },
      vwtest::cb_label({1, 1, 0.5f}), "tag1"};
  create_flatbuffer_and_validate(*all, ex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_MultiExample)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer"));

  vwtest::prototype_multiexample_t multiex_prototype = {{
      {{
           {"U_a", {{"a", 1.f}, {"b", 2.f}}},
           {"U_b", {{"a", 3.f}, {"b", 4.f}}},
       },
          vwtest::cb_label_shared(), "tag1"},
      {
          {
              {"T_a", {{"a", 5.f}, {"b", 6.f}}},
              {"T_b", {{"a", 7.f}, {"b", 8.f}}},
          },
          vwtest::cb_label({{1, 1, 0.5f}}),
      },
  }};
  create_flatbuffer_and_validate(*all, multiex_prototype);
}

TEST(FlatbufferParser, ValidateTestAffordances_ExampleCollectionMultiline)
{
  auto all = VW::initialize(vwtest::make_args("--no_stdin", "--quiet", "--flatbuffer", "--cb_explore_adf"));

  vwtest::example_data_generator data_gen;
  vwtest::prototype_example_collection_t prototype = data_gen.create_cb_adf_log(2, 2, 0.5f);

  create_flatbuffer_and_validate(*all, prototype);
}
