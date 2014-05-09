#include "gd.h"
#include "rand48.h"
#include "simple_label.h"
#include "allreduce.h"

//#undef NDEBUG
//#define DEBUG
#include <cassert>

#define PARENT_BIT 1
#define CYCLE_BIT 2
#define TREE_ATOMICS 134
#define MURMER_CONST 95104348457
#define TOL 1e-9
#define STRIDE_SHIFT(poly, idx) ((idx) << (poly).all->reg.stride_shift)
#define STRIDE_UN_SHIFT(poly, idx) ((idx) >> (poly).all->reg.stride_shift)
#define CONSTANT_FEAT(poly) (STRIDE_SHIFT(poly, constant))
#define CONSTANT_FEAT_MASKED(poly) (WID_MASK(CONSTANT_FEAT(poly), poly))
#define WID_MASK(wid, poly)  ((wid) & poly.all->reg.weight_mask)
#define WID_MASK_UN_SHIFTED(wid, poly)  (STRIDE_UN_SHIFT((poly), ((wid) & poly.all->reg.weight_mask)))

//#define PARALLEL_ENABLE

using namespace std;
using namespace LEARNER;

namespace StagewisePoly
{
  struct sort_data {
    float wval;
    uint32_t wid;
  };

  struct stagewise_poly
  {
    vw *all;

    float sched_exponent;
    float magic_argument;
    uint32_t batch_sz;

    char *bits;
    sort_data *sd;
    uint32_t *min_depths;
    uint32_t num_features;

    uint64_t sum_sparsity; //of synthetic example
    uint64_t sum_input_sparsity; //of input example

    example synth_ec;
    //following is bookkeeping in synth_ec creation (dfs)
    feature synth_rec_f;
    example *original_ec;
    uint32_t cur_depth;

#ifdef DEBUG
    uint32_t max_depth;
    uint32_t depths[100000];
#endif //DEBUG
  };

  void bits_create(stagewise_poly &poly)
  {
    poly.bits = (char *) calloc(poly.all->length(), sizeof(char));
  }

  void bits_destroy(stagewise_poly &poly)
  {
    free(poly.bits);
  }

  inline bool parent_get(const stagewise_poly &poly, uint32_t wid)
  {
    assert(wid % STRIDE_SHIFT(poly, 1) == 0);
    return poly.bits[WID_MASK_UN_SHIFTED(wid, poly)] & PARENT_BIT;
  }

  inline void parent_toggle(stagewise_poly &poly, uint32_t wid)
  {
    assert(wid % STRIDE_SHIFT(poly, 1) == 0);
    poly.bits[WID_MASK_UN_SHIFTED(wid, poly)] ^= PARENT_BIT;
  }

  inline bool cycle_get(const stagewise_poly &poly, uint32_t wid)
  {
    assert(wid % STRIDE_SHIFT(poly, 1) == 0);
    return poly.bits[WID_MASK_UN_SHIFTED(wid, poly)] & CYCLE_BIT;
  }

  inline void cycle_toggle(stagewise_poly &poly, uint32_t wid)
  {
    assert(wid % STRIDE_SHIFT(poly, 1) == 0);
    poly.bits[WID_MASK_UN_SHIFTED(wid, poly)] ^= CYCLE_BIT;
  }

  void min_depths_create(stagewise_poly &poly)
  {
    poly.min_depths = (uint32_t *) malloc(poly.all->length() * sizeof(uint32_t));
    memset(poly.min_depths, 0xff, poly.all->length() * sizeof(uint32_t));
  }

  void min_depths_destroy(stagewise_poly &poly)
  {
    free(poly.min_depths);
  }

  inline uint32_t min_depths_get(const stagewise_poly &poly, uint32_t wid)
  {
    assert(wid % STRIDE_SHIFT(poly, 1) == 0);
    return poly.min_depths[STRIDE_UN_SHIFT(poly, wid)];
  }

  inline void min_depths_set(stagewise_poly &poly, uint32_t wid, uint32_t depth)
  {
    assert(wid % STRIDE_SHIFT(poly, 1) == 0);
    poly.min_depths[STRIDE_UN_SHIFT(poly, wid)] = depth;
  }

