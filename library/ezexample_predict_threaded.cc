#include <stdio.h>
#include "../vowpalwabbit/vw.h"
#include "ezexample.h"

#include <boost/thread/thread.hpp>

using namespace std;

int runcount = 100;

class Worker
{
public:
  Worker(vw & instance, vector<double> & ref)
    : m_vw(instance)
    , m_referenceValues(ref)
  { }

  void operator()()
  {
    int errorCount = 0;
    for (int i = 0; i < runcount; ++i)
    {
      vector<double>::iterator it = m_referenceValues.begin();
      ezexample ex(&m_vw, false);
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
  }

private:
  vw & m_vw;
  vector<double> & m_referenceValues;
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
  vw*vw = VW::initialize("--hash all -q st --noconstant -i train.w -t --csoaa_ldf s --quiet");
  //vw vw0 = VW::initialize("--hash all -q st --noconstant -i train.w -t --quiet"); vw*vw = &vw0;
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
    Worker w(*vw, results);
    w();
  }
  else
  {
    boost::thread_group tg;
    for (int t = 0; t < threadcount; ++t)
    {
      cerr << "starting thread " << t << endl;
      boost::thread * pt = tg.create_thread(Worker(*vw, results));
    }
    tg.join_all();
    cerr << "finished!" << endl;
  }


  // AND FINISH UP
  VW::finish(*vw);
}
