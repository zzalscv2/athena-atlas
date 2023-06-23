/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Source file for the JetConstituentModSequence.h
// Michael Nelson, CERN & University of Oxford 
// Will later add the intermediate step

#include "JetRecTools/JetConstituentModSequence.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODPFlow/PFO.h"
#include "xAODPFlow/PFOContainer.h"
#include "xAODPFlow/PFOAuxContainer.h"
#include "xAODPFlow/TrackCaloCluster.h"
#include "xAODPFlow/TrackCaloClusterContainer.h"
#include "xAODPFlow/TrackCaloClusterAuxContainer.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODPFlow/FlowElementAuxContainer.h"

#include "AsgDataHandles/WriteHandle.h"

#ifndef XAOD_ANALYSIS
#include "AthenaMonitoringKernel/Monitored.h"
#endif

JetConstituentModSequence::JetConstituentModSequence(const std::string &name):
  asg::AsgTool(name) {

#ifdef ASG_TOOL_ATHENA
  declareInterface<IJetConstituentModifier>(this);
#endif
  declareProperty("InputType", m_inputType, "The xAOD type name for the input container.");
  declareProperty("SaveAsShallow", m_saveAsShallow=true, "Save as shallow copy");

}

StatusCode JetConstituentModSequence::initialize() {
  ATH_MSG_INFO("Initializing tool " << name() << "...");
  ATH_MSG_DEBUG("initializing version with data handles");


  ATH_CHECK( m_modifiers.retrieve() );

#ifndef XAOD_ANALYSIS
  ATH_CHECK( m_monTool.retrieve( DisableTool{m_monTool.empty()} ) );
#endif
  
  // Set and initialise DataHandleKeys only for the correct input type
  // Die if the input type is unsupported
  switch(m_inputType) {
  case xAOD::Type::CaloCluster:
    {
      m_inClusterKey = m_inputContainer;
      m_outClusterKey = m_outputContainer;
  
      ATH_CHECK(m_inClusterKey.initialize());
      ATH_CHECK(m_outClusterKey.initialize());
      break;
    }
  case xAOD::Type::ParticleFlow:
    {
      std::string inputContainerBase = m_inputContainer;
      std::string outputContainerBase = m_outputContainer;

      // Know what the user means if they give the full input/output container name in this format
      size_t pos = inputContainerBase.find("ParticleFlowObjects");
      if(pos != std::string::npos) inputContainerBase.erase(pos);

      pos = outputContainerBase.find("ParticleFlowObjects");
      if(pos != std::string::npos) outputContainerBase.erase(pos);

      m_inChargedPFOKey = inputContainerBase + "ChargedParticleFlowObjects";
      m_inNeutralPFOKey = inputContainerBase + "NeutralParticleFlowObjects";

      m_outChargedPFOKey = outputContainerBase + "ChargedParticleFlowObjects";
      m_outNeutralPFOKey = outputContainerBase + "NeutralParticleFlowObjects";
      m_outAllPFOKey = outputContainerBase + "ParticleFlowObjects";

      ATH_CHECK(m_inChargedPFOKey.initialize());
      ATH_CHECK(m_inNeutralPFOKey.initialize());
      ATH_CHECK(m_outChargedPFOKey.initialize());
      ATH_CHECK(m_outNeutralPFOKey.initialize());
      ATH_CHECK(m_outAllPFOKey.initialize());
      break;
    }
    break;
  case xAOD::Type::TrackCaloCluster:
    m_inTCCKey = m_inputContainer;
    m_outTCCKey = m_outputContainer;

    ATH_CHECK(m_inTCCKey.initialize());
    ATH_CHECK(m_outTCCKey.initialize());
    break;
  case xAOD::Type::FlowElement:
    {
      // TODO: This assumes a PFlow-style neutral and charged collection.
      //       More general FlowElements (e.g. CaloClusters) may necessitate a rework here later.

      const std::string subString = "ParticleFlowObjects";
      const std::string subStringCharged = "ChargedParticleFlowObjects";
      const std::string subStringNeutral = "NeutralParticleFlowObjects";

      std::string inputContainerBase = m_inputContainer;
      std::string outputContainerBase = m_outputContainer;

      m_inChargedFEKey = inputContainerBase;
      m_inNeutralFEKey = inputContainerBase;

      m_outChargedFEKey = outputContainerBase;
      m_outNeutralFEKey = outputContainerBase;
      
      //For both the input and output container basenames we first check if the string
      //contains "ParticleFlowObjects". If it does we swap this for "ChargedParticleFlowObjects"
      //and "NeutralParticleFlowObjects" respectively. If it doesn't we just append these two
      //substrings to the end of the strings.

      size_t pos = inputContainerBase.find(subString);
      if(pos != std::string::npos) {
        std::string inChargedString = m_inChargedFEKey.key();
        m_inChargedFEKey = inChargedString.replace(pos,subString.size(),subStringCharged);
        std::string inNeutralString = m_inNeutralFEKey.key();
        m_inNeutralFEKey = inNeutralString.replace(pos,subString.size(),subStringNeutral);
      }
      else {
        m_inChargedFEKey = inputContainerBase + subStringCharged;
        m_inNeutralFEKey = inputContainerBase + subStringNeutral;
      }

      pos = outputContainerBase.find(subString);
      if(pos != std::string::npos) {
        std::string outChargedString = m_outChargedFEKey.key();
        m_outChargedFEKey = outChargedString.replace(pos,subString.size(),subStringCharged);
        std::string outNeutralString = m_outNeutralFEKey.key();
        m_outNeutralFEKey = outNeutralString.replace(pos,subString.size(),subStringNeutral);
      }
      else{
        m_outChargedFEKey = outputContainerBase + subStringCharged;
        m_outNeutralFEKey = outputContainerBase + subStringNeutral;
      }

      //The all FE container is a bit different. If the input container base contains
      //"ParticleFlowObjects" we do nothing, otherwise we add this string onto the end.
      pos = outputContainerBase.find(subString);  
      if(pos == std::string::npos) m_outAllFEKey = outputContainerBase + subString;
      else m_outAllFEKey = outputContainerBase;


      ATH_CHECK(m_inChargedFEKey.initialize());
      ATH_CHECK(m_inNeutralFEKey.initialize());
      ATH_CHECK(m_outChargedFEKey.initialize());
      ATH_CHECK(m_outNeutralFEKey.initialize());
      ATH_CHECK(m_outAllFEKey.initialize());
      break;
    }
  default:
    ATH_MSG_ERROR(" Unsupported input type "<< m_inputType );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
  
int JetConstituentModSequence::execute() const {

#ifndef XAOD_ANALYSIS
  // Define monitored quantities
  auto t_exec     = Monitored::Timer<std::chrono::milliseconds>( "TIME_constitmod"  );
#endif

  // Create the shallow copy according to the input type
  switch(m_inputType){

    case xAOD::Type::CaloCluster: {
      auto sc  = copyModRecord(m_inClusterKey,
                               m_outClusterKey);
      if(!sc.isSuccess()) return 1;
      break;
    }

    case xAOD::Type::ParticleFlow: {
      auto sc = copyModRecordPFO();
      if(!sc.isSuccess()) return 1;
      break;
    }

    case xAOD::Type::FlowElement: {
      auto sc = copyModRecordFE();
      if(!sc.isSuccess()) return 1;
      break;
    }

    case xAOD::Type::TrackCaloCluster : {
      auto sc  = copyModRecord(m_inTCCKey, 
                                m_outTCCKey);
      if(!sc.isSuccess()){return 1;}
      break;
    }

    default: {
      ATH_MSG_WARNING( "Unsupported input type " << m_inputType );
      return 1;
    }
    
  }

#ifndef XAOD_ANALYSIS
  auto mon = Monitored::Group(m_monTool, t_exec);
#endif
  return 0;
}

StatusCode
JetConstituentModSequence::copyModRecordPFO() const {

  // Cannot be handled the same way as other objects (e.g. clusters),
  // because the data is split between two containers, but we need
  // information from both to do the modifications.
  //
  // The logic is:
  //   1. Copy the charged & neutral containers independently
  //   2. Merge into a combined view container
  //   3. Modify the combined container

  // 1. Retrieve the input containers
  auto inNeutralPFOHandle = makeHandle(m_inNeutralPFOKey);
  auto inChargedPFOHandle = makeHandle(m_inChargedPFOKey);
  if(!inNeutralPFOHandle.isValid()){
    ATH_MSG_WARNING("Unable to retrieve input PFO containers \""
                    << m_inNeutralPFOKey.key() << "\" and \""
                    << m_inChargedPFOKey.key() << "\"");
    return StatusCode::FAILURE;
  }

  //    Copy the input containers individually, set I/O option and record
  //    Neutral PFOs
  std::pair<xAOD::PFOContainer*, xAOD::ShallowAuxContainer* > neutralCopy =
    xAOD::shallowCopyContainer(*inNeutralPFOHandle);
  neutralCopy.second->setShallowIO(m_saveAsShallow);
  //
  auto outNeutralPFOHandle = makeHandle(m_outNeutralPFOKey);
  ATH_CHECK(outNeutralPFOHandle.record(std::unique_ptr<xAOD::PFOContainer>(neutralCopy.first),
                                       std::unique_ptr<xAOD::ShallowAuxContainer>(neutralCopy.second)));
  //    Charged PFOs
  std::pair<xAOD::PFOContainer*, xAOD::ShallowAuxContainer* > chargedCopy =
    xAOD::shallowCopyContainer(*inChargedPFOHandle);
  chargedCopy.second->setShallowIO(m_saveAsShallow);
  //
  auto outChargedPFOHandle = makeHandle(m_outChargedPFOKey);
  ATH_CHECK(outChargedPFOHandle.record(std::unique_ptr<xAOD::PFOContainer>(chargedCopy.first),
                                       std::unique_ptr<xAOD::ShallowAuxContainer>(chargedCopy.second)));

  // 2. Set up output handle for merged (view) container and record
  auto outAllPFOHandle = makeHandle(m_outAllPFOKey);
  ATH_CHECK(outAllPFOHandle.record(std::make_unique<xAOD::PFOContainer>(SG::VIEW_ELEMENTS)));
  //    Merge charged & neutral PFOs into the viw container
  (*outAllPFOHandle).assign((*outNeutralPFOHandle).begin(), (*outNeutralPFOHandle).end());
  (*outAllPFOHandle).insert((*outAllPFOHandle).end(),
			    (*outChargedPFOHandle).begin(), 
			    (*outChargedPFOHandle).end());

  // 3. Now process modifications on all PFOs
  for (auto t : m_modifiers) {ATH_CHECK(t->process( &*outAllPFOHandle));}

  return StatusCode::SUCCESS;
}

StatusCode JetConstituentModSequence::copyModRecordFE() const {

  // Cannot be handled the same way as other objects (e.g. clusters),
  // because the data is split between two containers, but we need
  // information from both to do the modifications.
  //
  // The logic is:
  //   1. Copy the charged & neutral containers independently
  //   2. Merge into a combined view container
  //   3. Modify the combined container

  // 1. Retrieve the input containers
  SG::ReadHandle<xAOD::FlowElementContainer> inNeutralFEHandle = makeHandle(m_inNeutralFEKey);
  SG::ReadHandle<xAOD::FlowElementContainer> inChargedFEHandle = makeHandle(m_inChargedFEKey);
  if(!inNeutralFEHandle.isValid()){
    ATH_MSG_WARNING("Unable to retrieve input FlowElement containers \""
                    << m_inNeutralFEKey.key() << "\" and \""
                    << m_inChargedFEKey.key() << "\"");
    return StatusCode::FAILURE;
  }

  //    Copy the input containers individually, set I/O option and record
  //    Neutral FEs
  std::pair<xAOD::FlowElementContainer*, xAOD::ShallowAuxContainer* > neutralCopy = xAOD::shallowCopyContainer(*inNeutralFEHandle);
  neutralCopy.second->setShallowIO(m_saveAsShallow);
  xAOD::setOriginalObjectLink(*inNeutralFEHandle, *neutralCopy.first);

  
  SG::WriteHandle<xAOD::FlowElementContainer> outNeutralFEHandle = makeHandle(m_outNeutralFEKey);
  ATH_CHECK(outNeutralFEHandle.record(std::unique_ptr<xAOD::FlowElementContainer>(neutralCopy.first),
                                      std::unique_ptr<xAOD::ShallowAuxContainer>(neutralCopy.second)));
  //    Charged FEs
  std::pair<xAOD::FlowElementContainer*, xAOD::ShallowAuxContainer* > chargedCopy = xAOD::shallowCopyContainer(*inChargedFEHandle);
  chargedCopy.second->setShallowIO(m_saveAsShallow);
  xAOD::setOriginalObjectLink(*inChargedFEHandle, *chargedCopy.first);

  
  SG::WriteHandle<xAOD::FlowElementContainer> outChargedFEHandle = makeHandle(m_outChargedFEKey);
  ATH_CHECK(outChargedFEHandle.record(std::unique_ptr<xAOD::FlowElementContainer>(chargedCopy.first),
                                      std::unique_ptr<xAOD::ShallowAuxContainer>(chargedCopy.second)));

  // 2. Set up output handle for merged (view) container and record
  SG::WriteHandle<xAOD::FlowElementContainer> outAllFEHandle = makeHandle(m_outAllFEKey);
  ATH_CHECK(outAllFEHandle.record(std::make_unique<xAOD::FlowElementContainer>(SG::VIEW_ELEMENTS)));
  //    Merge charged & neutral FEs into the view container
  (*outAllFEHandle).assign((*outNeutralFEHandle).begin(), (*outNeutralFEHandle).end());
  (*outAllFEHandle).insert((*outAllFEHandle).end(),
			   (*outChargedFEHandle).begin(), 
			   (*outChargedFEHandle).end());

  // 3. Now process modifications on all FEs
  for (auto t : m_modifiers) {ATH_CHECK(t->process(&*outAllFEHandle));}

  return StatusCode::SUCCESS;
}
