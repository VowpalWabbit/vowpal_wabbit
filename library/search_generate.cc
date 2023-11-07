#include "libsearch.h"
#include "vw/config/options_cli.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"

#include <cstdio>
#include <cstdlib>  // for system
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using std::cerr;
using std::endl;

size_t sed(
    const std::string& s1, const std::string& s2, size_t subst_cost = 1, size_t ins_cost = 1, size_t del_cost = 1);

action char2action(char c)  // 1=EOS, 2=' ', 3..28=a..z, 29=other
{
  if (c == '$') { return 1; }
  if (c == ' ') { return 2; }
  if (c >= 'a' && c <= 'z') { return (action)(c - 'a' + 3); }
  return 29;
}

char action2char(action a)
{
  if (a == 1) { return '$'; }
  if (a == 2) { return ' '; }
  if (a >= 3 && a <= 28) { return (char)(a - 3 + 'a'); }
  return '_';
}

struct nextstr
{
  char c;
  float cw;
  std::string s;
  float sw;
  nextstr(char _c, float _cw, std::string _s, float _sw) : c(_c), cw(_cw), s(std::move(_s)), sw(_sw) {}
};

inline float min_float(float a, float b) { return (a < b) ? a : b; }

class Trie  // NOLINT
{
public:
  Trie() : _terminus(false), _count(0), _max_count(0), _max_string("") {}

  ~Trie()
  {
    for (Trie* t : _children) { delete t; }
  }

  Trie* step(const char c)
  {
    size_t id = char2action(c) - 1;
    if (_children.size() <= id) { return nullptr; }
    return _children[id];
  }

  void insert(const char* str, size_t c = 1)
  {
    if (str == nullptr || *str == 0)
    {
      _terminus += c;
      _count += c;
    }
    else
    {
      _count += c;
      size_t id = char2action(*str) - 1;
      while (_children.size() <= id) { _children.push_back(nullptr); }
      if (_children[id] == nullptr) { _children[id] = new Trie(); }
      _children[id]->insert(str + 1, c);
    }
  }

  size_t contains(const char* str)
  {
    if (str == nullptr || *str == 0) { return _terminus; }
    size_t id = char2action(*str) - 1;
    if (_children.size() <= id) { return 0; }
    if (_children[id] == nullptr) { return 0; }
    return _children[id]->contains(str + 1);
  }

  void get_next(const char* prefix, std::vector<nextstr>& next)
  {
    if (prefix == nullptr || *prefix == 0)
    {
      next.clear();
      float c = 1.0f / (float)_count;
      next.push_back(
          nextstr('$', std::log(1.0f + c * (float)_terminus), _max_string, std::log(1.0f + (float)_max_count)));
      for (size_t id = 0; id < _children.size(); id++)
      {
        if (_children[id])
        {
          next.push_back(nextstr(action2char((action)(id + 1)), c * (float)_children[id]->_count,
              _children[id]->_max_string, std::log(1.0f + (float)_children[id]->_max_count)));
        }
      }
    }
    else
    {
      size_t id = char2action(*prefix) - 1;
      if (_children.size() <= id) { return; }
      if (_children[id] == nullptr) { return; }
      _children[id]->get_next(prefix + 1, next);
    }
  }

  void build_max(const std::string& prefix = "")
  {
    _max_count = _terminus;
    _max_string = prefix;
    for (size_t id = 0; id < _children.size(); id++)
    {
      if (_children[id])
      {
        char c = action2char((action)(id + 1));
        _children[id]->build_max(prefix + c);
        if (_children[id]->_max_count > _max_count)
        {
          _max_count = _children[id]->_max_count;
          _max_string = _children[id]->_max_string;
        }
      }
    }
  }

