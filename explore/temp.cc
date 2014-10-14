#include <string>
#include <functional>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  hash<string> foo;
  
  cout << foo("0") << endl;
  cout << foo("1") << endl;
  cout << foo("2") << endl;
  cout << foo("3") << endl;
  cout << foo("4") << endl;
  cout << foo("5") << endl;
  cout << foo("6") << endl;
  cout << foo("7") << endl;
  cout << foo("8") << endl;
  cout << foo("9") << endl;
  cout << foo("10") << endl;
}
