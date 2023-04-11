/*
Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ONNXUTIL_H
#define ONNXUTIL_H

#include "nlohmann/json.hpp"
#include <core/session/onnxruntime_cxx_api.h>
#include <map>

#include "FlavorTagDiscriminants/GNNConfig.h"

namespace FlavorTagDiscriminants {

  typedef std::pair<std::vector<float>, std::vector<int64_t>> input_pair;

  enum class OnnxModelVersion{
    UNKNOWN,
    V0,
    V1
  };

  NLOHMANN_JSON_SERIALIZE_ENUM( OnnxModelVersion , {
    { OnnxModelVersion::UNKNOWN, "" },
    { OnnxModelVersion::V0, "v0" },
    { OnnxModelVersion::V1, "v1" },
  })

  struct ONNXOutputNode {
    std::string name;
    ONNXTensorElementDataType type;
    int rank;

    // needed for the old metadata format
    std::string name_in_model;
  };

  //
  // Utility class that loads the onnx model from the given path
  // and runs inference based on the user given inputs

  class OnnxUtil final{

    public:

      OnnxUtil(const std::string& path_to_onnx);
      ~OnnxUtil();

      void initialize();

      std::tuple<
        std::map<std::string, float>,
        std::map<std::string, std::vector<char>>,
        std::map<std::string, std::vector<float>> >
      runInference(
        std::map<std::string, input_pair> & gnn_inputs) const;

      std::string getMetaData(const std::string& key) const;
      GNNConfig::Config get_config() const;
      GNNConfig::OutputNodeType getOutputNodeType(
        const ONNXTensorElementDataType& type, int rank) const;

      std::vector<ONNXOutputNode> getOutputNodeInfo() const;

      OnnxModelVersion getOnnxModelVersion() const;

    private:

      std::string m_path_to_onnx;

      std::unique_ptr< Ort::Session > m_session;
      std::unique_ptr< Ort::Env > m_env;

      std::vector<std::string> m_input_node_names;
      std::vector<ONNXOutputNode> m_output_nodes;

      OnnxModelVersion m_onnx_model_version = OnnxModelVersion::UNKNOWN;

  }; // Class OnnxUtil
} // end of FlavorTagDiscriminants namespace

#endif //ONNXUTIL_H