  void print(char c = '^', size_t indent = 0)
  {
    cerr << std::string(indent * 2, ' ');
    cerr << '\'' << c << "' " << _count << " [max_string=" << _max_string << " max_count=" << _max_count << "]" << endl;
    for (size_t i = 0; i < _children.size(); i++)
    {
      if (_children[i]) { _children[i]->print(action2char((action)(i + 1)), indent + 1); }
    }
  }

private:
  size_t _terminus;         // count of words that end here?
  size_t _count;            // count of all words under here (including us)
  size_t _max_count;        // count of most frequent word under here
  std::string _max_string;  // the corresponding std::string
  std::vector<Trie*> _children;
};

class IncrementalEditDistance  // NOLINT
{
public:
  IncrementalEditDistance(std::string& target, size_t subst_cost = 1, size_t ins_cost = 1, size_t del_cost = 1)
      : _target(target)
      , _subst_cost(subst_cost)
      , _ins_cost(ins_cost)
      , _del_cost(del_cost)
      , _N(target.length())
      , _output_string("")
  {
    _prev_row = new size_t[_N + 1];
    _cur_row = new size_t[_N + 1];

    for (size_t n = 0; n <= _N; n++) { _prev_row[n] = del_cost * n; }

    _prev_row_min = 0;
  }

  void append(char c)
  {
    _output_string += c;
    _cur_row[0] = _prev_row[0] + _ins_cost;
    _prev_row_min = _cur_row[0];
    for (size_t n = 1; n <= _N; n++)
    {
      _cur_row[n] = min3(_prev_row[n] + _ins_cost, _prev_row[n - 1] + ((_target[n - 1] == c) ? 0 : _subst_cost),
          _cur_row[n - 1] + _del_cost);
      _prev_row_min = std::min(_prev_row_min, _cur_row[n]);
    }
    // swap cur_row and prev_row
    size_t* tmp = _cur_row;
    _cur_row = _prev_row;
    _prev_row = tmp;
  }

  void append(const std::string& s)
  {
    for (char c : s) { append(c); }
  }

  std::vector<char>& next()
  {
    _A.clear();
    for (size_t n = 0; n <= _N; n++)
    {
      if (_prev_row[n] == _prev_row_min) { _A.push_back((n < _N) ? _target[n] : '$'); }
    }
    return _A;
  }

  std::vector<std::pair<action, float>> all_next()
  {
    std::vector<std::pair<action, float>> b;
    for (action a = 1; a <= 29; a++) { b.push_back(std::make_pair(a, 1.f)); }

    b[char2action('$') - 1].second = min_float(100.f, (float)(_prev_row[_N] - _prev_row_min));

    for (action n = 0; n < _N; n++)
    {
      if (_prev_row[n] == _prev_row_min) { b[char2action(_target[n]) - 1].second = 0.f; }
    }
    return b;
  }

  std::string out() { return _output_string; }
  size_t distance() { return _prev_row_min; }
  size_t finish_distance()
  {  // find last occurrence of prev_row_min
    int n = (int)_N;
    while (n >= 0 && _prev_row[n] > _prev_row_min) { n--; }
    return (_N - n) * _ins_cost + _prev_row_min;
  }

  ~IncrementalEditDistance()
  {
    delete[] _prev_row;
    delete[] _cur_row;
  }

private:
  size_t* _prev_row;
  size_t* _cur_row;
  std::string _target;
  size_t _subst_cost;
  size_t _ins_cost;
  size_t _del_cost;
  size_t _prev_row_min;
  size_t _N;
  std::string _output_string;
  std::vector<char> _A;

  inline size_t min3(size_t a, size_t b, size_t c) { return (a < b) ? (a < c) ? a : c : (b < c) ? b : c; }
};

struct input
{
  std::string in;
  std::string out;
  float weight;
  input(std::string _in, std::string _out, float _weight) : in(std::move(_in)), out(std::move(_out)), weight(_weight) {}
  input(std::string _in, std::string _out) : in(std::move(_in)), out(std::move(_out)), weight(1.) {}
  input(const std::string& _in) : in(_in), out(_in), weight(1.) {}
  input() : weight(1.) {}
};

