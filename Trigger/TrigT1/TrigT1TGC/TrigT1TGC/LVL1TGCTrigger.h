/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1TGC_LVL1TGCTRIGGER_H
#define TRIGT1TGC_LVL1TGCTRIGGER_H

#include "AthenaBaseComps/AthAlgorithm.h"

// STL
#include <string>
#include <vector>
#include <map>

// Gaudi includes
#include "Gaudi/Property.h"

// Other stuff
#include "TrigT1Interfaces/SlinkWord.h"
#include "TrigT1Interfaces/Lvl1MuCTPIInput.h"
#include "TrigT1Interfaces/Lvl1MuCTPIInputPhase1.h"
#include "Identifier/Identifier.h"

// EIFI-SL connection
#include "TrigT1TGC/TGCInnerTrackletSlotHolder.h"

#include "StoreGate/ReadCondHandle.h"
#include "MuonCondSvc/TGCTriggerData.h"
#include "TGCTriggerCondSvc/TGCTriggerLUTs.h"

// BS metadata container
#include "ByteStreamData/ByteStreamMetadataContainer.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonDigitContainer/TgcDigit.h"

#include "TrigT1TGC/TGCArguments.h"
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonRDO/TgcRdoContainer.h"

// Tile-Muon
#include "TileEvent/TileMuContainer.h"
#include "TileEvent/TileMuonReceiverObj.h"

//NSW Trigger Output
#include "MuonRDO/NSW_TrigRawDataContainer.h"

// RPC BIS78 Trigger Output
#include "MuonRDO/RpcBis78_TrigRawDataContainer.h"

class TgcRdo;
class TgcRawData;
class ITGCcablingSvc;


namespace LVL1TGCTrigger {
  
class TGCSector;
class TGCTrackSelectorOut;
class TGCElectronicsSystem;
class TGCTimingManager;
class TGCDatabaseManager;
class TGCEvent;
  
class LVL1TGCTrigger : public AthAlgorithm
{
 public:
    
    /** standard constructor and destructor for algorithms
     */
    LVL1TGCTrigger( const std::string& name, ISvcLocator* pSvcLocator ) ;
    virtual ~LVL1TGCTrigger();
    
    // standard algorithm methods:
    virtual StatusCode initialize() override;
    virtual StatusCode start() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;
    
 private:
    StatusCode processOneBunch(const TgcDigitContainer*,
			       LVL1MUONIF::Lvl1MuCTPIInputPhase1*,
			       std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&);
    void doMaskOperation(const TgcDigitContainer* ,std::map<Identifier, int>& );
    void fillTGCEvent(const std::map<Identifier, int>& ,  TGCEvent&);
    
    // record bare-RDO for LowPT coincidences (on m_OutputTgcRDO=True):
    void recordRdoSLB(TGCSector *, std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&);
    
    // record bare-RDO for HighPT coincidences (on OutputTgcRDO=True):
    void recordRdoHPT(TGCSector *, std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&);
    
    // record bare-RDO for Inner coincidences (on OutputTgcRDO=True):
    void recordRdoInner(TGCSector *, std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&);
    
    // record bare-RDO for R-phi coincidences (on m_OutputTgcRDO=True):
    void recordRdoSL(TGCSector *, std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&);
    
    // Retrieve Masked channel list
    StatusCode getMaskedChannel();
    
    // pointers to various external services
    const ITGCcablingSvc*       m_cabling ;
    
    // useful functions
    int getCharge(int dR, int Zdir);
    void extractFromString(const std::string&, std::vector<int>&);
    //bool addRawData(TgcRawData * rawdata,
    bool addRawData(std::unique_ptr<TgcRawData> rawdata,
		    std::map<std::pair<int, int>, std::unique_ptr<TgcRdo>>&  tgcrdo );
    int getLPTTypeInRawData(int type);
    void FillSectorLogicData(LVL1MUONIF::Lvl1MuSectorLogicDataPhase1* sldata,
			     const TGCTrackSelectorOut *trackSelectorOut);
    
    
    // Properties

    // Location of LVL1MUONIF::Lvl1MuSectorLogicData (output from SL)
    StringProperty m_keyMuCTPIInput_TGC{this,"MuCTPIInput_TGC","L1MuctpiStoreTGC"};

    StringProperty    m_MaskFileName12{this,"MaskFileName12",""};   //!< property, see @link LVL1TGCTrigger::LVL1TGCTrigger @endlink
    ShortProperty     m_CurrentBunchTag{this,"CurrentBunchTag",TgcDigit::BC_CURRENT};  //!< property, see @link LVL1TGCTrigger::LVL1TGCTrigger @endlink
    BooleanProperty   m_ProcessAllBunches{this,"ProcessAllBunhes",true};
    BooleanProperty   m_OutputTgcRDO{this,"OutputTgcRDO",true};   //!< property, see @link LVL1TGCTrigger::LVL1TGCTrigger @endlink
    