  //Note.  OUTPUT & INPUT masked.
  inline uint32_t child_wid(const stagewise_poly &poly, uint32_t wi_atomic, uint32_t wi_general)
  {
    assert(wi_atomic == WID_MASK(wi_atomic, poly));
    assert(wi_general == WID_MASK(wi_general, poly));

    if (wi_atomic == CONSTANT_FEAT_MASKED(poly))
      return wi_general;
    else if (wi_general == CONSTANT_FEAT_MASKED(poly))
      return wi_atomic;
    else if (wi_atomic == wi_general) {
      uint64_t wi_2_64 = wi_general;
      //Note, weight_mask used to be (?) all.length()-1, but a vw update (?) made it (all.length() << stride_shift) - 1
      return WID_MASK(STRIDE_SHIFT(poly, (size_t)(merand48(wi_2_64) * poly.all->reg.weight_mask)), poly);
    } else
      return WID_MASK((wi_atomic ^ wi_general ) * MURMER_CONST, poly);
  }

  void sort_data_create(stagewise_poly &poly)
  {
    poly.sd = (sort_data *) malloc(poly.all->length() * sizeof(sort_data));
  }

  void sort_data_destroy(stagewise_poly &poly)
  {
    free(poly.sd);
  }

#ifdef DEBUG
  int sort_data_compar(const void *a_v, const void *b_v)
  {
    return 2 * ( ((sort_data *) a_v)->wval < ((sort_data *) b_v)->wval ) - 1;
  }
#endif //DEBUG

  int sort_data_compar_heap(sort_data &a_v, sort_data &b_v)
  {
    return (a_v.wval > b_v.wval);
  }

  /*
   * Performance note.
   *
   * On my laptop (Intel(R) Core(TM) i7-3520M CPU @ 2.90GHz), with compiler
   * optimizations enabled, this routine takes ~0.001 seconds with -b 18 and
   * ~0.06 seconds with -b 24.  Since it is intended to run ~8 times in 1-pass
   * mode and otherwise once per pass, it is considered adequate.
   *
   * Another choice (implemented in another version) is to never process the
   * whole weight vector (e.g., by updating a hash set of nonzero weights after
   * creating the synthetic example, and only processing that here).  This
   * choice was implemented in a similar algorithm and performed well.
   */
  void sort_data_update_support(stagewise_poly &poly)
  {
    assert(poly.synth_ec.example_counter);
    uint32_t num_new_features = pow((poly.sum_input_sparsity * 1.0 / poly.synth_ec.example_counter), poly.sched_exponent);

    cout<<"Updating support after "<<poly.synth_ec.example_counter<<" examples\n";

    sort_data *heap_end = poly.sd;
    make_heap(poly.sd, heap_end, sort_data_compar_heap); //redundant
    for (uint32_t i = 0; i != poly.all->length(); ++i) {
      uint32_t wid = STRIDE_SHIFT(poly, i);
      if (!parent_get(poly, wid) && wid != CONSTANT_FEAT_MASKED(poly)) {
        float wval = (fabsf(poly.all->reg.weight_vector[wid])
            * poly.all->reg.weight_vector[poly.all->normalized_idx + (wid)])
          /*
           * here's some depth penalization code.  It was found to not improve
           * statistical performance, and meanwhile it is verified as giving
           * a nontrivial computational hit, thus commented out.
           *
           * - poly.magic_argument
           * sqrtf(min_depths_get(poly, STRIDE_SHIFT(poly, i)) * 1.0 / poly.synth_ec.example_counter)
           */
          ;
        if (wval > TOL) {
          assert(heap_end >= poly.sd);
          assert(heap_end <= poly.sd + num_new_features);

          if (heap_end - poly.sd == num_new_features && poly.sd->wval < wval) {
            pop_heap(poly.sd, heap_end, sort_data_compar_heap);
            --heap_end;
          }

          assert(heap_end >= poly.sd);
          assert(heap_end < poly.sd + poly.all->length());

          if (heap_end - poly.sd < num_new_features) {
            heap_end->wval = wval;
            heap_end->wid = wid;
            ++heap_end;
            push_heap(poly.sd, heap_end, sort_data_compar_heap);
          }
        }
      }
    }
    num_new_features = (uint32_t) (heap_end - poly.sd);

#ifdef DEBUG
    //eyeballing weights a pain if unsorted.
    qsort(poly.sd, num_new_features, sizeof(sort_data), sort_data_compar);
#endif //DEBUG

    for (uint32_t pos = 0; pos < num_new_features && pos < poly.all->length(); ++pos) {
      assert(!parent_get(poly, poly.sd[pos].wid)
          && poly.sd[pos].wval > TOL
          && poly.sd[pos].wid != CONSTANT_FEAT_MASKED(poly));
      parent_toggle(poly, poly.sd[pos].wid);
#ifdef DEBUG
      cout
        << "Adding feature " << pos << "/" << num_new_features
        << " || wid " << poly.sd[pos].wid
        << " || sort value " << poly.sd[pos].wval
        << endl;
#endif //DEBUG
    }

#ifdef DEBUG
    cout << "depths:";
    for (uint32_t depth = 0; depth <= poly.max_depth && depth < sizeof(poly.depths) / sizeof(*poly.depths); ++depth)
      cout << "  [" << depth << "] = " << poly.depths[depth];
    cout << endl;
#endif //DEBUG
  }

