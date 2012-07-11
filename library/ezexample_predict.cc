#include <stdio.h>
#include "../vowpalwabbit/vw.h"
#include "ezexample.h"

using namespace std;

int main(int argc, char *argv[])
{
  // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE -- THIS READS IN A MODEL FROM train.w
  vw vw = VW::initialize("--hash all -q st --noconstant -i train.w -t --quiet");

  // HAL'S SPIFFY INTERFACE USING C++ CRAZINESS
  ezexample ex(&vw, false);
  ex(vw_namespace('s'))
    ("p^the_man")
    ("w^the")
    ("w^man")
    (vw_namespace('t'))
    ("p^le_homme")
    ("w^le")
    ("w^homme");
  cerr << "should be near zero = " << ex() << endl;

  --ex;   // remove the most recent namespace
  ex(vw_namespace('t'))
    ("p^un_homme")
    ("w^un")
    ("w^homme");
  cerr << "should be near one  = " << ex() << endl;

  --ex;   // remove the most recent namespace
  // add features with explicit ns
  ex('t', "p^un_homme")
    ('t', "w^un")
    ('t', "w^homme");
  cerr << "should be near one  = " << ex() << endl;

  // AND FINISH UP
  vw.finish(&vw);
}
