/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCaloEventTPCnv/TrigTauClusterCnv_tlp1.h"


//constructor
TrigTauClusterCnv_tlp1::TrigTauClusterCnv_tlp1(){

  // add the "main" base class converter (ie. TrigTauClusterCnv)
  addMainTPConverter();

  // add all converters needed in the top level converter
  // do not change the order of adding converters
  addTPConverter( &m_trigCaloClusterCnv );
}

void TrigTauClusterCnv_tlp1::setPStorage( TrigTauCluster_tlp1* storage ){


   //for the base class converter
   setMainCnvPStorage( &storage->m_trigTauCluster );

   //for all other converters defined in the base class
   m_trigCaloClusterCnv.              setPStorage( &storage->m_trigCaloCluster );

}