using output = std::string;

float max_cost = 100.;

float get_or_one(std::vector<std::pair<char, size_t>>& v, char c)
{  // TODO: could binary search
  for (auto& p : v)
  {
    if (p.first == c) { return min_float(max_cost, (float)p.second); }
  }
  return 1.;
}

class Generator : public SearchTask<input, output>  // NOLINT
{
public:
  Generator(VW::workspace& vw_obj, Trie* _dict = nullptr)
      : SearchTask<input, output>(vw_obj), _dist(0), _dict(_dict)  // must run parent constructor!
  {
    // TODO: if action costs is specified but no allowed actions provided, don't segfault :P
    sch.set_options(Search::AUTO_CONDITION_FEATURES | Search::NO_CACHING | Search::ACTION_COSTS);
    HookTask::task_data& d = *sch.get_task_data<HookTask::task_data>();
    if (d.num_actions != 29) { THROW("Error: d.num_actions was not 29"); }
  }

  void _run(Search::search& sch, input& in, output& out)
  {
    IncrementalEditDistance ied(in.out);

    auto& vw_obj = sch.get_vw_pointer_unsafe();

    VW::v_array<action> ref;
    int n = (int)in.in.length();
    out = "^";
    std::vector<nextstr> next;
    for (int m = 1; m <= n * 2; m++)  // at most |in|*2 outputs
    {
      VW::example ex;

      // length info
      auto ns_hash_l = VW::hash_space(vw_obj, "l");
      auto& fs_l = ex.feature_space['l'];
      ex.indices.push_back('l');
      fs_l.push_back(static_cast<float>(n), VW::hash_feature(vw_obj, "in", ns_hash_l));
      fs_l.push_back(static_cast<float>(m), VW::hash_feature(vw_obj, "out", ns_hash_l));
      if (n != m) { fs_l.push_back(static_cast<float>(n - m), VW::hash_feature(vw_obj, "diff", ns_hash_l)); }

      // suffixes thus far
      auto ns_hash_s = VW::hash_space(vw_obj, "s");
      auto& fs_s = ex.feature_space['s'];
      ex.indices.push_back('s');
      std::string tmp("$");
      for (int i = m; i >= m - 15 && i >= 0; i--)
      {
        std::stringstream ss;
        ss << out[i] << tmp;
        tmp = ss.str();
        fs_s.push_back(1.f, VW::hash_feature(vw_obj, "p=" + tmp, ns_hash_s));
      }

      // characters thus far
      auto ns_hash_c = VW::hash_space(vw_obj, "c");
      auto& fs_c = ex.feature_space['c'];
      ex.indices.push_back('c');
      for (char c : out) { fs_c.push_back(1.f, VW::hash_feature(vw_obj, "c=" + std::string(1, c), ns_hash_c)); }
      fs_c.push_back(1.f, VW::hash_feature(vw_obj, "c=$", ns_hash_c));

      // words thus far
      auto ns_hash_w = VW::hash_space(vw_obj, "w");
      auto& fs_w = ex.feature_space['w'];
      ex.indices.push_back('w');
      tmp = "";
      for (char c : out)
      {
        if (c == '^') { continue; }
        if (c == ' ')
        {
          fs_w.push_back(1.f, VW::hash_feature(vw_obj, "w=" + tmp + "$", ns_hash_w));
          tmp = "";
        }
        else { tmp += c; }
      }
      fs_w.push_back(1.f, VW::hash_feature(vw_obj, "w=" + tmp, ns_hash_w));

      // do we match the trie?
      if (_dict)
      {
        next.clear();
        _dict->get_next(nullptr, next);

        auto ns_hash_d = VW::hash_space(vw_obj, "d");
        auto& fs_d = ex.feature_space['d'];
        ex.indices.push_back('d');

        char best_char = '~';
        float best_count = 0.;
        for (const auto& xx : next)
        {
          if (xx.cw > 0.) { fs_d.push_back(xx.cw, VW::hash_feature(vw_obj, "c=" + std::string(1, xx.c), ns_hash_d)); }
          if (xx.sw > 0.) { fs_d.push_back(xx.sw, VW::hash_feature(vw_obj, "mc=" + xx.s, ns_hash_d)); }
          if (xx.sw > best_count)
          {
            best_count = xx.sw;
            best_char = xx.c;
          }
        }
        if (best_count > 0.)
        {
          fs_d.push_back(best_count, VW::hash_feature(vw_obj, "best=" + std::string(1, best_char), ns_hash_d));
        }
      }

      // input
      /*
      ex(vw_namespace('i'));
      ex("c=^");
      for (int n=0; n<N; n++)
        ex("c=" + in.in[n]);
      ex("c=$");
      */
      auto ns_hash_i = VW::hash_space(vw_obj, "i");
      auto& fs_i = ex.feature_space['i'];
      ex.indices.push_back('i');
      tmp = "";
      for (char c : in.in)
      {
        if (c == ' ')
        {
          fs_i.push_back(1.f, VW::hash_feature(vw_obj, "w=" + tmp, ns_hash_i));
          tmp = "";
        }
        else { tmp += c; }
      }
      fs_i.push_back(1.f, VW::hash_feature(vw_obj, "w=" + tmp, ns_hash_i));

      ref.clear();

      /*
      std::vector<char>& best = ied.next();
      if (best.size() == 0) ref.push_back( char2action('$') );
      else for (char c : best) ref.push_back( char2action(c) );
      char c = action2char( Search::predictor(sch, m)
                            .set_input(* ex.get())
                            .set_oracle(ref)
                            .predict() );
      */
      VW::setup_example(vw_obj, &ex);
      std::vector<std::pair<action, float>> all = ied.all_next();
      char c = action2char(Search::predictor(sch, m).set_input(ex).set_allowed(all).predict());

      VW::finish_example(vw_obj, ex);

      if (c == '$') { break; }
      out += c;
      ied.append(c);
      if (_dict) { _dict = _dict->step(c); }
    }

    _dist = ied.finish_distance();
    sch.loss((float)_dist * in.weight);
  }

