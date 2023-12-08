#include <ONNXUtils/ONNXWrapper.h>
#include <iostream>

int main(int narg, char* argv[]) {
  std::cout<<narg;
  if (narg != 3 && narg != 2) {
    std::cout << "usage: " << argv[0] << " <onnx_file> [key]" << std::endl;
    return 1;
  }

  ONNXWrapper* model = new ONNXWrapper(argv[1]);
  

  // no key provided, print out all
  if (narg == 2) {  // should be extend to more that 2 inputs and outputs
    // print input/output shape and names
    for ( const auto &p : model->GetModelInputs() )
    {
      std::string separator;
      std::cout << "Input: " << p.first << ", shape: (";
      for (auto x : p.second) {
        std::cout << separator << x;
        separator = ",";
      }
      std::cout << ")" << std::endl;
    } 
    for ( const auto &p : model->GetModelOutputs() )
    {
      std::string separator;
      std::cout << "Outputs: " << p.first << ", shape: (";
      for (auto x : p.second) {
        std::cout << separator << x;
        separator = ",";
      }
      std::cout << ")" << std::endl;
    }
    for ( const auto &p : model->GetMETAData() )
    {
      std::cout <<"META data: "<< p.first << '\t' << p.second.at(0) << std::endl;
    } 
    // print meta data
    delete model;
    return 2;
  }

  // key provided, print out key
  std::cout << model->GetMETADataByKey(argv[2]) << std::endl;
  delete model;

  return 0;
}
