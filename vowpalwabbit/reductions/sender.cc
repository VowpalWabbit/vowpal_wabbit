// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "learner.h"
#include "setup_base.h"
#include "vw_fwd.h"

#include <vector>
#ifdef _WIN32
#  define NOMINMAX
#  include <WinSock2.h>
#  ifndef SHUT_RD
#    define SHUT_RD SD_RECEIVE
#  endif

#  ifndef SHUT_WR
#    define SHUT_WR SD_SEND
#  endif

#  ifndef SHUT_RDWR
#    define SHUT_RDWR SD_BOTH
#  endif
#else
#  include <netdb.h>
#endif

#include "cache.h"
#include "config/options.h"
#include "global_data.h"
#include "io_buf.h"
#include "loss_functions.h"
#include "network.h"
#include "parser.h"

using namespace VW::config;

struct sender
{
  io_buf* buf = nullptr;
  std::unique_ptr<VW::io::socket> _socket;
  std::unique_ptr<VW::io::reader> _socket_reader;
  VW::workspace* all = nullptr;  // loss example_queue_limit others
  VW::example** delay_ring = nullptr;
  size_t sent_index = 0;
  size_t received_index = 0;

  ~sender()
  {
    free(delay_ring);
    delete buf;
  }
};

void open_sockets(sender& s, const std::string& host)
{
  s._socket = VW::io::wrap_socket_descriptor(open_socket(host.c_str(), s.all->logger));
  s._socket_reader = s._socket->get_reader();
  s.buf = new io_buf();
  s.buf->add_file(s._socket->get_writer());
}

void send_features(io_buf* b, VW::example& ec, uint32_t mask)
{
  // note: subtracting 1 b/c not sending constant
  output_byte(*b, static_cast<unsigned char>(ec.indices.size() - 1));

  for (VW::namespace_index ns : ec.indices)
  {
    if (ns == constant_namespace) { continue; }
    char* c;
    cache_index(*b, ns, ec.feature_space[ns], c);
    cache_features(*b, ec.feature_space[ns], mask, c);
  }
  b->flush();
}

void receive_result(sender& s)
{
  float res;
  float weight;

  get_prediction(s._socket_reader.get(), res, weight);
  VW::example& ec = *s.delay_ring[s.received_index++ % s.all->example_parser->example_queue_limit];
  ec.pred.scalar = res;

  label_data& ld = ec.l.simple;
  ec.loss = s.all->loss->getLoss(s.all->sd, ec.pred.scalar, ld.label) * ec.weight;

  return_simple_example(*(s.all), nullptr, ec);
}

void learn(sender& s, VW::LEARNER::base_learner& /*unused*/, VW::example& ec)
{
  if (s.received_index + s.all->example_parser->example_queue_limit / 2 - 1 == s.sent_index) { receive_result(s); }

  s.all->set_minmax(s.all->sd, ec.l.simple.label);
  s.all->example_parser->lbl_parser.cache_label(
      ec.l, ec._reduction_features, *s.buf, "", false);  // send label information.
  cache_tag(*s.buf, ec.tag);
  send_features(s.buf, ec, static_cast<uint32_t>(s.all->parse_mask));
  s.delay_ring[s.sent_index++ % s.all->example_parser->example_queue_limit] = &ec;
}

void finish_example(VW::workspace& /*unused*/, sender& /*unused*/, VW::example& /*unused*/) {}

void end_examples(sender& s)
{
  // close our outputs to signal finishing.
  while (s.received_index != s.sent_index) { receive_result(s); }
  s.buf->close_files();
}

VW::LEARNER::base_learner* sender_setup(VW::setup_base_i& stack_builder)
{
  VW::config::options_i& options = *stack_builder.get_options();
  VW::workspace& all = *stack_builder.get_all_pointer();
  std::string host;

  option_group_definition sender_options("Network sending");
  sender_options.add(make_option("sendto", host).keep().necessary().help("Send examples to <host>"));

  if (!options.add_parse_and_check_necessary(sender_options)) { return nullptr; }

  auto s = VW::make_unique<sender>();
  open_sockets(*s, host);

  s->all = &all;
  s->delay_ring = calloc_or_throw<VW::example*>(all.example_parser->example_queue_limit);

  auto* l = VW::LEARNER::make_base_learner(std::move(s), learn, learn, stack_builder.get_setupfn_name(sender_setup),
      VW::prediction_type_t::scalar, VW::label_type_t::simple)
                .set_finish_example(finish_example)
                .set_end_examples(end_examples)
                .build();
  return make_base(*l);
}
