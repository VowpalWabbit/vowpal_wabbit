#include <sys/timeb.h>
#include "parse_args.h"
#include "parse_regressor.h"
#include "accumulate.h"
#include "best_constant.h"
#include "vw_exception.h"
#include <fstream>

#include "vw.h"
#include "options.h"
#include "options_boost_po.h"
#include "hashed_input_generated.h"
#include "flatbuffers/flatbuffers.h"

namespace VW
{

void convert_txt_to_flat(vw& all)
{
  flatbuffers::FlatBufferBuilder builder(1024);
  std::vector<flatbuffers::Offset<Example>> examplecollection;
  example* v = all.p->ready_parsed_examples.pop();
  while (v!=nullptr)
  {
    // Create Label for current example
    flatbuffers::Offset<Label> label = CreateLabel(builder, v->l.simple.label, v->l.simple.weight);

    //Iterate through namespaces to first create features
    std::vector<flatbuffers::Offset<Namespace>> namespaces;
    for (namespace_index& ns : v->indices)
    {
      std::vector<flatbuffers::Offset<Feature>> fts;
      for (size_t i=0; i<v->feature_space[ns].values.size(); i++)
      {
        fts.push_back(CreateFeature(builder, v->feature_space[ns].values[i], v->feature_space[ns].indicies[i]));
      }
      namespaces.push_back(CreateNamespaceDirect(builder, ns, &fts));
    }
    examplecollection.push_back(CreateExampleDirect(builder, &namespaces, label));

    v = all.p->ready_parsed_examples.pop();
  }

  flatbuffers::Offset<ExampleCollection> egcollection = CreateExampleCollectionDirect(builder, &examplecollection);

  builder.Finish(egcollection);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  std::ofstream outfile;
  outfile.open("first.dat", std::ios::binary | std::ios::out);
  outfile.write(reinterpret_cast<char*>(buf), size);
}

}