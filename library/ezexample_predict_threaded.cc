#include <stdio.h>
#include "../vowpalwabbit/vw.h"
#include "ezexample.h"

#include <boost/thread/thread.hpp>

using namespace std;

int runcount = 100;

class Worker
{
public:
  Worker(vw & instance, string & vw_init_string, vector<double> & ref)
    : m_vw(instance)
    , m_referenceValues(ref)
    , vw_init_string(vw_init_string)
  { }

  void operator()()
  {
    m_vw_parser = VW::initialize(vw_init_string);
    if (m_vw_parser == NULL) {
      cerr << "cannot initialize vw parser" << endl;
      exit(-1);
    }

    int errorCount = 0;
    for (int i = 0; i < runcount; ++i)
    {
      vector<double>::iterator it = m_referenceValues.begin();
      ezexample ex(&m_vw, false, m_vw_parser);
      
      ex(vw_namespace('s'))
        ("p^the_man")
        ("w^the")
        ("w^man")
        (vw_namespace('t'))
        ("p^le_homme")
        ("w^le")
        ("w^homme");
      ex.set_label("1");
      if (*it != ex()) { cerr << "fail!" << endl; ++errorCount; }
      //if (*it != pred) { cerr << "fail!" << endl; ++errorCount; }
      //VW::finish_example(m_vw, vec2);
      ++it;

      --ex;   // remove the most recent namespace
      ex(vw_namespace('t'))
        ("p^un_homme")
        ("w^un")
        ("w^homme");
      ex.set_label("1");
      if (*it != ex()) { cerr << "fail!" << endl; ++errorCount; }
      ++it;

      --ex;   // remove the most recent namespace
      // add features with explicit ns
      ex('t', "p^un_homme")
        ('t', "w^un")
        ('t', "w^homme");
      ex.set_label("1");
      if (*it != ex()) { cerr << "fail!" << endl; ++errorCount; }
      ++it;

      //cout << "."; cout.flush();
    }
    cerr << "error count = " << errorCount << endl;
    VW::finish(*m_vw_parser);
    m_vw_parser = NULL;
  }

private:
  vw & m_vw;
  vw * m_vw_parser;
  vector<double> & m_referenceValues;
  string & vw_init_string;
};

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    cerr << "need two args: threadcount runcount" << endl;
    return 1;
  }
  int threadcount = atoi(argv[1]);
  runcount = atoi(argv[2]);
  // INITIALIZE WITH WHATEVER YOU WOULD PUT ON THE VW COMMAND LINE -- THIS READS IN A MODEL FROM train.w
  string vw_init_string_all    = "-t --csoaa_ldf s --quiet -q st --noconstant --hash all -i train.w";
  string vw_init_string_parser = "-t --csoaa_ldf s --quiet -q st --noconstant --hash all --noop";   // this needs to have enough arguments to get the parser right
  vw*vw = VW::initialize(vw_init_string_all);
  vector<double> results;

  // HAL'S SPIFFY INTERFACE USING C++ CRAZINESS
  {
    ezexample ex(vw, false);
    ex(vw_namespace('s'))
      ("p^the_man")
      ("w^the")
      ("w^man")
      (vw_namespace('t'))
      ("p^le_homme")
      ("w^le")
      ("w^homme");
    ex.set_label("1");
    results.push_back(ex.predict());
    cerr << "should be near zero = " << ex.predict() << endl;

    --ex;   // remove the most recent namespace
    ex(vw_namespace('t'))
      ("p^un_homme")
      ("w^un")
      ("w^homme");
    ex.set_label("1");
    results.push_back(ex.predict());
    cerr << "should be near one  = " << ex.predict() << endl;

    --ex;   // remove the most recent namespace
    // add features with explicit ns
    ex('t', "p^un_homme")
      ('t', "w^un")
      ('t', "w^homme");
    ex.set_label("1");
    results.push_back(ex.predict());
    cerr << "should be near one  = " << ex.predict() << endl;
  }

  if (threadcount == 0)
  {
    Worker w(*vw, vw_init_string_parser, results);
    w();
  }
  else
  {
    boost::thread_group tg;
    for (int t = 0; t < threadcount; ++t)
    {
      cerr << "starting thread " << t << endl;
      boost::thread * pt = tg.create_thread(Worker(*vw, vw_init_string_parser, results));
    }
    tg.join_all();
    cerr << "finished!" << endl;
  }


  // AND FINISH UP
  VW::finish(*vw);
}
