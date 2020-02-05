#include <stdio.h>
#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/ezexample.h"

void run(vw*vw)
{ ezexample ex(vw, true);   // we're doing csoaa_ldf so we need multiline examples

  /// BEGIN FIRST MULTILINE EXAMPLE
  ex(vw_namespace('s'))
  ("p^the_man")
  ("w^the")
  ("w^man")
  (vw_namespace('t'))
  ("p^un_homme")
  ("w^un")
  ("w^homme");

  ex.set_label("1:1");
  ex.train();

  --ex;   // remove the most recent namespace
  ex(vw_namespace('t'))
  ("p^le_homme")
  ("w^le")
  ("w^homme");

  ex.set_label("2:0");
  ex.train();

  // push it through VW for training
  ex.finish();

  /// BEGIN SECOND MULTILINE EXAMPLE
  ex(vw_namespace('s'))
  ("p^a_man")
  ("w^a")
  ("w^man")
  (vw_namespace('t'))
  ("p^un_homme")
  ("w^un")
  ("w^homme");

  ex.set_label("1:0");
  ex.train();

  --ex;   // remove the most recent namespace
  ex(vw_namespace('t'))
  ("p^le_homme")
  ("w^le")
  ("w^homme");

  ex.set_label("2:1");
  ex.train();

  // push it through VW for training
  ex.finish();
}

int main(int argc, char *argv[])
{ // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE -- THIS WILL STORE A MODEL TO train.ezw
  vw* vw = VW::initialize("--hash all -q st --noconstant -f train.w --quiet --csoaa_ldf m");

  run(vw);

  // AND FINISH UP
  std::cerr << "ezexample_train finish"<< std::endl;
  VW::finish(*vw);
}
