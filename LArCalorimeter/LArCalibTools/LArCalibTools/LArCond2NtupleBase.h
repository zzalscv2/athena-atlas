/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef LARCALIBTOOLS_LARCOND2NTUPLEBASE_H
#define LARCALIBTOOLS_LARCOND2NTUPLEBASE_H

//#include "GaudiKernel/Algorithm.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ToolHandle.h"

#include "LArElecCalib/ILArFEBTempTool.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

class HWIdentifier;
class LArOnlineID_Base;
class CaloCell_Base_ID;
class StoreGateSvc;
class LArEM_Base_ID;
class LArHEC_Base_ID;
class LArFCAL_Base_ID;
class MsgStream;
class CaloDetDescrManager_Base;

class LArCond2NtupleBase : public AthAlgorithm {

 public:
  LArCond2NtupleBase(const std::string & name, ISvcLocator * pSvcLocator);
  ~LArCond2NtupleBase();

  //Standard algo methods
  StatusCode initialize();
  StatusCode execute()    {return StatusCode::SUCCESS;}
  //Finalize needs to be implemented by the deriving class


  //StatusCode initializeBase(const std::string& path, const std::string& name);
  bool fillFromIdentifier(const HWIdentifier& id); //returns true if connected

  const SG::ReadCondHandleKey<LArOnOffIdMapping>& cablingKey() const;

 private:
  bool m_initialized;

 protected:   
  Gaudi::Property< bool > m_addBC{this, "AddBadChannelInfo", true, "dump BadChan info ?"};
  Gaudi::Property< bool > m_addFEBTemp{this, "AddFEBTempInfo", false, "dump FEB temperature info ?"};
  Gaudi::Property< bool > m_isSC{this, "isSC", false, "are we working with SC?"};
  Gaudi::Property< bool > m_isFlat{this, "isFlat", false, "are we working with Flat conditions ?"};
  Gaudi::Property< bool > m_OffId{this, "OffId", false, "dump also offline ID ?"};
  Gaudi::Property< bool > m_addHash{this, "AddHash", false, "add also ID hash info ?"};
  Gaudi::Property< bool > m_addCalib{this, "AddCalib", false, "add also calib line info info ?"};
  Gaudi::Property< bool > m_realgeom{this, "RealGeometry", false, "add real geometry values ?"};
  Gaudi::Property< bool > m_expandId{this,"ExpandId", true ,"add online Id decoded fields ?"}; 

  enum {NOT_VALID = -999};

  std::string m_ntpath, m_ntTitle;

  //Ntuple pointer
  NTuple::Tuple* m_nt;

  //Ntuple variables:
  NTuple::Item<long> m_detector, m_region, m_layer, m_eta, m_phi, m_onlChanId, m_oflChanId;
  NTuple::Item<long> m_pos_neg, m_barrel_ec, m_FT, m_slot, m_channel;
  NTuple::Item<long> m_calibLine,m_badChanWord;
  NTuple::Item<long> m_isConnected;
  NTuple::Item<long> m_chanHash, m_febHash, m_oflHash;
  
  NTuple::Item<float> m_reta, m_rphi;
  NTuple::Item<float> m_FEBTemp1, m_FEBTemp2;


  StoreGateSvc* m_detStore; 
  const LArEM_Base_ID* m_emId;
  const LArHEC_Base_ID* m_hecId;
  const LArFCAL_Base_ID* m_fcalId;
  const LArOnlineID_Base* m_onlineId;
  const CaloCell_Base_ID* m_caloId;
  ToolHandle<ILArFEBTempTool> m_FEBTempTool;

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this,"CablingSCKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey{this, "BadChanKey", "LArBadChannel", "SG bad channels key"};
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapKey{this,"CalibMapKey","LArCalibLineMap","SG Key of calib line mapping object"};
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapSCKey{this,"CalibMapSCKey","LArCalibIdMapSC","SG Key of calib line mapping object"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store"};
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this, "CaloSuperCellDetDescrManager", "CaloSuperCellDetDescrManager", "SG key of the resulting CaloSuperCellDetDescrManager" };

};
#endif