    // expert usage
    BooleanProperty m_USE_CONDDB{this, "USE_CONDDB", true};  //< flag to use the Condition DB
    BooleanProperty m_SHPTORED  {this, "SHPTORED",   true};  //< flag for E1/E2 chamber ORED in Strip HPT
    BooleanProperty m_USEINNER  {this, "USEINNER",   true};  //< flag for using Inner Station for SL
    BooleanProperty   m_INNERVETO{this,"INNERVETO",true}; // flag for using VETO by Inner Station for SL
    BooleanProperty   m_FULLCW{this,"FULLCW",false};   // flag for using differne CW for each octant
    BooleanProperty   m_TILEMU{this,"TILEMU",true};    // flag for using TileMu
    BooleanProperty   m_USENSW{this,"USENSW",false};     // flag for using NSW
    BooleanProperty   m_FORCENSWCOIN{this,"FORCENSWCOIN",true};     // flag to enable innerCoincidenceFlag at the TGC sectors expecting NSW hits even if there is no NSW hit
    BooleanProperty   m_USEBIS78{this,"USEBIS78",false};     // flag for using RPC BIS78
    BooleanProperty   m_useRun3Config{this,"useRun3Config",false}; // flag for using switch between Run3 and Run2 algorithms

   StringProperty     m_NSWSideInfo{this,"NSWSideInfo",""};// Information about NSW geometry. It should be "" or "AC" or "A" or "C"

    
    bool              m_firstTime{true};
    uint16_t m_bctagInProcess{TgcDigit::BC_UNDEFINED};

    TGCDatabaseManager *m_db;
    std::unique_ptr<TGCTimingManager> m_TimingManager;
    std::unique_ptr<TGCElectronicsSystem> m_system;
    
    int m_nEventInSector;
    
    // EIFI-SL connection
    TGCInnerTrackletSlotHolder m_innerTrackletSlotHolder;
    
    // getCabling method
    StatusCode getCabling();
    
    // log
    bool                m_debuglevel;
    
    TGCArguments m_tgcArgs;
    TGCArguments* tgcArgs();

    SG::WriteHandleKey<TgcRdoContainer> m_keyTgcRdo{this,"TgcRdo","TGCRDO2","Location of TgcRdoContainer"};
    SG::ReadHandleKey<TgcRdoContainer> m_keyTgcRdoIn{this,"InputRDO","TGCRDO","Location of input TgcRdoContainer"};
    SG::ReadHandleKey<TgcDigitContainer> m_keyTgcDigit{this,"InputData_perEvent","TGC_DIGITS","Location of TgcDigitContainer"};
    SG::ReadHandleKey<TileMuonReceiverContainer> m_keyTileMu{this,"TileMuRcv_Input","TileMuRcvCnt","Location of TileMuonReceiverContainer"};
    SG::ReadHandleKey<Muon::NSW_TrigRawDataContainer> m_keyNSWTrigOut{this,"NSWTrigger_Input","L1_NSWTrigContainer","Location of NSW_TrigRawDataContainer"};
    SG::ReadHandleKey<Muon::RpcBis78_TrigRawDataContainer> m_keyBIS78TrigOut{this,"BIS78Trig_Input","BIS78TrigContainer","Location of RpcBis78_TrigRawDataContainer"};
    SG::ReadCondHandleKey<TGCTriggerData> m_readCondKey{this,"ReadCondKey","TGCTriggerData"};
    SG::ReadCondHandleKey<TGCTriggerLUTs> m_readLUTs_CondKey{this,"ReadLUTCondKey","TGCTriggerLUTs"};
    SG::WriteHandleKey<LVL1MUONIF::Lvl1MuCTPIInputPhase1> m_muctpiPhase1Key{this, "MuctpiPhase1LocationTGC", "L1MuctpiStoreTGC", "Location of muctpiPhase1 for Tgc"};

    //StoreGate key for the ByteStreamMetadata container to retrieve detector mask
    SG::ReadHandleKey<ByteStreamMetadataContainer> m_bsMetaDataContRHKey {this, "ByteStreamMetadataRHKey", "ByteStreamMetadata", "Location to retrieve the detector mask"};

    /// mask channel map
    std::map<Identifier, int> m_MaskedChannel;

};  // class LVL1TGCTrigger

inline TGCArguments* LVL1TGCTrigger::tgcArgs() {
  return &m_tgcArgs;
}
  
}  // namespace LVL1TGCTrigger

#endif  // TRIGT1TGC_LVL1TGCTRIGGER_H
