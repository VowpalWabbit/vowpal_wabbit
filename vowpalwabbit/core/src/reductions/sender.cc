// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw/core/reductions/sender.h"

#include "vw/cache_parser/parse_example_cache.h"
#include "vw/config/options.h"
#include "vw/core/daemon_utils.h"
#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/learner.h"
#include "vw/core/loss_functions.h"
#include "vw/core/network.h"
#include "vw/core/parser.h"
#include "vw/core/setup_base.h"
#include "vw/core/simple_label.h"
#include "vw/core/vw_fwd.h"
#include "vw/io/errno_handling.h"

#include <vector>

using namespace VW::config;
using namespace VW::LEARNER;

namespace
{
struct sent_example_info
{
  VW::simple_label label;
  float weight{};
  bool test_only{};
  // Note: this will not include counts of interacted features.
  // This info is not sent back.
  uint64_t num_features{};
  VW::v_array<char> tag;

  sent_example_info() = default;
  sent_example_info(VW::simple_label l, float w, bool t, uint64_t n, VW::v_array<char> tg)
      : label(l), weight(w), test_only(t), num_features(n), tag(std::move(tg))
  {
  }
};

class sender
{
public:
  VW::io_buf socket_output_buffer;
  std::unique_ptr<VW::io::socket> socket;
  std::unique_ptr<VW::io::reader> socket_reader;
  VW::workspace* all = nullptr;  // loss example_queue_limit others
  std::vector<sent_example_info> delay_ring;
  size_t sent_index = 0;
  size_t received_index = 0;
  VW::parsers::cache::details::cache_temp_buffer cache_buffer;
};

void open_sockets(sender& s, const std::string& host)
{
  s.socket = VW::details::open_vw_binary_socket(host, s.all->logger);
  s.socket_reader = s.socket->get_reader();
  s.socket_output_buffer.add_file(s.socket->get_writer());
}

void update_stats_sender(VW::shared_data& sd, const sent_example_info& info, float loss)
{
  sd.update(info.test_only, info.label.label != FLT_MAX, loss, info.weight, info.num_features);
  if (info.label.label != FLT_MAX && !info.test_only)
  {
    sd.weighted_labels += (static_cast<double>(info.label.label)) * info.weight;
  }
}

void output_example_prediction_sender(
    VW::workspace& all, const sent_example_info& info, float prediction, VW::io::logger& logger)
{
  for (auto& f : all.final_prediction_sink) { all.print_by_ref(f.get(), prediction, 0, info.tag, logger); }
}

void print_update_sender(VW::workspace& all, VW::shared_data& sd, const sent_example_info& info, float prediction)
{
  const bool should_print_driver_update = sd.weighted_examples() >= sd.dump_interval && !all.quiet;

  if (should_print_driver_update)
  {
    sd.print_update(
        *all.trace_message, all.holdout_set_off, all.current_pass, info.label.label, prediction, info.num_features);
  }
}

void receive_result(sender& s)
{
  float prediction{};
  float weight{};

  VW::details::get_prediction(s.socket_reader.get(), prediction, weight);
  const auto& sent_info = s.delay_ring[s.received_index++ % s.all->example_parser->example_queue_limit];

  const auto& ld = sent_info.label;
  const auto loss = s.all->loss->get_loss(s.all->sd.get(), prediction, ld.label) * sent_info.weight;

  update_stats_sender(*(s.all->sd), sent_info, loss);
  output_example_prediction_sender(*s.all, sent_info, prediction, s.all->logger);
  print_update_sender(*s.all, *(s.all->sd), sent_info, prediction);
}

void send_example(sender& s, VW::example& ec)
{
  if (s.received_index + s.all->example_parser->example_queue_limit / 2 - 1 == s.sent_index) { receive_result(s); }

  if (s.all->set_minmax) { s.all->set_minmax(ec.l.simple.label); }
  VW::parsers::cache::write_example_to_cache(
      s.socket_output_buffer, &ec, s.all->example_parser->lbl_parser, s.all->parse_mask, s.cache_buffer);
  s.socket_output_buffer.flush();
  s.delay_ring[s.sent_index++ % s.all->example_parser->example_queue_limit] =
      sent_example_info{ec.l.simple, ec.weight, ec.test_only, ec.get_num_features(), ec.tag};
}

void end_examples(sender& s)
{
  // close our outputs to signal finishing.
  while (s.received_index != s.sent_index) { receive_result(s); }
  s.socket_output_buffer.close_files();
}
}  // namespace

// This reduction does not actually produce a prediction despite claiming it
// does, since it waits to receive the result and then outputs it.
std::shared_ptr<VW::LEARNER::learner> VW::reductions::sender_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::string host;

  option_group_definition sender_options("[Reduction] Network sending");
  sender_options.add(make_option("sendto", host)
                         .keep()
                         .necessary()
                         .help("Send examples to <host>. Host can be of form hostname or hostname:port"));

  if (!options.add_parse_and_check_necessary(sender_options)) { return nullptr; }

  auto s = VW::make_unique<sender>();
  s->all = &all;
  s->delay_ring.resize(all.example_parser->example_queue_limit);
  open_sockets(*s, host);

  auto l = make_bottom_learner(std::move(s), send_example, send_example, stack_builder.get_setupfn_name(sender_setup),
      VW::prediction_type_t::SCALAR, VW::label_type_t::SIMPLE)
               // Set at least one of update_stats, output_example_prediction or print_update so that the old finish
               // has an implementation.
               .set_update_stats(
                   [](const VW::workspace&, VW::shared_data&, const sender&, const VW::example&, VW::io::logger&) {})
               .set_end_examples(end_examples)
               .build();
  return l;
}
