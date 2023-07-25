/*
Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "AthViews/ViewHelper.h"
#include "CxxUtils/phihelper.h"
#include "ViewCreatorJetSuperROITool.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"

ViewCreatorJetSuperROITool::ViewCreatorJetSuperROITool( const std::string& type, 
												const std::string& name, 
												const IInterface* parent)
  : base_class(type, name, parent)
{}


StatusCode ViewCreatorJetSuperROITool::initialize()  {
  ATH_CHECK( m_roisWriteHandleKey.initialize() );
  
  return StatusCode::SUCCESS;
}


StatusCode ViewCreatorJetSuperROITool::attachROILinks( TrigCompositeUtils::DecisionContainer& decisions, 
										 const EventContext& ctx ) const {

  // ===================================================================================== //                                                                                                                                       
  // ===================================================================================== // 
  // Create output RoI collection

  SG::WriteHandle<TrigRoiDescriptorCollection> roisWriteHandle =  TrigCompositeUtils::createAndStoreNoAux(m_roisWriteHandleKey, ctx);
  // ===================================================================================== // 

  // Only expect one object in container
  if(decisions.size()!=1) { 
    ATH_MSG_ERROR("Did not find exactly one decision object in decision container containing " << decisions.size() << " decisions");
    return StatusCode::FAILURE;
  }
  TrigCompositeUtils::Decision* outputDecision = decisions[0];

  // Link name should probably be configurable
  const TrigCompositeUtils::LinkInfo< xAOD::JetContainer> leadingjetlink = TrigCompositeUtils::findLink< xAOD::JetContainer >(outputDecision, "LeadingPreselJet", TrigDefs::lastFeatureOfType);
  if(!leadingjetlink.link.isValid()) {
    ATH_MSG_ERROR("Received invalid jet link from decision object! " << leadingjetlink.link);
    return StatusCode::FAILURE;
  }

  const xAOD::JetContainer* jetCont = static_cast<const xAOD::JetContainer*>((*leadingjetlink.link)->container());  
  ATH_MSG_DEBUG("Found " << jetCont->size() << " presel jets linked from decision object.");
  
  // This is needed to merge the RoIs from each jet
  std::unique_ptr<TrigRoiDescriptor> superRoI = std::make_unique<TrigRoiDescriptor>();

  /// not really composite until a constituent has been added
  /// so whether setting here or only when adding a constituent, depends
  /// on how it should be used in the future - if we want it to be treated as 
  /// empty with no constituents, then setComposite should be true even with 
  /// no constituents, otherwise, with no constituantes it will be treated
  /// as a normal fullscan Roi which may not be what is intended  
  superRoI->setComposite(true);
  superRoI->manageConstituents(false);

  for(const xAOD::Jet* jet : *jetCont) {
    if(jet->pt()>m_jetMinPt && std::abs(jet->eta())<m_jetMaxAbsEta) {
      double jetEta{jet->eta()}, jetPhi{jet->phi()};

      ATH_MSG_DEBUG("JET  -- pt=" << jet->pt() <<
                  " eta=" << jet->eta() <<
                  " phi=" << jet->phi() );

      // create ROIs
      ATH_MSG_DEBUG("Adding RoI to RoI container");
      ATH_MSG_DEBUG( "  ** Imposing Z constraint while building RoI" );
      double etaMinus = jetEta - m_roiEtaWidth;
      double etaPlus  = jetEta + m_roiEtaWidth;
      
      double phiMinus = CxxUtils::wrapToPi( jetPhi - m_roiPhiWidth );
      double phiPlus  = CxxUtils::wrapToPi( jetPhi + m_roiPhiWidth );

      // Should retrieve beamspot offset from somewhere
      double zMinus = -1. * m_roiZWidth;
      double zPlus  = m_roiZWidth;

      /// don't mess about with unique_ptr here as the pointer management is 
      /// done by the Roi itself
      superRoI->push_back( new TrigRoiDescriptor( jetEta, etaMinus, etaPlus,
						  jetPhi, phiMinus, phiPlus,
						  0.,zMinus,zPlus ) );

      /// only set this to true here, just in case. It will still be false, 
      /// if there are no constituents, but it doesn;t really matter one way 
      /// or another in that case 
      superRoI->manageConstituents(true);
    }
  }

  roisWriteHandle->push_back(superRoI.release()); /// why is the superRoi a unique pointer if it is just going to be released ? 
  const ElementLink< TrigRoiDescriptorCollection > roiEL = ElementLink< TrigRoiDescriptorCollection >( *roisWriteHandle, 0, ctx );
  outputDecision->setObjectLink( TrigCompositeUtils::roiString(), roiEL );

  ATH_MSG_DEBUG("PRINTING DECISION");
  ATH_MSG_DEBUG( *outputDecision );

  return StatusCode::SUCCESS;
}
