/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
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
#include "TrigT1TGC/TGCSLSelectorOut.h" // for Run2
#include "TrigT1TGC/TGCTrackSelectorOut.h" // for Run3
#include "TrigT1TGC/TGCElectronicsSystem.h"
#include "TrigT1TGC/TGCTimingManager.h"
#include "TrigT1TGC/TGCDatabaseManager.h"
#include "TrigT1TGC/TGCSector.h"
#include "TrigT1TGC/TGCNumbering.h"
#include "TrigT1TGC/TrigT1TGC_ClassDEF.h"
#include "TrigT1TGC/TGCNumbering.h"
#include "TrigT1TGC/TGCTMDBOut.h"
#include "TrigT1TGC/TGCNSW.h"

// Other stuff
#include "TrigConfL1Data/TriggerThreshold.h"
#include "TrigConfL1Data/ThresholdConfig.h"
#include "TrigT1Interfaces/Lvl1MuCTPIInput.h"
#include "TrigT1Interfaces/Lvl1MuEndcapSectorLogicData.h"
#include "TrigT1Interfaces/Lvl1MuForwardSectorLogicData.h"
#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"
#include "TrigT1Interfaces/Lvl1MuEndcapSectorLogicDataPhase1.h"
#include "TrigT1Interfaces/Lvl1MuForwardSectorLogicDataPhase1.h"

// MuonSpectrometer
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonDigitContainer/TgcDigitCollection.h"
#include "MuonDigitContainer/TgcDigit.h"
#include "MuonRDO/TgcRdoContainer.h"


#include "MuonRDO/NSW_TrigRawData.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"

#include "TGCcablingInterface/ITGCcablingSvc.h"
#include "TGCcablingInterface/ITGCcablingServerSvc.h"
#include "PathResolver/PathResolver.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

namespace LVL1TGCTrigger {

  /////////////////////////////////////////////////////////////////////////////////
  LVL1TGCTrigger::LVL1TGCTrigger(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name,pSvcLocator),
    m_cabling(0),
    m_bctagInProcess(0),
    m_db(0),
    m_TimingManager(0),
    m_system(0),
    m_nEventInSector(0),
    m_innerTrackletSlotHolder( tgcArgs() ),
    m_debuglevel(false)
  {
  }

  ////////////////////////////////////////////////////////////
  LVL1TGCTrigger::~LVL1TGCTrigger()
  {
    ATH_MSG_DEBUG("LVL1TGCTrigger destructor called");
    // delete m_TimingManager ,ElectronicsSystem and Database
    if (m_TimingManager){
      delete m_TimingManager;
      m_TimingManager =0;
    }
    if (m_system) {
      delete m_system;
      m_system = 0;
    }
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
    m_tgcArgs.set_INNER_VETO( m_INNERVETO.value() && m_tgcArgs.USE_INNER() );
    m_tgcArgs.set_TILE_MU( m_TILEMU.value() && m_tgcArgs.USE_INNER() );
    m_tgcArgs.set_USE_NSW( m_USENSW.value() && m_tgcArgs.USE_INNER() );

    m_tgcArgs.set_USE_CONDDB( m_USE_CONDDB.value() );
    m_tgcArgs.set_useRun3Config( m_useRun3Config.value() );

    ATH_CHECK( m_readCondKey.initialize(!m_useRun3Config.value()) );
    ATH_CHECK( m_readLUTs_CondKey.initialize(m_useRun3Config.value()) );

    // initialize TGCDataBase
    m_db = new TGCDatabaseManager(&m_tgcArgs, m_readCondKey, m_readLUTs_CondKey, m_VerCW);

    // initialize the TGCcabling
    ATH_CHECK(getCabling());

    // read and write handle key
    ATH_CHECK(m_keyTgcDigit.initialize());
    ATH_CHECK(m_keyTileMu.initialize());
    ATH_CHECK(m_keyNSWTrigOut.initialize());
    ATH_CHECK(m_muctpiPhase1Key.initialize(tgcArgs()->useRun3Config()));
    ATH_CHECK(m_muctpiKey.initialize(!tgcArgs()->useRun3Config()));

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
    if (m_system) delete m_system;
    m_system = 0;
    if (m_TimingManager) delete m_TimingManager;
    m_TimingManager=0;
    
    return StatusCode::SUCCESS;
  }

