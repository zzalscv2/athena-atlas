/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAURECTOOLBASE_H
#define TAURECTOOLS_TAURECTOOLBASE_H
/**
 * @brief The base class for all tau tools.
 * 
 * @author Lukasz Janyst
 * @author Justin Griffiths
 * Thanks to Lianyou Shan, Lorenz Hauswald
 */

#include <string>

#include "tauRecTools/ITauToolBase.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

class TauRecToolBase : public asg::AsgTool, virtual public ITauToolBase {
 public:

  ASG_TOOL_INTERFACE(TauRecToolBase)
  ASG_TOOL_CLASS1( TauRecToolBase, ITauToolBase )

  TauRecToolBase(const std::string& name);
  virtual ~TauRecToolBase() {}

  //-----------------------------------------------------------------
  //! Tool initializer
  //-----------------------------------------------------------------
  virtual StatusCode initialize() override;

  //-----------------------------------------------------------------
  //! Event initializer - called at the beginning of each event
  //-----------------------------------------------------------------
  virtual StatusCode eventInitialize() override;

  //-----------------------------------------------------------------
  //! Execute - called for each tau candidate
  //-----------------------------------------------------------------
  virtual StatusCode execute(xAOD::TauJet& pTau) const override;
#ifdef XAOD_ANALYSIS
  virtual StatusCode executeDev(xAOD::TauJet& pTau) override;
#else
  virtual StatusCode executePi0CreateROI(xAOD::TauJet& pTau, CaloConstCellContainer& caloCellContainer, boost::dynamic_bitset<>& map) const override;
#endif
  virtual StatusCode executeVertexFinder(xAOD::TauJet& pTau, 
                                         const xAOD::VertexContainer* vertexContainer = nullptr, 
                                         const xAOD::TrackParticleContainer* trackContainer = nullptr) const override;
  virtual StatusCode executeTrackFinder(xAOD::TauJet& pTau, xAOD::TauTrackContainer& tauTrackContainer) const override;
  virtual StatusCode executeTrackClassifier(xAOD::TauJet& pTau, xAOD::TauTrackContainer& tauTrackContainer) const override;
  virtual StatusCode executeShotFinder(xAOD::TauJet& pTau, xAOD::CaloClusterContainer& shotClusterContainer, xAOD::PFOContainer& PFOContainer ) const override;
  virtual StatusCode executePi0ClusterCreator(xAOD::TauJet& pTau, xAOD::PFOContainer& neutralPFOContainer, 
					      xAOD::PFOContainer& hadronicPFOContainer, 
					      const xAOD::CaloClusterContainer& pCaloClusterContainer ) const override;
  virtual StatusCode executeVertexVariables(xAOD::TauJet& pTau, xAOD::VertexContainer& vertexContainer ) const override;  
  virtual StatusCode executePi0ClusterScaler(xAOD::TauJet& pTau, xAOD::PFOContainer& neutralPFOContainer, xAOD::PFOContainer& chargedPFOContainer ) const override;  
  virtual StatusCode executePi0nPFO(xAOD::TauJet& pTau, xAOD::PFOContainer& neutralPFOContainer) const override;
  virtual StatusCode executePanTau(xAOD::TauJet& pTau, xAOD::ParticleContainer& particleContainer, xAOD::PFOContainer& neutralPFOContainer) const override;

  //-----------------------------------------------------------------
  //! Event finalizer - called at the end of each event
  //-----------------------------------------------------------------
  virtual StatusCode eventFinalize() override;

  //-----------------------------------------------------------------
  //! Finalizer
  //-----------------------------------------------------------------
  virtual StatusCode finalize() override;

  std::string find_file(const std::string& fname) const;
  virtual StatusCode readConfig() override;

 protected:
  Gaudi::Property<bool>        m_in_trigger     {this, "inTrigger",   false,                     "Indicate if the tool is running on trigger"};
  Gaudi::Property<bool>        m_in_AOD         {this, "inAOD",       false,                     "Indicate if the tool is running on AOD"};
  Gaudi::Property<bool>        m_in_EleRM       {this, "inEleRM",       false,                     "Indicate if the tool is running on EleRM routine"};
  Gaudi::Property<std::string> m_tauRecToolsTag {this, "calibFolder", "tauRecTools/R22_preprod", "CVMFS path to the tau calibration folder"};

  bool inTrigger() const;
  bool inAOD() const;
  bool inEleRM() const;

};

inline bool TauRecToolBase::inTrigger() const { return m_in_trigger; }
inline bool TauRecToolBase::inAOD() const { return m_in_AOD; }
inline bool TauRecToolBase::inEleRM() const { return m_in_EleRM; }

#endif // TAURECTOOLS_TAURECTOOLBASE_H
