/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include <iostream>

#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "PathResolver/PathResolver.h"
#include "CxxUtils/checker_macros.h"

namespace FlavorTagDiscriminants {

  // Constructor
  OnnxUtil::OnnxUtil(const std::string& path_to_onnx)
    //load the onnx model to memory using the path m_path_to_onnx
    : m_env (std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_FATAL, ""))
  {

    // initialize session options if needed
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    // Ignore all non-fatal errors. This isn't a good idea, but it's
    // what we get for uploading semi-working graphs.
    session_options.SetLogSeverityLevel(4);
    session_options.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    // create session and load model into memory
    m_session = std::make_unique< Ort::Session >(*m_env, path_to_onnx.c_str(),
                                                 session_options);


    std::string metadata = getMetaData("gnn_config");
    nlohmann::json j = nlohmann::json::parse(metadata);
    if (j.contains("onnx_model_version")){
      m_onnx_model_version = j["onnx_model_version"].get<OnnxModelVersion>();
      if (m_onnx_model_version == OnnxModelVersion::UNKNOWN){
        throw std::runtime_error("Unknown Onnx model version!");
      }
    } else {
      if (j.contains("outputs")){
        m_onnx_model_version = OnnxModelVersion::V0;
      } else {
        throw std::runtime_error("Onnx model version not found in metadata");
      }
    }

    Ort::AllocatorWithDefaultOptions allocator;

    // get the input nodes
    size_t num_input_nodes = m_session->GetInputCount();

    // iterate over all input nodes
    for (std::size_t i = 0; i < num_input_nodes; i++) {
      char* input_name = m_session->GetInputName(i, allocator);
      m_input_node_names.push_back(std::string(input_name));
    }

    // get the output nodes
    size_t num_output_nodes = m_session->GetOutputCount();
    std::vector<int64_t> output_node_dims;

    // iterate over all output nodes
    for(std::size_t i = 0; i < num_output_nodes; i++ ) {
      char* output_name = m_session->GetOutputName(i, allocator);
      ONNXOutputNode output_node;
      output_node.name = std::string(output_name);
      output_node.name_in_model = output_node.name;
      output_node.type = 
        m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetElementType();
      output_node.rank = 
        m_session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape().size();

      // needed for backward compatibility
      if (m_onnx_model_version == OnnxModelVersion::V0) {
        output_node.name = j["outputs"].begin().key() + "_" + output_node.name;
        output_node.rank = 0;
      }
      
      m_output_nodes.push_back(output_node);
    }
  }

  // Destructor
  OnnxUtil::~OnnxUtil() = default;

  std::string OnnxUtil::getMetaData(const std::string& key) const {

    Ort::AllocatorWithDefaultOptions allocator;
    Ort::ModelMetadata metadata = m_session->GetModelMetadata();
    std::string val = metadata.LookupCustomMetadataMap(key.c_str(), allocator);
    return val;
  }

  std::vector<ONNXOutputNode> OnnxUtil::getOutputNodeInfo() const {
    return m_output_nodes;
  }

  OnnxModelVersion OnnxUtil::getOnnxModelVersion() const {
    return m_onnx_model_version;
  }

  GNNConfig::OutputNodeType OnnxUtil::getOutputNodeType(
    const ONNXTensorElementDataType& type, int rank) const {

    if (type == ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
      if (rank == 0) {
        return GNNConfig::OutputNodeType::FLOAT;
      }
      else if (rank == 1) {
        return GNNConfig::OutputNodeType::VECFLOAT;
      }
    }
    else if (type == ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8) {
      return GNNConfig::OutputNodeType::VECCHAR;
    }
    return GNNConfig::OutputNodeType::UNKNOWN;
  }

  GNNConfig::Config OnnxUtil::get_config() const {

    GNNConfig::Config config;
    std::vector<FlavorTagDiscriminants::ONNXOutputNode> 
      outputnodes = getOutputNodeInfo();

    for (const auto& output : outputnodes) {
      GNNConfig::OutputNodeConfig output_config;
      output_config.label = output.name;

      GNNConfig::OutputNodeType type = getOutputNodeType(output.type, output.rank);
      output_config.type = type;
      config.outputs.push_back(output_config);
    }

    return config;
  }

  std::tuple<
    std::map<std::string, float>,
    std::map<std::string, std::vector<char>>,
    std::map<std::string, std::vector<float>> >
  OnnxUtil::runInference(
    std::map<std::string, input_pair>& gnn_inputs) const {

    // Args:
    //    gnn_inputs : {string: input_pair}
    //    outputs    : {string: float}

    std::vector<float> input_tensor_values;

    // create input tensor object from data values
    auto memory_info = Ort::MemoryInfo::CreateCpu(
      OrtArenaAllocator, OrtMemTypeDefault
    );

    std::vector<Ort::Value> input_tensors;
    for (auto const &node_name : m_input_node_names){
      input_tensors.push_back(Ort::Value::CreateTensor<float>(
        memory_info, gnn_inputs.at(node_name).first.data(), gnn_inputs.at(node_name).first.size(),
        gnn_inputs.at(node_name).second.data(), gnn_inputs.at(node_name).second.size())
      );
    }

    // casting vector<string> to vector<const char*>. this is what ORT expects
    std::vector<const char*> input_node_names(m_input_node_names.size(),nullptr);
    for (int i=0; i<static_cast<int>(m_input_node_names.size()); i++) {
      input_node_names[i]= m_input_node_names.at(i).c_str();
    }
    std::vector<const char*> output_node_names(m_output_nodes.size(),nullptr);
    for (int i=0; i<static_cast<int>(m_output_nodes.size()); i++) {
      output_node_names[i]= m_output_nodes.at(i).name_in_model.c_str();
    }

    // score model & input tensor, get back output tensor
    // Although Session::Run is non-const, the onnx authors say
    // it is safe to call from multiple threads:
    //  https://github.com/microsoft/onnxruntime/discussions/10107
    Ort::Session& session ATLAS_THREAD_SAFE = *m_session;
    auto output_tensors = session.Run(Ort::RunOptions{nullptr},
      input_node_names.data(), input_tensors.data(), input_node_names.size(),
      output_node_names.data(), output_node_names.size()
    );

    std::map<std::string, float> output_f;
    std::map<std::string, std::vector<char>> output_vc;
    std::map<std::string, std::vector<float>> output_vf;

    for (unsigned int node_idx=0; node_idx<m_output_nodes.size(); node_idx++){

      auto tensor_type = 
        output_tensors.at(node_idx).GetTypeInfo().GetTensorTypeAndShapeInfo().GetElementType();
      auto tensor_shape = 
        output_tensors.at(node_idx).GetTypeInfo().GetTensorTypeAndShapeInfo().GetShape();

      if (tensor_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT){
        if (tensor_shape.size() == 0){
          output_f.insert({m_output_nodes[node_idx].name,
            output_tensors.at(node_idx).GetTensorData<float>()[0]});
        }
        else if (tensor_shape.size() == 1){
          const float *float_ptr = output_tensors.at(node_idx).GetTensorData<float>();
          int float_ptr_len = 
            output_tensors[node_idx].GetTensorTypeAndShapeInfo().GetElementCount();

          output_vf.insert({m_output_nodes[node_idx].name,
            {float_ptr, float_ptr + float_ptr_len}});
        }
        else{
          throw std::runtime_error("OnnxUtil::runInference: unsupported tensor shape");
        }
      } 
      else if (tensor_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8){
        if (tensor_shape.size() == 1){
          const char *char_ptr = output_tensors.at(node_idx).GetTensorMutableData<char>();
          int char_ptr_len = 
            output_tensors[node_idx].GetTensorTypeAndShapeInfo().GetElementCount();

          output_vc.insert({m_output_nodes[node_idx].name,
            {char_ptr, char_ptr + char_ptr_len}});
        }
        else{
          throw std::runtime_error("OnnxUtil::runInference: unsupported tensor shape");
        }
      } else{
        throw std::runtime_error("OnnxUtil::runInference: unsupported tensor type");
      }
    }

    return std::make_tuple(output_f, output_vc, output_vf);
  }

} // end of FlavorTagDiscriminants namespace
