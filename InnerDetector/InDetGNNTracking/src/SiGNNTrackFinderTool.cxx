/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiGNNTrackFinderTool.h"
#include "ExaTrkXUtils.hpp"

// Framework include(s).
#include "PathResolver/PathResolver.h"
#include "AthOnnxruntimeUtils/OnnxUtils.h"
#include <cmath>

InDet::SiGNNTrackFinderTool::SiGNNTrackFinderTool(
  const std::string& type, const std::string& name, const IInterface* parent):
    base_class(type, name, parent)
  {
    declareInterface<IGNNTrackFinder>(this);
  }

StatusCode InDet::SiGNNTrackFinderTool::initialize() {
  initTrainedModels();
  return StatusCode::SUCCESS;
}

void InDet::SiGNNTrackFinderTool::initTrainedModels() {
  std::string embedModelPath(m_inputMLModuleDir + "/torchscript/embedding.onnx");
  std::string filterModelPath(m_inputMLModuleDir + "/torchscript/filtering.onnx");
  std::string gnnModelPath(m_inputMLModuleDir + "/torchscript/gnn.onnx");

  m_embedSession = AthONNX::CreateORTSession(embedModelPath, m_useCUDA);
  m_filterSession = AthONNX::CreateORTSession(filterModelPath, m_useCUDA);
  m_gnnSession = AthONNX::CreateORTSession(gnnModelPath, m_useCUDA);
}

StatusCode InDet::SiGNNTrackFinderTool::finalize() {
  StatusCode sc = AlgTool::finalize();
  return sc;
}

MsgStream&  InDet::SiGNNTrackFinderTool::dump( MsgStream& out ) const
{
  out<<std::endl;
  return dumpevent(out);
}

std::ostream& InDet::SiGNNTrackFinderTool::dump( std::ostream& out ) const
{
  return out;
}

MsgStream& InDet::SiGNNTrackFinderTool::dumpevent( MsgStream& out ) const
{
  out<<"|---------------------------------------------------------------------|"
       <<std::endl;
  out<<"| Number output tracks    | "<<std::setw(12)  
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  return out;
}

