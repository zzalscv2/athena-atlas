/** emacs: this is -*- c++ -*- **/
/**
 **   @file    CSC_RegSelCondAlg.h        
 **                   
 **   @author  sutt
 **   @date    Tue  4 Feb 2020 15:25:00 CET
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/
 
#ifndef CSC_RegSelCondAlg_h
#define CSC_RegSelCondAlg_h


#include "MuonRegSelCondAlg.h"


class CSC_RegSelCondAlg : public MuonRegSelCondAlg {

public:

  CSC_RegSelCondAlg( const std::string& name, ISvcLocator* pSvcLocator );

  std::unique_ptr<RegSelSiLUT> createTable( const EventContext& ctx, EventIDRange& id_range ) const override;

  
};

#endif // CSC_RegSelCondAlg_h
