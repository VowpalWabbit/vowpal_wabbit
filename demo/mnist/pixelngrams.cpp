#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

//======================================================================
//                              vhd_ngrams                             =
//                                                                     =
// vertical, horizontal, and diagonal 2-skip-2 grams.                  =
//======================================================================/

void
vhd_ngrams (unsigned char*const buf,
            size_t              p,
            size_t              stride,
            size_t              total,
            double*             v)
{
  double x = static_cast<double> (buf[p]) / 256.0;
  double xp1 = static_cast<double> (buf[(p + 1) % total]) / 256.0;
  double xp2 = static_cast<double> (buf[(p + 2) % total]) / 256.0;
  double xp3 = static_cast<double> (buf[(p + 3) % total]) / 256.0;
  double xd1 = static_cast<double> (buf[(p + stride) % total]) / 256.0;
  double xd2 = static_cast<double> (buf[(p + 2 * stride) % total]) / 256.0;
  double xd3 = static_cast<double> (buf[(p + 3 * stride) % total]) / 256.0;
  double xpd1 = static_cast<double> (buf[(p + stride + 1) % total]) / 256.0;
  double xpd2 = static_cast<double> (buf[(p + 2 * (stride + 1)) % total]) / 256.0;
  double xpd3 = static_cast<double> (buf[(p + 3 * (stride + 1)) % total]) / 256.0;
  double xmd1 = static_cast<double> (buf[(p + stride - 1) % total]) / 256.0;
  double xmd2 = static_cast<double> (buf[(p + 2 * (stride - 1)) % total]) / 256.0;
  double xmd3 = static_cast<double> (buf[(p + 3 * (stride - 1)) % total]) / 256.0;

  v[0] = ::sqrt (x * xp1);
  v[1] = ::sqrt (x * xd1);
  v[2] = ::sqrt (x * xpd1);
  v[3] = ::sqrt (x * xmd1);
  v[4] = ::sqrt (x * xp2);
  v[5] = ::sqrt (x * xd2);
  v[6] = ::sqrt (x * xpd2);
  v[7] = ::sqrt (x * xmd2);
  v[8] = ::sqrt (x * xp3);
  v[9] = ::sqrt (x * xd3);
  v[10] = ::sqrt (x * xpd3);
  v[11] = ::sqrt (x * xmd3);
}

}

int
main (void)
{
  using std::cin;
  using std::cout;
  using std::getline;
  using std::endl;
  using std::flush;
  using std::string;

  size_t n_rows = 28;
  size_t n_columns = 28;
  size_t rc = n_rows * n_columns;
  unsigned char buf[n_rows * n_columns];

  while (! cin.eof ())
    {
      string line;

      getline (cin, line);

      if (line.length ())
        {
          unsigned int feature;
          unsigned int value;
          size_t start = 0;
          int increment;

          char* pipe = const_cast<char*> (strchr (line.c_str (), '|'));

          if (pipe)
            {
              *pipe++ = '\0';
              cout << line.c_str ();
              start = pipe - line.c_str ();
            }

          ::memset (buf, 0, n_rows * n_columns * sizeof (buf[0]));

          while (::sscanf (line.c_str () + start,
                           "%u:%u%n",
                           &feature, &value, &increment) >= 2)
            {
              buf[feature] = value;
              start += increment;
            }

          size_t offset = 1;
          cout << "|p";

          for (unsigned int p = 0; p < n_rows * n_columns; ++p)
            {
              if (buf[p])
                {
                  cout << " " << offset << ":" << static_cast<double>(buf[p])/256.0;

                  double v[12]; vhd_ngrams (buf, p, n_rows, rc, v);

                  for (unsigned int j = 0; j < 12; ++j)
                    {
                      if (v[j])
                        {
                          cout << " " << offset + 1 + j << ":" << v[j];
                        }
                    }
                }

              offset += 13;
            }

          cout << endl << flush;
        }
    }

  return 0;
}
