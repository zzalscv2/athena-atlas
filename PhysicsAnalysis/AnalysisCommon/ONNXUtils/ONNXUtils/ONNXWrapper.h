#ifndef ONNXUtils_h
#define ONNXUtils_h

// STL includes
#include <string>
#include <vector>

// ONNX Library
#include <core/session/onnxruntime_cxx_api.h>


class ONNXWrapper {

  private:

    // Class properties
    std::string m_modelName; // Path to the onnx file
    std::string m_modelPath; // Output of the path resolver

    // Features of the network structure

    // input and output nodes
    size_t m_nr_inputs;
    size_t m_nr_output;

    // define input tensor
    // std::vector<Ort::Value> input_tensor;
    // std::vector<Ort::Value> output_tensor;


    // dimensions of the input and output 
    std::vector<std::vector<int64_t>> m_input_dims;
    std::vector<std::vector<int64_t>> m_output_dims;


    // ONNX session objects
    std::unique_ptr<Ort::Session> m_onnxSession;
    std::unique_ptr< Ort::Env > m_onnxEnv;

    // onnx session options
    Ort::SessionOptions m_session_options;
    Ort::AllocatorWithDefaultOptions m_allocator;

    // allocate memory
    // Ort::MemoryInfo memory_info;
    // name of the input - access name map

    // name of the outputs
    std::vector<const char*> m_output_names;
    std::vector<int64_t> getShape(Ort::TypeInfo model_info);

  public:
    
    std::vector<const char*> m_input_names;

    // output vector - will be init with zero later
    std::vector<std::vector<float>> m_outputs;
    // Constructor with parameters
    ONNXWrapper(){};
    // ~ONNXWrapper(){};

    ONNXWrapper(std::string model_path);
    // void LoadModel(std::string model_path);
    void Run(std::map<std::string, std::vector<float>> inputs);
    void ModelINFO();
    void GetMETAData();
    std::string GetMETADataByKey(std::string key);
    std::vector<int64_t> getInputShape(int input_nr);
    std::vector<int64_t> getOutputShape(int output_nr);
    int getNumInputs();
    int getNumOutputs();

    // void Run(std::vector<std::vector<float>> inputs);
};
#endif
