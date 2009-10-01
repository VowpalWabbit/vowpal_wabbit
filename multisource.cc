#include "multisource.h"
#include "simple_label.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

bool get_prediction(int sock, prediction &p)
{
  return (recv(sock, &p, sizeof(p), MSG_DONTWAIT) == sizeof(p));
}

bool blocking_get_prediction(int sock, prediction &p)
{
  return (read(sock, &p, sizeof(p)) == sizeof(p));
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
  ex.counter = 0;
  ex.features.erase();
}

int receive_features(parser* p, void* ex)
{
  example* ae = (example*)ex;
  fd_set fds;
  FD_ZERO(&fds);
  for (int* sock= p->input.files.begin; sock != p->input.files.end; sock++)
    {
      FD_SET(*sock,&fds);
      cout << "socket = " << *sock << endl;
    }  

  cout << "max_fd " << p->max_fd << endl;
  while (true)
    {
      cout << "select begins" << endl;
      if (select(p->max_fd,&fds,NULL, NULL, NULL) == -1)
	{
	  cerr << "Select failed!" << endl;
	  exit (1);
	}
      cout << "select ends" << endl;
      for (size_t index = 0; index <= p->input.files.index(); index++)
	{
	  int sock = p->input.files[index];
	  if (FD_ISSET(sock, &fds))
	    {//there is a feature or label to read
	      prediction pre;
	      get_prediction(sock, pre);
	      size_t index = pre.example_number % p->pes.index();
	      if (p->pes[index].example_number != pre.example_number)
		if (p->pes[index].counter == 0)
		  p->pes[index].example_number = pre.example_number;
		else
		  cerr << "Error, two examples map to the same index" << endl;
	      feature f = {pre.p, p->ids[index]};
	      push(p->pes[index].features,f);
	      if (sock == p->label_sock) // The label source
		{
		  label_data ld;
		  size_t len = sizeof(ld.label)+sizeof(ld.weight);
		  char c[len];
		  read(sock,c,len);
		  bufread_simple_label(&(p->pes[index].ld), c);
		}
	      if( p->pes[index].counter == p->input.files.index() ) 
		{
		  push( ae->indices, multindex );
		  push_many( ae->atomics[multindex], p->pes[index].features.begin, p->pes[index].features.index() );
		  label_data* ld = (label_data*)ae->ld;
		  *ld = p->pes[index].ld;
		  reset(p->pes[index]);
		  return ae->atomics[multindex].index();
		}
	    }
	  else
	    FD_SET(sock,&fds);
	}
    }
}

