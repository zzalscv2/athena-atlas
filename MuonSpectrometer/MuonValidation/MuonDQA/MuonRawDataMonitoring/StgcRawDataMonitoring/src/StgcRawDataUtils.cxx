/*                                                                             
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration                              
*/

///////////////////////////////////////////////////////////////////////////                          
// Utils for the main sTGCRawDataMonAlg.cxx                                                            
// Part of StgcRawDataMonAlg.h                                                                         
// see StgcRawDataMonAlg.cxx                                
///////////////////////////////////////////////////////////////////////////                      
         
#include "StgcRawDataMonitoring/StgcRawDataMonAlg.h"

int sTgcRawDataMonAlg::getSectors(const Identifier& id) const { 
  return m_idHelperSvc -> sector(id)*(m_idHelperSvc -> stationEta(id) > 0 ? 1. : -1.);
}