  size_t get_dist() { return _dist; }

private:
  size_t _dist;
  Trie* _dict;
};

void run_easy()
{
  auto vw_obj = VW::initialize(
      VW::make_unique<VW::config::options_cli>(std::vector<std::string>{"--search", "29", "--quiet", "--search_task",
          "hook", "--example_queue_limit", "1024", "--search_rollin", "learn", "--search_rollout", "none"}));
  Generator task(*vw_obj);
  output out("");

  std::vector<input> training_data = {input("maison", "house"), input("lune", "moon"),
      input("petite lune", "little moon"), input("la fleur", "the flower"), input("petite maison", "little house"),
      input("fleur", "flower"), input("la maison", "the house"), input("grande lune", "big moon"),
      input("grande fleur", "big flower")};
  std::vector<input> test_data = {input("petite fleur", "little flower"), input("grande maison", "big house")};
  for (size_t i = 0; i < 100; i++)
  {  // if (i == 9999) max_cost = 1.;
    if (i % 10 == 0) { cerr << '.'; }
    for (auto x : training_data) { task.learn(x, out); }
  }
  cerr << endl;

  for (auto x : training_data)
  {
    task.predict(x, out);
    cerr << "output = " << out << endl;
  }
  for (auto x : test_data)
  {
    task.predict(x, out);
    cerr << "output = " << out << endl;
  }
}

Trie load_dictionary(const char* fname)
{
  std::ifstream h(fname);
  Trie t;
  std::string line;
  while (getline(h, line))
  {
    const char* str = line.c_str();
    char* space = (char*)strchr(str, ' ');
    if (space)
    {
      *space = 0;
      space++;
      t.insert(space, atoi(str));
    }
    else { t.insert(str); }
  }
  return t;
}

