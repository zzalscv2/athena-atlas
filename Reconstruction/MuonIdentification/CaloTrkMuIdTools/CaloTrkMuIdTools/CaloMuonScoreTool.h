/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOTRKMUIDTOOLS_CALOMUONSCORETOOL_H
#define CALOTRKMUIDTOOLS_CALOMUONSCORETOOL_H

#include "ICaloTrkMuIdTools/ICaloMuonScoreTool.h"
#include "ICaloTrkMuIdTools/ICaloMuonScoreONNXRuntimeSvc.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "RecoToolInterfaces/IParticleCaloCellAssociationTool.h"

#include <vector>
#include <memory>

/** @class CaloMuonScoreTool

    Fetch the calorimeter cells around a track particle and compute the muon score.
 
    The muon score is computed by doing inference on a 7-colour-channel convolutional neural 
    network. The inputs to the convolutional neural network are the energy deposits in 30 eta 
    and 30 phi bins around the track particle. Seven colour channels are considered, 
    corresponding to the seven calorimeter layers (CaloSamplingIDs) in the low-eta region 
    (eta < 0.1).

    The convolutional neural network was trained using tensorflow. Inference on this model 
    is done using ONNX (the tensorflow model having been converted to ONNX format).

    @author ricardo.wolker@cern.ch
*/
class CaloMuonScoreTool : public AthAlgTool, virtual public ICaloMuonScoreTool {
public:
  CaloMuonScoreTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~CaloMuonScoreTool()=default;

  virtual StatusCode initialize();
  
  // Compute the muon score given a track particle
  float getMuonScore(const xAOD::TrackParticle* trk) const;

  // run the ONNX inference on the input tensor
  float runOnnxInference(std::vector<float> &tensor) const;

  // fill vectors from the particle cell association
  void fillInputVectors(std::unique_ptr<const Rec::ParticleCellAssociation>& association, std::vector<float> &eta, std::vector<float> &phi, std::vector<float> &energy, std::vector<int> &samplingId) const;

  // Compute the median of a vector of floats (can be even or odd in length)
  float getMedian(std::vector<float> v) const;

  // Get a linearly spaced vector of size `nBins`, ranging from `min` to `max` (both values included)
  std::vector<float> getLinearlySpacedBins(float min, float max, int nNins) const;

  // Given a vector of bins, return the index of the matching bin
  int getBin(std::vector<float> &bins, float &val) const;

  // Given a calo sampling ID (as integer), return the corresponding "RGB"-like channel ID (0,1,2,3,4,5,6)
  int channelForSamplingId(int &samplingId) const;

  // for a given particle, consume vectors for eta, phi, energy, sampling ID, and return the input tensor to be used in ONNX
  std::vector<float> getInputTensor(std::vector<float> &eta, std::vector<float> &phi, std::vector<float> &energy, std::vector<int> &sampling) const;

  std::vector<const char*> input_node_names;

  std::vector<const char*> output_node_names;

  std::vector<int64_t> input_node_dims;

private:
  // Number of bins in eta
  int m_etaBins;

  // Number of bins in phi
  int m_phiBins;

  // window in terms of abs(eta) to consider around the median eta value
  float m_etaCut;

  // window in terms of abs(phi) to consider around the median phi value
  float m_phiCut;

  // Number of colour channels to consider in the convolutional neural network
  int m_nChannels;

  // name of the model to use
  std::string m_modelFileName;

  ToolHandle <Trk::IParticleCaloExtensionTool> m_caloExtensionTool{this, "ParticleCaloExtensionTool", ""};
  ToolHandle <Rec::IParticleCaloCellAssociationTool> m_caloCellAssociationTool{this, "ParticleCaloCellAssociationTool", ""}; 

  std::unique_ptr< Ort::Session > m_session;
};

#endif
