#include <arpa/inet.h>
#include <cassert>
#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <string>

int
main (void)
{ using std::cin;
  using std::cout;
  using std::endl;
  using std::setprecision;

  uint32_t magic;
  cin.read (reinterpret_cast<char*> (&magic), sizeof (uint32_t));
  magic = ntohl (magic);
  assert (magic == 2051);

  uint32_t n_images;
  cin.read (reinterpret_cast<char*> (&n_images), sizeof (uint32_t));
  n_images = ntohl (n_images);

  uint32_t n_rows;
  cin.read (reinterpret_cast<char*> (&n_rows), sizeof (uint32_t));
  n_rows = ntohl (n_rows);

  uint32_t n_columns;
  cin.read (reinterpret_cast<char*> (&n_columns), sizeof (uint32_t));
  n_columns = ntohl (n_columns);

  uint32_t rc = n_rows * n_columns;
  unsigned char buf[rc];

  for (cin.read (reinterpret_cast<char*> (buf), rc);
       ! cin.eof ();
       cin.read (reinterpret_cast<char*> (buf), rc))
  { for (unsigned int p = 0; p < n_rows * n_columns; ++p)
    { if (buf[p])
        cout << " " << p << ":" << static_cast<unsigned int>(buf[p]);
    }

    cout << endl;
  }

  return 0;
}
