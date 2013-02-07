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

using namespace std;

namespace SENDER {
  struct sender {
    io_buf* buf;
    
    int sd;
  };

  void open_sockets(sender& s, string host)
{
  s.sd = open_socket(host.c_str());
  s.buf = new io_buf();
  s.buf->files.push_back(s.sd);
}

void send_features(io_buf *b, example* ec)
{
  // note: subtracting 1 b/c not sending constant
  output_byte(*b,(unsigned char) (ec->indices.size()-1));
  
  for (unsigned char* i = ec->indices.begin; i != ec->indices.end; i++) {
    if (*i == constant_namespace)
      continue;
    output_features(*b, *i, ec->atomics[*i].begin, ec->atomics[*i].end);
  }
  b->flush();
}

  void save_load(void* in, void* d, io_buf& model_file, bool read, bool text) {}

  void drive_send(void* in, void* d)
{
  vw* all = (vw*)in;
  sender* s = (sender*)d;
  example* ec = NULL;
  v_array<char> null_tag;
  null_tag.erase();

  example** delay_ring = (example**) calloc(all->p->ring_size, sizeof(example*));
  size_t sent_index =0;
  size_t received_index=0;

  bool parser_finished = false;
  while ( true )
    {//this is a poor man's select operation.
      if (received_index + all->p->ring_size == sent_index || (parser_finished & (received_index != sent_index)))
	{
	  float res, weight;
	  get_prediction(s->sd,res,weight);
	  
	  ec=delay_ring[received_index++ % all->p->ring_size];
	  label_data* ld = (label_data*)ec->ld;
	  
	  ec->final_prediction = res;
	  
	  ec->loss = all->loss->getLoss(all->sd, ec->final_prediction, ld->label) * ld->weight;
	  
	  return_simple_example(*all, ec);
	}
      else if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
        {
          label_data* ld = (label_data*)ec->ld;
          all->set_minmax(all->sd, ld->label);
	  simple_label.cache_label(ld, *s->buf);//send label information.
	  cache_tag(*s->buf, ec->tag);
	  send_features(s->buf,ec);
	  delay_ring[sent_index++ % all->p->ring_size] = ec;
        }
      else if (parser_done(all->p))
        { //close our outputs to signal finishing.
	  parser_finished = true;
	  if (received_index == sent_index)
	    {
	      shutdown(s->buf->files[0],SHUT_WR);
	      s->buf->files.delete_v();
	      s->buf->space.delete_v();
	      free(delay_ring);
	      return;
	    }
	}
      else 
	;
    }
  return;
}
  void learn(void*in, void* d, example*ec) { cout << "sender can't be used under reduction" << endl; }
  void finish(void*in, void* d) { cout << "sender can't be used under reduction" << endl; }

  void parse_send_args(vw& all, po::variables_map& vm, vector<string> pairs)
{
  sender* s = (sender*)calloc(1,sizeof(sender));
  s->sd = -1;
  if (vm.count("sendto"))
    {      
      vector<string> hosts = vm["sendto"].as< vector<string> >();
      open_sockets(*s, hosts[0]);
    }

  learner ret = {s,drive_send,learn,finish,save_load};
  all.l = ret;
}

}
