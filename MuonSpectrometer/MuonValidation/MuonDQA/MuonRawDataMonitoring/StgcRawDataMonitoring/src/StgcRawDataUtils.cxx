/*                                                                             
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration                              
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

int sTgcRawDataMonAlg::getLayer(int multiplet, int gasGap) const {
  return 4*(multiplet -1 ) + gasGap;
}

int32_t sTgcRawDataMonAlg::sourceidToSector(uint32_t sourceid, bool isSideA) const {
  uint32_t sectorNumber = sourceid & 0xf;
  return (isSideA) ? sectorNumber + 1: -sectorNumber - 1;
}
