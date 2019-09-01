#include <fstream>

std::ostream* nullstream()
{
  static std::ofstream os;
  if (!os.is_open())
    os.open("/dev/null", std::ofstream::out | std::ofstream::app);
  return &os;
}
