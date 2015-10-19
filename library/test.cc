#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char**argv)
{ vector< pair< char, vector<int> > > u = vector< pair< char, vector<int> > >();
  u.push_back( pair< char, vector<int> >('a', vector<int>()) );
  vector<int>*v = &(u[0].second);
  v->push_back(0);
  cout << "i want this to say one: " << u[0].second.size() << endl;
}
