#include "multisource.h"
#include "simple_label.h"

void get_prediction(int sock, prediction &p)
{
  if (read(sock, &p, sizeof(prediction)) < (int)sizeof(prediction))
    cerr << "argh! bad read!" << endl;
}

void reset(partial_example &ex)
{
  ex.counter = 0;
  ex.features.erase();
}

int recieve_features(parser* p, void* ex)
{
  example* ae = (example*)ex;
  while (true)
    {
      fd_set fds;
      FD_ZERO(&fds);
      for (int* sock= p->input.files.begin; sock != p->input.files.end; sock++)
	FD_SET(*sock,&fds);
      
      if (select(p->max_fd,&fds,NULL, NULL, NULL) == -1)
	{
	  cerr << "Select failed!" << endl;
	  exit (1);
	}
      for (int* sock = p->input.files.begin; sock != p->input.files.end; sock++)
	if (FD_ISSET(*sock, &fds))
	  {//there is a feature or label to read
	    prediction pre;
	    get_prediction(*sock, pre);
	    size_t index = pre.example_number % p->pes.index();
	    if (p->pes[index].example_number != pre.example_number)
	      if (p->pes[index].counter == 0)
		p->pes[index].example_number = pre.example_number;
	      else
		cerr << "Error, two examples map to the same index" << endl;
	    if (*sock != p->label_sock) // Not the label source
	      {
		feature f = {pre.p, *sock };
		push(p->pes[index].features,f);
	      }
	    else // The label source
	      {
		p->pes[index].ld.weight = pre.p;
		get_prediction(*sock, pre);
		p->pes[index].ld.label = pre.p;
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
    }
}

