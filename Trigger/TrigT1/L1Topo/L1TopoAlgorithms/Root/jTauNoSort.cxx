/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
// jTauNoSort.cxx
// TopoCore
// Algorithm to generate ALL lists of jTau TOBs

#include "L1TopoAlgorithms/jTauNoSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jTauTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(jTauNoSort)

// constructor
TCS::jTauNoSort::jTauNoSort(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 120 ); // for fw
   defineParameter( "OutputWidth", 120 );    
}


// destructor
TCS::jTauNoSort::~jTauNoSort() {}

TCS::StatusCode
TCS::jTauNoSort::initialize() {
   m_numberOfjTaus = parameter("OutputWidth").value();
   m_iso = parameter("Isolation").value();

   m_isoFW_JTAU = isolationFW_JTAU();

   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::jTauNoSort::sort(const InputTOBArray & input, TOBArray & output) {

   const jTauTOBArray & clusters = dynamic_cast<const jTauTOBArray&>(input);

   // fill output array with GenericTOB built from clusters
   for(jTauTOBArray::const_iterator jtau = clusters.begin(); jtau!= clusters.end(); ++jtau ) {

      // Isolation cut
      if ( !isocut(m_iso, convertIsoToBit(*jtau)) ) {continue;}
 
      const GenericTOB gtob(**jtau);
      output.push_back( gtob );
   }


   // keep only max number of clusters
   int par = m_numberOfjTaus ;
   unsigned int maxNumberOfjTaus = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfjTaus>0) {
      while( output.size()> maxNumberOfjTaus ) {
         output.pop_back();
      }
   }
   return TCS::StatusCode::SUCCESS;
}


unsigned int
TCS::jTauNoSort::convertIsoToBit(const TCS::jTauTOB * jtau) const {
  unsigned int bit = 0;

  // Assign the tightest accept WP as default bit
  if( jtau->EtIso()*1024 < jtau->Et()*m_isoFW_JTAU.at("Loose") ) bit = 1;
  if( jtau->EtIso()*1024 < jtau->Et()*m_isoFW_JTAU.at("Medium") ) bit = 2;
  if( jtau->EtIso()*1024 < jtau->Et()*m_isoFW_JTAU.at("Tight") ) bit = 3;

  return bit;
}

