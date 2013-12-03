#include <vector>
#ifdef _WIN32
#include <WinSock2.h>
#ifndef SHUT_RD
#   define SHUT_RD SD_RECEIVE
#endif

#ifndef SHUT_WR
#   define SHUT_WR SD_SEND
#endif

#ifndef SHUT_RDWR
#   define SHUT_RDWR SD_BOTH
#endif
#else
#include <netdb.h>
#endif
#include "io_buf.h"
#include "parse_args.h"
#include "cache.h"
#include "simple_label.h"
#include "network.h"
#include "vw.h"

using namespace std;

namespace SENDER {
  struct sender {
    io_buf* buf;
    int sd;
    vw* all;
    example** delay_ring;
    size_t sent_index;
    size_t received_index;
  };

  void open_sockets(sender& s, string host)
{
  s.sd = open_socket(host.c_str());
  s.buf = new io_buf();
  s.buf->files.push_back(s.sd);
}

  void send_features(io_buf *b, example* ec, uint32_t mask)
{
  // note: subtracting 1 b/c not sending constant
  output_byte(*b,(unsigned char) (ec->indices.size()-1));
  
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) {
    if (*i == constant_namespace)
      continue;
    output_features(*b, *i, ec->atomics[*i].begin, ec->atomics[*i].end, mask);
  }
  b->flush();
}

void receive_result(sender& s)
{
  float res, weight;
  get_prediction(s.sd,res,weight);
  
  example* ec=s.delay_ring[s.received_index++ % s.all->p->ring_size];
  label_data* ld = (label_data*)ec->ld;
  
  ec->final_prediction = res;
  
  ec->loss = s.all->loss->getLoss(s.all->sd, ec->final_prediction, ld->label) * ld->weight;
  
  return_simple_example(*(s.all), NULL, ec);  
}

  void learn(void* d, learner& base, example* ec) 
  { 
    sender* s = (sender*)d;
    if (s->received_index + s->all->p->ring_size - 1 == s->sent_index)
      receive_result(*s);

    label_data* ld = (label_data*)ec->ld;
    s->all->set_minmax(s->all->sd, ld->label);
    simple_label.cache_label(ld, *s->buf);//send label information.
    cache_tag(*s->buf, ec->tag);
    send_features(s->buf,ec, (uint32_t)s->all->parse_mask);
    s->delay_ring[s->sent_index++ % s->all->p->ring_size] = ec;
  }

  void finish_example(vw& all, void*, example*ec)
{}

void end_examples(void* d)
{
  sender* s = (sender*)d;
  //close our outputs to signal finishing.
  while (s->received_index != s->sent_index)
    receive_result(*s);
  shutdown(s->buf->files[0],SHUT_WR);
}

  void finish(void* d) 
  { 
    sender* s = (sender*)d;
    s->buf->files.delete_v();
    s->buf->space.delete_v();
    free(s->delay_ring);
    delete s->buf;
  }

  void save_load(void*, io_buf& io, bool read, bool text){}

  learner* setup(vw& all, po::variables_map& vm, vector<string> pairs)
{
  sender* s = (sender*)calloc(1,sizeof(sender));
  s->sd = -1;
  if (vm.count("sendto"))
    {      
      vector<string> hosts = vm["sendto"].as< vector<string> >();
      open_sockets(*s, hosts[0]);
    }

  s->all = &all;
  s->delay_ring = (example**) calloc(all.p->ring_size, sizeof(example*));

  learner* l = new learner(s,learn, save_load, 1);
  l->set_finish(finish);
  l->set_finish_example(finish_example); 
  l->set_end_examples(end_examples);
  return l;
}

}
