#include "multisource.h"
#include "simple_label.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

bool get_prediction(int sock, prediction &p)
{
  bool ret = (recv(sock, &p, sizeof(p), MSG_DONTWAIT) == sizeof(p));
  return ret;
}

bool blocking_get_prediction(int sock, prediction &p)
{
  int count = read(sock, &p, sizeof(p));
  bool ret = (count == sizeof(p));
  return ret;
}

void send_prediction(int sock, prediction pred)
{
  if (write(sock,&pred,sizeof(prediction)) < (int)sizeof(prediction))
    {
      cerr << "argh! bad write! " << endl;
      perror(NULL);
      exit(0);
    }
  fsync(sock);
}

void reset(partial_example &ex)
{
  ex.features.erase();
}

int receive_count = 0;

int receive_features(parser* p, void* ex)
{
  example* ae = (example*)ex;
  fd_set fds;
  FD_ZERO(&fds);
  for (int* sock= p->input.files.begin; sock != p->input.files.end; sock++)
    FD_SET(*sock,&fds);
  
  while (p->input.files.index() > 0)
    {
      if (select(p->max_fd,&fds,NULL, NULL, NULL) == -1)
	{
	  cerr << "Select failed!" << endl;
	  perror(NULL);
	  exit (1);
	}
      for (int index = 0; index < (int)p->input.files.index(); index++)
	{
	  int sock = p->input.files[index];
	  if (FD_ISSET(sock, &fds))
	    {//there is a feature or label to read
	      prediction pre;
	      if (!blocking_get_prediction(sock, pre) )
		{
		  FD_CLR(sock, &fds);
		  close(sock);
		  memmove(p->input.files.begin+index, 
			  p->input.files.begin+index+1, 
			  p->input.files.index() - index-1);
		  memmove(p->ids.begin+index, 
			  p->ids.begin+index+1, 
			  p->ids.index() - index-1);
		  p->input.files.pop();
		  p->ids.pop();
		  index--;
		}
	      else
		{
		  size_t ring_index = pre.example_number % p->pes.index();
		  if (p->pes[ring_index].features.index() == 0)
		    p->pes[ring_index].example_number = pre.example_number;
		  if (p->pes[ring_index].example_number != pre.example_number)
		    cerr << "Error, example " << p->pes[ring_index].example_number << " != " << pre.example_number << endl;
		  feature f = {pre.p, p->ids[index]};
		  push(p->pes[ring_index].features, f);
		  if (sock == p->label_sock) // The label source
		    {
		      label_data ld;
		      size_t len = sizeof(ld.label)+sizeof(ld.weight);
		      char c[len];
		      read(sock,c,len);
		      bufread_simple_label(&(p->pes[ring_index].ld), c);
		    }
		  if( p->pes[ring_index].features.index() == p->input.count )
		    {
		      push( ae->indices, multindex );
		      push_many( ae->atomics[multindex], p->pes[ring_index].features.begin, p->pes[ring_index].features.index() );
		      label_data* ld = (label_data*)ae->ld;
		      *ld = p->pes[ring_index].ld;
		      reset(p->pes[ring_index]);
		      return ae->atomics[multindex].index();
		    }
		}
	    }
	  else
	    FD_SET(sock,&fds);
	}
    }
  return 0;
}

