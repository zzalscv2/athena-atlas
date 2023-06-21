/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LArClusterCellMonAlg.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloIdentifier/CaloGain.h"
#include "Identifier/Identifier.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include <algorithm>



////////////////////////////////////////////
StatusCode LArClusterCellMonAlg::initialize() {

  ATH_MSG_DEBUG("LArClusterCellMonAlg::initialize() start");

  // Initialize superclass
  ATH_CHECK( CaloMonAlgBase::initialize() );

  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID")); 
  ATH_CHECK( m_caloMgrKey.initialize() );
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_clusterContainerKey.initialize() );

  //JobO consistency check:
  if (m_useTrigger && std::all_of(m_triggerNames.begin(),m_triggerNames.end(),[](const std::string& trigName){return trigName.empty();})) {
      ATH_MSG_WARNING("UseTrigger set to true but no trigger names given! Forcing useTrigger to false");
      m_useTrigger=false;
  }

  ATH_MSG_DEBUG("LArClusterCellMonAlg::initialize() is done!");

  return StatusCode::SUCCESS;
}


void LArClusterCellMonAlg::checkTrigger() const {

  auto mon_trig = Monitored::Scalar<float>("trigType",-1);
  mon_trig=0.5;
  fill(m_MonGroupName,mon_trig);

  const ToolHandle<Trig::TrigDecisionTool>& trigTool = getTrigDecisionTool();
  if (m_useTrigger && !trigTool.empty()) {
    for (unsigned i=0;i<m_triggerNames.size();++i) {
      const std::string& chainName=m_triggerNames[i];
      if(!chainName.empty()) {
        const Trig::ChainGroup* cg = trigTool->getChainGroup(chainName);
        if(cg->isPassed()) {
          mon_trig=0.5+i;
          fill(m_MonGroupName,mon_trig);
        }
      }
    }//end of loop over trigger types

  } //end if trigger used
  else {
    mon_trig=6.5;
    fill(m_MonGroupName,mon_trig);
  } 
}




////////////////////////////////////////////////////////////////////////////
StatusCode LArClusterCellMonAlg::fillHistograms(const EventContext& ctx) const{  

  ATH_MSG_DEBUG("LArClusterCellMonAlg::fillHistograms() starts");

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
  ATH_CHECK(caloMgrHandle.isValid());
  const CaloDetDescrManager* caloDDMgr = *caloMgrHandle;

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping* cabling{*cablingHdl};

  SG::ReadHandle<xAOD::CaloClusterContainer> clusterHdl{m_clusterContainerKey, ctx};

  bool ifPass = true;
  bool passBeamBackgroundRemoval = true;
  ATH_CHECK(checkFilters(ifPass,passBeamBackgroundRemoval,m_MonGroupName,ctx)); //Check ATLAS-Ready, beam-background, etc from base class
  if(!ifPass) return StatusCode::SUCCESS;

  auto eventCounter = Monitored::Scalar<size_t>("eventCounter",0);
  fill(m_MonGroupName,eventCounter);

  checkTrigger();
  
  IdentifierHash caloCellMin, caloCellMax;
  m_calo_id->calo_cell_hash_range(CaloCell_ID::LARFCAL,caloCellMin,caloCellMax);
  //hash-end of FCAL is also the hash-end of LAr
  std::vector<unsigned> hitMap(caloCellMax,0);
  std::vector<std::pair<IdentifierHash,float> > clusteredCells;  
  for (const xAOD::CaloCluster* cluster : *clusterHdl) {
    if (cluster->e()<m_clusterECut) continue;
    //Loop over cells in cluster:
    auto cellIt=cluster->cell_begin();
    auto cellIt_e=cluster->cell_end();
    clusteredCells.clear();
    clusteredCells.reserve(cluster->size());
    for (;cellIt!=cellIt_e;++cellIt) {
       const IdentifierHash hash=cellIt->caloDDE()->calo_hash();
       if (hash<hitMap.size()) { //Ignore tile cells  
          const float cellE=cellIt->energy();
          clusteredCells.emplace_back(std::make_pair(hash,cellE));
       }
    }//end loop over cells in cluster
    //Sort & filter list of cells in this cluster if needed:
    if (m_nCellsPerCluster>0 && m_nCellsPerCluster<clusteredCells.size()) {
      auto middle=clusteredCells.begin()+m_nCellsPerCluster;
      //sort per cell energy
      std::partial_sort(clusteredCells.begin(),middle,clusteredCells.end(),
                         [](const std::pair<IdentifierHash,float>& p1,
                            const std::pair<IdentifierHash,float>& p2) {return (p1.second>p2.second);});
      //Shrink container to required cells
      clusteredCells.resize(m_nCellsPerCluster);
    }//end if m_nClusteredCells>0
    for (const auto& cc : clusteredCells) {  
	    ++hitMap[cc.first];
      }//end if hash<size
  }//end loop over clusteres


  for (const CaloDetDescrElement* caloDDE : caloDDMgr->element_range()) {
    const IdentifierHash h=caloDDE->calo_hash();
    if (h<hitMap.size() && hitMap[h]) {
      float celleta, cellphi;
      unsigned iLyr, iLyrNS;
      getHistoCoordinates(caloDDE, celleta, cellphi, iLyr, iLyrNS);
    
      auto mon_eta = Monitored::Scalar<float>("celleta_"+m_layerNames[iLyr],celleta);
      auto mon_phi = Monitored::Scalar<float>("cellphi_"+m_layerNames[iLyr],cellphi);
      auto mon_hit = Monitored::Scalar<unsigned>("NClusteredCells_"+m_layerNames[iLyr],hitMap[h]);

      const HWIdentifier chid=cabling->createSignalChannelID(caloDDE->identify());
      const IdentifierHash onlHash=m_onlineID->channel_Hash(chid);
      auto mon_id = Monitored::Scalar<unsigned>("cellhash",onlHash);

      fill(m_MonGroupName,mon_eta,mon_phi,mon_hit,mon_id);
    }//end if cell used in a cluster
  }
  return StatusCode::SUCCESS;
}