void run_istream(Generator& gen, const char* fname, bool is_learn = true, size_t print_every = 0)
{
  std::ifstream h(fname);
  if (!h.is_open()) { THROW("cannot open file " << fname); }
  std::string line;
  output out;
  size_t n = 0;
  float dist = 0.;
  float weight = 0.;
  while (getline(h, line))
  {
    n++;
    if (n % 500 == 0) { cerr << '.'; }
    size_t i = line.find(" ||| ");
    size_t j = line.find(" ||| ", i + 1);
    if (i == std::string::npos || j == std::string::npos)
    {
      cerr << "skipping line " << n << ": '" << line << "'" << endl;
      continue;
    }
    input dat(line.substr(j + 5), line.substr(i + 5, j - i - 5), (float)(atof(line.substr(0, i).c_str()) / 10.));
    weight += dat.weight;
    if (is_learn) { gen.learn(dat, out); }
    else
    {
      gen.predict(dat, out);
      if (print_every > 0 && (n % print_every == 0))
      {
        std::cout << gen.get_dist() << "\t" << out << "\t\t\t" << dat.in << " ||| " << dat.out << endl;
      }
      dist += dat.weight * (float)gen.get_dist();
    }
  }
  if (n > 500) { cerr << endl; }
  if (!is_learn) { std::cout << "AVERAGE DISTANCE: " << (dist / weight) << endl; }
}

void train()
{  // initialize VW as usual, but use 'hook' as the search_task
  Trie dict = load_dictionary("phrase-table.vocab");
  dict.build_max();
  // dict.print();

  std::string init_str(
      "--search 29 -b 28 --quiet --search_task hook --example_queue_limit 1024 --search_rollin learn --search_rollout "
      "none -q i: --ngram i15 --skips i5 --ngram c15 --ngram w6 --skips c3 --skips w3");  //  --search_use_passthrough_repr");
                                                                                          //  // -q si -q wi -q ci -q di
                                                                                          //  -f my_model
  auto vw_obj = VW::initialize(VW::make_unique<VW::config::options_cli>(VW::split_command_line(init_str)));

  cerr << init_str << endl;
  // Generator gen(*vw_obj, nullptr); // &dict);
  for (size_t pass = 1; pass <= 20; pass++)
  {
    cerr << "===== pass " << pass << " =====" << endl;
    // run_istream(gen, "phrase-table.tr", true);
    // run_istream(gen, "phrase-table.tr", false, 300000);
    // run_istream(gen, "phrase-table.te", false, 100000);
    run_easy();
  }
  vw_obj->finish();
}

void predict()
{
  auto vw_obj = VW::initialize(VW::make_unique<VW::config::options_cli>(
      std::vector<std::string>{"--quiet", "-t", "--example_queue_limit", "1024", "-i", "my_model"}));
  vw_obj->finish();
}

int main(int argc, char* argv[])
{ /*
  std::string target(argv[1]);
  cerr << "target = " << target << endl;
  IncrementalEditDistance ied(target);
  cerr << "^: ";
  for (size_t i=0; i<=strlen(argv[2]); i++) {
   std::vector< std::pair<action,float> > next = ied.all_next();
    for (auto& p : next)
      cerr << action2char(p.first) << ' ' << p.second << "\t";
    cerr << endl;
    cerr << argv[2][i] << ": ";
    ied.append(argv[2][i]);
  }
  cerr << endl;
  */
  /*
  std::string target("abcde");
  IncrementalEditDistance ied(target);
  ied.append(std::string("cde"));
  while (true) {
   std::vector<char>& best = ied.next();
    cerr << ied.out() << " / " << ied.distance() << " -> "; for (char c : best) cerr << c; cerr << endl;
    char c = best[0];
    if (c == '$') break;
    ied.append(c);
  }
  cerr << "final: " << ied.distance() << "\t" << ied.out() << endl;
  return 0;
  */
  train();
  // predict();
  // run_easy();
}
