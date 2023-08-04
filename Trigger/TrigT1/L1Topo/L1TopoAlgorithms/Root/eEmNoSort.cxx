/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
// eEmNoSort.cxx
// TopoCore
// Algorithm to generate ALL lists of eEm TOBs

#include "L1TopoAlgorithms/eEmNoSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/eEmTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(eEmNoSort)

// constructor
TCS::eEmNoSort::eEmNoSort(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 120 ); // for fw
   defineParameter( "OutputWidth", 120 );    
   defineParameter( "REtaMin", 0);
   defineParameter( "RHadMin", 0);
   defineParameter( "WsTotMin", 0);
}


// destructor
TCS::eEmNoSort::~eEmNoSort() {}

TCS::StatusCode
TCS::eEmNoSort::initialize() {
   m_numberOfeEms = parameter("OutputWidth").value();
   m_minREta = parameter("REtaMin").value();
   m_minRHad = parameter("RHadMin").value();
   m_minWsTot = parameter("WsTotMin").value();
   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::eEmNoSort::sort(const InputTOBArray & input, TOBArray & output) {

   const eEmTOBArray & clusters = dynamic_cast<const eEmTOBArray&>(input);

   // fill output array with GenericTOB built from clusters
   for(eEmTOBArray::const_iterator eem = clusters.begin(); eem!= clusters.end(); ++eem ) {

      // Isolation cut
      if ( !isocut(m_minREta, (*eem)-> Reta()) ) {continue;}
      if ( !isocut(m_minRHad, (*eem)-> Rhad()) ) {continue;}
      if ( !isocut(m_minWsTot, (*eem)-> Wstot()) ) {continue;}
     
      const GenericTOB gtob(**eem);
      output.push_back( gtob );
   }


   // keep only max number of clusters
   int par = m_numberOfeEms ;
   unsigned int maxNumberOfeEms = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfeEms>0) {
      while( output.size()> maxNumberOfeEms ) {
         output.pop_back();
      }
   }
   return TCS::StatusCode::SUCCESS;
}

