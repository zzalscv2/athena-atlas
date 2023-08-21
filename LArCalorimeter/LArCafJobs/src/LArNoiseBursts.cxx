/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/// LArNoiseBursts
/// Author: Josu Cantero
/// UAM, November, 2010
/// Modified by Hideki Okawa
/// BNL, June, 2012

#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/ReadCondHandle.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloLVL1_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArMiniFCAL_ID.h"
#include "CaloIdentifier/LArID_Exception.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "Identifier/Range.h" 
#include "Identifier/IdentifierHash.h"
#include "Identifier/HWIdentifier.h"

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArHVLineID.h"
#include "LArIdentifier/LArElectrodeID.h"

#include "LArRecEvent/LArEventBitInfo.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArRecEvent/LArNoisyROSummary.h"
#include "LArRecEvent/LArCollisionTime.h"
#include "CaloConditions/CaloNoise.h"
 
#include "NavFourMom/IParticleContainer.h"
#include "NavFourMom/INavigable4MomentumCollection.h"

// Lar HV
#include "CaloDetDescr/CaloDetectorElements.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

///////////////////////////////////////////////////////////////////

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "xAODEventInfo/EventInfo.h"

#include "CaloEvent/CaloCellContainer.h"

#include "LArCafJobs/LArNoiseBursts.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>

using namespace std;

int nlarcell=0;
int n_noisy_cell_part[8] = {0,0,0,0,0,0,0,0};
int n_cell_part[8] = {0,0,0,0,0,0,0,0};
std::vector<short> v_ft_noisy, v_slot_noisy, v_channel_noisy;
std::vector<bool> v_isbarrel, v_isendcap, v_isfcal, v_ishec;
std::vector<short> v_layer; 
std::vector<int> v_partition,v_noisycellHVphi,v_noisycellHVeta;
std::vector<float> v_energycell, v_qfactorcell, v_signifcell;
std::vector<float> v_phicell, v_etacell;
std::vector<bool> v_isbadcell;
std::vector<IdentifierHash>  v_IdHash;
std::vector<int> v_cellpartlayerindex;
std::vector<Identifier> v_cellIdentifier;

//////////////////////////////////////////////////////////////////////////////////////
/// Constructor


LArNoiseBursts::LArNoiseBursts(const std::string& name,
			 ISvcLocator* pSvcLocator) 
  : AthAlgorithm(name, pSvcLocator),
    m_thistSvc(0),
    m_tree(0),
    m_trigDec( "Trig::TrigDecisionTool/TrigDecisionTool" ),
    m_LArOnlineIDHelper(0),
    m_LArHVLineIDHelper(0),
    m_LArElectrodeIDHelper(0),
    m_LArEM_IDHelper(0),
    m_LArFCAL_IDHelper(0),
    m_LArHEC_IDHelper(0),
    m_CosmicCaloStream(false),
    m_nb_sat(0),
    m_lowqfactor(0),
    m_medqfactor(0),
    m_hiqfactor(0),
    m_noisycell(0),
    m_nt_larcellsize(0),
    m_nt_cellsize(0),
    m_nt_run(0),
    m_nt_evtId(0),
    m_nt_evtTime(0),
    m_nt_evtTime_ns(0),
    m_nt_lb(0),
    m_nt_bcid(0),
    //m_nt_ntracks(0),
    m_nt_isbcidFilled(0),
    m_nt_isbcidInTrain(0),
    m_nt_isBunchesInFront(0),
    m_nt_bunchtype(0),
    m_nt_bunchtime(0),
    m_nt_atlasready(0),
    m_nt_stablebeams(0),
    m_nt_streamTagName(0),
    m_nt_streamTagType(0),
    m_nt_larnoisyro(0),
    m_nt_larnoisyro_opt(0),
    m_nt_larnoisyro_satTwo(0),
    m_nt_larmnbnoisy(0),
    m_nt_larmnbnoisy_sat(0),
//    m_nt_veto_mbts(0),
//    //m_nt_veto_indet(0),
//    m_nt_veto_bcm(0),
//    m_nt_veto_lucid(0),
//    m_nt_veto_pixel(0),
//    m_nt_veto_sct(0),
//    m_nt_veto_mbtstdHalo(0),
//    m_nt_veto_mbtstdCol(0),
//    m_nt_veto_lartdHalo(0),
//    m_nt_veto_lartdCol(0),
//    m_nt_veto_csctdHalo(0),
//    m_nt_veto_csctdCol(0),
//    m_nt_veto_bcmtHalo(0),
//    m_nt_veto_bcmtCol(0),
//    m_nt_veto_muontCol(0),
//    m_nt_veto_muontCosmic(0),
    m_nt_larflag_badFEBs(false),
    m_nt_larflag_mediumSaturatedDQ(false),
    m_nt_larflag_tightSaturatedDQ(false),
    m_nt_larflag_noiseBurstVeto(false),
    m_nt_larflag_dataCorrupted(false),
    m_nt_larflag_dataCorruptedVeto(false),
    m_nt_L1_J75(false),
    m_nt_L1_J10_EMPTY(false),
    m_nt_L1_J30_FIRSTEMPTY(false),
    m_nt_L1_J30_EMPTY(false),
    m_nt_L1_XE40(false),
    m_nt_L1_XE50(false),
    m_nt_L1_XE50_BGRP7(false),
    m_nt_L1_XE70(false),
    m_nt_EF_j165_u0uchad_LArNoiseBurst(false),
    m_nt_EF_j30_u0uchad_empty_LArNoiseBurst(false),
    m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst(false),
    m_nt_EF_j55_u0uchad_empty_LArNoiseBurst(false),
    m_nt_EF_xe45_LArNoiseBurst(false),
    m_nt_EF_xe55_LArNoiseBurst(false),
    m_nt_EF_xe60_LArNoiseBurst(false),
    m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT(false),
    m_nt_EF_j100_u0uchad_LArNoiseBurstT(false),
    m_nt_EF_j165_u0uchad_LArNoiseBurstT(false),
    m_nt_EF_j130_u0uchad_LArNoiseBurstT(false),
    m_nt_EF_j35_u0uchad_empty_LArNoiseBurst(false),
    m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst(false),
    m_nt_EF_j80_u0uchad_LArNoiseBurstT(false),
    m_nt_ECTimeDiff(0),
    m_nt_ECTimeAvg(0),
    m_nt_nCellA(0),
    m_nt_nCellC(0),
    m_nt_energycell(0),
    m_nt_qfactorcell(0),
    m_nt_phicell(0),
    m_nt_etacell(0),
    m_nt_signifcell(0),
    //m_nt_noisycellpercent(0),
    m_nt_ft_noisy(0),
    m_nt_slot_noisy(0),
    m_nt_channel_noisy(0),
    m_nt_cellpartlayerindex(0),
    m_nt_cellIdentifier(0),
    m_nt_noisycellpart(0),
    m_nt_noisycellHVphi(0),
    m_nt_noisycellHVeta(0),
    m_nt_samples(0),
    m_nt_gain(0),
    m_nt_isbadcell(0),
    m_nt_partition(0),
    m_nt_layer(0),
    m_nt_isbadcell_sat(0),
    m_nt_barrelec_sat(0),
    m_nt_posneg_sat(0),
    m_nt_ft_sat(0),
    m_nt_slot_sat(0),
    m_nt_channel_sat(0),
    m_nt_partition_sat(0),
    m_nt_energy_sat(0),
    m_nt_phicell_sat(0),
    m_nt_etacell_sat(0),
    m_nt_layer_sat(0),
    m_nt_cellIdentifier_sat(0)
 {

   // Trigger
   declareProperty( "TrigDecisionTool", m_trigDec );
   
   
   //event cuts
   declareProperty("SigmaCut", m_sigmacut = 3.0);
   declareProperty("NumberOfBunchesInFront",m_frontbunches = 36);
   
   // Keep cell properties
   declareProperty("KeepOnlyCellID",          m_keepOnlyCellID = false);
 }

/////////////////////////////////////////////////////////////////////////////////////
/// Destructor - check up memory allocation
/// delete any memory allocation on the heap

LArNoiseBursts::~LArNoiseBursts() {}