void InDet::SiGNNTrackFinderTool::getTracks (
  const std::vector<const Trk::SpacePoint*>& spacepoints,
  std::vector<std::vector<uint32_t> >& tracks) const
{
  int64_t numSpacepoints = (int64_t)spacepoints.size();
  std::vector<float> inputValues;
  std::vector<uint32_t> spacepointIDs;

  int64_t spacepointFeatures = 3;
  int sp_idx = 0;
  for(const auto& sp: spacepoints){
    // depending on the trained embedding and GNN models, the input features
    // may need to be updated.

    float z = sp->globalPosition().z() / 1000.;
    float r = sp->r() / 1000.;
    float phi = sp->phi() / M_PI;
    inputValues.push_back(r);
    inputValues.push_back(phi);
    inputValues.push_back(z);


    spacepointIDs.push_back(sp_idx++);
  }

    Ort::AllocatorWithDefaultOptions allocator;
    auto memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    // ************
    // Embedding
    // ************

    std::vector<int64_t> eInputShape{numSpacepoints, spacepointFeatures};

    std::vector<const char*> eInputNames{"sp_features"};
    std::vector<Ort::Value> eInputTensor;
    eInputTensor.push_back(
        Ort::Value::CreateTensor<float>(
            memoryInfo, inputValues.data(), inputValues.size(),
            eInputShape.data(), eInputShape.size())
    );

    std::vector<float> eOutputData(numSpacepoints * m_embeddingDim);
    std::vector<const char*> eOutputNames{"embedding_output"};
    std::vector<int64_t> eOutputShape{numSpacepoints, m_embeddingDim};
    std::vector<Ort::Value> eOutputTensor;
    eOutputTensor.push_back(
        Ort::Value::CreateTensor<float>(
            memoryInfo, eOutputData.data(), eOutputData.size(),
            eOutputShape.data(), eOutputShape.size())
    );
    AthONNX::InferenceWithIOBinding(m_embedSession, eInputNames, eInputTensor, eOutputNames, eOutputTensor);

    // ************
    // Building Edges
    // ************
    std::vector<int64_t> edgeList;
    buildEdges(eOutputData, edgeList, numSpacepoints, m_embeddingDim, m_rVal, m_knnVal);
    int64_t numEdges = edgeList.size() / 2;

    // ************
    // Filtering
    // ************
    std::vector<const char*> fInputNames{"f_nodes", "f_edges"};
    std::vector<Ort::Value> fInputTensor;
    fInputTensor.push_back(
        std::move(eInputTensor[0])
    );
    std::vector<int64_t> fEdgeShape{2, numEdges};
    fInputTensor.push_back(
        Ort::Value::CreateTensor<int64_t>(
            memoryInfo, edgeList.data(), edgeList.size(),
            fEdgeShape.data(), fEdgeShape.size())
    );

    // filtering outputs
    std::vector<const char*> fOutputNames{"f_edge_score"};
    std::vector<float> fOutputData(numEdges);
    std::vector<int64_t> fOutputShape{numEdges, 1};
    std::vector<Ort::Value> fOutputTensor;
    fOutputTensor.push_back(
        Ort::Value::CreateTensor<float>(
            memoryInfo, fOutputData.data(), fOutputData.size(), 
            fOutputShape.data(), fOutputShape.size())
    );
    AthONNX::InferenceWithIOBinding(m_filterSession, fInputNames, fInputTensor, fOutputNames, fOutputTensor);

    // apply sigmoid to the filtering output data
    // and remove edges with score < filterCut
    std::vector<int64_t> rowIndices;
    std::vector<int64_t> colIndices;
    for (int64_t i = 0; i < numEdges; i++){
        float v = 1.f / (1.f + std::exp(-fOutputData[i]));  // sigmoid, float type
        if (v > m_filterCut){
            rowIndices.push_back(edgeList[i]);
            colIndices.push_back(edgeList[numEdges + i]);
        };
    };
    std::vector<int64_t> edgesAfterFiltering;
    edgesAfterFiltering.insert(edgesAfterFiltering.end(), rowIndices.begin(), rowIndices.end());
    edgesAfterFiltering.insert(edgesAfterFiltering.end(), colIndices.begin(), colIndices.end());

    int64_t numEdgesAfterF = edgesAfterFiltering.size() / 2;

    // ************
    // GNN
    // ************
    std::vector<const char*> gInputNames{"g_nodes", "g_edges"};
    std::vector<Ort::Value> gInputTensor;
    gInputTensor.push_back(
        std::move(fInputTensor[0])
    );
    std::vector<int64_t> gEdgeShape{2, numEdgesAfterF};
    gInputTensor.push_back(
        Ort::Value::CreateTensor<int64_t>(
            memoryInfo, edgesAfterFiltering.data(), edgesAfterFiltering.size(),
            gEdgeShape.data(), gEdgeShape.size())
    );
    // gnn outputs
    std::vector<const char*> gOutputNames{"gnn_edge_score"};
    std::vector<float> gOutputData(numEdgesAfterF);
    std::vector<int64_t> gOutputShape{numEdgesAfterF};
    std::vector<Ort::Value> gOutputTensor;
    gOutputTensor.push_back(
        Ort::Value::CreateTensor<float>(
            memoryInfo, gOutputData.data(), gOutputData.size(), 
            gOutputShape.data(), gOutputShape.size())
    );
    AthONNX::InferenceWithIOBinding(m_gnnSession, gInputNames, gInputTensor, gOutputNames, gOutputTensor);
    // apply sigmoid to the gnn output data
    for(auto& v : gOutputData){
        v = 1.f / (1.f + std::exp(-v));
    };

    // ************
    // Track Labeling with cugraph::connected_components
    // ************
    std::vector<int32_t> trackLabels(numSpacepoints);
    weaklyConnectedComponents<int64_t,float,int32_t>(numSpacepoints, rowIndices, colIndices, gOutputData, trackLabels);

    if (trackLabels.size() == 0)  return;

    tracks.clear();

    int existTrkIdx = 0;
    // map labeling from MCC to customized track id.
    std::map<int32_t, int32_t> trackLableToIds;

    for(int32_t idx=0; idx < numSpacepoints; ++idx) {
        int32_t trackLabel = trackLabels[idx];
        uint32_t spacepointID = spacepointIDs[idx];

        int trkId;
        if(trackLableToIds.find(trackLabel) != trackLableToIds.end()) {
            trkId = trackLableToIds[trackLabel];
            tracks[trkId].push_back(spacepointID);
        } else {
            // a new track, assign the track id
            // and create a vector
            trkId = existTrkIdx;
            tracks.push_back(std::vector<uint32_t>{spacepointID});
            trackLableToIds[trackLabel] = trkId;
            existTrkIdx++;
        }
    }
}

