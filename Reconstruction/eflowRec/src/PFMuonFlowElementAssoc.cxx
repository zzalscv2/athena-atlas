/*  
 Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "eflowRec/PFMuonFlowElementAssoc.h" 
#include "StoreGate/WriteDecorHandle.h" 
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODPFlow/FlowElementContainer.h" 
#include "xAODPFlow/FlowElement.h" 

//includes for CaloCluster algo
#include "CaloEvent/CaloCell.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "Identifier/Identifier.h"

typedef ElementLink<xAOD::MuonContainer> MuonLink_t; 
typedef ElementLink<xAOD::FlowElementContainer> FlowElementLink_t; 
//
//      Algorithm created by M.T. Anthony
//
// ============================================================= 
PFMuonFlowElementAssoc::PFMuonFlowElementAssoc(const std::string& name, 
		   ISvcLocator* pSvcLocator): 
  AthReentrantAlgorithm(name, pSvcLocator)
{   
  // Declare the decoration keys   
 
}

PFMuonFlowElementAssoc::~PFMuonFlowElementAssoc() {} 

// ============================================================= 
StatusCode PFMuonFlowElementAssoc::initialize() {   

  ATH_MSG_INFO("Initializing " << name() << "...");   

  // Initialise the decoration keys   
  ATH_CHECK(m_muonChargedFEWriteHandleKey.initialize());
  ATH_CHECK(m_muonNeutralFEWriteHandleKey.initialize());
  
  ATH_CHECK(m_ChargedFEmuonWriteHandleKey.initialize());
  ATH_CHECK(m_NeutralFEmuonWriteHandleKey.initialize());
  ATH_CHECK(m_NeutralFE_efrac_match_muonWriteHandleKey.initialize());
  
  //init the experimental keys
  ATH_CHECK(m_muonNeutralFE_muon_efrac_WriteDecorHandleKey.initialize());
  ATH_CHECK(m_NeutralFEmuon_nMatches_WriteDecorHandleKey.initialize());
  ATH_CHECK(m_muon_ClusterInfo_deltaR_WriteDecorHandleKey.initialize());

  //init ReadHandleKeys
  ATH_CHECK(m_muonReadHandleKey.initialize());
  ATH_CHECK(m_chargedFEReadHandleKey.initialize());
  ATH_CHECK(m_neutralFEReadHandleKey.initialize());


  ATH_MSG_INFO("Initialization completed successfully");   

  return StatusCode::SUCCESS; 
} 




// ========================================================================= 
StatusCode PFMuonFlowElementAssoc::execute(const EventContext & ctx) const 
{   
  
  // WriteDecorHandles for the charged/neutral Flow Elements and Muons
  // Links a Muon that has a track to a charged flow element if possible
  
  ATH_MSG_INFO("Started execute step");

  // Get container for muons
  SG::WriteDecorHandle<xAOD::MuonContainer,std::vector<FlowElementLink_t> > muonChargedFEWriteDecorHandle (m_muonChargedFEWriteHandleKey,ctx);
  SG::WriteDecorHandle<xAOD::MuonContainer,std::vector<FlowElementLink_t> > muonNeutralFEWriteDecorHandle (m_muonNeutralFEWriteHandleKey,ctx);
  // get container for charged flow elements
  SG::WriteDecorHandle<xAOD::FlowElementContainer,std::vector<MuonLink_t> > ChargedFEmuonWriteDecorHandle (m_ChargedFEmuonWriteHandleKey,ctx);
  SG::WriteDecorHandle<xAOD::FlowElementContainer,std::vector<MuonLink_t> > NeutralFEmuonWriteDecorHandle(m_NeutralFEmuonWriteHandleKey,ctx);

  //extra container handles between neutral FE cluster and Muon CaloCluster - these are all for studies based on neutral Flow Element matching - All of these handles are experimental
  SG::WriteDecorHandle<xAOD::FlowElementContainer,std::vector<double> > NeutralFE_efrac_match_muonWriteDecorHandle(m_NeutralFE_efrac_match_muonWriteHandleKey,ctx);
  SG::WriteDecorHandle<xAOD::MuonContainer ,std::vector<double> >muonNeutralFE_muon_efrac_WriteDecorHandle(m_muonNeutralFE_muon_efrac_WriteDecorHandleKey,ctx);
  SG::WriteDecorHandle<xAOD::FlowElementContainer ,int> NeutralFEmuon_nMatches_WriteDecorHandle(m_NeutralFEmuon_nMatches_WriteDecorHandleKey,ctx);
  SG::WriteDecorHandle<xAOD::MuonContainer ,double > muon_ClusterInfo_deltaR_WriteDecorHandle(m_muon_ClusterInfo_deltaR_WriteDecorHandleKey,ctx);
  
  //store readhandles for muon and charged flow elements
  SG::ReadHandle<xAOD::MuonContainer> muonReadHandle (m_muonReadHandleKey,ctx); // readhandle for muon
  SG::ReadHandle<xAOD::FlowElementContainer> ChargedFEReadHandle(m_chargedFEReadHandleKey,ctx);
  SG::ReadHandle<xAOD::FlowElementContainer> NeutralFEReadHandle(m_neutralFEReadHandleKey,ctx);
  
  //now init some Flow element link containers
  std::vector<std::vector<FlowElementLink_t> > muonChargedFEVec(muonReadHandle->size());
  std::vector<std::vector<FlowElementLink_t> > muonNeutralFEVec(muonReadHandle->size());
  
  //for neutral flow element studies
  std::vector<std::vector<double> >muonNeutralFE_frac_cluster_energy_matched_Vec(muonReadHandle->size()); 

  
  //Loop over the Flow Elements 

  //////////////////////////////
  //     CHARGED LOOP
  //////////////////////////////
  for (const xAOD::FlowElement* FE: *ChargedFEmuonWriteDecorHandle){
    //get the track associated to the charged flow element (or at least the index of said track)
    size_t FETrackIndex=FE->chargedObjects().at(0)->index();
    // Init a vector of element links to muons
    std::vector<MuonLink_t> FEMuonLinks;
    
    //loop over muons in container
    for(const xAOD::Muon* muon: *muonChargedFEWriteDecorHandle){
      const xAOD::TrackParticle* muon_trk=muon->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);      
      size_t MuonTrkIndex=muon_trk->index();
      if(MuonTrkIndex==FETrackIndex){
	// Add Muon element link to a vector
	// index() is the unique index of the muon in the muon container
	FEMuonLinks.push_back( MuonLink_t(*muonReadHandle, muon->index()));
	// Add flow element link to a vector
	// index() is the unique index of the cFlowElement in the cFlowElementcontaine
	muonChargedFEVec.at(muon->index()).push_back(FlowElementLink_t(*ChargedFEReadHandle,FE->index()));
      } // matching block
    }// end of muon loop
    
    // Add vector of muon element links as decoration to FlowElement container
    ChargedFEmuonWriteDecorHandle(*FE) = FEMuonLinks;
  } // end of charged Flow Element loop

  //////////////////////////////////////////////////
  //   Loop over Neutral FlowElements
  //////////////////////////////////////////////////
  /** 
      In short, this feature is an experimental linker between neutral FEs and Muons either by the following:
      Case 1) Retrieve the CaloCluster(s) from the muon, and get any topocluster(s) associated to those, then link the topoclusters to the neutral FEs
      Case 2) Retrieve the CaloCluster(s) from the muon then link to the neutral FEs
 
      This code is switched using two switches:
      m_LinkNeutralFEClusters (turns on the experimental feature)
      m_UseMuonTopoClusters (True= Case 1, False = Case 2)
  **/
  if(m_LinkNeutralFEClusters){
    ATH_MSG_INFO("Experimental: Cluster Linkers between neutral FEs and Muons are used");
    for (const xAOD::FlowElement* FE: *NeutralFEmuonWriteDecorHandle){
      int nMatchedFE=0;
      //get the index of the cluster corresponding to the Neutral FlowElements
      size_t FEclusterindex=FE->otherObjects().at(0)->index();
      
      // FE->otherObjects returns a vector of IParticles. We only want the first one
      const xAOD::IParticle* FE_Iparticle=FE->otherObjects().at(0);
      //dynamic cast to CaloCluster
      const xAOD::CaloCluster* FE_cluster=dynamic_cast<const xAOD::CaloCluster*>(FE_Iparticle); //cast to CaloCluster
      
      // retrieve the link to cells
      const CaloClusterCellLink* CellLink = FE_cluster->getCellLinks();
      // build the iterator(s) for the looping over the elements inside the CellLink
      CaloClusterCellLink::const_iterator FE_FirstCell=CellLink->begin();
      CaloClusterCellLink::const_iterator FE_LastCell=CellLink->end();
					     
      
      //design the vector of ElementLinks
      std::vector<MuonLink_t> FEMuonLinks;
      std::vector<double> FE_efrac_clustermatch;
      std::vector<double> Muon_efrac_clustermatch;
      for (const xAOD::Muon* muon: *muonNeutralFEWriteDecorHandle ){
	//Retrieve the ElementLink vector of clusters      
	const ElementLink<xAOD::CaloClusterContainer> ClusterLink=muon->clusterLink();
	
	//access object from element link
	const xAOD::CaloCluster* cluster = *ClusterLink; // de-ref the element link to retrieve the pointer to the original object
	  if(m_UseMuonTopoClusters){
	    // get the linker to the topo clusters
	    std::vector<ElementLink<xAOD::CaloClusterContainer>> linksToTopoClusters=cluster->auxdata<std::vector<ElementLink<xAOD::CaloClusterContainer>> >("constituentClusterLinks");
	    for (ElementLink<xAOD::CaloClusterContainer> TopoClusterLink: linksToTopoClusters){
	      const xAOD::CaloCluster* MuonTopoCluster=*TopoClusterLink; // de-ref the link to get the topo-cluster
	      size_t MuonTopoCluster_index=MuonTopoCluster->index();
	      if(MuonTopoCluster_index==FEclusterindex){
		// Add Muon element link to a vector
		// index() is the unique index of the muon in the muon container   
		FEMuonLinks.push_back(MuonLink_t(*muonReadHandle,muon->index()));
		// index() is the unique index of the cFlowElement in the cFlowElementcontaine
		muonNeutralFEVec.at(muon->index()).push_back(FlowElementLink_t(*NeutralFEReadHandle,FE->index()));
		ATH_MSG_INFO("Got a match between NFE and Muon");
		nMatchedFE++; // count number of matches between FE and muons
	      } // check block of index matching	      
	    } // end of loop over element links
	  } //end of TopoCluster specific block
	  else{ // case when we don't use Topoclusters, just match the caloclusters to the flow element
	    //if we don't match topoclusters, do something more complex: 
	    // Retrieve cells in both the FE cluster and muon cluster
	    // Define the link as where at least one calo cell is shared between the FE cluster and the Muon Cluster
	    
	    //retrieve cells associated to Muon cluster
	    const CaloClusterCellLink* Muon_Clus_CellLink=cluster->getCellLinks();
	    //get the iterator on the links
	    CaloClusterCellLink::const_iterator Muon_Clus_FirstCell=Muon_Clus_CellLink->begin();
	    CaloClusterCellLink::const_iterator Muon_Clus_LastCell=Muon_Clus_CellLink->end();

	    //Now we check if any match and sum the energy of the cells that are matched. Current algo allows for at least one match.
	    bool isCellMatched=false;
	    double FE_sum_matched_cellEnergy=0;
	    double Muon_sum_matched_cellEnergy=0;
	    
	    for(;FE_FirstCell != FE_LastCell; ++FE_FirstCell){
	      Identifier index_FECell=FE_FirstCell->ID();
	      for(; Muon_Clus_FirstCell != Muon_Clus_LastCell; ++Muon_Clus_FirstCell){
		Identifier index_muoncell=Muon_Clus_FirstCell->ID();
		if(index_FECell==index_muoncell){
		  isCellMatched=true;
		  double FE_cell_energy=FE_FirstCell->e();
		  FE_sum_matched_cellEnergy=FE_sum_matched_cellEnergy+FE_cell_energy;
		  double Muon_cell_energy=Muon_Clus_FirstCell->e();
		  Muon_sum_matched_cellEnergy= Muon_sum_matched_cellEnergy+Muon_cell_energy;
		}
	      }
	    } // end of cell matching double loop
	    double frac_FE_cluster_energy_matched=0;
	    //retrieve total cluster energy from the FE cluster
	    double tot_FE_cluster_energy=FE_cluster->e();
	    if(tot_FE_cluster_energy!=0){ // ! div 0
	      frac_FE_cluster_energy_matched=FE_sum_matched_cellEnergy / tot_FE_cluster_energy ;
	    }
	    double tot_muon_cluster_energy=cluster->e();
	    double frac_muon_cluster_energy_matched=0;
	    if(tot_muon_cluster_energy!=0){
	      frac_muon_cluster_energy_matched=Muon_sum_matched_cellEnergy/tot_muon_cluster_energy;
	    }
	    if(frac_FE_cluster_energy_matched>0){ 
	      ATH_MSG_INFO("Fraction of FE cluster energy used in match: "<<frac_FE_cluster_energy_matched<<", ismatched? "<<isCellMatched<<"");
	      ATH_MSG_INFO("Fraction of Muon cluster energy used in match: "<<frac_muon_cluster_energy_matched<<"");
	    }

	    if(isCellMatched){ // cell matched => Link the two objects.
	      // Add Muon element link to a vector
	      // index() is the unique index of the muon in the muon container   
	      FEMuonLinks.push_back(MuonLink_t(*muonReadHandle,muon->index()));
	      // index() is the unique index of the nFlowElement in the nFlowElementcontainer
	      muonNeutralFEVec.at(muon->index()).push_back(FlowElementLink_t(*NeutralFEReadHandle,FE->index()));
	      // save the energy fraction used in the cluster matching - mostly for debug/extension studies
	      FE_efrac_clustermatch.push_back(frac_FE_cluster_energy_matched); // fraction of FE cluster energy matched
	      muonNeutralFE_frac_cluster_energy_matched_Vec.at(muon->index()).push_back(frac_muon_cluster_energy_matched);//fraction of Muon cluster energy matched
	      nMatchedFE++; // count number of matches incrementally
	    }

	    
	  } // end of calocluster specific block
      	  
	  // loop over caloclusters
      } // loop over muons
      NeutralFEmuon_nMatches_WriteDecorHandle(*FE)=nMatchedFE;
      NeutralFEmuonWriteDecorHandle(*FE)=FEMuonLinks;
      NeutralFE_efrac_match_muonWriteDecorHandle(*FE)=FE_efrac_clustermatch;
    } // loop over neutral FE
  }// end of the Gaudi check block
  
  //////////////////////////////////////////////////
  //   WRITE OUTPUT: ADD HANDLES TO MUON CONTAINERS
  //////////////////////////////////////////////////
  // Add the vectors of the Flow Element Links as decoations to the muon container
  for(const xAOD::Muon* muon: *muonChargedFEWriteDecorHandle){
    muonChargedFEWriteDecorHandle(*muon)=muonChargedFEVec.at(muon->index());    
  } // end of muon loop
  if(m_LinkNeutralFEClusters){// Experimental
    for(const xAOD::Muon* muon: *muonNeutralFEWriteDecorHandle){
      if(muonNeutralFEVec.size()>0){
	muonNeutralFEWriteDecorHandle(*muon)=muonNeutralFEVec.at(muon->index());
	muonNeutralFE_muon_efrac_WriteDecorHandle(*muon)=muonNeutralFE_frac_cluster_energy_matched_Vec.at(muon->index());
      }
      // For debug of the muon clusters used, add also: dR between caloclusters and number of caloclusters associated to each muon.
      //retrieve element link again to cluster
      const ElementLink<xAOD::CaloClusterContainer> ClusterContLink=muon->clusterLink();
      // use elem link to retrieve container
      const xAOD::CaloCluster* MuonCluster=*ClusterContLink;
      TLorentzVector muon_fourvec=muon->p4();
      TLorentzVector muon_cluster_fourvec=MuonCluster->p4();
      double deltaR_muon_cluster=0;
      deltaR_muon_cluster=muon_fourvec.DeltaR(muon_cluster_fourvec);
      // retrieve the vector of delta R between muon and its associated calo cluster.
      muon_ClusterInfo_deltaR_WriteDecorHandle(*muon)=deltaR_muon_cluster;
    }
  }// end of experimental block
  ATH_MSG_INFO("Execute completed successfully");   
  
  return StatusCode::SUCCESS;
}