  ////////////////////////////////////////////
  StatusCode LVL1TGCTrigger::execute()
  {
    ATH_MSG_DEBUG("execute() called");
    
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
    
    StatusCode sc = StatusCode::SUCCESS;
    // Tile Mu Data
    bool doTileMu = m_tgcArgs.TILE_MU();
    
    if (m_tgcArgs.USE_CONDDB()) {
      SG::ReadCondHandle<TGCTriggerData> readHandle{m_readCondKey};
      const TGCTriggerData* readCdo{*readHandle};
      doTileMu = readCdo->isActive(TGCTriggerData::CW_TILE);
    }


    // NSW data
    bool doNSW = m_tgcArgs.USE_NSW();

    
    // TgcRdo
    m_tgcrdo.clear();
    const TgcRdoContainer * rdoCont;
    sc = evtStore()->retrieve( rdoCont, "TGCRDO" );
    if (sc.isFailure()) {
      ATH_MSG_WARNING("Cannot retrieve TgcRdoContainer with key=TGCRDO");
      return StatusCode::SUCCESS;
    }
    if (rdoCont->size()>0) {
      TgcRdoContainer::const_iterator itR = rdoCont->begin();
      for(; itR!= rdoCont->end(); itR++) {
        const TgcRdo * rdo = (*itR);
        TgcRdo * thisRdo = const_cast<TgcRdo *>(rdo);
        std::pair<int, int> subDetectorRod(thisRdo->subDetectorId(), thisRdo->rodId());
        m_tgcrdo.insert(std::map<std::pair<int, int>, TgcRdo*>::value_type(subDetectorRod, thisRdo));
      }
    }
    
    SG::ReadHandle<TgcDigitContainer> readTgcDigitContainer( m_keyTgcDigit );
    if(!readTgcDigitContainer.isValid()){
      ATH_MSG_ERROR("Cannot retrieve TgcDigitContainer");
      return StatusCode::FAILURE;
    }
    const TgcDigitContainer* tgc_container = readTgcDigitContainer.cptr();
    
    LVL1MUONIF::Lvl1MuCTPIInputPhase1* muctpiinputPhase1 = nullptr;
    LVL1MUONIF::Lvl1MuCTPIInput* muctpiinput = nullptr;
    if(tgcArgs()->useRun3Config()){
      SG::WriteHandle<LVL1MUONIF::Lvl1MuCTPIInputPhase1> wh_muctpiTgc(m_muctpiPhase1Key);
      ATH_CHECK(wh_muctpiTgc.record(std::make_unique<LVL1MUONIF::Lvl1MuCTPIInputPhase1>()));
      muctpiinputPhase1 = wh_muctpiTgc.ptr();
    }else{
      SG::WriteHandle<LVL1MUONIF::Lvl1MuCTPIInput> wh_muctpiTgc(m_muctpiKey);
      ATH_CHECK(wh_muctpiTgc.record(std::make_unique<LVL1MUONIF::Lvl1MuCTPIInput>()));
      muctpiinput = wh_muctpiTgc.ptr();
    }
    
    // process one by one
    for (int bc=TgcDigit::BC_PREVIOUS; bc<=TgcDigit::BC_NEXT; bc++){
      sc = StatusCode::SUCCESS;
      
      // Use TileMu only if BC_CURRENT
      if (doTileMu && bc==m_CurrentBunchTag) {
        sc = fillTMDB();
        if (sc.isFailure()) {
          ATH_MSG_WARNING("Cannot retrieve Tile Mu Data");
          return sc;
        }
      }

      // Use NSW trigger output 
      if(doNSW && bc==m_CurrentBunchTag){
	ATH_CHECK(fillNSW());
      }


      if (m_ProcessAllBunches || bc==m_CurrentBunchTag){
        m_bctagInProcess =bc;
        sc=processOneBunch(tgc_container, muctpiinput, muctpiinputPhase1);
      }
      if (sc.isFailure()) {
        ATH_MSG_FATAL("Fail to process the bunch " << m_bctagInProcess);
        return sc;
      }
    }
    
    return sc;
  }

  StatusCode LVL1TGCTrigger::processOneBunch(const TgcDigitContainer* tgc_container,
                                             LVL1MUONIF::Lvl1MuCTPIInput* muctpiinput,
                                             LVL1MUONIF::Lvl1MuCTPIInputPhase1* muctpiinputPhase1)
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
    for( int i=0; i<m_system->getNumberOfSide(); i+=1){ // i=0:Z>0(A) , i=1:Z<0(C)
      for( int j=0; j<m_system->getNumberOfOctant(); j+=1){
        for( int k=0; k<m_system->getNumberOfModule(); k+=1){
          TGCSector* sector = m_system->getSector(i,j,k);
          if((sector!=0)&&(sector->hasHit())){
            m_nEventInSector++;
            m_TimingManager->startPatchPanel(sector, m_db);
            m_TimingManager->startSlaveBoard(sector);
            if (m_OutputTgcRDO.value()) recordRdoSLB(sector);
            // EIFI trigger bits for SL are filled in this method.
          }
        }
      }
    }
    
