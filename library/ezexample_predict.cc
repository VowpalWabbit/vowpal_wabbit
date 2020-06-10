#include <stdio.h>
#include "../vowpalwabbit/parser.h"
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/ezexample.h"

int main(int argc, char *argv[])
{ std::string init_string = "-t -q st --noconstant --ldf_override s --no_stdin -i ";
  if (argc > 1)
    init_string += argv[1];
  else
    init_string += "train.w";

  std::cerr << "initializing with: '" << init_string << "'" << std::endl;

  // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE -- THIS READS IN A MODEL FROM train.w
  vw* vw = VW::initialize(init_string); // "-t -q st --hash all --noconstant --ldf_override s -i train.w");

  { // HAL'S SPIFFY INTERFACE USING C++ CRAZINESS
    ezexample ex(vw, false);  // don't need multiline
    ex(vw_namespace('s'))
    ("p^the_man")
    ("w^the")
    ("w^man")
    (vw_namespace('t'))
    ("p^le_homme")
    ("w^le")
    ("w^homme");
    ex.set_label("1");
    std::cerr << ex.predict_partial() << std::endl;

    //    ex.clear_features();

    --ex;   // remove the most recent namespace
    ex(vw_namespace('t'))
    ("p^un_homme")
    ("w^un")
    ("w^homme");
    ex.set_label("2");
    std::cerr << ex.predict_partial() << std::endl;

    --ex;   // remove the most recent namespace, and add features with explicit ns
    ex('t', "p^un_homme")
    ('t', "w^un")
    ('t', "w^homme");
    ex.set_label("2");
    std::cerr << ex.predict_partial() << std::endl;
  }

  // AND FINISH UP
  VW::finish(*vw);
}