////////////////////////////////////////////////////////////////////////////////////
/// Initialize
/// initialize StoreGate
/// get a handle on the analysis tools
/// book histograms
/*
StatusCode LArNoiseBursts::initializeBeforeEventLoop() {
  MsgStream mLog( messageService(), name() );

  ATH_MSG_DEBUG ( "Initializing LArNoiseBursts (before eventloop)" );
  
  // NEW
  

  // retrieve trigger decision tool
  // needs to be done before the first run/event since a number of
  // BeginRun/BeginEvents are registered by dependent services
  StatusCode sc = StatusCode::SUCCESS;

  return sc;
} */
//////////////////////////////////////////////////////////////////////////////////////
///////////////////          INITIALIZE        ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

StatusCode LArNoiseBursts::initialize() {

  ATH_MSG_DEBUG ( "Initializing LArNoiseBursts" );
 
  // Trigger Decision Tool
  if(!m_trigDec.empty()){
    if(m_trigDec.retrieve().isFailure()){
      ATH_MSG_WARNING ( "Failed to retrieve trigger decision tool " << m_trigDec );
    }else{
      ATH_MSG_INFO ( "Retrieved tool " << m_trigDec );
    }
   }
  
  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK( m_BCKey.initialize() );
  ATH_CHECK( m_totalNoiseKey.initialize() );
  ATH_CHECK( m_bcDataKey.initialize() );

  // Retrieve online ID helper
  const LArOnlineID* LArOnlineIDHelper = nullptr;
  ATH_CHECK( detStore()->retrieve(LArOnlineIDHelper, "LArOnlineID") );
  m_LArOnlineIDHelper = LArOnlineIDHelper;
  ATH_MSG_DEBUG( " Found LArOnline Helper");

  // Retrieve HV line ID helper
  const LArHVLineID* LArHVLineIDHelper = nullptr;
    ATH_CHECK( detStore()->retrieve(LArHVLineIDHelper, "LArHVLineID") );
  m_LArHVLineIDHelper = LArHVLineIDHelper;
  ATH_MSG_DEBUG( " Found LArOnlineIDHelper Helper");

  // Retrieve HV electrode ID helper
  const LArElectrodeID* LArElectrodeIDHelper = nullptr;
  ATH_CHECK( detStore()->retrieve(LArElectrodeIDHelper, "LArElectrodeID") );
  m_LArElectrodeIDHelper = LArElectrodeIDHelper;
  ATH_MSG_DEBUG( " Found LArElectrodeIDHelper Helper");

  ATH_CHECK(m_caloMgrKey.initialize());

  // Retrieve ID helpers
  const CaloCell_ID* idHelper = nullptr;
  ATH_CHECK( detStore()->retrieve (idHelper, "CaloCell_ID") );
  m_LArEM_IDHelper   = idHelper->em_idHelper();
  m_LArHEC_IDHelper  = idHelper->hec_idHelper();
  m_LArFCAL_IDHelper = idHelper->fcal_idHelper();
  
  /** get a handle on the NTuple and histogramming service */
  ATH_CHECK( service("THistSvc", m_thistSvc) );
 
  /*const AthenaAttributeList* fillparams(0);
  sc =  evtStore()->retrieve(fillparams, "/TDAQ/OLC/LHC/FILLPARAMS");
  if (sc.isFailure()) {
     ATH_MSG_WARNING ("Unable to retrieve fillparams information; falling back to" );
     return StatusCode::SUCCESS;
   }
 
  if (fillparams != 0) {
     ATH_MSG_DEBUG ("beam 1 #bunches are: " << (*fillparams)["Beam1Bunches"].data<uint32_t>() );
     ATH_MSG_DEBUG ("beam 2 #bunches are: " << (*fillparams)["Beam2Bunches"].data<uint32_t>() );
  }
*/

  /** Prepare TTree **/
  m_tree = new TTree( "CollectionTree", "CollectionTree" );
  std::string treeName =  "/TTREE/CollectionTree" ;
  ATH_CHECK( m_thistSvc->regTree(treeName, m_tree) );

  // General properties of events
  m_tree->Branch("Run",&m_nt_run,"Run/I");// Event ID
  m_tree->Branch("EventId",&m_nt_evtId,"EventId/l");// Event ID
  m_tree->Branch("EventTime",&m_nt_evtTime,"EventTime/I");// Event time
  m_tree->Branch("EventTime_ns",&m_nt_evtTime_ns,"EventTime_ns/I");// Event time in nanosecond
  m_tree->Branch("LumiBlock",&m_nt_lb,"LumiBlock/I"); // LB
  m_tree->Branch("BCID",&m_nt_bcid,"BCID/I"); // BCID
  m_tree->Branch("StreamTagName",&m_nt_streamTagName);// stream tag name
  m_tree->Branch("StreamTagType",&m_nt_streamTagType); // stream tag type 
  m_tree->Branch("IsBCIDFilled", &m_nt_isbcidFilled,"IsBCIDFilled/I"); // check if bunch is filled
  m_tree->Branch("IsBCIDInTrain",&m_nt_isbcidInTrain,"ISBCIDInTrain/I"); // check if bunch belong to a train
  m_tree->Branch("BunchesInFront",&m_nt_isBunchesInFront); // check front bunches
  m_tree->Branch("BunchType",&m_nt_bunchtype,"BunchType/I");// Empty = 0, FirstEmpty=1,&middleEmpty=2, Single=100,Front=200,&middle=201,Tail=202 
  m_tree->Branch("TimeAfterBunch",&m_nt_bunchtime,"TimeAfterBunch/F"); //time "distance" between the colliding bunch and the nearest one.
  m_tree->Branch("ATLASIsReady",&m_nt_atlasready,"AtlasIsReady/I"); //check if atlas is ready for physics 
  m_tree->Branch("StableBeams",&m_nt_stablebeams,"StableBeams/I");//check stablebeams

  // Background bits in EventInfo
//  m_tree->Branch("vetoMBTS",&m_nt_veto_mbts,"vetoMBST/S"); //Beam/collision veto based on mbts
//  m_tree->Branch("vetoPixel",&m_nt_veto_pixel,"vetoPixel/S"); //Beam/collision veto based on indet
//  m_tree->Branch("vetoSCT",&m_nt_veto_sct,"vetoSCT/S"); //Beam/collision veto based on indet
//  m_tree->Branch("vetoBcm",&m_nt_veto_bcm,"vetoBcm/S"); //Beam/collision veto based on bcm
//  m_tree->Branch("vetoLucid",&m_nt_veto_lucid,"vetoLucid/S"); //Beam/collision veto based on lucid
//  m_tree->Branch("vetoMBTSDtHalo",&m_nt_veto_mbtstdHalo,"vetoMBTSDtHalo/S");
//  m_tree->Branch("vetoMBTSDtCol",&m_nt_veto_mbtstdCol,"vetoMBTSDtCol/S");
//  m_tree->Branch("vetoLArDtHalo",&m_nt_veto_lartdHalo,"vetoLArDtHalo/S");
//  m_tree->Branch("vetoLArDtCol",&m_nt_veto_lartdCol,"vetoLArDtCol/S");
//  m_tree->Branch("vetoCSCDtHalo",&m_nt_veto_csctdHalo,"vetoCSCDtHalo/S");
//  m_tree->Branch("vetoCSCDtCol",&m_nt_veto_csctdCol,"vetoCSCDtCol/S");
//  m_tree->Branch("vetoBCMDtHalo",&m_nt_veto_bcmtHalo,"vetoBCMDtHalo/S");
//  m_tree->Branch("vetoBCMDtCol",&m_nt_veto_bcmtCol,"vetoBCMDtCol/S");
//  m_tree->Branch("vetoMuonTimmingCol", &m_nt_veto_muontCol,"vetoMuonTimmingCol/S");
//  m_tree->Branch("vetoMuonTimmingCosmic",&m_nt_veto_muontCosmic,"vetoMuonTimmingCosmic/S");

  // LAr event bit info
  m_tree->Branch("larflag_badFEBs",&m_nt_larflag_badFEBs,"larflag_badFEBs/O");
  m_tree->Branch("larflag_mediumSaturatedDQ",&m_nt_larflag_mediumSaturatedDQ,"larflag_mediumSaturatedDQ/O");
  m_tree->Branch("larflag_tightSaturatedDQ",&m_nt_larflag_tightSaturatedDQ,"larflag_tightSaturatedDQ/O");
  m_tree->Branch("larflag_noiseBurstVeto",&m_nt_larflag_noiseBurstVeto,"larflag_noiseBurstVeto/O");
  m_tree->Branch("larflag_dataCorrupted",&m_nt_larflag_dataCorrupted,"larflag_dataCorrupted/O");
  m_tree->Branch("larflag_dataCorruptedVeto",&m_nt_larflag_dataCorruptedVeto,"larflag_dataCorruptedVeto/O");

  // trigger flags
  m_tree->Branch("L1_J75",&m_nt_L1_J75,"L1_J75/O");
  m_tree->Branch("L1_J10_EMPTY",&m_nt_L1_J10_EMPTY,"L1_J10_EMPTY/O");
  m_tree->Branch("L1_J30_FIRSTEMPTY",&m_nt_L1_J30_FIRSTEMPTY,"L1_J30_FIRSTEMPTY/O");
  m_tree->Branch("L1_J30_EMPTY",&m_nt_L1_J30_EMPTY,"L1_J30_EMPTY/O");
  m_tree->Branch("L1_XE40",&m_nt_L1_XE40,"L1_XE40/O");
  m_tree->Branch("L1_XE50",&m_nt_L1_XE50,"L1_XE50/O");
  m_tree->Branch("L1_XE50_BGRP7",&m_nt_L1_XE50_BGRP7,"L1_XE50_BGRP7/O");
  m_tree->Branch("L1_XE70",&m_nt_L1_XE70,"L1_XE70/O");
  
  m_tree->Branch("EF_j165_u0uchad_LArNoiseBurst",&m_nt_EF_j165_u0uchad_LArNoiseBurst,"EF_j165_u0uchad_LArNoiseBurst/O");
  m_tree->Branch("EF_j30_u0uchad_empty_LArNoiseBurst",&m_nt_EF_j30_u0uchad_empty_LArNoiseBurst,"EF_j30_u0uchad_empty_LArNoiseBurst/O");
  m_tree->Branch("EF_j55_u0uchad_firstempty_LArNoiseBurst",&m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst,"EF_j55_u0uchad_firstempty_LArNoiseBurst/O");
  m_tree->Branch("EF_j55_u0uchad_empty_LArNoiseBurst",&m_nt_EF_j55_u0uchad_empty_LArNoiseBurst,"EF_j55_u0uchad_empty_LArNoiseBurst/O");
  m_tree->Branch("EF_xe45_LArNoiseBurst",&m_nt_EF_xe45_LArNoiseBurst,"EF_xe45_LArNoiseBurst/O");
  m_tree->Branch("EF_xe55_LArNoiseBurst",&m_nt_EF_xe55_LArNoiseBurst,"EF_xe55_LArNoiseBurst/O");
  m_tree->Branch("EF_xe60_LArNoiseBurst",&m_nt_EF_xe60_LArNoiseBurst,"EF_xe60_LArNoiseBurst/O");
  m_tree->Branch("EF_j55_u0uchad_firstempty_LArNoiseBurstT",&m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT,"EF_j55_u0uchad_firstempty_LArNoiseBurstT/O");
  m_tree->Branch("EF_j100_u0uchad_LArNoiseBurstT",&m_nt_EF_j100_u0uchad_LArNoiseBurstT,"EF_j100_u0uchad_LArNoiseBurstT/O");
  m_tree->Branch("EF_j165_u0uchad_LArNoiseBurstT",&m_nt_EF_j165_u0uchad_LArNoiseBurstT,"EF_j165_u0uchad_LArNoiseBurstT/O");
  m_tree->Branch("EF_j130_u0uchad_LArNoiseBurstT",&m_nt_EF_j130_u0uchad_LArNoiseBurstT,"EF_j130_u0uchad_LArNoiseBurstT/O");
  m_tree->Branch("EF_j35_u0uchad_empty_LArNoiseBurst",&m_nt_EF_j35_u0uchad_empty_LArNoiseBurst,"EF_j35_u0uchad_empty_LArNoiseBurst/O");
  m_tree->Branch("EF_j35_u0uchad_firstempty_LArNoiseBurst",&m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst,"EF_j35_u0uchad_firstempty_LArNoiseBurst/O");
  m_tree->Branch("EF_j80_u0uchad_LArNoiseBurstT",&m_nt_EF_j80_u0uchad_LArNoiseBurstT,"EF_j80_u0uchad_LArNoiseBurstT/O");
  
  // 
  m_tree->Branch("LArCellSize", &m_nt_larcellsize,"LArCellSize/I"); // NEW number of online conected LAr cells
  m_tree->Branch("CaloCellSize",&m_nt_cellsize,"CaloCellSize/I");// NEW number of total cells.
  // LAr time difference as computed by LArCollisionTime info
  m_tree->Branch("LArTime_Diff",&m_nt_ECTimeDiff,"LArTime_Diff/F"); // time diff between 2 endcaps
  m_tree->Branch("LArTime_Avg",&m_nt_ECTimeAvg,"LArTime_Avg/F"); // time average of 2 endcaps
  m_tree->Branch("LArTime_nCellA",&m_nt_nCellA,"LArTime_nCellA/I"); // nb of cells used to compute endcap A time
  m_tree->Branch("LArTime_nCellC",&m_nt_nCellC,"LArTime_nCellC/I"); // nb of cells used to compute endcap C time

  // Event properties related to yield of channels in 3 sigma tails 
  //m_tree->Branch("PerCentNoisyCell",&m_nt_noisycellpercent,"PerCentNoisyCell/F"); // Yield of channels in 3sigma tails in whole LAr
  m_tree->Branch("PerCentNoisyCellPartition",&m_nt_noisycellpart); // Yield in each partition:0:embc 1:emba 2:emecc 3:emeca 4:fcalc 5:fcala 6:hecc 7:heca

  // LArNoisyRO output
  m_tree->Branch("LArNoisyRO_Std", &m_nt_larnoisyro,"LArNoisyRO_Std/S"); // standard flag (>5 FEB with more than 30 cells with q factor > 4000)
  m_tree->Branch("LArNoisyRO_Std_optimized", &m_nt_larnoisyro_opt,"LArNoisyRO_Std_optimized/S"); // standard flag with a double weight for critical FEB (>5 FEB with more than 30 cells with q factor > 4000)
  m_tree->Branch("LArNoisyRO_SatTight",&m_nt_larnoisyro_satTwo,"LArNoisyRO_SatTight/S"); // tight flag (> 20 cells with E>1000MeV and saturated q factor) 
  m_tree->Branch("LArNoisyRO_MNB",&m_nt_larmnbnoisy,"LArNoisyRO_MNB/S");
  m_tree->Branch("LArNoisyRO_MNB_Sat",&m_nt_larmnbnoisy_sat,"LArNoisyRO_MNB_Sat/S");

  // Properties of cells with fabs(energy/noise)>3
  m_tree->Branch("NoisyCellPartitionLayerIndex",&m_nt_cellpartlayerindex); /// NEW Identifier of the cell
  m_tree->Branch("NoisyCellOnlineIdentifier",&m_nt_cellIdentifier); // Identifier of the noisy cell
  m_tree->Branch("NoisyCellPartition",&m_nt_partition); // Partition in 1 integer: 0:embc 1:emba 2:emecc 3:emeca 4:fcalc 5:fcala 6:hecc 7:heca
  m_tree->Branch("NoisyCellFT",&m_nt_ft_noisy);                        // FT 
  m_tree->Branch("NoisyCellSlot",&m_nt_slot_noisy);                    // Slot
  m_tree->Branch("NoisyCellChannel",&m_nt_channel_noisy);              // Channel
  m_tree->Branch("NoisyCellADCvalues", &m_nt_samples);                 // ADC values
  m_tree->Branch("NoisyCellGain",&m_nt_gain);                          // Gain
  m_tree->Branch("NoisyCellPhi",&m_nt_phicell);                        // Phi
  m_tree->Branch("NoisyCellEta",&m_nt_etacell);                        // Eta
  m_tree->Branch("NoisyCellLayer", &m_nt_layer);                       // layer
  m_tree->Branch("NoisyCellHVphi", &m_nt_noisycellHVphi);              // Phi of HV
  m_tree->Branch("NoisyCellHVeta", &m_nt_noisycellHVeta);              // Eta of HV
  m_tree->Branch("NoisyCellEnergy",&m_nt_energycell);                  // Energy
  m_tree->Branch("NoisyCellSignificance",&m_nt_signifcell);            // Significance (energy/noise)
  m_tree->Branch("NoisyCellQFactor",&m_nt_qfactorcell);                // Q factor
  m_tree->Branch("NoisyCellIsBad",&m_nt_isbadcell);                    // Bad channel status
  // Event properties related to q factor
  m_tree->Branch("nbLowQFactor", &m_lowqfactor,"m_lowqfactor/i"); // Nb of cells per event with q factor<1000
  m_tree->Branch("nbMedQFactor", &m_medqfactor,"m_medqfactor/i"); // Nb of cells per event with 1000<q factor<10000
  m_tree->Branch("nbHighQFactor", &m_hiqfactor,"m_hiqfactor/i");  // Nb of cells per event with q 10000<factor<65535
  m_tree->Branch("nbSatQFactor",&m_nb_sat,"nbSat/i"); // Nb of cells per event with saturated q factor (65535) 
  // Properties of cells with q factor saturated
  m_tree->Branch("SatCellPartition",&m_nt_partition_sat);
  m_tree->Branch("SatCellBarrelEc",&m_nt_barrelec_sat);
  m_tree->Branch("SatCellPosNeg",&m_nt_posneg_sat);
  m_tree->Branch("SatCellFT",&m_nt_ft_sat);
  m_tree->Branch("SatCellSlot",&m_nt_slot_sat);
  m_tree->Branch("SatCellChannel",&m_nt_channel_sat);
  m_tree->Branch("SatCellEnergy",&m_nt_energy_sat);
  m_tree->Branch("SatCellPhi", &m_nt_phicell_sat);
  m_tree->Branch("SatCellEta",&m_nt_etacell_sat);
  m_tree->Branch("SatCellLayer",&m_nt_layer_sat);
  m_tree->Branch("SatCellIsBad", &m_nt_isbadcell_sat);
  m_tree->Branch("SatCellOnlineIdentifier",&m_nt_cellIdentifier_sat);


  ATH_MSG_DEBUG ( "End of Initializing LArNoiseBursts" );
 
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////
/// Finalize - delete any memory allocation from the heap

StatusCode LArNoiseBursts::finalize() {
  ATH_MSG_DEBUG ( "in finalize()" );
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////
/// Clear - clear CBNT members
StatusCode LArNoiseBursts::clear() {
  /// For Athena-Aware NTuple

  ATH_MSG_DEBUG ( "start clearing variables " );
  
  m_nb_sat     = 0;
  m_noisycell  = 0;
  m_lowqfactor = 0;
  m_medqfactor = 0;
  m_hiqfactor  = 0;

  m_nt_run        = 0;
  m_nt_evtId      = 0;
  m_nt_evtTime    = 0;
  m_nt_evtTime_ns = 0;
  m_nt_lb         = 0;
  m_nt_bcid       = -1;
 
  m_nt_streamTagName.clear();
  m_nt_streamTagType.clear();
  m_nt_isbcidFilled     = -1;
  m_nt_isbcidInTrain    = -1;
  m_nt_isBunchesInFront.clear();
  m_nt_bunchtype        = -1;
  m_nt_bunchtime        = -1.0;
  m_nt_atlasready       = -1;
  m_nt_stablebeams      = -1;
  m_nt_larnoisyro       = -1;
  m_nt_larnoisyro_opt   = -1;
  m_nt_larnoisyro_satTwo= -1;
  m_nt_larmnbnoisy      = -1;
  m_nt_larmnbnoisy_sat  = -1;

  ATH_MSG_DEBUG ( "clearing event info veto variables " );

  //clearing event info veto variables
//  m_nt_veto_mbts        = -1;
//  m_nt_veto_pixel       = -1;
//  m_nt_veto_sct         = -1;
//  m_nt_veto_bcm         = -1;
//  m_nt_veto_lucid       = -1;
//  m_nt_veto_mbtstdHalo  = -1;
//  m_nt_veto_mbtstdCol   = -1;
//  m_nt_veto_lartdHalo   = -1;
//  m_nt_veto_lartdCol    = -1;
//  m_nt_veto_csctdHalo   = -1;
//  m_nt_veto_csctdCol    = -1;
//  m_nt_veto_bcmtHalo    = -1;
//  m_nt_veto_bcmtCol     = -1;
//  m_nt_veto_muontCol    = -1;
//  m_nt_veto_muontCosmic = -1;
//
//  mLog << MSG::DEBUG << "clearing LAr event flags " << endmsg;

  ATH_MSG_DEBUG ( "clearing LAr event flags " );

  // lar bit event info
  m_nt_larflag_badFEBs = false;
  m_nt_larflag_mediumSaturatedDQ = false;
  m_nt_larflag_tightSaturatedDQ = false;
  m_nt_larflag_noiseBurstVeto = false;
  m_nt_larflag_dataCorrupted = false;
  m_nt_larflag_dataCorruptedVeto = false;

  ATH_MSG_DEBUG ( "clearing Pixel variables " );

  // Trigger flags
  m_nt_L1_J75 = true; // turned on for tests
  m_nt_L1_J10_EMPTY = false;
  m_nt_L1_J30_FIRSTEMPTY = false;
  m_nt_L1_J30_EMPTY = false;
  m_nt_L1_XE40 = false;
  m_nt_L1_XE50 = false;
  m_nt_L1_XE50_BGRP7 = false;
  m_nt_L1_XE70 = false;
  
  m_nt_EF_j165_u0uchad_LArNoiseBurst = false;
  m_nt_EF_j30_u0uchad_empty_LArNoiseBurst = false;
  m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst = false;
  m_nt_EF_j55_u0uchad_empty_LArNoiseBurst = false;
  m_nt_EF_xe45_LArNoiseBurst = false;
  m_nt_EF_xe55_LArNoiseBurst = false;
  m_nt_EF_xe60_LArNoiseBurst = false;
  m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT = false;
  m_nt_EF_j100_u0uchad_LArNoiseBurstT = false;
  m_nt_EF_j165_u0uchad_LArNoiseBurstT = false;
  m_nt_EF_j130_u0uchad_LArNoiseBurstT = false;
  m_nt_EF_j35_u0uchad_empty_LArNoiseBurst = false;
  m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst = false;
  m_nt_EF_j80_u0uchad_LArNoiseBurstT = false;
   
  ATH_MSG_DEBUG ( "clearing trigger flags " );

  ATH_MSG_DEBUG ( "clearing noisy cells variables " );

  //Quantities for noisy cells
  m_nt_energycell.clear();
  m_nt_qfactorcell.clear();
  m_nt_phicell.clear();
  m_nt_etacell.clear();
  m_nt_signifcell.clear();
  m_nt_isbadcell.clear();
  m_nt_partition.clear();
  m_nt_layer.clear();
  //m_nt_noisycellpercent = -1;
  m_nt_ft_noisy.clear();
  m_nt_slot_noisy.clear();
  m_nt_channel_noisy.clear();
  m_nt_larcellsize = -1;
  m_nt_cellsize    = -1;
  m_nt_cellpartlayerindex.clear();
  m_nt_cellIdentifier.clear();
  m_nt_noisycellpart.clear();
  m_nt_samples.clear();
  m_nt_gain.clear();

  ///HV branches
  m_nt_noisycellHVphi.clear();
  m_nt_noisycellHVeta.clear();

  ATH_MSG_DEBUG ( "clearing saturated cell variables " );

  //clearing quantities for sat cells
  m_nt_barrelec_sat.clear();
  m_nt_posneg_sat.clear();
  m_nt_ft_sat.clear();
  m_nt_slot_sat.clear();
  m_nt_channel_sat.clear();
  m_nt_partition_sat.clear();
  m_nt_energy_sat.clear();
  m_nt_phicell_sat.clear();
  m_nt_etacell_sat.clear();
  m_nt_layer_sat.clear();
  m_nt_cellIdentifier_sat.clear();
  m_nt_isbadcell_sat.clear();

  ATH_MSG_DEBUG ( "clearing LArTimeDiff variables " );
  
  //DiffTime computed with LAR
  m_nt_ECTimeDiff  = 9999;
  m_nt_ECTimeAvg   = 9999;
  m_nt_nCellA      = -1;
  m_nt_nCellC      = -1;

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////
/// Execute - on event by event

StatusCode LArNoiseBursts::execute() {
  ATH_MSG_DEBUG ( "in execute()" );

  StatusCode sc = clear();
  if(sc.isFailure()) {
    ATH_MSG_WARNING ( "The method clear() failed" );
    return StatusCode::SUCCESS;
  }
  
  if(!m_trigDec.empty()){

  sc = doTrigger();
  if(sc.isFailure()) {
    ATH_MSG_WARNING ( "The method doTrigger() failed" );
    return StatusCode::SUCCESS;
  }
  }

  sc = doEventProperties();
  if(sc.isFailure()) {
    ATH_MSG_WARNING ( "The method doEventProperties() failed" );
    return StatusCode::SUCCESS;
  }

  sc = doLArNoiseBursts();
  if (sc.isFailure()) {
    ATH_MSG_WARNING ( "The method doLArNoiseBursts() failed" );
    return StatusCode::SUCCESS;
  }

//  sc = doPhysicsObjects();
//  if (sc.isFailure()) {
//    ATH_MSG_WARNING ( "The method doPhysicsObjects() failed" );
//    return StatusCode::SUCCESS;
//  }
  
  m_tree->Fill();

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doTrigger        ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doTrigger(){
  ATH_MSG_DEBUG ("in doTrigger ");
  
  std::string mychain( "L1_J75" );
  if( ! m_trigDec->getListOfTriggers(mychain).empty() ){
    m_nt_L1_J75 = m_trigDec->isPassed( mychain );
  }
  mychain = "L1_J10_EMPTY";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_J10_EMPTY =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_J30_FIRSTEMPTY";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_J30_FIRSTEMPTY =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_J30_EMPTY";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_J30_EMPTY =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE40";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE40 =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE50";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE50 =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE50_BGRP7";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE50_BGRP7 =  m_trigDec->isPassed( mychain );
  }
  mychain = "L1_XE70";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_L1_XE70 =  m_trigDec->isPassed( mychain );
  }

  // EF
  mychain = "EF_j165_u0uchad_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j165_u0uchad_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j30_u0uchad_empty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j30_u0uchad_empty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j55_u0uchad_firstempty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j55_u0uchad_empty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j55_u0uchad_empty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_xe45_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_xe45_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_xe55_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_xe55_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_xe60_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_xe60_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j55_u0uchad_firstempty_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j55_u0uchad_firstempty_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j100_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j100_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j165_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j165_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j130_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j130_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j35_u0uchad_empty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j35_u0uchad_empty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j35_u0uchad_firstempty_LArNoiseBurst";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j35_u0uchad_firstempty_LArNoiseBurst =  m_trigDec->isPassed( mychain );
  }
  mychain = "EF_j80_u0uchad_LArNoiseBurstT";
  if( ! m_trigDec->getListOfTriggers(mychain).empty()){
    m_nt_EF_j80_u0uchad_LArNoiseBurstT =  m_trigDec->isPassed( mychain );
  }
 
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doEventProperties        //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doEventProperties(){
  ATH_MSG_DEBUG ("in doEventProperties ");


  //////////////////////////////// EventInfo variables /////////////////////////////////////////////////
  // Retrieve event info
  const xAOD::EventInfo* eventInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve(eventInfo) );

  int run = eventInfo->runNumber();
  m_nt_run        = eventInfo->runNumber();
  m_nt_evtId      = eventInfo->eventNumber();
  m_nt_evtTime    = eventInfo->timeStamp();
  m_nt_evtTime_ns = eventInfo->timeStampNSOffset();
  m_nt_lb         = eventInfo->lumiBlock();
  m_nt_bcid       = eventInfo->bcid();

  ATH_MSG_DEBUG("Run Number: "<<run<<", event Id "<<m_nt_evtId<<", bcid = "<<m_nt_bcid);
  
  SG::ReadCondHandle<BunchCrossingCondData> bccd (m_bcDataKey);
  const BunchCrossingCondData* bunchCrossing=*bccd;
  if (!bunchCrossing) {
    ATH_MSG_ERROR("Failed to retrieve Bunch Crossing obj");
    return StatusCode::FAILURE;
  }

  m_nt_isbcidFilled = bunchCrossing->isFilled(m_nt_bcid);
  m_nt_isbcidInTrain = bunchCrossing->isInTrain(m_nt_bcid);
  m_nt_bunchtype = bunchCrossing->bcType(m_nt_bcid);
  ATH_MSG_DEBUG("BCID is Filled: "<<m_nt_isbcidFilled);
  ATH_MSG_DEBUG("BCID is in Train: "<<m_nt_isbcidInTrain);
  ATH_MSG_DEBUG("bunch type "<<m_nt_bunchtype);

  unsigned int distFromFront = bunchCrossing->distanceFromFront(m_nt_bcid,BunchCrossingCondData::BunchCrossings);
  if(m_frontbunches < distFromFront) distFromFront=m_frontbunches;

  bool checkfirstbunch = true;
  for(unsigned int i=1;i<=distFromFront;i++){
     bool isFilled=bunchCrossing->isFilled(m_nt_bcid-i);
     ATH_MSG_DEBUG("bunch "<<i<<" is Filled "<<isFilled);
     m_nt_isBunchesInFront.push_back(isFilled);
       if(isFilled){
         if(i!=1){
           if(checkfirstbunch){
             float time = 25.0*i;
             m_nt_bunchtime = time;
             ATH_MSG_DEBUG("next bunch time: "<<time<<" ns ");
             checkfirstbunch = false;
	   }
	 }
     }
  }

  m_CosmicCaloStream = false;
  //std::vector<TriggerInfo::StreamTag>::const_iterator streamInfoIt=myTriggerInfo->streamTags().begin();
  //std::vector<TriggerInfo::StreamTag>::const_iterator streamInfoIt_e=myTriggerInfo->streamTags().end();
  //for (;streamInfoIt!=streamInfoIt_e;streamInfoIt++) { 
  for (const auto& streamInfo : eventInfo->streamTags()) {
    const std::string& stream_name = streamInfo.name();
    const std::string& stream_type = streamInfo.type();
    m_nt_streamTagName.push_back(stream_name);
    m_nt_streamTagType.push_back(stream_type);
    ATH_MSG_DEBUG("event stream tag name "<<streamInfo.name());
    ATH_MSG_DEBUG("event stream tag type "<<streamInfo.type());
    if(streamInfo.name()=="CosmicCalo" && streamInfo.type()=="physics"){
      m_CosmicCaloStream = true;
    }
  }
  

  ATH_MSG_DEBUG("CosmicCalo stream value: "<<m_CosmicCaloStream);

  // Retrieve output of LArNoisyRO
  bool larnoisyro = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::BADFEBS);
  bool larnoisyro_opt =eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::BADFEBS_W);
  bool larnoisyro_satTwo = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::TIGHTSATURATEDQ);
  bool larmnbnoisy = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::MININOISEBURSTLOOSE);
  bool larmnbnoisy_sat = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::MININOISEBURSTTIGHT);
  m_nt_larnoisyro        = larnoisyro ? 1 : 0;
  m_nt_larnoisyro_opt    = larnoisyro_opt ? 1 : 0;
  m_nt_larnoisyro_satTwo = larnoisyro_satTwo ? 1 : 0;
  m_nt_larmnbnoisy       = larmnbnoisy ? 1 : 0;
  m_nt_larmnbnoisy_sat   = larmnbnoisy_sat ? 1 : 0;
  
 // Retrieve output of EventInfo veto - COMMENTED NOW TO MAKE IT COMPLIANT WITH xAOD::EventInfo
