#include <iostream>
#include <string>

namespace
{
using namespace std;

unsigned int
codec (const string::const_iterator& c)
{ return *c == 'A' ? 0 :
         *c == 'C' ? 1 :
         *c == 'G' ? 2 : 3;
}
}

int
main (void)
{ using namespace std;

  while (! cin.eof ())
  { string line;
    string label;

    getline (cin, line);

    if (line.length ())
    { string::iterator s = line.begin ();
      while (*s != ' ')
      { cout << *s;
        ++s;
      }

      string::const_iterator ppp = s + 1;
      string::const_iterator pp = ppp + 1;
      string::const_iterator p = pp + 1;
      unsigned int offset = 1;

      cout << " |f";

      for (string::const_iterator c = p + 1;
           c != line.end ();
           ++ppp, ++pp, ++p, ++c)
      { unsigned int val = 64 * codec (ppp) +
                           16 * codec (pp) +
                           4 * codec (p) +
                           codec (c);

        cout << " " << offset + val << ":1";
        offset += 256;
      }

      cout << endl;
    }
  }

  return 0;
}