  void synthetic_reset(stagewise_poly &poly, example &ec)
  {
    poly.synth_ec.ld = ec.ld;
    poly.synth_ec.tag = ec.tag;
    poly.synth_ec.example_counter = ec.example_counter;
    poly.synth_ec.ft_offset = ec.ft_offset;

    poly.synth_ec.test_only = ec.test_only;
    poly.synth_ec.end_pass = ec.end_pass;
    poly.synth_ec.sorted = ec.sorted;
    poly.synth_ec.in_use = ec.in_use;

    poly.synth_ec.atomics[TREE_ATOMICS].erase();
    poly.synth_ec.audit_features[TREE_ATOMICS].erase();
    poly.synth_ec.num_features = 0;
    poly.synth_ec.total_sum_feat_sq = 0;
    poly.synth_ec.sum_feat_sq[TREE_ATOMICS] = 0;
    poly.synth_ec.example_t = ec.example_t;

    if (poly.synth_ec.indices.size()==0)
      poly.synth_ec.indices.push_back(TREE_ATOMICS);
  }

  void synthetic_cleanup(stagewise_poly &poly)
  {
    for (feature *f = poly.synth_ec.atomics[TREE_ATOMICS].begin;
        f != poly.synth_ec.atomics[TREE_ATOMICS].end; ++f) {
      assert(cycle_get(poly, f->weight_index));
      cycle_toggle(poly, f->weight_index);
    }
  }

  void synthetic_create_rec(stagewise_poly &poly, float v, float &w)
  {
    uint32_t wid_atomic = (uint32_t)((&w - poly.all->reg.weight_vector));
    uint32_t wid_cur = child_wid(poly, wid_atomic, poly.synth_rec_f.weight_index);
    assert(wid_atomic % STRIDE_SHIFT(poly, 1) == 0);

    if (poly.cur_depth < min_depths_get(poly, wid_cur) && poly.all->training) {
      if (parent_get(poly, wid_cur)) {
#ifdef DEBUG
        cout
          << "FOUND A TRANSPLANT!!! moving [" << wid_cur
          << "] from depth " << min_depths_get(poly, wid_cur)
          << " to depth " << poly.cur_depth << endl;
#endif //DEBUG
        parent_toggle(poly, wid_cur);
      }
      min_depths_set(poly, wid_cur, poly.cur_depth);
    }

    if ( ! cycle_get(poly, wid_cur)
        && (poly.cur_depth == min_depths_get(poly, wid_cur))
       ) {
      cycle_toggle(poly, wid_cur);

#ifdef DEBUG
      ++poly.depths[poly.cur_depth];
#endif //DEBUG

      feature new_f = { v * poly.synth_rec_f.x, wid_cur };
      poly.synth_ec.atomics[TREE_ATOMICS].push_back(new_f);
      poly.synth_ec.num_features++;
      poly.synth_ec.sum_feat_sq[TREE_ATOMICS] += new_f.x * new_f.x;

      if (parent_get(poly, new_f.weight_index)) {
        feature parent_f = poly.synth_rec_f;
        poly.synth_rec_f = new_f;
        ++poly.cur_depth;
#ifdef DEBUG
        poly.max_depth = (poly.max_depth > poly.cur_depth) ? poly.max_depth : poly.cur_depth;
#endif //DEBUG
        GD::foreach_feature<stagewise_poly, synthetic_create_rec>(*(poly.all), *(poly.original_ec), poly);
        --poly.cur_depth;
        poly.synth_rec_f = parent_f;
      }
    }
  }

