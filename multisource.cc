#include "multisource.h"
#include "simple_label.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>

int really_read(int sock, void* in, size_t count)
{
  char* buf = (char*)in;
  size_t done = 0;
  int r= 0;
  while (done < count)
    {
      if ((r = read(sock,buf,count-done)) == 0)
	return 0;
      else
	if (r < 0)
	  {
	    cerr << "argh! bad read! " << endl;
	    perror(NULL);
	    exit(0);
	  }
	else
	  {
	    done += r;
	    buf += r;
	  }
    }
  return done;
}

bool blocking_get_prediction(int sock, prediction &p)
{
  int count = really_read(sock, &p, sizeof(p));
  bool ret = (count == sizeof(p));
  return ret;
}

size_t recv_count = 0;
bool blocking_get_global_prediction(int sock, global_prediction &p)
{
  int count = really_read(sock, &p, sizeof(p));
  bool ret = (count == sizeof(p));
  if (ret)
    recv_count++;
  return ret;
}

void send_prediction(int sock, prediction &p)
{
  if (write(sock, &p, sizeof(p)) < (int)sizeof(p))
    {
      cerr << "argh! bad write! " << endl;
      perror(NULL);
      exit(0);
    }
}

size_t send_count = 0;

void send_global_prediction(int sock, global_prediction p)
{
  if (write(sock, &p, sizeof(p)) < (int)sizeof(p))
    {
      cerr << "argh! bad global write! " << sock << endl;
      perror(NULL);
      exit(0);
    }
  else
    send_count++;
}

void reset(partial_example &ex)
{
  ex.features.erase();
}

size_t num_finished = 0;
  
int receive_features(parser* p, void* ex)
{
  example* ae = (example*)ex;
  io_buf* input = p->input;
  fd_set fds;
  FD_ZERO(&fds);
  for (int* sock= input->files.begin; sock != input->files.end; sock++)
    FD_SET(*sock,&fds);
  
  while (input->files.index() > num_finished)
    {
      if (select(p->max_fd,&fds,NULL, NULL, NULL) == -1)
	{
	  cerr << "Select failed!" << endl;
	  perror(NULL);
	  exit (1);
	}
      for (int index = 0; index < (int)input->files.index(); index++)
	{
	  int sock = input->files[index];
	  if (FD_ISSET(sock, &fds))
	    {//there is a feature or label to read
	      prediction pre;
	      if (!blocking_get_prediction(sock, pre) )
		{
		  FD_CLR(sock, &fds);
		  num_finished++;
		}
	      else
		{
		  if (pre.example_number != ++ (p->counts[index]))
		    cout << "count is off! " << pre.example_number << " != " << p->counts[index] << 
		      " for source " << index << " prediction = " << pre.p << endl;
		  if (pre.example_number == p->finished_count + ring_size - 1)
		    FD_CLR(sock,&fds);//this ones to far ahead, let the buffer fill for awhile.
		  size_t ring_index = pre.example_number % p->pes.index();
		  if (p->pes[ring_index].features.index() == 0)
		    p->pes[ring_index].example_number = pre.example_number;
		  if (p->pes[ring_index].example_number != (int)pre.example_number)
		    cerr << "Error, example " << p->pes[ring_index].example_number << " != " << pre.example_number << endl;
		  feature f = {pre.p, p->ids[index]};
		  push(p->pes[ring_index].features, f);
		  if (sock == p->label_sock) // The label source
		    {
		      label_data ld;
		      size_t len = sizeof(ld.label)+sizeof(ld.weight);
		      char c[len];
		      really_read(sock,c,len);
		      bufread_simple_label(&(p->pes[ring_index].ld), c);
		    }

		  if( p->pes[ring_index].features.index() == input->count )
		    {
		      push( ae->indices, multindex );
		      push_many( ae->atomics[multindex], p->pes[ring_index].features.begin, p->pes[ring_index].features.index() );
		      label_data* ld = (label_data*)ae->ld;
		      *ld = p->pes[ring_index].ld;
		      reset(p->pes[ring_index]);
		      p->finished_count++;
		      return ae->atomics[multindex].index();
		    }
		}
	    }
	  else  if (p->counts[index] < p->finished_count + ring_size -1)
	    FD_SET(sock,&fds);
	}
    }
  return 0;
}

