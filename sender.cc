#include <pthread.h>
#include <vector>
#include <netdb.h>
#include "io.h"
#include "parse_args.h"
#include "cache.h"
#include "vw.h"
#include "simple_label.h"
#include "network.h"
#include "delay_ring.h"

using namespace std;
io_buf* buf;

void open_sockets(string host)
{
  int sd = open_socket(host.c_str());
  global.local_prediction = sd;
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

void setup_send()
{
  example* ec = NULL;
  v_array<char> null_tag;
  null_tag.erase();

  bool finished = false;
  while ( true )
    {//this is a poor man's select operation.
      if ((ec = get_example()) != NULL)//semiblocking operation.
        {
	  if (finished) 
	    cout << "NOT POSSIBLE! " << endl;
          label_data* ld = (label_data*)ec->ld;
          set_minmax(ld->label);
	  simple_label.cache_label(ld, *buf);//send label information.
	  cache_tag(*buf, ec->tag);
	  send_features(buf,ec);
          delay_example(ec,0);
        }
      else if (!finished && parser_done())
        { //close our outputs to signal finishing.
	  finished = true;
	  buf->flush();
	  shutdown(buf->files[0],SHUT_WR);
	  free(buf->files.begin);
	  free(buf->space.begin);
        }
      else if (!finished)
	;
      else
	return;
    }
  return;
}

void destroy_send()
{
}
