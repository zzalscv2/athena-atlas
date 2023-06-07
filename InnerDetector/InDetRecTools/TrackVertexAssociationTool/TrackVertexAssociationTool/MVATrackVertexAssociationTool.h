/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MVATRACKVERTEXASSOCIATIONTOOL_H
#define MVATRACKVERTEXASSOCIATIONTOOL_H

// Includes from this package
#include "TrackVertexAssociationTool/ITrackVertexAssociationTool.h"
#include "TrackVertexAssociationTool/MVAInputEvaluator.h"

// FrameWork includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AthLinks/ElementLink.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODTracking/TrackParticleContainerFwd.h"
#include "xAODTracking/VertexFwd.h"
#include "xAODTracking/VertexContainerFwd.h"

// lwtnn includes
#include "lwtnn/LightweightNeuralNetwork.hh"
#include "lwtnn/LightweightGraph.hh"

// STL includes
#include <map>
#include <memory>

namespace CP {

class MVATrackVertexAssociationTool : public asg::AsgTool, virtual public ITrackVertexAssociationTool
{
  ASG_TOOL_CLASS(MVATrackVertexAssociationTool, ITrackVertexAssociationTool)

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////
public:

  MVATrackVertexAssociationTool(const std::string& name);

  virtual StatusCode initialize() override;

  virtual bool isCompatible(const xAOD::TrackParticle& trk, const xAOD::Vertex& vx) const override;
  virtual bool isCompatible(const xAOD::TrackParticle& trk) const override;

  virtual xAOD::TrackVertexAssociationMap getMatchMap(std::vector<const xAOD::TrackParticle*>& trk_list, std::vector<const xAOD::Vertex*>& vx_list) const override;
  virtual xAOD::TrackVertexAssociationMap getMatchMap(const xAOD::TrackParticleContainer& trk_cont, const xAOD::VertexContainer& vx_cont) const override;

  virtual ElementLink<xAOD::VertexContainer> getUniqueMatchVertexLink(const xAOD::TrackParticle& trk, const xAOD::VertexContainer& vx_cont) const override;
  virtual const xAOD::Vertex* getUniqueMatchVertex(const xAOD::TrackParticle& trk, std::vector<const xAOD::Vertex*>& vx_list) const override;

  virtual xAOD::TrackVertexAssociationMap getUniqueMatchMap(std::vector<const xAOD::TrackParticle*>& trk_list, std::vector<const xAOD::Vertex*>& vx_list) const override;
  virtual xAOD::TrackVertexAssociationMap getUniqueMatchMap(const xAOD::TrackParticleContainer& trk_cont, const xAOD::VertexContainer& vx_cont) const override;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
private:

  bool isMatch(const xAOD::TrackParticle& trk, const xAOD::Vertex& vx, float& mvaOutput, const xAOD::EventInfo* evtInfo = nullptr) const;

  template <typename T, typename V>
  xAOD::TrackVertexAssociationMap getMatchMapInternal(const T& trk_list, const V& vx_list) const;

  template <typename T, typename V>
  xAOD::TrackVertexAssociationMap getUniqueMatchMapInternal(const T& trk_list, const V& vx_list) const;

  template <typename T>
  const xAOD::Vertex* getUniqueMatchVertexInternal(const xAOD::TrackParticle& trk, const T& vx_list) const;

  StatusCode initializeNetwork();
  float evaluateNetwork(const xAOD::TrackParticle& trk, const xAOD::Vertex& vx, const xAOD::EventInfo& evt) const;


  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
private:
  /// @name The properties that can be defined via the python job options
  /// @{

  /// Input lwtnn network file
  Gaudi::Property<std::string> m_fileName{this, "NetworkFileName", "" };

  /// Vector of input variable names
  Gaudi::Property<std::vector<std::string>> m_inputNames{this, "InputNames", {},
                  "Vector of the network's input variable names (std::vector<std::string>)." };

  /// Vector of input variable types
  Gaudi::Property<std::vector<int>> m_inputTypes{this,"InputTypes", {},
                  "Vector of the network's input variable evaluator types (std::vector<CP::MVAEvaluatorInput::Input>)."};

  /// Name of the output node to cut on
  Gaudi::Property<std::string> m_outputName{this, "OutputNodeName", "", 
                               "Name of the output node to cut on for TVA."} ;

  /// Is the network sequential or functional
  Gaudi::Property<bool> m_isSequential{this, "IsSequential", true,
                "Is the network sequential (true) or functional (false)."};

  /// TVA working point
  Gaudi::Property<std::string> m_wp{this, "WorkingPoint", "Tight", "TVA working point to apply."};

  /// TVA cut value on the output discriminant
  Gaudi::Property<float> m_cut{this, "OutputCut" , -1.0, 
                        "TVA cut value on the output value (set manually with \"Custom\" WP)."};

  /// Use the PathResolver to find our input file
  Gaudi::Property<bool> m_usePathResolver{this, "UsePathResolver", true, 
                        "Use the PathResolver for finding the input lwtnn network file."};

  /// The decoration name of the ElementLink to the hardscatter vertex (found on xAOD::EventInfo)
  Gaudi::Property<std::string> m_hardScatterDeco{this, "HardScatterLinkDeco",  "hardScatterVertexLink",
                              "The decoration name of the ElementLink to the hardscatter vertex (found on xAOD::EventInfo)"};

  /// @}

private:

  /// @name Truly private internal data members
  /// @{

  /// Input variable name/type map
  MVAInputEvaluator::InputSelectionMap m_inputMap;

  /// Input variable evaluator
  MVAInputEvaluator m_inputEval;

  /// EventInfo key
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo {this, "EventInfo", "EventInfo", "EventInfo key"};

  /// Hardscatter vertex link key
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_hardScatterDecoKey{this, "VertexScatterKey", "" , 
                              "Overwrriten in the configuration step by the <HardScatterLinkDeco> property"};

  /// Name of the input node (for functional modes)
  std::string m_inputNodeName = ""; //!

  /// Network as implemented using lwtnn
  std::unique_ptr<lwt::LightweightNeuralNetwork> m_network; //!
  std::unique_ptr<lwt::LightweightGraph>         m_graph;   //!

  /// @}

};

} // namespace CP

#endif // MVATRACKVERTEXASSOCIATIONTOOL_H