  void synthetic_create(stagewise_poly &poly, example &ec)
  {
    synthetic_reset(poly, ec);

    poly.cur_depth = 0;

    poly.synth_rec_f.x = 1.0;
    poly.synth_rec_f.weight_index = CONSTANT_FEAT_MASKED(poly);
    poly.original_ec = &ec;
    /*
     * Another choice is to mark the constant feature as the single initial
     * parent, and recurse just on that feature (which arguably correctly interprets poly.cur_depth).
     * Problem with this is if there is a collision with the root...
     */
    GD::foreach_feature<stagewise_poly, synthetic_create_rec>(*poly.all, *poly.original_ec, poly);
    poly.synth_ec.total_sum_feat_sq = poly.synth_ec.sum_feat_sq[TREE_ATOMICS];

    poly.sum_sparsity += poly.synth_ec.num_features;
    poly.sum_input_sparsity += ec.num_features;
  }

  void learn(stagewise_poly &poly, learner &base, example &ec)
  {
    synthetic_create(poly, ec);

    base.learn(poly.synth_ec);
    poly.num_features = poly.synth_ec.num_features;

    ((label_data *) ec.ld)->prediction = ((label_data *)(poly.synth_ec.ld))->prediction;
    ec.loss = poly.synth_ec.loss;

    if (poly.all->training && !ec.test_only
        && ec.example_counter
        && poly.batch_sz
        && !(ec.example_counter % poly.batch_sz)
       ) {
      sort_data_update_support(poly);
    }

    synthetic_cleanup(poly);
  }


#ifdef PARALLEL_ENABLE
  void reduce_min(uint32_t &v1,const uint32_t &v2)
  {
    v1 = (v1 <= v2) ? v1 : v2;
  }

  void end_pass(stagewise_poly& poly)
  {
  //vw &all = *poly.all;
  //if(all.span_server != "")
  //  all_reduce<uint32_t, reduce_min>(poly.min_depths, all.total, all.span_server, all.unique_id, all.total, all.node, all.socks);

    if (!poly.batch_sz) {
      sort_data_update_support(poly);
    }
  }
#endif //PARALLEL_ENABLE

  void finish_example(vw& all, stagewise_poly& poly, example& ec)
  {
    size_t temp_num_features = ec.num_features;
    ec.num_features = poly.num_features;
    output_and_account_example(all,ec);
    ec.num_features = temp_num_features;
    VW::finish_example(all, &ec);
  }

  void finish(stagewise_poly &poly)
  {
#ifdef DEBUG
    cout<<"total feature number (after poly expansion!) = " << poly.sum_sparsity << endl;
#endif //DEBUG

    poly.synth_ec.atomics[TREE_ATOMICS].delete_v();
    bits_destroy(poly);
    sort_data_destroy(poly);
    min_depths_destroy(poly);
  }


  void save_load(stagewise_poly &poly, io_buf &model_file, bool read, bool text)
  {
    cout<<"In save_load\n";

    uint32_t length = poly.all->length();
    if (model_file.files.size() > 0) {
      bin_text_read_write_fixed(model_file, poly.bits, length * sizeof(char),"", read, "", 0, text);
      bin_text_read_write_fixed(model_file, (char *) poly.min_depths, length * sizeof(uint32_t),"", read, "", 0, text);
    }
  }


  learner *setup(vw &all, po::variables_map &vm)
  {
    stagewise_poly *poly = (stagewise_poly *) calloc(1, sizeof(stagewise_poly));
    poly->all = &all;

    bits_create(*poly);
    sort_data_create(*poly);
    min_depths_create(*poly);
    
    po::options_description sp_opt("Stagewise poly options");
    sp_opt.add_options()      
      ("sched_exponent", po::value<float>(), "exponent on schedule")
      ("magic_argument", po::value<float>(), "magical feature flag")
      ("batch_sz", po::value<uint32_t>(), "batch size");
    vm = add_options(all, sp_opt);
        
    poly->sched_exponent = vm.count("sched_exponent") ? vm["sched_exponent"].as<float>() : 0.;
    poly->magic_argument = vm.count("magic_argument") ? vm["magic_argument"].as<float>() : 0.;
    poly->batch_sz = vm.count("batch_sz") ? vm["batch_sz"].as<uint32_t>() : 0;

    poly->sum_sparsity = 0;
    poly->sum_input_sparsity = 0;

    learner *l = new learner(poly, all.l);
    l->set_learn<stagewise_poly, learn>();
    l->set_finish<stagewise_poly, finish>();
    l->set_save_load<stagewise_poly, save_load>();
    l->set_finish_example<stagewise_poly,finish_example>();
#ifdef PARALLEL_ENABLE
    l->set_end_pass<stagewise_poly, end_pass>();
#endif
    return l;
  }
}
