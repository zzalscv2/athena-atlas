/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// STL
#include <sstream>
#include <fstream>
#include <vector>

// TGC
#include "TrigT1TGC/LVL1TGCTrigger.h"
#include "TrigT1TGC/TGCASDOut.h"
#include "TrigT1TGC/TGCEvent.h"
#include "TrigT1TGC/TGCReadoutIndex.h"
#include "TrigT1TGC/TGCTrackSelectorOut.h"
#include "TrigT1TGC/TGCElectronicsSystem.h"
#include "TrigT1TGC/TGCTimingManager.h"
#include "TrigT1TGC/TGCDatabaseManager.h"
#include "TrigT1TGC/TGCSector.h"
#include "TrigT1TGC/TGCSectorLogic.h"
#include "TrigT1TGC/TGCNumbering.h"
#include "TrigT1TGC/TrigT1TGC_ClassDEF.h"
#include "TrigT1TGC/TGCNSW.h"
#include "TrigT1TGC/NSWTrigOut.h"
#include "TrigT1TGC/TGCBIS78.h"
#include "TrigT1TGC/BIS78TrigOut.h"

// Other stuff
#include "TrigConfL1Data/TriggerThreshold.h"
#include "TrigConfL1Data/ThresholdConfig.h"
#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"
#include "TrigT1Interfaces/Lvl1MuEndcapSectorLogicDataPhase1.h"
#include "TrigT1Interfaces/Lvl1MuForwardSectorLogicDataPhase1.h"

// MuonSpectrometer
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonDigitContainer/TgcDigitCollection.h"
#include "MuonDigitContainer/TgcDigit.h"

#include "TGCcablingInterface/ITGCcablingSvc.h"
#include "TGCcablingInterface/ITGCcablingServerSvc.h"
#include "PathResolver/PathResolver.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

// DetMask stuff
#include "eformat/DetectorMask.h"
#include "eformat/SourceIdentifier.h"

