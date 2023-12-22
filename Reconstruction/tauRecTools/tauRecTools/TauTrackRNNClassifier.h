/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUTRACKRNNCLASSIFIER_H
#define TAURECTOOLS_TAUTRACKRNNCLASSIFIER_H

// ASG include(s)
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandleArray.h"
#include "AsgDataHandles/ReadHandleKey.h"

// xAOD include(s)
#include "xAODTau/TauJet.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTracking/VertexContainer.h"

// local include(s)
#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/lwtnn/LightweightGraph.h"
#include "tauRecTools/lwtnn/parse_json.h"

#include <memory>

/**
 * @brief Implementation of a TrackClassifier based on an RNN 
 * 
 * @author Max Maerker
 *                                                                              
 */

namespace tauRecTools
{
  
class TrackRNN;

// We currently allow several input types
// The "ValueMap" is for simple rank-1 inputs
typedef std::map<std::string, double> ValueMap;
// The "VectorMap" is for sequence inputs
typedef std::map<std::string, std::vector<double> > VectorMap;

typedef std::map<std::string, ValueMap> NodeMap;
typedef std::map<std::string, VectorMap> SeqNodeMap;

//______________________________________________________________________________
class TauTrackRNNClassifier
  : public TauRecToolBase
{
public:

  ASG_TOOL_CLASS2( TauTrackRNNClassifier, TauRecToolBase, ITauToolBase )

  TauTrackRNNClassifier(const std::string& name="TauTrackRNNClassifier");
  ~TauTrackRNNClassifier();

  // retrieve all track classifier sub tools
  virtual StatusCode initialize() override;
 // pass all tracks in the tau cone to all track classifier sub tools
  virtual StatusCode executeTrackClassifier(xAOD::TauJet& pTau, xAOD::TauTrackContainer& tauTrackContainer) const override;

 private:
  ToolHandleArray<TrackRNN> m_vClassifier {this, "Classifiers", {}};

  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey {this, "Key_vertexInputContainer", "PrimaryVertices", "Vertex container key"};

  bool m_classifyLRT;

}; // class TauTrackRNNClassifier
  
//______________________________________________________________________________
class TrackRNN
  : public TauRecToolBase
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2( TrackRNN,
                   TauRecToolBase,
		   ITauToolBase)
  
  public:
  
  TrackRNN(const std::string& name);
  ~TrackRNN();

  // configure the MVA object and build a general map to store variables
  // for possible MVA inputs. Only Variables defined in the root weights file
  // are passed to the MVA object
  virtual StatusCode initialize() override;
  
  // executes MVA object to get the RNN scores and set classification flags
  StatusCode classifyTracks(std::vector<xAOD::TauTrack*>& vTracks,
			    xAOD::TauJet& xTau,
			    const xAOD::VertexContainer* vertexContainer,
			    bool skipTracks=false) const;
  
private:
  // set RNN input variables in the corresponding map entries
  StatusCode calculateVars(const std::vector<xAOD::TauTrack*>& vTracks,
			   const xAOD::TauJet& xTau,
			   const xAOD::VertexContainer* vertexContainer,
			   VectorMap& valueMap) const;

  // configurable variables
  std::string m_inputWeightsPath; 
  unsigned int m_nMaxNtracks;

  std::unique_ptr<lwtDev::LightweightGraph> m_RNNClassifier; //!

}; // class TrackRNN

} // namespace tauRecTools

#endif // TAURECTOOLS_TAUTRACKRNNCLASSIFIER_H
