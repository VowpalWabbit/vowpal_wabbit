#include "reductions.h"
#include "simple_label.h"
#include "gd.h"
#include "multiclass.h"

using namespace LEARNER;

namespace CENTERING {

  struct centering	
  {
    double* means;
    size_t ec_cnt; 	
    size_t f_cnt; 	
    example ec;
    
    bool check;    
    
    vw* all;
  };	

  inline void update_mean(centering& b, float x, float& fw, size_t index)
  {
    if(index == 0)
    {
      if(b.check)
	return;
      else
        b.check = true;
    }
    
    b.means[index] += x;
    cout << index << ":" << x << ":" << b.means[index] <<"\t";
  }
  
  void add_mean_to_features(vw& all, example& ec)
  {
    for (unsigned char* i = ec.indices.begin; i != ec.indices.end; i++) 
      {
        size_t original_length = ec.atomics[*i].size();
        for (uint32_t j = 0; j < original_length; j++)
          {
            feature* f = &ec.atomics[*i][j];
	    cout << f->x << "\t";
            f->x = f->x + 1;
          }
      }
      cout << endl;
  }

  template <bool is_learn>
  void predict_or_learn(centering& b, learner& base, example& ec) {
    b.check = false;
    GD::foreach_feature<centering, update_mean>(*b.all, ec, b);
    b.ec_cnt++;
    
    //cout << endl;
    cin.ignore();    
    
    VW::copy_example_data(b.all->audit, &b.ec, &ec);
    
    add_mean_to_features(*b.all, b.ec);
    
    b.ec.ld = ec.ld;
    
    if (is_learn)
      base.learn(b.ec);
    else
      base.predict(b.ec);   
      
    ec.final_prediction = b.ec.final_prediction;
    ec.partial_prediction = b.ec.partial_prediction;
      
    //VW::copy_example_data(b.all->audit, &ec, &b.ec);   
  }

  learner* setup(vw& all, std::vector<std::string>&opts, po::variables_map& vm, po::variables_map& vm_file)
  {//parse and set arguments
    centering* data = (centering*)calloc(1, sizeof(centering));
    data->all = &all;
    data->means = new double[3];
    for (size_t i = 0; i < 3; i++)
      data->means[i] = 0;
    data->ec_cnt = 0;
    
    if (!vm_file.count("centering"))
    {
      std::stringstream ss;
      ss << " --centering ";
      all.options_from_file.append(ss.str());
    }

    //Create new learner
    learner* ret = new learner(data, all.l);
    ret->set_learn<centering, predict_or_learn<true> >();
    ret->set_predict<centering, predict_or_learn<false> >();
    return ret;
  }
}
