/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TauProcessorAlg.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODPFlow/PFOAuxContainer.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "NavFourMom/INavigable4MomentumCollection.h"
#include <boost/dynamic_bitset.hpp>

using Gaudi::Units::GeV;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
TauProcessorAlg::TauProcessorAlg(const std::string &name,
				 ISvcLocator * pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
TauProcessorAlg::~TauProcessorAlg() = default;

//-----------------------------------------------------------------------------
// Initializer
//-----------------------------------------------------------------------------
StatusCode TauProcessorAlg::initialize() {
    
  ATH_CHECK( m_jetInputContainer.initialize() );
  ATH_CHECK( m_tauOutputContainer.initialize() );
  ATH_CHECK( m_tauTrackOutputContainer.initialize() );
  ATH_CHECK( m_tauShotClusOutputContainer.initialize() );
  ATH_CHECK( m_tauShotClusLinkContainer.initialize() );
  ATH_CHECK( m_tauShotPFOOutputContainer.initialize() );
  ATH_CHECK( m_tauPi0CellOutputContainer.initialize(SG::AllowEmpty) );

  ATH_CHECK(m_pixelDetEleCollKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_SCTDetEleCollKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_trtDetEleContKey.initialize(SG::AllowEmpty));

  if(!m_tauPi0CellOutputContainer.empty()) {
    ATH_CHECK( detStore()->retrieve(m_cellID) );
    ATH_CHECK( m_cellMakerTool.retrieve() );
  }

  //-------------------------------------------------------------------------
  // No tools allocated!
  //-------------------------------------------------------------------------
  if (m_tools.empty()) {
    ATH_MSG_ERROR("no tools given!");
    return StatusCode::FAILURE;
  }

  //-------------------------------------------------------------------------
  // Allocate tools
  //-------------------------------------------------------------------------    
  ATH_MSG_INFO("List of tools in execution sequence:");
  ATH_MSG_INFO("------------------------------------");

  for (const ToolHandle<ITauToolBase>& tool : m_tools) {
    ATH_CHECK( tool.retrieve() );
    ATH_MSG_INFO(tool->type() << " - " << tool->name());
  }

  ATH_MSG_INFO(" ");
  ATH_MSG_INFO("------------------------------------");

  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------
StatusCode TauProcessorAlg::execute(const EventContext& ctx) const {

  /// record output containers
  SG::WriteHandle<xAOD::TauJetContainer> tauHandle( m_tauOutputContainer, ctx );
  ATH_CHECK(tauHandle.record(std::make_unique<xAOD::TauJetContainer>(), std::make_unique<xAOD::TauJetAuxContainer>()));
  xAOD::TauJetContainer* pContainer = tauHandle.ptr();

  SG::WriteHandle<xAOD::TauTrackContainer> tauTrackHandle( m_tauTrackOutputContainer, ctx );
  ATH_CHECK(tauTrackHandle.record(std::make_unique<xAOD::TauTrackContainer>(), std::make_unique<xAOD::TauTrackAuxContainer>()));
  xAOD::TauTrackContainer* pTauTrackCont = tauTrackHandle.ptr();

  SG::WriteHandle<xAOD::CaloClusterContainer> tauShotClusHandle( m_tauShotClusOutputContainer, ctx );
  ATH_CHECK(tauShotClusHandle.record(std::make_unique<xAOD::CaloClusterContainer>(), std::make_unique<xAOD::CaloClusterAuxContainer>()));
  xAOD::CaloClusterContainer* tauShotClusContainer = tauShotClusHandle.ptr();

  SG::WriteHandle<xAOD::PFOContainer> tauShotPFOHandle( m_tauShotPFOOutputContainer, ctx );
  ATH_CHECK(tauShotPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
  xAOD::PFOContainer* tauShotPFOContainer = tauShotPFOHandle.ptr();

  CaloConstCellContainer* Pi0CellContainer = nullptr;
  boost::dynamic_bitset<> addedCellsMap;

  if(!m_tauPi0CellOutputContainer.empty()) {
    SG::WriteHandle<CaloConstCellContainer> tauPi0CellHandle( m_tauPi0CellOutputContainer, ctx );
    ATH_CHECK(tauPi0CellHandle.record(std::make_unique<CaloConstCellContainer>(SG::VIEW_ELEMENTS )));
    Pi0CellContainer = tauPi0CellHandle.ptr();

    // Initialize the cell map per event, used to avoid dumplicate cell in TauPi0CreateROI
    IdentifierHash hashMax = m_cellID->calo_cell_hash_max();
    ATH_MSG_DEBUG("CaloCell Hash Max: " << hashMax);
    addedCellsMap.resize(hashMax,false);
  }

  // retrieve the input jet seed container
  SG::ReadHandle<xAOD::JetContainer> jetHandle( m_jetInputContainer, ctx );
  if (!jetHandle.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << jetHandle.key());
    return StatusCode::FAILURE;
  }
  const xAOD::JetContainer *pSeedContainer = jetHandle.cptr();
    
  //---------------------------------------------------------------------                                                        
  // Loop over seeds
  //---------------------------------------------------------------------                                                 
  ATH_MSG_VERBOSE("Number of seeds in the container: " << pSeedContainer->size());
    
  for (const xAOD::Jet* pSeed : *pSeedContainer) {
    ATH_MSG_VERBOSE("Seeds eta:" << pSeed->eta() << ", pt:" << pSeed->pt());

    if (std::abs(pSeed->eta()) > m_maxEta) {
      ATH_MSG_VERBOSE("--> Seed rejected, eta out of range!");
      continue;
    }

    if (pSeed->pt() < m_minPt) {
      ATH_MSG_VERBOSE("--> Seed rejected, pt out of range!");
      continue;
    }

    //-----------------------------------------------------------------                                                                 
    // Seed passed cuts --> create tau candidate
    //-----------------------------------------------------------------                                                                           
    xAOD::TauJet* pTau = new xAOD::TauJet();
    pContainer->push_back( pTau );
    pTau->setJet(pSeedContainer, pSeed);
      
    //-----------------------------------------------------------------
    // Loop stops when Failure indicated by one of the tools
    //-----------------------------------------------------------------
    StatusCode sc;
    for (const ToolHandle<ITauToolBase>& tool : m_tools) {
      ATH_MSG_DEBUG("ProcessorAlg Invoking tool " << tool->name());

      if (tool->type() == "TauVertexFinder") {
        sc = tool->executeVertexFinder(*pTau);
      } else if (tool->type() == "TauTrackFinder") {
        sc = tool->executeTrackFinder(*pTau, *pTauTrackCont);
      } else if (tool->type() == "tauRecTools::TauTrackRNNClassifier") {
        sc = tool->executeTrackClassifier(*pTau, *pTauTrackCont);

        // skip candidate if it has too many classifiedCharged tracks, if
        // skimming is required
        if (m_maxNTracks > 0 &&
            static_cast<int>(pTau->nTracks()) > m_maxNTracks) {
          sc = StatusCode::FAILURE;
          break;
        }
      } else if (tool->type() == "TauShotFinder") {
        sc = tool->executeShotFinder(*pTau, *tauShotClusContainer,
                                     *tauShotPFOContainer);
      } else if (tool->type() == "TauPi0CreateROI") {
        sc = tool->executePi0CreateROI(*pTau, *Pi0CellContainer, addedCellsMap);
      } else {
        sc = tool->execute(*pTau);
      }
      if (sc.isFailure())
        break;
    }

    if (sc.isSuccess()) {
      ATH_MSG_VERBOSE("The tau candidate has been registered");
    } 
    else {
      //remove orphaned tracks before tau is deleted via pop_back
      xAOD::TauJet* bad_tau = pContainer->back();
      ATH_MSG_DEBUG("Deleting " << bad_tau->nAllTracks() << "Tracks associated with tau: ");
      pTauTrackCont->erase(pTauTrackCont->end()-bad_tau->nAllTracks(), pTauTrackCont->end());

      pContainer->pop_back();
    } 
  }// loop through seeds

  // build cell link container for shot clusters
  SG::WriteHandle<CaloClusterCellLinkContainer> tauShotClusLinkHandle( m_tauShotClusLinkContainer, ctx );
  ATH_CHECK(CaloClusterStoreHelper::finalizeClusters (tauShotClusLinkHandle, tauShotClusContainer));

  if(Pi0CellContainer) {
    // sort the cell container by hash 
    // basically call the finalizer
    ATH_CHECK( m_cellMakerTool->process(Pi0CellContainer, ctx) );
    //Since this is recorded we can retrieve it later as const DV
    //do it here due to symlink below
    const CaloCellContainer* cellPtrs = Pi0CellContainer->asDataVector(); 
    // Check this is needed for the cell container?
    // symlink as INavigable4MomentumCollection (as in CaloRec/CaloCellMaker)
    ATH_CHECK(evtStore()->symLink(cellPtrs, static_cast<INavigable4MomentumCollection*> (nullptr)));
 }

  ATH_MSG_VERBOSE("The tau candidate container has been modified");
  
  return StatusCode::SUCCESS;
}

