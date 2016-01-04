typedef float feature_value;
typedef uint64_t feature_index;
typedef pair<char*, char*> audit_strings;

struct feature {//sparse feature definition for the library interface
  float x;
  uint64_t weight_index;
  feature(float _x, uint64_t _index): x(_x), weight_index(_index) {}
  feature() {feature(0.f,0);}
};

struct feature_slice{ //a helper struct for functions using the set {v,i,space_name}
  feature_value v;
  feature_index i;
  audit_strings space_name;
};

inline int order_features(const void* first, const void* second)
{ if (((feature_slice*)first)->i != ((feature_slice*)second)->i)
    return (int)(((feature_slice*)first)->i - ((feature_slice*)second)->i);
  else if (((feature_slice*)first)->v > ((feature_slice*)second)->v)
    return 1;
  else
    return -1;
}

struct features { // the core definition of a set of features.
  v_array<feature_value> values; // Always needed.
  v_array<feature_index> indicies; //Optional for dense data.
  v_array<audit_strings> space_names; //Optional for audit mode.
  float sum_feat_sq;
  features()
  {
    values = v_init<feature_value>();
    indicies = v_init<feature_index>();
    space_names = v_init<audit_strings>();
    sum_feat_sq = 0.f;
  }
  inline size_t size() const { return values.size(); }
  inline bool nonempty() const { return values.begin != values.end; }

  void free_space_names(size_t i)
  {
    for (; i<space_names.size(); i++)
      { free_it(space_names[i].first);
        free_it(space_names[i].second);
      }
  }

  template<class R, void (*T)(R&, feature_value, feature_index)> inline void foreach_feature(R& dat, size_t j, size_t k)
  {
    feature_value* vp = values.begin + j;
    feature_index* ip = indicies.begin + j;
    feature_value* end = values.begin + k;
    for (;vp != end;++vp, ++ip)
      T(dat,*vp,*ip);
  }

  template<class R, void (*T)(R&, feature_value, feature_index)> inline void foreach_feature(R& dat, size_t j)
  { 
    feature_value* vp = values.begin + j;
    feature_index* ip = indicies.begin + j;
    for (;vp != values.end ;++vp, ++ip)
      T(dat,*vp,*ip);
  }

  template<class R, void (*T)(R&, feature_value, feature_index, audit_strings*)> inline void foreach_feature(R& dat, size_t j, size_t k)
  {
    feature_value* vp = values.begin + j;
    feature_index* ip = indicies.begin + j;
    audit_strings* ap = space_names.begin + j;
    feature_value* end = values.begin + k;
    for (;vp != end; ++vp, ++ip, ++ap)
      T(dat,*vp,*ip, ap);
  }

  template<class R, void (*T)(R&, feature_value, feature_index, audit_strings*)> inline void foreach_feature(R& dat, size_t j)
  { foreach_feature<R,T>(dat,j,size()); }

  void erase()
  {
    sum_feat_sq = 0.f;
    values.erase();
    indicies.erase();
    free_space_names(0);
    space_names.erase();
  }

  void truncate_to(size_t i)
  {
    values.end = values.begin + i;
    if (indicies.end != indicies.begin)
      indicies.end = indicies.begin + i;
    if (space_names.begin != space_names.end)
      {
	free_space_names(i);
	space_names.end = space_names.begin + i;
      }
  }

  void delete_v()
  {
    values.delete_v();
    indicies.delete_v();
    free_space_names(0);
    space_names.delete_v();
  }
  void push_back(feature_value v, feature_index i)
  {
    values.push_back(v);
    indicies.push_back(i);
    sum_feat_sq += v*v;
  }

  bool sort(uint64_t parse_mask)
  {
    if (indicies.size() == 0)
      return false;
    v_array<feature_slice> slice = v_init<feature_slice>();
    for (size_t i = 0; i < indicies.size(); i++)
      {
        feature_slice temp = {values[i], indicies[i] & parse_mask, audit_strings(nullptr, nullptr)};
        if (space_names.size() != 0)
          temp.space_name = space_names[i];
        slice.push_back(temp);
      }
    qsort(slice.begin, slice.size(), sizeof(feature_slice), order_features);
    for (size_t i = 0; i < slice.size(); i++)
      {
        values[i] = slice[i].v;
        indicies[i] = slice[i].i;
        if (space_names.size() > 0)
          space_names[i] = slice[i].space_name;
      }
    slice.delete_v();
    return true;
  }
};

inline void copy(features& dst, features& src)
{
  copy_array(dst.values, src.values);
  copy_array(dst.indicies, src.indicies);
  dst.free_space_names(0);
  copy_array(dst.space_names, src.space_names);
  dst.sum_feat_sq = src.sum_feat_sq;
}
