/** emacs: this is -*- c++ -*- **/
/**
 **   @file    MDT_RegSelCondAlg.h        
 **                   
 **   @author  sutt
 **   @date    Tue  4 Feb 2020 15:25:00 CET
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/
 
#ifndef MDT_RegSelCondAlg_h
#define MDT_RegSelCondAlg_h

#include "MuonRegSelCondAlg.h"
#include "MuonCondData/MdtCondDbData.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"

class MDT_RegSelCondAlg : public MuonRegSelCondAlg { 

public:

  MDT_RegSelCondAlg( const std::string& name, ISvcLocator* pSvcLocator );

  std::unique_ptr<RegSelSiLUT> createTable( const EventContext& ctx, EventIDRange& id_range ) const override;   

  virtual StatusCode initialize() override;

  SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingKey
    { this, "Cabling", "MuonMDT_CablingMap", "Key of output MDT cabling map" };

  SG::ReadCondHandleKey<MdtCondDbData> m_condKey{this, "Conditions", 
                                                       "MdtCondDbData", "Key of the MDT conditions to reject dead tubes"};
};

#endif // MDT_RegSelCondAlg_h