namespace LVL1TGCTrigger {

LVL1TGCTrigger::LVL1TGCTrigger(const std::string& name, ISvcLocator* pSvcLocator)
: AthAlgorithm(name,pSvcLocator),
  m_cabling(0),
  m_db(0),
  m_nEventInSector(0),
  m_innerTrackletSlotHolder( tgcArgs() ),
  m_debuglevel(false)
{}

////////////////////////////////////////////////////////////
LVL1TGCTrigger::~LVL1TGCTrigger()
{
  ATH_MSG_DEBUG("LVL1TGCTrigger destructor called");
  if (m_db) {
    delete m_db;
    m_db =0;
  }
}

////////////////////////////////////////////////////////////
StatusCode LVL1TGCTrigger::initialize()
{
    ATH_MSG_DEBUG("LVL1TGCTrigger::initialize()");

    m_debuglevel = (msgLevel() <= MSG::DEBUG); // save if threshold for debug

    m_tgcArgs.set_MSGLEVEL(msgLevel());
    m_tgcArgs.set_SHPT_ORED( m_SHPTORED.value() );
    m_tgcArgs.set_USE_INNER( m_USEINNER.value() );
    m_tgcArgs.set_INNER_VETO( m_INNERVETO.value() );
    m_tgcArgs.set_TILE_MU( m_TILEMU.value() );
    m_tgcArgs.set_USE_NSW( m_USENSW.value() );
    m_tgcArgs.set_USE_BIS78( m_USEBIS78.value() );
    m_tgcArgs.set_FORCE_NSW_COIN( m_FORCENSWCOIN.value() );

    m_tgcArgs.set_USE_CONDDB( m_USE_CONDDB.value() );
    m_tgcArgs.set_useRun3Config( m_useRun3Config.value() );

    m_tgcArgs.set_NSWSideInfo( m_NSWSideInfo.value() );

    ATH_CHECK( m_readCondKey.initialize(!m_useRun3Config.value()) );
    ATH_CHECK( m_readLUTs_CondKey.initialize(m_useRun3Config.value()) );

    // CondDB is not available for Run3 config. set USE_CONDDB to false to avoid errors.
    // will be removed the below part.
    if(m_useRun3Config.value()){
      m_tgcArgs.set_USE_CONDDB(false);
    }

    // initialize TGCDataBase
    m_db = new TGCDatabaseManager(&m_tgcArgs, m_readCondKey, m_readLUTs_CondKey);

    // initialize the TGCcabling
    ATH_CHECK(getCabling());

    // read and write handle key
    ATH_CHECK(m_keyTgcRdoIn.initialize());
    ATH_CHECK(m_keyTgcDigit.initialize());
    ATH_CHECK(m_keyTileMu.initialize());
    ATH_CHECK(m_keyNSWTrigOut.initialize(tgcArgs()->USE_NSW())); // to be updated once the Run 3 CondDb becomes available (should be automatically configured by db info)
    ATH_CHECK(m_keyBIS78TrigOut.initialize(tgcArgs()->USE_BIS78())); // to be updated as well
    ATH_CHECK(m_muctpiPhase1Key.initialize(tgcArgs()->useRun3Config()));
    ATH_CHECK(m_keyTgcRdo.initialize(tgcArgs()->useRun3Config()));
    ATH_CHECK(m_bsMetaDataContRHKey.initialize(SG::AllowEmpty));

    // clear mask channel map
    m_MaskedChannel.clear();

    return StatusCode::SUCCESS;
}

////////////////////////////////////////////////
StatusCode LVL1TGCTrigger::finalize()
{
    ATH_MSG_DEBUG("LVL1TGCTrigger::finalize() called" << " m_nEventInSector = " << m_nEventInSector);
    
    if (m_db) delete m_db;
    m_db = 0 ;

    return StatusCode::SUCCESS;
}

////////////////////////////////////////////
StatusCode LVL1TGCTrigger::execute()
{
    ATH_MSG_DEBUG("execute() called");
    const EventContext& ctx = getContext();
    
    if(!m_cabling) {
      // get cabling svc
      if(getCabling().isFailure()) return StatusCode::FAILURE;
    }
    
    // doMaskOperation is performed at the first event
    // It is better to implement callback against
    // MuonTGC_CablingSvc::updateCableASDToPP (Susumu Oda, 2010/10/27)
    if(m_firstTime) {
      // do mask operation
      if(getMaskedChannel().isFailure()) return StatusCode::FAILURE;
      m_firstTime = false;
    }
    
    // Tile-Muon data
    bool doTileMu = m_tgcArgs.TILE_MU();

    if (doTileMu && !m_tgcArgs.useRun3Config()) {   // for Run-2
      SG::ReadCondHandle<TGCTriggerData> readHandle{m_readCondKey, ctx};
      const TGCTriggerData* readCdo{*readHandle};
      doTileMu = readCdo->isActive(TGCTriggerData::CW_TILE);
    }

    // NSW data
    bool doNSW = m_tgcArgs.USE_NSW();

    // BIS78 data
    bool doBIS78 = m_tgcArgs.USE_BIS78();

    // TgcRdo
    std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>  tgcrdo;
    


    SG::ReadHandle<TgcDigitContainer> readTgcDigitContainer(m_keyTgcDigit, ctx);
    if(!readTgcDigitContainer.isValid()){
      ATH_MSG_ERROR("Cannot retrieve TgcDigitContainer");
      return StatusCode::FAILURE;
    }
    const TgcDigitContainer* tgc_container = readTgcDigitContainer.cptr();
    
    SG::WriteHandle<LVL1MUONIF::Lvl1MuCTPIInputPhase1> wh_muctpiTgc(m_muctpiPhase1Key, ctx);
    ATH_CHECK(wh_muctpiTgc.record(std::make_unique<LVL1MUONIF::Lvl1MuCTPIInputPhase1>()));
    LVL1MUONIF::Lvl1MuCTPIInputPhase1* muctpiinputPhase1 = wh_muctpiTgc.ptr();
    
    // process one by one
    StatusCode sc = StatusCode::SUCCESS;
    for (int bc=TgcDigit::BC_PREVIOUS; bc<=TgcDigit::BC_NEXTNEXT; bc++){
      sc = StatusCode::SUCCESS;
      
      // Use TileMu only if BC_CURRENT
      if (doTileMu && bc == m_CurrentBunchTag) {
        ATH_CHECK(m_system->getTMDB()->retrieve(m_keyTileMu));
      }

      // Use NSW trigger output 
      if(doNSW && bc==m_CurrentBunchTag){  // To implement BC-calculation
	ATH_CHECK(m_system->getNSW()->retrieve(m_keyNSWTrigOut));
      }

      // Use RPC BIS78 trigger output
      if(doBIS78 && bc == m_CurrentBunchTag){  // Todo: implement BC-calculation
	ATH_CHECK(m_system->getBIS78()->retrieve(m_keyBIS78TrigOut));
      }

      if (m_ProcessAllBunches || bc == m_CurrentBunchTag) {
        m_bctagInProcess = bc;
        sc = processOneBunch(tgc_container, muctpiinputPhase1, tgcrdo);
      }
      if (sc.isFailure()) {
        ATH_MSG_FATAL("Fail to process the bunch " << m_bctagInProcess);
        return sc;
      }
    }

    
    // before writing the output TgcRdo container,
    // read input TgcRdo and copy the tracklet etc.
    SG::ReadHandle<TgcRdoContainer> rdoContIn(m_keyTgcRdoIn);
    if(!rdoContIn.isValid()){
      ATH_MSG_WARNING("Cannot retrieve TgcRdoContainer with key=" << m_keyTgcRdoIn.key());
      return sc;
    }else if(rdoContIn->size()>0) {	
      TgcRdoContainer::const_iterator itR = rdoContIn->begin();
      for(; itR!=rdoContIn->end(); ++itR){
	const TgcRdo* rdoIn = (*itR);
        std::pair<int, int> subDetectorRod(rdoIn->subDetectorId(), rdoIn->rodId());
        std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>::iterator itRdo = tgcrdo.find(subDetectorRod);
        if (itRdo!=tgcrdo.end()) {  
          // if subDetectorId and rodId for input and output RDOs are the same,
          // copy the tracklet info etc. from input to the output TgcRdo
          for ( const TgcRawData* rd : *rdoIn ) {
            itRdo->second->push_back(std::make_unique<TgcRawData>(*rd));
          }
        }
      }
    }

    // write tgcL1rdo container
    SG::WriteHandle<TgcRdoContainer> tgcL1rdoHandle (m_keyTgcRdo, ctx);
    auto trgContainer=std::make_unique<TgcRdoContainer>();
    for(const auto& tgcRdoMap : tgcrdo){
      for(const auto rawData : *tgcRdoMap.second){
	trgContainer->push_back(rawData);
      }
    }
    ATH_CHECK(tgcL1rdoHandle.record(std::move(trgContainer)));
    
    return sc;
}

StatusCode LVL1TGCTrigger::processOneBunch(const TgcDigitContainer* tgc_container,
                                           LVL1MUONIF::Lvl1MuCTPIInputPhase1* muctpiinputPhase1,
					   std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&  tgcrdo)
{
    ATH_MSG_DEBUG("start processOneBunch: for BC=" << m_bctagInProcess);

    std::map<Identifier, int> tgcDigitIDs;
    std::map<Identifier, int>::iterator itCh;
    
    // doMaskOperation (masked & fired)
    doMaskOperation(tgc_container, tgcDigitIDs);
    
    // fill ASDOut to this event
    TGCEvent event;
    fillTGCEvent(tgcDigitIDs, event);
    tgcDigitIDs.clear();
    
    // process trigger electronics emulation...
    m_TimingManager->increaseBunchCounter();
    m_system->distributeSignal(&event);
    
    // EIFI trigger bits for SL are cleared.
    m_innerTrackletSlotHolder.clearTriggerBits();
    
    // PatchPanel, SlaveBoard
    for (LVL1TGC::TGCSide i : {LVL1TGC::TGCSide::ASIDE, LVL1TGC::TGCSide::CSIDE}) {
      for( int j=0; j<m_system->getNumberOfOctant(); j+=1){
        for( int k=0; k<m_system->getNumberOfModule(); k+=1){
          TGCSector* sector = m_system->getSector(i,j,k);
          if((sector!=0)&&(sector->hasHit())){
            m_nEventInSector++;
            m_TimingManager->startPatchPanel(sector, m_db);
            m_TimingManager->startSlaveBoard(sector);
            if (m_OutputTgcRDO.value()) recordRdoSLB(sector, tgcrdo);
            // EIFI trigger bits for SL are filled in this method.
          }
        }
      }
    }
    
    // HighPtBoard, SectorLogic
    const int muctpiBcId_offset = TgcDigit::BC_CURRENT;
    int muctpiBcId = m_bctagInProcess - muctpiBcId_offset;
    for(int i=0; i< LVL1TGC::kNSide; i++) {
      int sectoraddr_endcap = 0;
      int sectoraddr_forward = 0;
      for(int j=0; j<m_system->getNumberOfOctant(); j+=1){
        for(int k=0; k<m_system->getNumberOfModule(); k+=1){
          if(k>=9) continue;// skip Inner TGC
          TGCSector* sector = m_system->getSector(i,j,k);
          if(sector==0) continue;

          m_TimingManager->startHighPtBoard(sector);
          if (m_OutputTgcRDO.value()) recordRdoHPT(sector, tgcrdo);

          // EIFI trigger bits are checked if Endcap
          if(sector->getRegionType() == TGCRegionType::ENDCAP && sector->getSL()) {
            if((sector->hasHit())){
              // Pointers to store EIFI trigger bits for Endcap SL
              const TGCInnerTrackletSlot* innerTrackletSlots[TGCInnerTrackletSlotHolder::NUMBER_OF_SLOTS_PER_TRIGGER_SECTOR]
                = {0, 0, 0, 0};
              m_innerTrackletSlotHolder.getInnerTrackletSlots(i, j, k, innerTrackletSlots);
              sector->getSL()->setInnerTrackletSlots(innerTrackletSlots);
            }
          }
          m_TimingManager->startSectorLogic(sector);
          if(sector->hasHit()) sector->clearNumberOfHit();

          // Fill inner (EIFI/Tile) words
          if (m_OutputTgcRDO.value() && m_tgcArgs.USE_INNER()) recordRdoInner(sector, tgcrdo);

          // Fill Lvl1MuCTPInput
          if (m_OutputTgcRDO.value()) recordRdoSL(sector, tgcrdo);

          size_t tgcsystem=0,subsystem=0;
          if(i==0) subsystem = LVL1MUONIF::Lvl1MuCTPIInput::idSideA();
          if(i==1) subsystem = LVL1MUONIF::Lvl1MuCTPIInput::idSideC();

	  std::shared_ptr<TGCTrackSelectorOut>  trackSelectorOut;
	  sector->getSL()->getTrackSelectorOutput(trackSelectorOut);

	  std::shared_ptr<LVL1TGC::TGCNSW> nsw = m_system->getNSW();
	  int module = sector->getModuleId();
	  int sectorId;
          if(sector->getRegionType() == TGCRegionType::ENDCAP) {
            LVL1MUONIF::Lvl1MuEndcapSectorLogicDataPhase1 sldata;
            tgcsystem = LVL1MUONIF::Lvl1MuCTPIInputPhase1::idEndcapSystem();
            if(trackSelectorOut != 0) FillSectorLogicData(&sldata,trackSelectorOut.get());
	    
	    if ( m_tgcArgs.USE_NSW() ){
	      sectorId = ((module/3)*2+module%3) + sector->getOctantId()*6;
	      std::shared_ptr<const LVL1TGC::NSWTrigOut> pNSWOut = nsw->getOutput(sector->getRegionType(),
									 sector->getSideId(),
									 sectorId);
	      if ( pNSWOut ){
		// set monitoring flag
		for(bool NSWmonitor : pNSWOut->getNSWmonitor() ){
		  if ( NSWmonitor ) {
		    sldata.nsw(true);
		    break;
		  }
		}
	      }
	    }
            muctpiinputPhase1->setSectorLogicData(sldata,tgcsystem,subsystem,sectoraddr_endcap++,muctpiBcId);
          } else if(sector->getRegionType() == TGCRegionType::FORWARD) {
            LVL1MUONIF::Lvl1MuForwardSectorLogicDataPhase1 sldata;
            tgcsystem = LVL1MUONIF::Lvl1MuCTPIInputPhase1::idForwardSystem();
            if(trackSelectorOut != 0) FillSectorLogicData(&sldata,trackSelectorOut.get());

	    if ( m_tgcArgs.USE_NSW() ) {
	      sectorId =  (module/3) +  sector->getOctantId()*3;
	      std::shared_ptr<const LVL1TGC::NSWTrigOut> pNSWOut = nsw->getOutput(sector->getRegionType(),
									 sector->getSideId(),
									 sectorId);
	      if ( pNSWOut ){
		// set monitoring flag
		for(bool NSWmonitor : pNSWOut->getNSWmonitor() ){
		  if ( NSWmonitor ) {
		    sldata.nsw(true);
		    break;
		  }
		}
	      }
	    }
            muctpiinputPhase1->setSectorLogicData(sldata,tgcsystem,subsystem,sectoraddr_forward++,muctpiBcId);
          }

	  trackSelectorOut.get()->reset();


        } // k Module
      } // j Octant
    } // i Side
    
    event.Clear();
    
    return StatusCode::SUCCESS;
}
  
  
////////////////////////////////////////////////////////
void LVL1TGCTrigger::doMaskOperation(const TgcDigitContainer* tgc_container,
                                     std::map<Identifier, int>& TgcDigitIDs)
{
    // (1) skip masked channels
    for (const TgcDigitCollection* c : *tgc_container) {
      for (const TgcDigit* h : *c) {

        // check BCID
        if (h->bcTag()!=m_bctagInProcess) continue;

        Identifier channelId = h->identify();
        const auto itCh = m_MaskedChannel.find(channelId);
        if (itCh!=m_MaskedChannel.end() && itCh->second==0) {
          ATH_MSG_DEBUG("This channel is masked! offlineID=" << channelId);
          continue;
        }
        TgcDigitIDs.emplace(channelId, 1);
      }
    }

    // (2) add fired channels by force
    for(const auto& [Id, OnOff] : m_MaskedChannel) {
      if (OnOff==1) {
        ATH_MSG_VERBOSE("This channel is fired by force! offlineID=" << Id);
        TgcDigitIDs.emplace(Id, 1);
      }
    }

    ATH_MSG_DEBUG("# of total hits    " << TgcDigitIDs.size());

    return;
}
  
//////////////////////////////////////////////////
void  LVL1TGCTrigger::fillTGCEvent(const std::map<Identifier, int>& tgcDigitIDs, TGCEvent& event)
{
    // Loop on TGC detectors (collections)
    for(const auto& itCh : tgcDigitIDs) {
      const Identifier channelId = itCh.first;
      int subsystemNumber;
      int octantNumber;
      int moduleNumber;
      int layerNumber;
      int rNumber;
      int wireOrStrip;
      int channelNumber;
      bool status = m_cabling->getOnlineIDfromOfflineID(channelId,
                                                        subsystemNumber,
                                                        octantNumber,
                                                        moduleNumber,
                                                        layerNumber,
                                                        rNumber,
                                                        wireOrStrip,
                                                        channelNumber);

      if(!status) {
        ATH_MSG_INFO("Fail to getOnlineIDfromOfflineID for " << channelId);
      } else {
        bool fstatus;
        int subDetectorID, srodID, sswID, sbLoc, channelID;
        int phi=0;
        int moduleType=0;
        int slbID=0;
        bool  isAside=true;
        bool  isEndcap=true;

        fstatus = m_cabling->getReadoutIDfromOfflineID(channelId,
                                                       subDetectorID,
                                                       srodID,sswID,
                                                       sbLoc,channelID);

        if (fstatus) {
          fstatus = m_cabling->getSLBIDfromReadoutID(phi, isAside, isEndcap,
                                                     moduleType, slbID,
                                                     subDetectorID,
                                                     srodID, sswID,sbLoc);
        }
        if (fstatus) {
          ATH_MSG_VERBOSE("hit : subsys#=" << subsystemNumber
                          << " octant#=" << octantNumber
                          << " mod#=" << moduleNumber
                          << " layer#=" << layerNumber << " r#=" << rNumber
                          << " isStrip=" << wireOrStrip
                          << " ch#=" << channelNumber << endmsg
                          << " --> readoutID: sudetID=" << subDetectorID
                          << " srodID=" << srodID << " sswID=" << sswID
                          << " slbID=" << slbID << " chID=" << channelID);

          TGCZDirection zdire = (subsystemNumber==1)? kZ_FORWARD : kZ_BACKWARD;
          TGCReadoutIndex index(zdire,octantNumber,moduleNumber,rNumber,layerNumber);
          TGCSignalType signal = (wireOrStrip==1)? STRIP : WIRE;
          event.NewASDOut(index,
                          signal,
                          channelNumber,
                          0);
        } else {
          ATH_MSG_INFO("Fail to getSLBIDfromOfflineID for  " << channelId);
        }
      }
    }     // End Loop on TGC detectors (collections)
    if (m_debuglevel) {
      ATH_MSG_DEBUG("Could make TGCEvent with TgcDigitContainer."
                    << "  vector size : " << event.GetNASDOut() );
      for(int iout=1; iout<= event.GetNASDOut(); iout++){
        TGCASDOut* asdout = (event.GetASDOutVector()[iout-1]);
        ATH_MSG_DEBUG( " Z:" << asdout->GetTGCReadoutIndex().GetZDirection() <<
                       " O:" << asdout->GetTGCReadoutIndex().GetOctantNumber() <<
                       " M:" << asdout->GetTGCReadoutIndex().GetModuleNumber() <<
                       " R:" << asdout->GetTGCReadoutIndex().GetRNumber() <<
                       " L:" << asdout->GetTGCReadoutIndex().GetLayerNumber() <<
                       " S:" << asdout->GetSignalType() <<
                       " I:" << asdout->GetHitID() <<
                       " T:" << asdout->GetHitToF() );
      }
    }
}


////////////////////////////////////////////////////////
void LVL1TGCTrigger::FillSectorLogicData(LVL1MUONIF::Lvl1MuSectorLogicDataPhase1 *sldata,
                                         const TGCTrackSelectorOut *trackSelectorOut)
{
    // M.Aoki (26/10/2019)
    // this function will be updated for Run3-specific configuration such as quality flags, 15 thresholds
    if(trackSelectorOut ==0) return;

    sldata->clear2candidatesInSector();// for temporary

    const int muctpiBcId_offset = TgcDigit::BC_CURRENT;
    sldata->bcid(m_bctagInProcess - muctpiBcId_offset);

    for(int trackNumber=0;trackNumber!=trackSelectorOut->getNCandidate();trackNumber++){

      sldata->roi(trackNumber,((trackSelectorOut->getR(trackNumber))<<2)+(trackSelectorOut->getPhi(trackNumber)));
      sldata->pt(trackNumber,trackSelectorOut->getPtLevel(trackNumber));
      if (trackSelectorOut->getInnerVeto(trackNumber)) sldata->ovl(trackNumber,1);
      else                                             sldata->ovl(trackNumber,0);
      sldata->charge(trackNumber, trackSelectorOut->getCharge(trackNumber));
      sldata->bw2or3(trackNumber, trackSelectorOut->getCoincidenceType(trackNumber));
      sldata->goodmf(trackNumber, trackSelectorOut->getGoodMFFlag(trackNumber));
      sldata->innercoin(trackNumber, trackSelectorOut->getInnerCoincidenceFlag(trackNumber));
    }
    for(int trackNumber=0;trackNumber!=TGCTrackSelectorOut::NCandidateInTrackSelector;trackNumber++){
      sldata->set2candidates(trackNumber);// not used for TGC
      sldata->clear2candidates(trackNumber);// not used for TGC
    } 
}

//////////////////////////////////////////
void LVL1TGCTrigger::recordRdoSLB(TGCSector * sector, 
				  std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&  tgcrdo)
{
    uint16_t bcTag=m_CurrentBunchTag, l1Id=0, bcId=0;
    // readoutID
    int subDetectorId, rodId, sswId, sbLoc, secId, secIdEIFI;
    // SLBID
    bool isAside, isEndcap; int phi, moduleType, id, phiEIFI;
    isAside = (sector->getSideId()==0  ? 1 : 0);
    isEndcap = (sector->getRegionType() == TGCRegionType::ENDCAP ? 1 : 0);
    int module = sector->getModuleId();
    // OnlineID moduleNumber
    //       <---- phi ----
    // EC:   7 6 4 3 1 0   11 10  9
    // FWD:   8   5   2    14 13 12
    //       [M1, M2, M3]  [EI/FI]
    // secId=0-5(EC), 0-2(FWD) for TgcRawData
    secId = (isEndcap ? (module/3)*2+module%3 : module/3);
    // phi=1-48(EC), 1-24(FWD) in detector ID scheme
    phi = (isEndcap ? (secId+46+sector->getOctantId()*6)%48+1 : (secId+23+sector->getOctantId()*3)%24+1);
    // secIdEIFI=0-2
    secIdEIFI = module%3;
    // phiEIFI=1-24
    phiEIFI = (secIdEIFI+23+sector->getOctantId()*3)%24+1;

    // SLB
    const int NumberOfSLBType = 6;
    // 0:  WT,  1: WD,  2: ST,  3: SD,  4: WI  5:SI
    for(int itype=0; itype<NumberOfSLBType; itype++) {
      moduleType = getLPTTypeInRawData(itype);

      // loop over all SB of each type
      for(unsigned int index=0; index<sector->getNumberOfSB(itype); index++) {
        TGCSlaveBoard * slb = sector->getSB(itype, index);
        if (0==slb) continue;
        id = slb->getId();
        const TGCSlaveBoardOut * out = slb->getOutput();
        if (0==out) continue;

        bool isEIFI = (moduleType==TgcRawData::SLB_TYPE_INNER_WIRE ||
                       moduleType==TgcRawData::SLB_TYPE_INNER_STRIP);

        // get ReadoutID
        bool status =
          m_cabling->getReadoutIDfromSLBID((isEIFI ? phiEIFI : phi),
                                           isAside, isEndcap,
                                           moduleType, id,
                                           subDetectorId, rodId,
                                           sswId, sbLoc);
        if (!status) {
          ATH_MSG_DEBUG("TGCcablignSvc::getReadoutIDfromSLBID fails");
          ATH_MSG_DEBUG( "phi=" << phi
                         << " side=" << ((isAside) ? "A": "C")
                         << " region=" << ((isEndcap) ? "Endcap" : "Forward")
                         << " type="   << moduleType
                         << " id="  << id
                         << " subDetectorId=" << subDetectorId
                         << " rodId=" << rodId
                         << " sswId=" << sswId
                         << " sbLoc=" << sbLoc);
          continue;
        }

        // fill TgcRawData
        for(int iData=0; iData<out->getNumberOfData(); iData++) { // max 8
          if (!out->getHit(iData)) continue;

          //  see TGCcabling/TGCId.h (WD=0,SD,WT,ST,SI,WI). Same as TgcRawData
          TgcRawData::SlbType type = (TgcRawData::SlbType)moduleType;
          int subMat = iData % 4;
          int seg    = 0;
          if (type==TgcRawData::SLB_TYPE_TRIPLET_STRIP ) {
            if (iData<4) seg= 1;
            // 13.Jan.2011 reversed by Hisaya
            // because Layer swap in TGCStripTripletSB::doCoincidence()
          } else if ( (type==TgcRawData::SLB_TYPE_INNER_WIRE )    ||
                      (type==TgcRawData::SLB_TYPE_INNER_STRIP)       ) {
            seg= iData/4;
          }
          std::unique_ptr<TgcRawData> rawdata(new TgcRawData(bcTag,
                                                                 static_cast<uint16_t>(subDetectorId),
                                                                 static_cast<uint16_t>(rodId),
                                                                 static_cast<uint16_t>(sswId),
                                                                 static_cast<uint16_t>(sbLoc),
                                                                 l1Id, bcId,
                                                                 type, out->getDev(iData), seg, subMat,
                                                                 out->getPos(iData)));
          addRawData(std::move(rawdata), tgcrdo);

          // EIFI trigger bits for SL are filled.
          if(isEIFI) {
            bool setEIFITriggerBit =
              m_innerTrackletSlotHolder.setTriggerBit(sector->getSideId(),
                                                      phiEIFI,
                                                      (isEndcap ?
                                                       TGCInnerTrackletSlot::EI : TGCInnerTrackletSlot::FI),
                                                      (type==TgcRawData::SLB_TYPE_INNER_WIRE ?
                                                       TGCInnerTrackletSlot::WIRE : TGCInnerTrackletSlot::STRIP),
                                                      static_cast<unsigned int>(subMat),
                                                      true);

            if(!setEIFITriggerBit) {
              ATH_MSG_INFO("Fail to set Inner trigger bit of"
                           << " sideId= " << sector->getSideId()
                           << " slotId= " << phiEIFI
                           << " region= " << (isEndcap ? "EI" : "FI")
                           << " readout= " << (type==TgcRawData::SLB_TYPE_INNER_WIRE ? "WIRE" : "STRIP")
                           << " subMat(iBit)= " << static_cast<unsigned int>(subMat) );
            }
          }

          ATH_MSG_DEBUG(" recordRdoSLB : reg=" << (isEndcap ? "EC" : "FWD")
                        << " srod=" << rodId << " sswId=" << sswId
                        << " SBLoc=" << sbLoc << " type=" << itype
                        << " iData(subMat:seg)=" << iData << " pos="
                        << out->getPos(iData) << " dev=" << out->getDev(iData) );
        }
        // end of filling TgcRawData

      }   // end of loop over SB
    }   // end loop for SLB type
}
  
////////////////////////////////////////////////////////
void LVL1TGCTrigger::recordRdoHPT(TGCSector* sector,
				  std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>& tgcrdo)
{
  if(sector->hasHit() == false) return;

    // readoutID
    int subDetectorId, rodId, sswId, sbLoc, secId;
    
    // get numbering scheme info from cabling svc
    int startEndcapSector, coverageOfEndcapSector;
    int startForwardSector, coverageOfForwardSector;
    rodId = 1;
    m_cabling->getCoveragefromSRodID(rodId,
                                     startEndcapSector,
                                     coverageOfEndcapSector,
                                     startForwardSector,
                                     coverageOfForwardSector
                                     ) ;
    
    uint16_t bcTag=m_CurrentBunchTag, l1Id=0, bcId=0;
    
    // HPTID
    bool isAside, isEndcap, isStrip; int phi;
    isAside = (sector->getSideId()==0);
    isEndcap = (sector->getRegionType() == TGCRegionType::ENDCAP);
    int module = sector->getModuleId();
    //  sector Id = 0..47 (Endcap) 0..23 (forward)
    int sectorId;
    if (isEndcap){
      sectorId = ((module/3)*2+module%3) + sector->getOctantId()*6;
    } else {
      sectorId =  (module/3) +  sector->getOctantId()*3;
    }
    // secId for TgcRawData
    //  0-3(EC), 0-1(FWD) for new TGCcabling (1/12sector)
    //  0-5(EC), 0-2(FWD) for new TGCcabling (octant)
    if (isEndcap){
      secId = sectorId % coverageOfEndcapSector;
    } else {
      secId = sectorId % coverageOfForwardSector;
    }
    // phi=1-48(EC), 1-24(FWD) in detector ID scheme
    phi = (isEndcap ? (sectorId+46)%48+1 : (sectorId+23)%24+1);
    
    for(int itype=0; itype<2; itype++) { // loop over HPB type(wire/strip)
      isStrip = (itype==0 ? 0 : 1); // 0=wire 1=strip
      for(unsigned int ihpb=0; ihpb<sector->getNumberOfHPB(itype); ihpb++) { // loop over # of HPB per sector
        TGCHighPtBoard * hpb = sector->getHPB(itype, ihpb);
        if (0==hpb) continue;
        TGCHighPtChipOut * out = hpb->getOutput();
        if (0==out) continue;

        // get ReadoutID
        bool status = m_cabling->getReadoutIDfromHPTID(phi, isAside, isEndcap, isStrip, hpb->getId(),
                                                       subDetectorId, rodId, sswId, sbLoc);
        if (!status) {
          ATH_MSG_WARNING("TGCcablignSvc::getReadoutIDfromHPTID fails");
          continue;
        }

        // loop over chip and candidate
        for(int ichip=0; ichip<NumberOfChip; ichip++) { // NumberOfChip=2
          for(int icand=0; icand<TGCHighPtChipOut::s_NHitInTrackSelector; icand++) {
            if (!out->getSel(ichip, icand)) continue; // should be 1 or 2
            int chip  = ichip;
            int index = ihpb;
            int hitId =  out->getHitID(ichip, icand);
            m_cabling->getRDOHighPtIDfromSimHighPtID(!isEndcap, isStrip,
                                                     index, chip, hitId);
            bool isHPT = out->getPt(ichip,icand)==PtHigh ? 1 : 0;
            std::unique_ptr<TgcRawData> rawdata(new TgcRawData(bcTag,
                                                                   static_cast<uint16_t>(subDetectorId),
                                                                   static_cast<uint16_t>(rodId),
                                                                   l1Id,
                                                                   bcId,
                                                                   isStrip, (!isEndcap), secId, chip, icand,
                                                                   isHPT, hitId,
                                                                   out->getPos(ichip, icand),
                                                                   out->getDev(ichip, icand),
                                                                   0));
            addRawData(std::move(rawdata), tgcrdo);

            // Print
            ATH_MSG_DEBUG( "recordRdoHPT : bdTag =" << bcTag
                           << " side=" << ( (isAside)? "A" : "C")
                           << (isEndcap ? "EC" : "FWD")
                           << " w/s=" << ( (isStrip)? "s" : "w")
                           << " id=" <<  hpb->getId()
                           << " ecId=" <<  secId
                           << " chip=" << ichip
                           << " cand=" << icand
                           << " block=" << out->getHitID(ichip, icand)
                           << " subMatrix=" << out->getPos(ichip, icand)
                           << " dev=" << out->getDev(ichip, icand)
                           << " srod=" << rodId << " sswId=" << sswId << " SBLoc=" << sbLoc );

            // Strip HPT hit may be duplicated
            if (   m_tgcArgs.SHPT_ORED() &&
                   isEndcap && isStrip &&
                   (chip==1) ) {
              int oredId = -1;
              if (hitId == 1) oredId = 5;
              else if (hitId == 2) oredId = 6;
              else if (hitId == 5) oredId = 1;
              else if (hitId == 6) oredId = 2;
              if (oredId >=0) {
                std::unique_ptr<TgcRawData> rawdata2(
                                                       new TgcRawData(bcTag,
                                                                        static_cast<uint16_t>(subDetectorId),
                                                                        static_cast<uint16_t>(rodId),
                                                                        l1Id,
                                                                        bcId,
                                                                        isStrip, (!isEndcap), secId, chip, icand,
                                                                        isHPT, oredId,
                                                                        out->getPos(ichip, icand),
                                                                        out->getDev(ichip, icand),
                                                                        0));
                addRawData(std::move(rawdata2), tgcrdo);
              }
              ////////////////////

            }

          }
        }
        // end loop of candidate and chip

      }   // end loop for # of HPB per sector
    }   // end loop for HPB type
}

  
////////////////////////////////////////////////////////
void LVL1TGCTrigger::recordRdoInner(TGCSector * sector,
				std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>& tgcrdo)
{
    bool isAside  = sector->getSideId()==0;
    bool isEndcap = (sector->getRegionType() == TGCRegionType::ENDCAP);
    if (!isEndcap) return;
    
    //  sector Id = 0..47, phi = 1..48
    int module = sector->getModuleId();
    int octant = sector->getOctantId();
    int sectorId = ((module/3)*2+module%3) + octant*6;
    int phi = (sectorId+46)%48+1;
    
    // get readout ID
    int subDetectorId=0, rodId=0, sswId=0, sbLoc=0;
    
    bool status = m_cabling->getSReadoutIDfromSLID(phi, isAside, isEndcap,
                                                   subDetectorId, rodId, sswId, sbLoc);
    if (!status) {
      ATH_MSG_WARNING("TGCcablingSvc::ReadoutIDfromSLID fails in recordRdoInner()" );
      return;
    }

    //  secID for TGCRawData
    //  0-3(EC), 0-1(FWD) for 1/12 sector
    //  0-15(EC), 0-7(FWD) for 1/3 sector covered by SROD in RUn3
    int startEndcapSector, coverageOfEndcapSector;
    int startForwardSector, coverageOfForwardSector;
    if (!m_cabling->getCoveragefromSRodID(rodId,
                                          startEndcapSector,
                                          coverageOfEndcapSector,
                                          startForwardSector,
                                          coverageOfForwardSector
                                          ) )
    {
      ATH_MSG_WARNING("LVL1TGCTrigger::recordRdoInner --- bad rodId " << rodId );
      return;
    }

    int secId = 0;
    if (isEndcap){
      secId = sectorId % coverageOfEndcapSector;
    } else {
      secId = sectorId % coverageOfForwardSector;
    }
    
    uint16_t bcTag = m_CurrentBunchTag, l1Id = 0, bcId = 0;
    
    // EIFI
    const int n_slots = TGCInnerTrackletSlotHolder::NUMBER_OF_SLOTS_PER_TRIGGER_SECTOR;
    
    const TGCInnerTrackletSlot* innerTrackletSlots[n_slots] = {0, 0, 0, 0};
    m_innerTrackletSlotHolder.getInnerTrackletSlots(sector->getSideId(),
                                                    octant, module, innerTrackletSlots);
    
    std::array<int, n_slots>inner_eifi;
    m_innerTrackletSlotHolder.getInnerTrackletBits(innerTrackletSlots, inner_eifi);

    for (int i_slot = 0; i_slot < n_slots; i_slot++) {
      if (inner_eifi[i_slot] > 0) {
        std::unique_ptr<TgcRawData> rawdata_eifi(new TgcRawData(bcTag,
                                                                    static_cast<uint16_t>(subDetectorId),
                                                                    static_cast<uint16_t>(rodId),
                                                                    l1Id,
                                                                    bcId,
                                                                    (!isEndcap),
                                                                    secId, /*to be checked*/
                                                                    static_cast<uint16_t>(inner_eifi[i_slot]),
                                                                    0, /*fi*/
								static_cast<uint16_t>(i_slot) /*chamber Id*/));
        addRawData(std::move(rawdata_eifi), tgcrdo);
      }
    }
    
    // Tile
    int inner_tile = m_system->getTMDB()->getInnerTileBits(sector->getSideId(), sectorId);

    if (inner_tile > 0) {
      //TgcRawData * rawdata_tile = new TgcRawData(bcTag,
      //std::shared_ptr<TgcRawData> rawdata_tile (
      std::unique_ptr<TgcRawData> rawdata_tile (new TgcRawData(bcTag,
                                                                   static_cast<uint16_t>(subDetectorId),
                                                                   static_cast<uint16_t>(rodId),
                                                                   l1Id,
                                                                   bcId,
                                                                   (!isEndcap),
                                                                   secId,
                                                                   inner_tile,
                                                                   0 /*bcid*/ ));
      addRawData(std::move(rawdata_tile), tgcrdo);
    }

    // NSW
    TGCRegionType region = sector->getRegionType();
    if ( m_USENSW ) {
      std::shared_ptr<const LVL1TGC::NSWTrigOut> nsw_trigout = m_system->getNSW()->getOutput(region, !isAside, sectorId);
      for ( int icand=0; icand<(int)nsw_trigout->getNSWeta().size(); icand++ ){
        std::unique_ptr<TgcRawData> rawdata_nsw (new TgcRawData(bcTag,
                                                                    static_cast<uint16_t>(subDetectorId),
                                                                    static_cast<uint16_t>(rodId),
                                                                    l1Id,
                                                                    bcId,
                                                                    (!isEndcap), 
                                                                    static_cast<uint16_t>(secId), /*?*/
                                                                    static_cast<uint16_t>(nsw_trigout->getNSWeta().at(icand)),
                                                                    static_cast<uint16_t>(nsw_trigout->getNSWphi().at(icand)),
                                                                    static_cast<uint16_t>(icand), //nswcand
                                                                    static_cast<uint16_t>(nsw_trigout->getNSWDtheta().at(icand)),
                                                                    0, //nswphires
                                                                    0, //nswlowres
                                                                    static_cast<uint16_t>(nsw_trigout->getNSWTriggerProcessor().at(icand))));
        addRawData(std::move(rawdata_nsw), tgcrdo);
      }
    }

    // RPC BIS78
    if ( m_USEBIS78 ) {
      std::shared_ptr<const LVL1TGC::BIS78TrigOut> bis78_trigout = m_system->getBIS78()->getOutput(sectorId);
      for ( int icand=0; icand<(int)bis78_trigout->getBIS78eta().size(); icand++ ){
        std::unique_ptr<TgcRawData> rawdata_bis78 (new TgcRawData(bcTag,
                                                                      static_cast<uint16_t>(subDetectorId),
                                                                      static_cast<uint16_t>(rodId),
                                                                      l1Id,
                                                                      bcId,
                                                                      (!isEndcap),
                                                                      static_cast<uint16_t>(secId), /*?*/
                                                                      static_cast<uint16_t>(bis78_trigout->getBIS78eta().at(icand)),
                                                                      static_cast<uint16_t>(bis78_trigout->getBIS78phi().at(icand)),
                                                                      static_cast<uint16_t>(icand),
                                                                      static_cast<uint16_t>(bis78_trigout->getBIS78Deta().at(icand)),
                                                                      static_cast<uint16_t>(bis78_trigout->getBIS78Dphi().at(icand))));
        addRawData(std::move(rawdata_bis78), tgcrdo);
      }
    }
}
  
///////////////////////////////////////////////////////
void LVL1TGCTrigger::recordRdoSL(TGCSector* sector,
				 std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>& tgcrdo)
{
  // check if whether trigger output exists or not
  std::shared_ptr<TGCTrackSelectorOut>  selectorOut;
  sector->getSL()->getTrackSelectorOutput(selectorOut);

  if (selectorOut == nullptr) return;
  if (selectorOut->getNCandidate() == 0) return;
    
  // trigger info
  // bool cand3plus = 0;
  bool isEndcap = (sector->getRegionType() == TGCRegionType::ENDCAP);
  bool isAside = (sector->getSideId()==0);
  bool veto=0;
  int phi=0, index=0, threshold=0, roi=0;
  int Zdir= (isAside) ? 1 : -1;

    //  sector Id = 0..47 (Endcap) 0..23 (forward)
    int module = sector->getModuleId();
    int sectorId;
    if (isEndcap){
      sectorId = ((module/3)*2+module%3) + sector->getOctantId()*6;
    } else {
      sectorId =  (module/3) +  sector->getOctantId()*3;
    }
    
    //  secID for TGCRawData
    //  0-3(EC), 0-1(FWD) for new TGCcabling (1/12sector)
    //  0-5(EC), 0-2(FWD) for new TGCcabling (octant)
    int startEndcapSector, coverageOfEndcapSector;
    int startForwardSector, coverageOfForwardSector;
    int rodId = 1;
    m_cabling->getCoveragefromSRodID(rodId,
                                     startEndcapSector,
                                     coverageOfEndcapSector,
                                     startForwardSector,
                                     coverageOfForwardSector
                                     ) ;
    int secId = 0;
    if (isEndcap){
      secId = sectorId % coverageOfEndcapSector;
    } else {
      secId = sectorId % coverageOfForwardSector;
    }
    
    // phi=1-48(EC), 1-24(FWD) in detector ID scheme
    phi = (isEndcap ? (sectorId+46)%48+1 : (sectorId+23)%24+1);
    
    // get readout ID
    int subDetectorId = 0, sswId = 0, sbLoc = 0;
    bool status = m_cabling->getSReadoutIDfromSLID(phi, isAside, isEndcap,
                                                   subDetectorId, rodId, sswId, sbLoc);
    if (!status) {
      ATH_MSG_WARNING("TGCcablignSvc::ReadoutIDfromSLID fails"
                      << (isEndcap ? "  Endcap-" : "  Forward-")
                      << (isAside  ? "A  " : "C  ")
                      << "  phi=" << phi );
      return;
    }

    // bool overlap = 0;
    int inner=0, coinFlag=0;
    uint16_t bcTag=m_CurrentBunchTag, l1Id=0, bcId=0;
    for (unsigned int icand=0; icand < (unsigned int)selectorOut->getNCandidate(); ++icand) {
      index=icand;
      bool muplus = getCharge(selectorOut->getDR(icand), Zdir)==1 ? 1 : 0;
      threshold = selectorOut->getPtLevel(icand);
      roi = ((selectorOut->getR(icand))<<2)+(selectorOut->getPhi(icand));
      if (selectorOut->getInnerVeto(icand)) veto = 1;
      else                                  veto = 0;
      
      inner = selectorOut->getInnerCoincidenceFlag(icand);
      coinFlag = selectorOut->getCoincidenceType(icand);

      // create TgcRawData
      std::unique_ptr<TgcRawData> rawdata(new TgcRawData(bcTag,
                                                             static_cast<uint16_t>(subDetectorId),
                                                             static_cast<uint16_t>(rodId),
                                                             l1Id,
                                                             bcId,
                                                             (!isEndcap), secId, 
                                                             inner, 
                                                             coinFlag,
                                                             muplus, threshold, roi));
      addRawData(std::move(rawdata), tgcrdo);
      
      ATH_MSG_DEBUG("recordRdoSL  : bcTag =" << bcTag
                    << " side=" << (isAside  ? "A  " : "C  ")
                    << " reg=" << (isEndcap ? "EC" : "FWD")
                    << " phi=" << phi
                    << " cand=" << index
                    << " charge=" << (muplus ? "mu+" : "mu-")
                    << " thre=" << threshold
                    << " veto=" << veto
                    << " roi=" << roi
                    << " srod=" << rodId << " sswId=" << sswId << " SBLoc=" << sbLoc 
		    << " inner=" << inner << " coinFlag=" << coinFlag );
    }
  }
  
///////////////////////////////////////////////////////////////////////////////////
// Mask=0/Fire=1
StatusCode LVL1TGCTrigger::getMaskedChannel()
{
    std::string fname=m_MaskFileName12.value();
    if (fname.empty()) return StatusCode::SUCCESS;
    
    std::string fullName = PathResolver::find_file (fname, "PWD");
    if( fullName.empty())
      fullName =  PathResolver::find_file (fname, "DATAPATH");
    
    std::ifstream fin(fullName.c_str());
    if (!fin) {
      ATH_MSG_FATAL("Cannot open file " << fullName);
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("Use mask file : " << fullName);
    }
    // read database ------------------------------------------------------------------------------
    std::vector<std::string> mask;
    std::string aLine;
    while(getline(fin,aLine)) {
      if (aLine.compare(0,3,"///")!=0) break;
    }
    int id_type = atoi(aLine.c_str());
    while(getline(fin,aLine)) {
      if (!aLine.empty()) mask.push_back(aLine);
    }
    fin.close();
    
    //
    std::vector<int> ids;
    Identifier ID;
    int nmasked=0, nfired=0;
    for(int ich=0; ich<(int)mask.size(); ich++) {
      std::string ch = mask[ich];
      extractFromString(ch, ids);
      int OnOff=ids[0]; // 0=off(masked) 1=on(fired)
      //
      if (id_type==1 && ids.size()==8) { // online
        int sysno1 = (ids[1]==-99 ? -1 : ids[1]);  int sysno2=(ids[1]==-99 ? 1 : ids[1]);// -1(B) 1(F)
        int octno1 = (ids[2]==-99 ? 0  : ids[2]);  int octno2=(ids[2]==-99 ? 7 : ids[2]);
        for(int sysno=sysno1; sysno<=sysno2; sysno+=2) {
          for(int octno=octno1; octno<=octno2; octno++) {
            bool status = m_cabling->getOfflineIDfromOnlineID(ID,sysno,octno,
                                                              ids[3],ids[4],ids[5],ids[6],ids[7]);
            ATH_MSG_VERBOSE( (OnOff==0 ? "Mask" : "Fire") << " : offlineID=" << ID
                             << " sys=" << sysno << " oct=" << octno << " modno=" << ids[3]
                             << " layerno=" << ids[4] << " rNumber=" << ids[5]
                             << " strip=" << ids[6] << " chno=" << ids[7] );

            if (!status) {
              ATH_MSG_WARNING("This onlineID is not valid and cannot be converted to offline ID." );
              ATH_MSG_WARNING("sys=" << sysno << " oct=" << octno << " modno=" << ids[3]
                              << " layerno=" << ids[4] << " rNumber=" << ids[5]
                              << " strip=" << ids[6] << " chno=" << ids[7] );
            } else {
              m_MaskedChannel.insert(std::map<Identifier, int>::value_type(ID, OnOff));
              if (OnOff==0)      nmasked+=1;
              else if (OnOff==1) nfired+=1;
            }
          }
        }

      } else if (id_type==2 && ids.size()==6) { // readout id
        int sysno1 = (ids[1]==-99 ? 103 : ids[1]);  int sysno2=(ids[1]==-99 ? 104 : ids[1]);// 103(F), 104(B)
        int octno1 = (ids[2]==-99 ? 0  : ids[2]);  int octno2=(ids[2]==-99 ? 7 : ids[2]);
        for(int sysno=sysno1; sysno<=sysno2; sysno+=1) {
          for(int octno=octno1; octno<=octno2; octno++) {
            bool status = m_cabling->getOfflineIDfromReadoutID(ID, sysno,octno,ids[3],ids[4],ids[5]);
            ATH_MSG_VERBOSE( (OnOff==0 ? "Mask" : "Fire") << " : offlineID=" << ID
                             << " subdetectorID=" << sysno << " rodId=" << octno << " sswID=" << ids[3]
                             << " SBLoc=" << ids[4] << " channelId=" << ids[5] );
            if (!status) {
              ATH_MSG_WARNING("This readoutID is not valid and cannot be converted to offline ID " );
              ATH_MSG_WARNING("subdetectorID=" << sysno << " rodId=" << octno << " sswID=" << ids[3]
                              << " SBLoc=" << ids[4] << " channelId=" << ids[5] );
            } else {
              m_MaskedChannel.insert(std::map<Identifier, int>::value_type(ID, OnOff));
              if (OnOff==0)      nmasked+=1;
              else if (OnOff==1) nfired+=1;
            }
          }
        }

      } else if (id_type==3 && ids.size()==2) { // offline id
        ID = Identifier((unsigned int)ids[1]);
        ATH_MSG_DEBUG((OnOff==0 ? "Mask" : "Fire") << " : offlineID=" << ID);
        m_MaskedChannel.insert(std::map<Identifier, int>::value_type(ID, OnOff));
        if (OnOff==0)      nmasked+=1;
        else if (OnOff==1) nfired+=1;

      } else {
        ATH_MSG_INFO("Invalid input. Idtype or number of parameters are invalid: idtype=" << id_type
                     << " number of elements = " << ids.size() );
        return StatusCode::FAILURE;
      }
    }
    //
    ATH_MSG_INFO("Total number of masked channels ... " << nmasked);
    ATH_MSG_INFO("Total number of fired  channels ... " << nfired);
    //
    return StatusCode::SUCCESS;
}

/////////////////////////////////////////
StatusCode LVL1TGCTrigger::start() {
  if (m_bsMetaDataContRHKey.key().empty()) return StatusCode::SUCCESS;

  ATH_MSG_DEBUG("Retrieving Detector Mask from ByteStream metadata container");
  auto bsmdc = SG::makeHandle(m_bsMetaDataContRHKey);
  if (bsmdc.isValid() && !bsmdc->empty()) {
    const ByteStreamMetadata* metadata = bsmdc->front();
    uint64_t detMaskLeast = metadata->getDetectorMask();
    uint64_t detMaskMost = metadata->getDetectorMask2();

    std::vector<eformat::SubDetector> subDetOff;
    eformat::helper::DetectorMask(~detMaskLeast, ~detMaskMost).sub_detectors(subDetOff);
    auto sideA = std::find_if(subDetOff.begin(), subDetOff.end(), [](const eformat::SubDetector &s) {
        return (s == eformat::MUON_MMEGA_ENDCAP_A_SIDE || s == eformat::MUON_STGC_ENDCAP_A_SIDE); });
    auto sideC = std::find_if(subDetOff.begin(), subDetOff.end(), [](const eformat::SubDetector &s) {
        return (s == eformat::MUON_MMEGA_ENDCAP_C_SIDE || s == eformat::MUON_STGC_ENDCAP_C_SIDE); });

    if (sideA != std::end(subDetOff)) tgcArgs()->set_NSWSideInfo(m_NSWSideInfo.value().erase(0,1));
    else if (sideC != std::end(subDetOff)) tgcArgs()->set_NSWSideInfo(m_NSWSideInfo.value().erase(1,1));
    else if (sideA != std::end(subDetOff) && sideC != std::end(subDetOff)) tgcArgs()->set_NSWSideInfo("");
  }
  return StatusCode::SUCCESS;
}

/////////////////////////////////////////
void LVL1TGCTrigger::extractFromString(const std::string& str, std::vector<int> & v) {
    v.clear();
    if (str.empty()) return;
    std::string line=str;
    while(1) {
      if (line.empty()) break;
      int i = line.find(' ');
      if (i==(int)std::string::npos && !line.empty()) {
        v.push_back(atoi(line.c_str()));
        break;
      }
      std::string temp = line;
      temp.erase(i,line.size());
      v.push_back(atoi(temp.c_str()));
      line.erase(0,i+1);
    }
}
  
////////////////////////////////////////////////
int LVL1TGCTrigger::getCharge(int dR, int /*Zdir*/) {
    // old scheme
    // if (dR==0) return (Zdir>0 ? -1 : 1);
    // return (dR*Zdir>0 ? 1 : -1);
    return (dR >=0 ? 1 : -1 );
}
  
////////////////////////////////////////////////
// see TGCNumbering.h 
int LVL1TGCTrigger::getLPTTypeInRawData(int type)
{
    switch(type) {
    case WTSB :
      return TgcRawData::SLB_TYPE_TRIPLET_WIRE;
    case WDSB :
      return TgcRawData::SLB_TYPE_DOUBLET_WIRE;
    case STSB :
      return TgcRawData::SLB_TYPE_TRIPLET_STRIP;
    case SDSB :
      return TgcRawData::SLB_TYPE_DOUBLET_STRIP;
    case WISB :
      return TgcRawData::SLB_TYPE_INNER_WIRE;
    case SISB :
      return TgcRawData::SLB_TYPE_INNER_STRIP;
    default :
      return -1;
    }
}

/////////////////////////////////////////////////
  bool LVL1TGCTrigger::addRawData(std::unique_ptr<TgcRawData> rawdata,
                                  std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>& tgcrdo)
{
    ATH_MSG_DEBUG("addRawData() is called.");
    std::pair<int, int> subDetectorRod(rawdata->subDetectorId(), rawdata->rodId());
    std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>::iterator itRdo = tgcrdo.find(subDetectorRod);

    if (itRdo==tgcrdo.end()) {
      // in case TgcRdo with the given subDetectorId and rodId is 
      // not registered yet, create new TgcRdo and add rawdata to it
      std::unique_ptr<TgcRdo> thisRdo(new TgcRdo(rawdata->subDetectorId(), rawdata->rodId(), rawdata->bcId(), rawdata->l1Id()));
      thisRdo->push_back(std::move(rawdata));
      tgcrdo.insert(std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>::value_type(subDetectorRod, std::move(thisRdo)));
    } else {
      itRdo->second->push_back(std::move(rawdata));
    }
    return true;
}
  
///////////////////////////////////////////////////////////
StatusCode LVL1TGCTrigger::getCabling()
{
    ATH_MSG_DEBUG("LVL1TGCTrigger::getCabling()");

    // TGCcablingSvc
    // get Cabling Server Service
    const ITGCcablingServerSvc* TgcCabGet = 0;
    StatusCode sc = service("TGCcablingServerSvc", TgcCabGet);
    if (sc.isFailure()){
      ATH_MSG_FATAL("Can't get TGCcablingServerSvc.");
      return StatusCode::FAILURE;
    }

    // get Cabling Service
    sc = TgcCabGet->giveCabling(m_cabling);
    if (sc.isFailure()){
      ATH_MSG_FATAL("Can't get TGCcablingSvc Server");
      return StatusCode::FAILURE;
    }
    
    int maxRodId, maxSRodId, maxSswId, maxSbloc,minChannelId, maxChannelId;
    m_cabling->getReadoutIDRanges( maxRodId, maxSRodId, maxSswId, maxSbloc,minChannelId, maxChannelId);
    if (maxRodId ==12) {
      ATH_MSG_INFO(m_cabling->name() << " is OK");
    } else {
      ATH_MSG_FATAL("Old TGCcablingSvc(octant segmentation) can not be used !");
      return StatusCode::FAILURE;
    }

    // create TGCElectronicsSystem
    m_system = std::make_unique<TGCElectronicsSystem>(&m_tgcArgs,m_db);
    
    m_TimingManager = std::make_unique<TGCTimingManager>(m_readCondKey);
    m_TimingManager->setBunchCounter(0);
    m_nEventInSector = 0;

    ATH_MSG_DEBUG("finished LVL1TGCTrigger::getCabling()");

    return sc;
}


}  // end of namespace

