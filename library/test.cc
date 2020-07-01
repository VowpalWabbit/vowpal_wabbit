#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
  std::vector<std::pair<char, std::vector<int>>> u = std::vector<std::pair<char, std::vector<int>>>();
  u.push_back(std::pair<char, std::vector<int>>('a', std::vector<int>()));
  std::vector<int>* v = &(u[0].second);
  v->push_back(0);
  std::cout << "i want this to say one: " << u[0].second.size() << std::endl;
}
