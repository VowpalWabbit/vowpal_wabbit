#include "test_generated.h"
#include "preamble.h"
#include <fstream>
#include <vector>
#include <string>

int main(){
  flatbuffers::FlatBufferBuilder builder(1024);

  auto feature_one_name = builder.CreateString("price");
  auto feature_one_one_value = 0.23;

  auto feature_two_name = builder.CreateString("sqft");
  auto feature_one_two_value = 0.25;

  auto feature_three_name = builder.CreateString("age");
  auto feature_one_three_value = 0.05;

  auto feature11 = CreateFeature(builder, feature_one_name, feature_one_one_value);
  auto feature12 = CreateFeature(builder, feature_two_name, feature_one_two_value);
  auto feature13 = CreateFeature(builder, feature_three_name, feature_one_three_value);

  std::vector<flatbuffers::Offset<Feature> > feature_vector1;
  feature_vector1.push_back(feature11);
  feature_vector1.push_back(feature12);
  feature_vector1.push_back(feature13);

  auto features1 = builder.CreateVector(feature_vector1);
  int label1 = 0;

  auto datapoint1 = CreateDatapoint(builder, label1, features1);

  auto feature_two_one_value = 0.18;
  auto feature_two_two_value = 0.15;
  auto feature_two_three_value = 0.35;

  auto feature21 = CreateFeature(builder, feature_one_name, feature_two_one_value);
  auto feature22 = CreateFeature(builder, feature_two_name, feature_two_two_value);
  auto feature23 = CreateFeature(builder, feature_three_name, feature_two_three_value);

  std::vector<flatbuffers::Offset<Feature>> feature_vector2;
  feature_vector2.push_back(feature21);
  feature_vector2.push_back(feature22);
  feature_vector2.push_back(feature23);

  auto features2 = builder.CreateVector(feature_vector2);
  int label2 = 0;

  auto datapoint2 = CreateDatapoint(builder, label2, features2);

  auto feature_three_one_value = 0.18;
  auto feature_three_two_value = 0.15;
  auto feature_three_three_value = 0.35;

  auto feature31 = CreateFeature(builder, feature_one_name, feature_three_one_value);
  auto feature32 = CreateFeature(builder, feature_two_name, feature_three_two_value);
  auto feature33 = CreateFeature(builder, feature_three_name, feature_three_three_value);

  std::vector<flatbuffers::Offset<Feature>> feature_vector3;
  feature_vector3.push_back(feature31);
  feature_vector3.push_back(feature32);
  feature_vector3.push_back(feature33);

  auto features3 = builder.CreateVector(feature_vector3);
  int label3 = 0;

  auto datapoint3 = CreateDatapoint(builder, label3, features3);

  std::vector<flatbuffers::Offset<Datapoint>> datapoints;
  datapoints.push_back(datapoint1);
  datapoints.push_back(datapoint2);
  datapoints.push_back(datapoint3);

  auto data = CreateDataDirect(builder, &datapoints);

  builder.Finish(data);

  uint8_t *buf = builder.GetBufferPointer();
  int size = builder.GetSize();

  char test_preamble[8];
  VW::preamble p;
  std::ofstream outfile;

  if (p.write_to_bytes(buf, size)) {
    outfile.open("first.dat", std::ios::binary | std::ios::out);
    outfile.write(reinterpret_cast<char*>(buf), size);
  }
}