#include <pthread.h>
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
#include "io.h"
#include "parse_args.h"
#include "cache.h"
#include "simple_label.h"
#include "network.h"

using namespace std;

//nonreentrant
io_buf* buf;

int sd = -1;

void open_sockets(string host)
{
  sd = open_socket(host.c_str());
  buf = new io_buf();
  push(buf->files,sd);
}

void parse_send_args(po::variables_map& vm, vector<string> pairs)
{
  if (vm.count("sendto"))
    {      
      vector<string> hosts = vm["sendto"].as< vector<string> >();
      open_sockets(hosts[0]);
    }
}

void send_features(io_buf *b, example* ec)
{
  // note: subtracting 1 b/c not sending constant
  output_byte(*b,ec->indices.index()-1);
  
  for (size_t* i = ec->indices.begin; i != ec->indices.end; i++) {
    if (*i == constant_namespace)
      continue;
    output_features(*b, *i, ec->atomics[*i].begin, ec->atomics[*i].end);
  }
  b->flush();
}

void drive_send(void* in)
{
  vw* all = (vw*)in;
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
	  get_prediction(sd,res,weight);
	  
	  ec=delay_ring[received_index++ % all->p->ring_size];
	  label_data* ld = (label_data*)ec->ld;
	  
	  ec->final_prediction = res;
	  
	  ec->loss = all->loss->getLoss(all->sd, ec->final_prediction, ld->label) * ld->weight;
	  
	  finish_example(*all, ec);
	}
      else if ((ec = get_example(all->p)) != NULL)//semiblocking operation.
        {
          label_data* ld = (label_data*)ec->ld;
          all->set_minmax(all->sd, ld->label);
	  simple_label.cache_label(ld, *buf);//send label information.
	  cache_tag(*buf, ec->tag);
	  send_features(buf,ec);
	  delay_ring[sent_index++ % all->p->ring_size] = ec;
        }
      else if (parser_done(all->p))
        { //close our outputs to signal finishing.
	  parser_finished = true;
	  if (received_index == sent_index)
	    {
	      shutdown(buf->files[0],SHUT_WR);
	      free(buf->files.begin);
	      free(buf->space.begin);
	      free(delay_ring);
	      return;
	    }
	}
      else 
	;
    }
  return;
}