//  mLog << MSG::DEBUG <<"Background: MBTSBeamVeto "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MBTSBeamVeto)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: PixSPNonEmpty "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::PixSPNonEmpty)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: SCTSPNonEmpty "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::SCTSPNonEmpty)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: BCMBeamVeto "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::BCMBeamVeto)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: LUCIDBeamVeto "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::LUCIDBeamVeto)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: MBTSTimeDiffHalo "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MBTSTimeDiffHalo)<<endmsg; 
//  mLog << MSG::DEBUG <<"Background: MBTSTimeDiffCol "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MBTSTimeDiffCol)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: LArECTimeDiffHalo "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::LArECTimeDiffHalo)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: LArECTimeDiffCol "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::LArECTimeDiffCol)<<endmsg;  
//  mLog << MSG::DEBUG <<"Background: CSCTimeDiffHalo "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::CSCTimeDiffHalo)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: CSCTimeDiffCol "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::CSCTimeDiffCol)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: BCMTimeDiffHalo "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::BCMTimeDiffHalo)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: BCMTimeDiffCol "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::BCMTimeDiffCol)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: MuonTimmingCol "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MuonTimingCol)<<endmsg;
//  mLog << MSG::DEBUG <<"Background: MuonTimmingCosmic "<<eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MuonTimingCosmic)<<endmsg;
//
//  m_nt_veto_mbts      = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MBTSBeamVeto);
//  m_nt_veto_pixel     = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::PixSPNonEmpty);
//  m_nt_veto_sct       = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::SCTSPNonEmpty);
//  m_nt_veto_bcm       = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::BCMBeamVeto);
//  m_nt_veto_lucid     = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::LUCIDBeamVeto);
//
//  //more variables
//  m_nt_veto_mbtstdHalo = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MBTSTimeDiffHalo); 
//  m_nt_veto_mbtstdCol  = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MBTSTimeDiffCol);
//  m_nt_veto_lartdHalo  = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::LArECTimeDiffHalo);
//  m_nt_veto_lartdCol   = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::LArECTimeDiffCol);  
//  m_nt_veto_csctdHalo  = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::CSCTimeDiffHalo);
//  m_nt_veto_csctdCol   = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::CSCTimeDiffCol);
//  m_nt_veto_bcmtHalo   = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::BCMTimeDiffHalo);
//  m_nt_veto_bcmtCol    = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::BCMTimeDiffCol);
//  m_nt_veto_muontCol   = eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MuonTimingCol);
//  m_nt_veto_muontCosmic= eventInfo->isEventFlagBitSet(xAOD::EventInfo::Background,xAOD::EventInfo::MuonTimingCosmic);
 
   // LArEventInfo

  ATH_MSG_DEBUG ("NOISEBURSTVETO bit " << eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO) );
  ATH_MSG_DEBUG ("BADFEBS bit " << eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::BADFEBS));
  ATH_MSG_DEBUG ("TIGHTSATURATEDQ bit " << eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::TIGHTSATURATEDQ));

  m_nt_larflag_badFEBs = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::BADFEBS);
  m_nt_larflag_mediumSaturatedDQ = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::MEDIUMSATURATEDQ);
  m_nt_larflag_tightSaturatedDQ = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::TIGHTSATURATEDQ);
  m_nt_larflag_noiseBurstVeto = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::NOISEBURSTVETO);
  m_nt_larflag_dataCorrupted = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::DATACORRUPTED);
  m_nt_larflag_dataCorruptedVeto = eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,LArEventBitInfo::DATACORRUPTEDVETO);

  ///////////////////////////////////////end EventInfo variables/////////////////////////////////////////////////////////////////////////

  //const AthenaAttributeList* attrList(0);
  //ATH_CHECK( evtStore()->retrieve(attrList, "/TDAQ/RunCtrl/DataTakingMode") );
  //if (attrList != 0) {
  //  ATH_MSG_DEBUG ("ReadyForPhysics is: " << (*attrList)["ReadyForPhysics"].data<uint32_t>());
  //  //m_valueCache = ((*attrList)["ReadyForPhysics"].data<uint32_t>() != 0);
  //  m_nt_atlasready = (*attrList)["ReadyForPhysics"].data<uint32_t>();
  // }

   /*const AthenaAttributeList* fillstate(0);
  sc =  evtStore()->retrieve(fillstate, "/LHC/DCS/FILLSTATE");
  if (sc.isFailure()) {
     ATH_MSG_WARNING ("Unable to retrieve fillstate information; falling back to" );
     return StatusCode::SUCCESS;
   }
   if (fillstate != 0) {
     ATH_MSG_DEBUG ("Stable beams is: " << (*fillstate)["StableBeams"].data<uint32_t>());
     //m_valueCache = ((*attrList)["ReadyForPhysics"].data<uint32_t>() != 0);
     m_nt_stablebeams.push_back((*fillstate)["StableBeams"].data<uint32_t>());
     }*/

  
  // 29/11/10 : Debug messages removed by BT 
  //   mLog << MSG::INFO << "Event LAr flags " << std::hex
  //      << eventInfo->errorState(xAOD::EventInfo::LAr) << " "
  //      << std::hex << eventInfo->eventFlags(xAOD::EventInfo::LAr)
  //      << ", bit 0: " << eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,0)
  //      << ", bit 1: " << eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,1)
  //      << ", bit 2: " << eventInfo->isEventFlagBitSet(xAOD::EventInfo::LAr,2)
  //      << endmsg;

  // Retrieve LArCollision Timing information
  const LArCollisionTime *  larTime=0;
  if (evtStore()->contains<LArCollisionTime>("LArCollisionTime")) {
     StatusCode sc =  evtStore()->retrieve(larTime,"LArCollisionTime");
     if( sc.isFailure()){
       ATH_MSG_WARNING  ( "Unable to retrieve LArCollisionTime event store" );
       //return StatusCode::SUCCESS; // Check if failure shd be returned. VB
     }else {
       ATH_MSG_DEBUG  ( "LArCollisionTime successfully retrieved from event store" );
     }
  }
  
  if (larTime) {
      m_nt_nCellA = larTime->ncellA();
      m_nt_nCellC = larTime->ncellC();
    if(m_nt_nCellA>0 && m_nt_nCellC>0){
      // Calculate the time diff between ECC and ECA
      m_nt_ECTimeDiff = larTime->timeC() - larTime->timeA();
      m_nt_ECTimeAvg  = (larTime->timeC() + larTime->timeA()) * 0.5;
      ATH_MSG_DEBUG  ( "LAr: Calculated time difference of " << m_nt_ECTimeDiff << " ns" );
    }
  }

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doLArNoiseBursts        ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doLArNoiseBursts(){
  ATH_MSG_DEBUG ( "in doLarCellInfo " );
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadCondHandle<LArOnOffIdMapping> larCablingHdl (m_cablingKey, ctx);
  const LArOnOffIdMapping* cabling=*larCablingHdl;
  if(!cabling) {
     ATH_MSG_WARNING("Do not have cabling info, not storing LarCellInfo ");
     return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<LArBadChannelCont> bcHdl (m_BCKey, ctx);
  const LArBadChannelCont* bcCont=*bcHdl;
  if(!bcCont) {
     ATH_MSG_WARNING("Do not have bad chan info, not storing LarCellInfo ");
     return StatusCode::SUCCESS;
  }

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  ATH_CHECK(caloMgrHandle.isValid());
  const CaloDetDescrManager* caloMgr = *caloMgrHandle;

 // Retrieve LAr calocells container
 // or LArRawChannel container, whatsever available...
  const CaloCellContainer* caloTES=0;
  const LArRawChannelContainer* LArTES=0;
  const LArRawChannelContainer* LArTES_dig=0;
  if(evtStore()->contains<CaloCellContainer>("AllCalo")) {
     ATH_CHECK(evtStore()->retrieve( caloTES, "AllCalo"));
  } else if (evtStore()->contains<LArRawChannelContainer>("LArRawChannels") || evtStore()->contains<LArRawChannelContainer>("LArRawChannels_fB")) {
         if (evtStore()->contains<LArRawChannelContainer>("LArRawChannels")) {
            ATH_CHECK(evtStore()->retrieve( LArTES_dig, "LArRawChannels"));
         }
         if (evtStore()->contains<LArRawChannelContainer>("LArRawChannels_fB")) {
            ATH_CHECK(evtStore()->retrieve( LArTES, "LArRawChannels_fB"));
         }
  }

  if (caloTES==0 && LArTES==0) {
    ATH_MSG_WARNING ( "Neither CaloCell nor LArRawChannel Containers found!" );
    return StatusCode::SUCCESS;
  }

  m_nb_sat = 0;
  m_noisycell =0;
  nlarcell = 0;
  v_ft_noisy.clear();v_slot_noisy.clear();v_channel_noisy.clear();
  v_isbarrel.clear();v_isendcap.clear();v_isfcal.clear();v_ishec.clear();
  v_layer.clear();v_partition.clear();v_energycell.clear();v_qfactorcell.clear(); 
  v_phicell.clear();v_etacell.clear();v_signifcell.clear();v_isbadcell.clear();
  v_IdHash.clear();v_noisycellHVeta.clear();v_noisycellHVphi.clear();
  v_cellpartlayerindex.clear();v_cellIdentifier.clear();

  float eCalo;
  float qfactor;
  CaloGain::CaloGain gain;

  SG::ReadCondHandle<CaloNoise> totalNoise (m_totalNoiseKey, ctx);

  if(caloTES) {
     for (const CaloCell* cell : *caloTES) {
       const CaloDetDescrElement* caloDDE = cell->caloDDE();
       if (caloDDE->is_tile())continue;
       HWIdentifier onlID;
       try {onlID = cabling->createSignalChannelID(cell->ID());}
       catch(LArID_Exception& except) {
         ATH_MSG_ERROR  ( "LArID_Exception " << m_LArEM_IDHelper->show_to_string(cell->ID()) << " " << (std::string) except );
         ATH_MSG_ERROR  ( "LArID_Exception " << m_LArHEC_IDHelper->show_to_string(cell->ID()) );
         ATH_MSG_ERROR  ( "LArID_Exception " << m_LArFCAL_IDHelper->show_to_string(cell->ID()) );
         continue;
       }
       bool connected = cabling->isOnlineConnected(onlID);
       if(!connected) continue;
       eCalo = cell->energy();
       qfactor = cell->quality();
       gain = cell->gain();
       //if(qfactor > 0. || cell->ID() == Identifier((IDENTIFIER_TYPE)0x33c9500000000000) ) ATH_MSG_DEBUG(cell->ID()<<" : "<<eCalo<<" "<<qfactor<<" "<<gain<<" prov.: "<<cell->provenance());
       ATH_CHECK(fillCell(onlID, eCalo, qfactor, gain, cabling, bcCont, **totalNoise,caloMgr));
     }//loop over cells
     ATH_MSG_DEBUG("Done cells "<<nlarcell);
  } else {
     std::vector<HWIdentifier> chdone; 
     if (LArTES_dig) {
       LArRawChannelContainer::const_iterator caloItr;
       for(caloItr=LArTES_dig->begin();caloItr!=LArTES_dig->end();++caloItr){
         HWIdentifier onlID=caloItr->identify();
         bool connected = cabling->isOnlineConnected(onlID);
         if(!connected) continue;
         eCalo = caloItr->energy();
         qfactor = caloItr->quality();
         gain = caloItr->gain();
         //if(qfactor>0 || cabling->cnvToIdentifier((*caloItr).identify()) == Identifier((IDENTIFIER_TYPE)0x33c9500000000000) ) ATH_MSG_DEBUG(cabling->cnvToIdentifier((*caloItr).identify())<<" : "<<eCalo<<" "<<qfactor<<" "<<gain);
         ATH_CHECK(fillCell(onlID, eCalo, qfactor, gain, cabling, bcCont, **totalNoise,caloMgr));
         chdone.push_back(onlID);
       }//loop over raw channels
     }  
     ATH_MSG_DEBUG("Done raw chan. from digi "<<nlarcell);
     // dirty hack, if we are already complete:
     if (nlarcell < 182468 && LArTES) { // add those raw channels, which were not from dig.
       LArRawChannelContainer::const_iterator caloItr;
       for(caloItr=LArTES->begin();caloItr!=LArTES->end();++caloItr){
         HWIdentifier onlID=caloItr->identify();
         if(std::find(chdone.begin(), chdone.end(), onlID) != chdone.end()) continue;
         bool connected = cabling->isOnlineConnected(onlID);
         if(!connected) continue;
         eCalo = caloItr->energy();
         qfactor = caloItr->quality();
         gain = caloItr->gain();
         //if(qfactor>0 || cabling->cnvToIdentifier((*caloItr).identify()) == Identifier((IDENTIFIER_TYPE)0x33c9500000000000)  ) ATH_MSG_DEBUG(cabling->cnvToIdentifier((*caloItr).identify())<<" : "<<eCalo<<" "<<qfactor<<" "<<gain);
         ATH_CHECK(fillCell(onlID, eCalo, qfactor, gain, cabling, bcCont, **totalNoise,caloMgr));
       }//loop over raw channels
     }  
     ATH_MSG_DEBUG("Done raw chan. "<<nlarcell);
  } // if raw chanels
  
  m_nt_larcellsize = nlarcell;
  if(caloTES) {
    m_nt_cellsize    = caloTES->size();
  }
  else if (LArTES) {
     m_nt_cellsize = LArTES->size();
  }
  ATH_MSG_DEBUG ("lar cell size = "<<int(nlarcell));
  if(caloTES) ATH_MSG_DEBUG ("all cell size = "<<int(caloTES->size()));
  else {
     if(LArTES_dig) ATH_MSG_DEBUG ("all LArRawCh. from digi size = "<<int(LArTES_dig->size()));
     if(LArTES) ATH_MSG_DEBUG ("all LArRawCh. size = "<<int(LArTES->size()));
  }

  //if (nlarcell > 0)
  //  m_nt_noisycellpercent = 100.0*double(n_noisycell)/double(nlarcell);
  //else
  //  m_nt_noisycellpercent = 0;
 
  bool checknoise = false;
  //ratio of noisy cells per partition
  for(unsigned int i=0;i<8;i++){
    float noise  =  100.0*(double(n_noisy_cell_part[i])/double(n_cell_part[i]));
    m_nt_noisycellpart.push_back(noise);
    if(noise> 1.0){
      checknoise = true;
      ATH_MSG_DEBUG ("noise burst in this  event ");
    }   
  }

  const LArDigitContainer* LArDigitCont=0;
  if (evtStore()->contains<LArDigitContainer>("FREE")) ATH_CHECK(evtStore()->retrieve( LArDigitCont, "FREE"));
  if(!LArDigitCont) {
     if (evtStore()->contains<LArDigitContainer>("LArDigitContainer_Thinned")) ATH_CHECK(evtStore()->retrieve( LArDigitCont, "LArDigitContainer_Thinned"));
  }
  if (!LArDigitCont) {
    ATH_MSG_WARNING ( "LArDigitContainer Container not found!" );
    return StatusCode::SUCCESS;
  }    

  bool store_condition = false;
  // CosmicCalo stream : Store detailed infos of cells only if Y3Sigma>1% or burst found by LArNoisyRO
  if(m_CosmicCaloStream){
    if(checknoise==true || m_nt_larnoisyro==1 || m_nt_larnoisyro_satTwo==1){
      store_condition = true;
    }
  }
  // Not cosmicCalo stream : Store detailed infos of cells only if burst found by LArNoisyRO
  if(!m_CosmicCaloStream){
    if(m_nt_larnoisyro==1 || m_nt_larnoisyro_satTwo==1){
      store_condition = true;
    }
  }

  //store the information of the noisy cell only when %noisycells > 1%.
  if(store_condition){
    std::vector<short> samples;
    samples.clear();
    for(unsigned int k=0;k<v_etacell.size();k++){
      m_nt_samples.push_back(samples);
      m_nt_gain.push_back(0);
    }
    for (const LArDigit* pLArDigit : *LArDigitCont) {
         HWIdentifier id2 = pLArDigit->hardwareID();
         IdentifierHash hashid2 = m_LArOnlineIDHelper->channel_Hash(id2);
          for(unsigned int j=0;j<v_IdHash.size();j++){
            if (hashid2 == v_IdHash[j] ){
              ATH_MSG_DEBUG ( "find a IdentifierHash of the noisy cell in LArDigit container " );
              samples = pLArDigit->samples();
              int gain=-1;
              if (pLArDigit->gain() == CaloGain::LARHIGHGAIN)   gain = 0;
              if (pLArDigit->gain() == CaloGain::LARMEDIUMGAIN) gain = 1;
              if (pLArDigit->gain() == CaloGain::LARLOWGAIN)    gain = 2;
              m_nt_gain.at(j)= gain;
              m_nt_samples.at(j) = samples;
              ATH_MSG_DEBUG ( "I got the samples!" );
              break;
	    }  
	 }
     }
    for(unsigned int i=0;i<v_etacell.size();i++){
      m_nt_energycell.push_back( v_energycell[i]);
      m_nt_qfactorcell.push_back( v_qfactorcell[i]);
      m_nt_signifcell.push_back( v_signifcell[i]);
      m_nt_partition.push_back( v_partition[i]);   
      m_nt_cellIdentifier.push_back(v_cellIdentifier[i].get_identifier32().get_compact());
      if(!m_keepOnlyCellID){
        m_nt_ft_noisy.push_back( v_ft_noisy[i]);
        m_nt_slot_noisy.push_back( v_slot_noisy[i]);
        m_nt_channel_noisy.push_back( v_channel_noisy[i]);
   
        /*
        m_nt_isbarrel.push_back( v_isbarrel[i]);
        m_nt_isendcap.push_back( v_isendcap[i]);
        m_nt_isfcal.push_back( v_isfcal[i]);
        m_nt_ishec.push_back( v_ishec[i]);
        */
   
        m_nt_layer.push_back( v_layer[i]);
        m_nt_phicell.push_back( v_phicell[i]);
        m_nt_etacell.push_back( v_etacell[i]);
        m_nt_isbadcell.push_back( v_isbadcell[i]);
        m_nt_noisycellHVphi.push_back(v_noisycellHVphi[i]);     
        m_nt_noisycellHVeta.push_back(v_noisycellHVeta[i]);
        m_nt_cellpartlayerindex.push_back(v_cellpartlayerindex[i]);
      }
    }		  
    ATH_MSG_DEBUG ("leaving if checknoise and larnoisyro");

  }//if(checknoisy==true ..)

  return StatusCode::SUCCESS;
  
}

StatusCode LArNoiseBursts::fillCell(HWIdentifier onlID
				    , float eCalo
				    , float qfactor
				    , CaloGain::CaloGain gain
				    , const LArOnOffIdMapping* cabling
				    , const LArBadChannelCont* bcCont
				    , const CaloNoise& totalNoise
				    , const CaloDetDescrManager* caloMgr)
{
    const Identifier idd = cabling->cnvToIdentifier(onlID);
    nlarcell++;
    IdentifierHash channelHash = m_LArOnlineIDHelper->channel_Hash(onlID);
    const CaloDetDescrElement *caloDDE = caloMgr->get_element(idd);
    int layer = caloDDE->getLayer();
    //    CaloCell_ID::CaloSample sampling = (*caloItr)->caloDDE()->getSampling();
    float phi = caloDDE->phi();
    float noise  = totalNoise.getNoise (idd, gain);
    float significance = eCalo / noise ;
    float eta = caloDDE->eta();
    bool badcell = ! (bcCont->status(onlID)).good();
    unsigned int partition = 0;
    bool is_lar_em_barrel = caloDDE->is_lar_em_barrel();
    if(is_lar_em_barrel){
       if(eta<0){
          partition = 0;
        }else{
          partition = 1;
        }
    }//barrel
    bool is_lar_em_endcap = caloDDE->is_lar_em_endcap();
    if(is_lar_em_endcap){
        if(eta<0){
          partition = 2;
        }else{
          partition = 3;
	}
    }//endcap
    bool is_lar_fcal   = caloDDE->is_lar_fcal();
    if(is_lar_fcal){
        if(eta<0){
          partition = 4;
        }else{
          partition = 5;
	}
    }//fcal
    bool is_lar_hec    = caloDDE->is_lar_hec();
    if(is_lar_hec){
      if(eta<0){
        partition = 6;
      }else{
        partition = 7;
      }
    }//hec
    for(unsigned int k=0;k<8;k++){
       if(partition==k){
          n_cell_part[k]++;
       }
    }
    if(qfactor <1000)                  {m_lowqfactor++;}
    if(qfactor>=1000 && qfactor<10000) {m_medqfactor++;}
    if(qfactor>=10000 && qfactor<65535){m_hiqfactor++;}
    if(qfactor==65535){
      ATH_MSG_DEBUG ("Satured cell at eta: "<<eta<<", phi: "<<phi<<", energy: "<<eCalo<<", partition: "<<partition);
       m_nb_sat = m_nb_sat +1;
       m_nt_partition_sat.push_back(partition);
       m_nt_energy_sat.push_back(eCalo);
       m_nt_cellIdentifier_sat.push_back((cabling->cnvToIdentifier(onlID)).get_identifier32().get_compact());
       if(!m_keepOnlyCellID){
         m_nt_barrelec_sat.push_back(m_LArOnlineIDHelper->barrel_ec(onlID));
         m_nt_posneg_sat.push_back(m_LArOnlineIDHelper->pos_neg(onlID));
         m_nt_ft_sat.push_back(m_LArOnlineIDHelper->feedthrough(onlID));
         m_nt_slot_sat.push_back(m_LArOnlineIDHelper->slot(onlID));
         m_nt_channel_sat.push_back(m_LArOnlineIDHelper->channel(onlID));
         m_nt_phicell_sat.push_back(phi);
         m_nt_etacell_sat.push_back(eta);
         m_nt_layer_sat.push_back(layer);
         m_nt_isbadcell_sat.push_back(badcell);
       }
    }
    // Store all cells in positive and negative 3 sigma tails...
    if(significance > m_sigmacut || qfactor > 4000){
      v_ft_noisy.push_back(m_LArOnlineIDHelper->feedthrough(onlID));
      v_slot_noisy.push_back(m_LArOnlineIDHelper->slot(onlID));
      v_channel_noisy.push_back(m_LArOnlineIDHelper->channel(onlID));

      /*
      v_isbarrel.push_back(is_lar_em_barrel);
      v_isendcap.push_back(is_lar_em_endcap);
      v_isfcal.push_back(is_lar_fcal);
      v_ishec.push_back(is_lar_hec);
      */

      v_layer.push_back(layer);
      v_energycell.push_back(eCalo);
      v_qfactorcell.push_back(qfactor);
      v_phicell.push_back(phi);
      v_etacell.push_back(eta);
      v_signifcell.push_back(significance);
      v_isbadcell.push_back(badcell);
      v_partition.push_back(partition);
      v_IdHash.push_back(channelHash);
      v_cellIdentifier.push_back(cabling->cnvToIdentifier(onlID));
    // ...but count only cells in positive 3 sigma tails!
      if (significance > m_sigmacut){
	m_noisycell++;
        ATH_MSG_DEBUG ("Noisy cell: "<<m_noisycell<<" "<<cabling->cnvToIdentifier(onlID)<<" q: "<<qfactor<<" sign.: "<<significance<<" noise: "<<noise);
	for(unsigned int k=0;k<8;k++){
	  if(partition==k){
	    n_noisy_cell_part[k]++;
	  }
	}
      }
      int caloindex = GetPartitionLayerIndex(idd);
      v_cellpartlayerindex.push_back(caloindex);
      v_noisycellHVphi.push_back(m_LArElectrodeIDHelper->hv_phi(onlID));
      v_noisycellHVeta.push_back(m_LArElectrodeIDHelper->hv_eta(onlID));

      
    }   
    return StatusCode::SUCCESS;
}    
//////////////////////////////////////////////////////////////////////////////////////
///////////////////          doPhysicsObjects        ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
StatusCode LArNoiseBursts::doPhysicsObjects(){
  ATH_MSG_DEBUG ("in doPhysicsObjects ");

//  const ElectronContainer* elecTES = 0;
//  StatusCode sc=evtStore()->retrieve( elecTES, m_elecContainerName);
//  if( sc.isFailure()  ||  !elecTES ) {
//    ATH_MSG_WARNING ( "No ESD electron container found in StoreGate" );
//    return StatusCode::SUCCESS;
//  }
//  ATH_MSG_DEBUG ( "ElectronContainer successfully retrieved. Size = " << elecTES->size() );

  return StatusCode::SUCCESS;
}


//

int LArNoiseBursts::GetPartitionLayerIndex(const Identifier& id)
{
  // O.Simard -- GetPartitionLayer index [0,32]:
  // Returns a single integer code that corresponds to the
  // mapping of CaloCells in CaloMonitoring histogram frames.

  int caloindex = -1;
  int bc = 0;
  int sampling = 0;

  if(m_LArEM_IDHelper->is_lar_em(id)){ // EMB
    bc = m_LArEM_IDHelper->barrel_ec(id);
    sampling = m_LArEM_IDHelper->sampling(id);
    if(abs(bc)==1){
      if(bc<0) caloindex = sampling+4;
      else caloindex = sampling;
    } else { // EMEC
      if(bc<0) caloindex = sampling+12;
      else caloindex = sampling+8;
    }
  } else if(m_LArHEC_IDHelper->is_lar_hec(id)) { // LAr HEC
    bc = m_LArHEC_IDHelper->pos_neg(id);
    sampling = m_LArHEC_IDHelper->sampling(id);
    if(bc<0) caloindex = sampling+20;
    else caloindex = sampling+16;
  } else if(m_LArFCAL_IDHelper->is_lar_fcal(id)) { // LAr FCAL
    bc = m_LArFCAL_IDHelper->pos_neg(id);
    sampling = (int)m_LArFCAL_IDHelper->module(id); // module instead of sampling
    if(bc<0) caloindex = sampling+26;
    else caloindex = sampling+23;
  }

  return caloindex;
}