    // HighPtBoard, SectorLogic
    const int muctpiBcId_offset = TgcDigit::BC_CURRENT;
    int muctpiBcId = m_bctagInProcess - muctpiBcId_offset;
    for(int i=0; i<m_system->getNumberOfSide(); i+=1){
      int sectoraddr_endcap = 0;
      int sectoraddr_forward = 0;
      for(int j=0; j<m_system->getNumberOfOctant(); j+=1){
        for(int k=0; k<m_system->getNumberOfModule(); k+=1){
          if(k>=9) continue;// skip Inner TGC
          TGCSector* sector = m_system->getSector(i,j,k);
          if(sector==0) continue;

          if((sector->hasHit())){
            m_TimingManager->startHighPtBoard(sector);
            if (m_OutputTgcRDO.value()) recordRdoHPT(sector);

            // EIFI trigger bits are checked if Endcap
            if(sector->getRegionType()==Endcap && sector->getSL()) {
              // Pointers to store EIFI trigger bits for Endcap SL
              const TGCInnerTrackletSlot* innerTrackletSlots[TGCInnerTrackletSlotHolder::NUMBER_OF_SLOTS_PER_TRIGGER_SECTOR]
                = {0, 0, 0, 0};
              m_innerTrackletSlotHolder.getInnerTrackletSlots(i, j, k, innerTrackletSlots);
              sector->getSL()->setInnerTrackletSlots(innerTrackletSlots);
            }

            m_TimingManager->startSectorLogic(sector);
            sector->clearNumberOfHit();
          }

          // Fill inner (EIFI/Tile) words
          if (m_OutputTgcRDO.value() && m_tgcArgs.USE_INNER()) recordRdoInner(sector);

          // Fill Lvl1MuCTPInput
          size_t tgcsystem=0,subsystem=0;
          if(i==0) subsystem = LVL1MUONIF::Lvl1MuCTPIInput::idSideA();
          if(i==1) subsystem = LVL1MUONIF::Lvl1MuCTPIInput::idSideC();
          if (m_OutputTgcRDO.value()) recordRdoSL(sector, subsystem);

          const TGCSLSelectorOut* selectorOut = sector->getSL()->getSelectorOutput();
	  std::shared_ptr<TGCTrackSelectorOut>  trackSelectorOut;
	  sector->getSL()->getTrackSelectorOutput(trackSelectorOut); 

          if(sector->getRegionType()==Endcap){
            if(m_tgcArgs.useRun3Config()){
              LVL1MUONIF::Lvl1MuEndcapSectorLogicDataPhase1 sldata;
              tgcsystem = LVL1MUONIF::Lvl1MuCTPIInputPhase1::idEndcapSystem();
              if(trackSelectorOut!=0) FillSectorLogicData(&sldata,trackSelectorOut.get());
              muctpiinputPhase1->setSectorLogicData(sldata,tgcsystem,subsystem,sectoraddr_endcap++,muctpiBcId);
            }else{
              LVL1MUONIF::Lvl1MuEndcapSectorLogicData sldata;
              tgcsystem = LVL1MUONIF::Lvl1MuCTPIInput::idEndcapSystem();
              if(selectorOut!=0) FillSectorLogicData(&sldata,selectorOut,subsystem);
              muctpiinput->setSectorLogicData(sldata,tgcsystem,subsystem,sectoraddr_endcap++,muctpiBcId);
            }
          } else if(sector->getRegionType()==Forward){
            if(m_tgcArgs.useRun3Config()){
              LVL1MUONIF::Lvl1MuForwardSectorLogicDataPhase1 sldata;
              tgcsystem = LVL1MUONIF::Lvl1MuCTPIInputPhase1::idForwardSystem();
              if(trackSelectorOut!=0) FillSectorLogicData(&sldata,trackSelectorOut.get());
              muctpiinputPhase1->setSectorLogicData(sldata,tgcsystem,subsystem,sectoraddr_forward++,muctpiBcId);
            }else{
              LVL1MUONIF::Lvl1MuForwardSectorLogicData sldata;
              tgcsystem = LVL1MUONIF::Lvl1MuCTPIInput::idForwardSystem();
              if(selectorOut!=0) FillSectorLogicData(&sldata,selectorOut,subsystem);
              muctpiinput->setSectorLogicData(sldata,tgcsystem,subsystem,sectoraddr_forward++,muctpiBcId);
            }
          }

	  //** Selector in  Run2
          // delete selectorOut
          sector->getSL()->eraseSelectorOut();
          if (selectorOut != 0 ) delete selectorOut;
          selectorOut=0;
	  
	  //** Selector in  Run3
          // delete selectorOut
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
    std::map<Identifier, int>::iterator itCh;
    // (1) skip masked channels
    for (TgcDigitContainer::const_iterator c = tgc_container->begin(); c != tgc_container->end(); ++c) {
      for (TgcDigitCollection::const_iterator h = (*c)->begin(); h != (*c)->end(); ++h) {

        // check BCID
        if ((*h)->bcTag()!=m_bctagInProcess) continue;

        Identifier channelId = (*h)->identify();
        itCh=m_MaskedChannel.find(channelId);
        if (itCh!=m_MaskedChannel.end() && itCh->second==0) {
          ATH_MSG_DEBUG("This channel is masked! offlineID=" << channelId);
          continue;
        }
        TgcDigitIDs.insert(std::map<Identifier,int>::value_type(channelId,1));
      }
    }

    // (2) add fired channels by force
    for(itCh=m_MaskedChannel.begin(); itCh!=m_MaskedChannel.end(); itCh++) {
      if (itCh->second==1) {
        ATH_MSG_VERBOSE("This channel is fired by force! offlineID=" << itCh->first);
        if (TgcDigitIDs.find(itCh->first)==TgcDigitIDs.end()) {
          TgcDigitIDs.insert(std::map<Identifier,int>::value_type(itCh->first,1));
        }
      }
    }

    ATH_MSG_DEBUG("# of total hits    " << TgcDigitIDs.size());

    return;
  }
  
  //////////////////////////////////////////////////
  void  LVL1TGCTrigger::fillTGCEvent(std::map<Identifier, int>& tgcDigitIDs, TGCEvent& event)
  {
    std::map<Identifier, int>::iterator itCh;

    // Loop on TGC detectors (collections)
    for(itCh=tgcDigitIDs.begin(); itCh!=tgcDigitIDs.end(); itCh++) {
      Identifier channelId = itCh->first;
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
        int subDetectorID, rodID, sswID, sbLoc, channelID;
        int phi=0;
        int moduleType=0;
        int slbID=0;
        bool  isAside=true;
        bool  isEndcap=true;

        fstatus = m_cabling->getReadoutIDfromOfflineID(channelId,
                                                       subDetectorID,
                                                       rodID,sswID,
                                                       sbLoc,channelID);

        if (fstatus) {
          fstatus = m_cabling->getSLBIDfromReadoutID(phi, isAside, isEndcap,
                                                     moduleType, slbID,
                                                     subDetectorID,
                                                     rodID, sswID,sbLoc);
        }
        if (fstatus) {
          ATH_MSG_VERBOSE("hit : subsys#=" << subsystemNumber
                          << " octant#=" << octantNumber
                          << " mod#=" << moduleNumber
                          << " layer#=" << layerNumber << " r#=" << rNumber
                          << " isStrip=" << wireOrStrip
                          << " ch#=" << channelNumber << endmsg
                          << " --> readoutID: sudetID=" << subDetectorID
                          << " rodID=" << rodID << " sswID=" << sswID
                          << " slbID=" << slbID << " chID=" << channelID);

          TGCZDirection zdire = (subsystemNumber==1)? kZ_FORWARD : kZ_BACKWARD;
          TGCReadoutIndex index(zdire,octantNumber,moduleNumber,rNumber,layerNumber);
          TGCSignalType signal = (wireOrStrip==1)? Strip : WireGroup;
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
  void LVL1TGCTrigger::FillSectorLogicData(LVL1MUONIF::Lvl1MuSectorLogicData *sldata,
                                           const TGCSLSelectorOut* selectorOut, unsigned int subsystem)
  {
    if(selectorOut ==0) return;
    int Zdir= (subsystem==LVL1MUONIF::Lvl1MuCTPIInput::idSideA() ? 1 : -1);
    
    sldata->clear2candidatesInSector();// for temporary

    const int muctpiBcId_offset = TgcDigit::BC_CURRENT;
    sldata->bcid(m_bctagInProcess - muctpiBcId_offset);

    if ((selectorOut->getNCandidate()) >= 1) {
      sldata->roi(0,((selectorOut->getR(0))<<2)+(selectorOut->getPhi(0)));
      //      ovl --> veto
      //      sldata->ovl(0,0);
      if (selectorOut->getInnerVeto(0)) sldata->ovl(0,1);
      else                              sldata->ovl(0,0);
      sldata->pt(0,selectorOut->getPtLevel(0));
      sldata->charge(0, getCharge(selectorOut->getDR(0),Zdir));
    } else {
      // no entry
    }
    if ((selectorOut->getNCandidate()) == 2) {
      sldata->roi(1,((selectorOut->getR(1))<<2)+(selectorOut->getPhi(1)));
      //      ovl --> veto
      //      sldata->ovl(1,0);
      if (selectorOut->getInnerVeto(1)) sldata->ovl(1,1);
      else                              sldata->ovl(1,0);
      sldata->pt(1,selectorOut->getPtLevel(1));
      sldata->charge(1, getCharge(selectorOut->getDR(1),Zdir));
    }
    sldata->set2candidates(0);// not used for TGC
    sldata->clear2candidates(0);// not used for TGC
    sldata->set2candidates(1);// not used for TGC
    sldata->clear2candidates(1);// not used for TGC
    
    // Print
    if(m_debuglevel) {
      if ((selectorOut->getNCandidate()) >= 1) {
        ATH_MSG_DEBUG( "SectorLogic: 1st candidate   "
                       << " roi:" << (selectorOut->getR(0))<<2 + selectorOut->getPhi(0)
                       << " pt:" << selectorOut->getPtLevel(0)
                       << " charge:" << getCharge(selectorOut->getDR(0),Zdir)
                       << " veto:" << sldata->ovl(0));
      }
      if ((selectorOut->getNCandidate()) == 2) {
        ATH_MSG_DEBUG( "SectorLogic: 2nd candidate   "
                       << " roi:" << (selectorOut->getR(1))<<2 + selectorOut->getPhi(1)
                       << " pt:" << selectorOut->getPtLevel(1)
                       << " charge:" << getCharge(selectorOut->getDR(1),Zdir)
                       << " veto:" << sldata->ovl(1));
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
  void LVL1TGCTrigger::recordRdoSLB(TGCSector * sector)
  {
    uint16_t bcTag=m_CurrentBunchTag, l1Id=0, bcId=0;
    // readoutID
    int subDetectorId, rodId, sswId, sbLoc, secId, secIdEIFI;
    // SLBID
    bool isAside, isEndcap; int phi, moduleType, id, phiEIFI;
    isAside = (sector->getSideId()==0  ? 1 : 0);
    isEndcap = (sector->getRegionType()==Endcap ? 1 : 0);
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
      for(int index=0; index<sector->getNumberOfSB(itype); index++) {
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
          TgcRawData * rawdata =
            new TgcRawData(bcTag,
                           static_cast<uint16_t>(subDetectorId),
                           static_cast<uint16_t>(rodId),
                           static_cast<uint16_t>(sswId),
                           static_cast<uint16_t>(sbLoc),
                           l1Id, bcId,
                           type, out->getDev(iData), seg, subMat,
                           out->getPos(iData));

          if (!addRawData(rawdata)) delete rawdata;

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
                        << " rod=" << rodId << " sswId=" << sswId
                        << " SBLoc=" << sbLoc << " type=" << itype
                        << " iData(subMat:seg)=" << iData << " pos="
                        << out->getPos(iData) << " dev=" << out->getDev(iData) );
        }
        // end of filling TgcRawData

      }   // end of loop over SB
    }   // end loop for SLB type
  }
  
  ////////////////////////////////////////////////////////
  void LVL1TGCTrigger::recordRdoHPT(TGCSector * sector)
  {
    // readoutID
    int subDetectorId, rodId, sswId, sbLoc, secId;
    
    // get numbering scheme info from cabling svc
    int startEndcapSector, coverageOfEndcapSector;
    int startForwardSector, coverageOfForwardSector;
    rodId = 1;
    m_cabling->getCoveragefromRodID(rodId,
                                    startEndcapSector,
                                    coverageOfEndcapSector,
                                    startForwardSector,
                                    coverageOfForwardSector
                                    ) ;
    
    uint16_t bcTag=m_CurrentBunchTag, l1Id=0, bcId=0;
    
    // HPTID
    bool isAside, isEndcap, isStrip; int phi;
    isAside = (sector->getSideId()==0);
    isEndcap = (sector->getRegionType()==Endcap);
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
      for(int ihpb=0; ihpb<sector->getNumberOfHPB(itype); ihpb++) { // loop over # of HPB per sector
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
          for(int icand=0; icand<NHitInTrackSelector; icand++) { // NHitInTrackSelector=2
            if (!out->getSel(ichip, icand)) continue; // should be 1 or 2
            int chip  = ichip;
            int index = ihpb;
            int hitId =  out->getHitID(ichip, icand);
            m_cabling->getRDOHighPtIDfromSimHighPtID(!isEndcap, isStrip,
                                                     index, chip, hitId);
            bool isHPT = out->getPt(ichip,icand)==PtHigh ? 1 : 0;
            TgcRawData * rawdata =
              new TgcRawData(bcTag,
                             static_cast<uint16_t>(subDetectorId),
                             static_cast<uint16_t>(rodId),
                             l1Id,
                             bcId,
                             isStrip, (!isEndcap), secId, chip, icand,
                             isHPT, hitId,
                             out->getPos(ichip, icand),
                             out->getDev(ichip, icand),
                             0);

            if (!addRawData(rawdata)) delete rawdata;

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
                           << " rod=" << rodId << " sswId=" << sswId << " SBLoc=" << sbLoc );

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
                TgcRawData * rawdata2 =
                  new TgcRawData(bcTag,
                                 static_cast<uint16_t>(subDetectorId),
                                 static_cast<uint16_t>(rodId),
                                 l1Id,
                                 bcId,
                                 isStrip, (!isEndcap), secId, chip, icand,
                                 isHPT, oredId,
                                 out->getPos(ichip, icand),
                                 out->getDev(ichip, icand),
                                 0);
                if (!addRawData(rawdata2)) delete rawdata2;
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
  void LVL1TGCTrigger::recordRdoInner(TGCSector * sector)
  {
    bool isAside  = sector->getSideId()==0;
    bool isEndcap = (sector->getRegionType()==Endcap);
    if (!isEndcap) return;
    
    //  sector Id = 0..47, phi = 1..48
    int module = sector->getModuleId();
    int octant = sector->getOctantId();
    int sectorId = ((module/3)*2+module%3) + octant*6;
    int phi = (sectorId+46)%48+1;
    
    // get readout ID
    int subDetectorId=0, rodId=0, sswId=0, sbLoc=0;
    
    bool status = m_cabling->getReadoutIDfromSLID(phi, isAside, isEndcap,
                                                  subDetectorId, rodId, sswId, sbLoc);
    if (!status) {
      ATH_MSG_WARNING("TGCcablignSvc::ReadoutIDfromSLID fails in recordRdoInner()" );
      return;
    }
    
    uint16_t bcTag = m_CurrentBunchTag, l1Id = 0, bcId = 0;
    
    // EIFI
    const int n_slots = TGCInnerTrackletSlotHolder::NUMBER_OF_SLOTS_PER_TRIGGER_SECTOR;
    
    const TGCInnerTrackletSlot* innerTrackletSlots[n_slots] = {0, 0, 0, 0};
    m_innerTrackletSlotHolder.getInnerTrackletSlots(sector->getSideId(),
                                                    octant, module, innerTrackletSlots);
    
    int inner_eifi =
      m_innerTrackletSlotHolder.getInnerTrackletBits(innerTrackletSlots);

    
    if (inner_eifi > 0) {
      TgcRawData * rawdata_eifi = new TgcRawData(bcTag,
                                                 static_cast<uint16_t>(subDetectorId),
                                                 static_cast<uint16_t>(rodId),
                                                 l1Id,
                                                 bcId,
                                                 0,
                                                 0,
                                                 static_cast<uint16_t>(sbLoc | 4),
                                                 0,
                                                 0,
                                                 0,
                                                 0,
                                                 0,
                                                 0,
                                                 static_cast<uint16_t>(inner_eifi));
      if (!addRawData(rawdata_eifi)) delete rawdata_eifi;
    }
    
    
    // Tile
    const TGCTMDB* tmdb = m_system->getTMDB();
    int inner_tile = tmdb->getInnerTileBits(sector->getSideId(), sectorId);
    
    if (inner_tile > 0) {
      TgcRawData * rawdata_tile = new TgcRawData(bcTag,
                                                 static_cast<uint16_t>(subDetectorId),
                                                 static_cast<uint16_t>(rodId),
                                                 l1Id,
                                                 bcId,
                                                 1,
                                                 0,
                                                 static_cast<uint16_t>(sbLoc | 4),
                                                 0,
                                                 0,
                                                 0,
                                                 0,
                                                 0,
                                                 0,
                                                 static_cast<uint16_t>(inner_tile));
      if (!addRawData(rawdata_tile)) delete rawdata_tile;
    }
    
    
  }
  
  ///////////////////////////////////////////////////////
  void LVL1TGCTrigger::recordRdoSL(TGCSector * sector, unsigned int subsystem)
  {
    // check if whether trigger exists or not
    const TGCSLSelectorOut* selectorOut = sector->getSL()->getSelectorOutput();
    if (selectorOut ==0) return;
    if (selectorOut->getNCandidate()==0) return;
    
    // readoutID
    int subDetectorId=0, rodId=0, sswId=0, sbLoc=0, secId=0;
    
    // trigger info
    bool cand3plus=0, isEndcap=0, isAside=0, muplus=0, overlap=0, veto=0;
    int phi=0, index=0, threshold=0, roi=0;
    int Zdir= (subsystem==LVL1MUONIF::Lvl1MuCTPIInput::idSideA() ? 1 : -1);
    
    isAside = (sector->getSideId()==0);
    isEndcap = (sector->getRegionType()==Endcap);
    cand3plus = 0;
    
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
    rodId = 1;
    m_cabling->getCoveragefromRodID(rodId,
                                    startEndcapSector,
                                    coverageOfEndcapSector,
                                    startForwardSector,
                                    coverageOfForwardSector
                                    ) ;
    if (isEndcap){
      secId = sectorId % coverageOfEndcapSector;
    } else {
      secId = sectorId % coverageOfForwardSector;
    }
    
    // phi=1-48(EC), 1-24(FWD) in detector ID scheme
    phi = (isEndcap ? (sectorId+46)%48+1 : (sectorId+23)%24+1);
    
    // get readout ID
    bool status = m_cabling->getReadoutIDfromSLID(phi, isAside, isEndcap,
                                                  subDetectorId, rodId, sswId, sbLoc);
    if (!status) {
      ATH_MSG_WARNING("TGCcablignSvc::ReadoutIDfromSLID fails"
                      << (isEndcap ? "  Endcap-" : "  Forward-")
                      << (isAside  ? "A  " : "C  ")
                      << "  phi=" << phi );
      return;
    }
    
    uint16_t bcTag=m_CurrentBunchTag, l1Id=0, bcId=0;
    for (int icand=0; icand < selectorOut->getNCandidate(); icand +=1) {
      index=icand;
      muplus = getCharge(selectorOut->getDR(icand), Zdir)==1 ? 1 : 0;
      threshold = selectorOut->getPtLevel(icand);
      roi = ((selectorOut->getR(icand))<<2)+(selectorOut->getPhi(icand));
      if (selectorOut->getInnerVeto(icand)) veto = 1;
      else                                  veto = 0;
      
      // create TgcRawData
      TgcRawData * rawdata =
        new TgcRawData(bcTag,
                       static_cast<uint16_t>(subDetectorId),
                       static_cast<uint16_t>(rodId),
                       l1Id,
                       bcId,
                       cand3plus, (!isEndcap), secId, index,
                       muplus, threshold, overlap, veto, roi);
      if (!addRawData(rawdata)) delete rawdata;
      
      ATH_MSG_DEBUG("recordRdoSL  : bcTag =" << bcTag
                    << " side=" << (isAside  ? "A  " : "C  ")
                    << " reg=" << (isEndcap ? "EC" : "FWD")
                    << " phi=" << phi
                    << " cand=" << index
                    << " charge=" << (muplus ? "mu+" : "mu-")
                    << " thre=" << threshold
                    << " veto=" << veto
                    << " roi=" << roi
                    << " rod=" << rodId << " sswId=" << sswId << " SBLoc=" << sbLoc );
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////////
  // Mask=0/Fire=1
  StatusCode LVL1TGCTrigger::getMaskedChannel()
  {
    std::string fname=m_MaskFileName12.value();
    if (fname.empty()) return StatusCode::SUCCESS;
    
    std::string fullName = PathResolver::find_file (fname.c_str(), "PWD");
    if( fullName.length() == 0 )
      fullName =  PathResolver::find_file (fname.c_str(), "DATAPATH");
    
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
      if (aLine.substr(0,3)!="///") break;
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
  void LVL1TGCTrigger::extractFromString(std::string str, std::vector<int> & v) {
    v.clear();
    if (str.empty()) return;
    std::string line=str;
    while(1) {
      if (line.empty()) break;
      int i = line.find(" ");
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
  bool LVL1TGCTrigger::addRawData(TgcRawData * rawdata)
  {
    ATH_MSG_DEBUG("addRawData() is called.");
    std::pair<int, int> subDetectorRod(rawdata->subDetectorId(), rawdata->rodId());
    std::map<std::pair<int, int>, TgcRdo*>::iterator itRdo = m_tgcrdo.find(subDetectorRod);
    
    if (itRdo==m_tgcrdo.end()) {
      
      bool isHipt  = rawdata->type()==TgcRawData::TYPE_HIPT;
      bool isInner = (rawdata->sector() & 4) != 0;

      // variables for non-registered data of EI/FI mask2=1 hits
      bool isTracklet = rawdata->type() == TgcRawData::TYPE_TRACKLET;
      TgcRawData::SlbType slbtype = rawdata->slbType();

      if(isTracklet && (slbtype == TgcRawData::SLB_TYPE_INNER_WIRE ||
                        slbtype == TgcRawData::SLB_TYPE_INNER_STRIP)) {

        ATH_MSG_DEBUG("Inner coincidence words without BW hits"
                      << " for subDetectorId=" << rawdata->subDetectorId()
                      << " for rodId=" << rawdata->rodId() );

      } else if (isHipt && isInner) {
        ATH_MSG_DEBUG("Inner coincidence words without BW hits"
                      << " for subDetectorId=" << rawdata->subDetectorId()
                      << " for rodId=" << rawdata->rodId() );
      } else {
        ATH_MSG_WARNING("Inconsistent RodId with hits "
                        << " for subDetectorId=" << rawdata->subDetectorId()
                        << " for rodId=" << rawdata->rodId()
                        << "  Type=" << static_cast<int>(rawdata->type()) );
      }
      return false;

    } else {
      TgcRdo * thisRdo = itRdo->second;
      thisRdo->push_back(rawdata);

      return true;
    }
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
    
    int maxRodId,maxSswId, maxSbloc,minChannelId, maxChannelId;
    m_cabling->getReadoutIDRanges( maxRodId,maxSswId, maxSbloc,minChannelId, maxChannelId);
    if (maxRodId ==12) {
      ATH_MSG_INFO(m_cabling->name() << " is OK");
    } else {
      ATH_MSG_FATAL("Old TGCcablingSvc(octant segmentation) can not be used !");
      return StatusCode::FAILURE;
    }
    
    // determine version of CW
    // !! NOTE : CW version is determined by jobOption of VersionCW
    // !!        independent from TrigConfigSvc
    // default set is 0x0007019
    //   00_00_0000 -> TILE_EIFI_BW
    //   see https://twiki.cern.ch/twiki/bin/view/Atlas/TgcSectorLogicLUT
    
    // Since m_VerCW is set as default value above,
    // Here don't overwrite by any version if proper version number is provided.
    std::string ver=m_VerCW.value();
    if((ver.size() != 10)) {
      // default CW is v00070022
      ver= "00_07_0022";
      m_VerCW = ver;
    }
    
    if (!m_tgcArgs.USE_CONDDB()) ATH_MSG_INFO("TGC CW version of " << ver << " is selected");
    
    // check Inner /TileMu
    std::vector<std::string> vers = TGCDatabaseManager::splitCW(ver, '_');
    if (m_tgcArgs.USE_INNER()) {
      if (vers.size() == 3) m_tgcArgs.set_USE_INNER( (vers[1] != "00") );
    }
    if (m_tgcArgs.TILE_MU()) {
      if (vers.size() == 3) m_tgcArgs.set_TILE_MU( (vers[0] != "00") );
    }
    
    // create DataBase and TGCElectronicsSystem
    //m_db = new TGCDatabaseManager(m_VerCW);
    m_system = new TGCElectronicsSystem(&m_tgcArgs,m_db);
    
    m_TimingManager = new TGCTimingManager (m_readCondKey);
    m_TimingManager->setBunchCounter(0);
    m_nEventInSector = 0;

    ATH_MSG_DEBUG("finished LVL1TGCTrigger::getCabling()");

    return sc;
  }

  StatusCode LVL1TGCTrigger::fillTMDB()
  {
    ATH_MSG_DEBUG("fillTMDB");
    StatusCode sc =  StatusCode::SUCCESS;
    TGCTMDB* tmdb = m_system->getTMDB();
    
    // clear TMDB
    tmdb->eraseOutput();
    
    SG::ReadHandle<TileMuonReceiverContainer> readTileMuonReceiverContainer(m_keyTileMu);
    if(!readTileMuonReceiverContainer.isValid()){
      ATH_MSG_ERROR("Cannot retrieve Tile Muon Receiver Container.");
      return StatusCode::FAILURE;
    }
    const TileMuonReceiverContainer* tileMuRecCont = readTileMuonReceiverContainer.cptr();

    // loop over all TileMuonReceiverObj in container
    TileMuonReceiverContainer::const_iterator tmItr = tileMuRecCont->begin();

    const TileMuonReceiverObj * tmObj_Thresholds = *tmItr;
    if ( (tmObj_Thresholds->GetThresholds()).size() == 4) {
      float thresholds[4];
      for (size_t ip=0;ip<4;ip++){
        thresholds[ip] = (tmObj_Thresholds->GetThresholds()).at(ip);
      }
      ATH_MSG_DEBUG("thresholds[] :" << thresholds[0] << thresholds[1] << thresholds[2] << thresholds[3] );
      ATH_MSG_DEBUG("type of GetThreshold : " << typeid((tmObj_Thresholds->GetThresholds())).name()
                    << "  ID of GetThreshold : "
                    << tmObj_Thresholds->GetID() );
    }


    //clear tmobj_Threshols
    tmObj_Thresholds = 0;
    // m_id and decision , etc ... from
    ++tmItr;

    for ( ; tmItr != tileMuRecCont->end(); ++tmItr) {

      const TileMuonReceiverObj * tmObj = *tmItr;
      // Tile Module
      int moduleID = tmObj-> GetID();
      int sideID   = (moduleID & 0xf00) >> 8;
      int mod      = (moduleID & 0x0ff);
      // TMDB decision
      bool tile2SL[4];
      //     [0]       [1]       [2]      [3]
      //     d5d6_hi   d5d6_lo   d6_hi    d6_lo
      for (size_t ip=0;ip<4;ip++){
        tile2SL[ip] = (tmObj->GetDecision()).at(ip);
      }
      //if ( moduleID < 300 || (moduleID > 363 && moduleID < 400) || moduleID > 463 ||
      if ( mod < 0 || mod > 63 || (sideID !=3 && sideID !=4) ||
           ((tmObj->GetDecision()).size() != 4) ) {
        continue;
      } else {

        // side 0: a-side, 1: c-side,  3: NA
        // mod 0~63
        int side = 3;
        if (sideID == 3) {
          side = 0;
        } else if (sideID == 4) {
          side = 1;
        }
        // setOutput(side, mod, hit56, hit6) -> hit56, 6 -> 0: No Hit, 1: Low, 2: High, 3: NA
        int hit56 = 3, hit6 = 3;
        if (tile2SL[0] == true && tile2SL[1] == false) {
          hit56 = 2;
        } else if (tile2SL[0] == false && tile2SL[1] == true) {
          hit56 = 1;
        } else if (tile2SL[0] == false && tile2SL[1] == false) {
          hit56 = 0;
        }

        if (tile2SL[2] == true && tile2SL[3] == false) {
          hit6 = 2;
        } else if (tile2SL[2] == false && tile2SL[3] == true) {
          hit6 = 1;
        } else if (tile2SL[2] == false && tile2SL[3] == false) {
          hit6 = 0;
        }

        int prehit56=(tmdb->getOutput(side, mod))->GetHit56();
        int prehit6=(tmdb->getOutput(side, mod))->GetHit6();
        if(prehit56 != 3 && prehit56 > hit56) { hit56=prehit56; }
        if(prehit6 != 3 && prehit6 > hit6) { hit6=prehit6; }

        tmdb->setOutput(side, mod, hit56, hit6);
      }
    }

    return sc;
  }



  //----------------------------------- 
  //NSW input
  //----------------------------------
  StatusCode LVL1TGCTrigger::fillNSW(){
    ATH_MSG_DEBUG("fillNSW");
    StatusCode sc = StatusCode::SUCCESS;
    std::shared_ptr<TGCNSW> nsw = m_system->getNSW();
    nsw->eraseOutput();

    //The following part will be available when NSW Trigger Output is available.

    /*
    SG::ReadHandle<Muon::NSW_TrigRawDataContainer> readNSW_TrigRawDataContainer(m_keyNSWTrigOut);
    if(!readNSW_TrigRawDataContainer.isValid()){
      ATH_MSG_ERROR("Cannot retrieve NSW TrigRawData Container.");
      return StatusCode::FAILURE;
    }
    const Muon::NSW_TrigRawDataContainer* nsw_TrigRawDataContainer = readNSW_TrigRawDataContainer.cptr();
    for(const Muon::NSW_TrigRawData* nsw_sector : *nsw_TrigRawDataContainer){
      for(const Muon::NSW_TrigRawDataSegment* nsw_trk : *nsw_sector){
	nsw->setOutput(nsw_sector->sideId(),        // side
		       //nsw_sector->sectorId(), // Sector number in NSW
		       //nsw_trk->rIndex(),      // R-index
		       //nsw_trk->phiIndex(),    // Phi-index
		       //nsw_trk->deltaTheta() // Delta theta index
	              );
      }
    } 
    */
  

    if(sc.isFailure()){
      ATH_MSG_WARNING("Couldn't retrieve NSW trigger output");
    }
    return sc; 
  }

} //end of namespace bracket

