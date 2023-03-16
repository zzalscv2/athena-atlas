#include <ONNXUtils/ONNXWrapper.h>
#include <iostream>

int main(int narg, char* argv[]) {
  if (narg != 3 && narg != 2) {
    std::cout << "usage: " << argv[0] << " <onnx_file> [key]" << std::endl;
    return 1;
  }

  ONNXWrapper* model = new ONNXWrapper(argv[1]);
  

  // no key provided, print out all
  if (narg == 2) {
    // print input/output shape and names
    model->ModelINFO();

    // print meta data
    model->GetMETAData();
    delete model;
    return 2;
  }

  // key provided, print out key
  std::cout << model->GetMETADataByKey(argv[2]) << std::endl;
  delete model;

  return 0;
}
