/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  eTauSelect.cxx
//  TopoCore
//  Algorithm to select the abbreviated list of eTaus , no order is applied
//
#include "L1TopoAlgorithms/eTauSelect.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/eTauTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(eTauSelect)

// constructor
TCS::eTauSelect::eTauSelect(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 120 ); // for fw
   defineParameter( "InputWidth1stStage", 30 ); // for fw
   defineParameter( "OutputWidth", 6 );
   defineParameter( "MinET", 0 );
   defineParameter( "RCoreMin", 0 );
   defineParameter( "RHadMin", 0 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
}


// destructor
TCS::eTauSelect::~eTauSelect() {}

TCS::StatusCode
TCS::eTauSelect::initialize() {
   m_numberOfeTaus = parameter("OutputWidth").value();
   m_et = parameter("MinET").value();
   m_minRCore = parameter("RCoreMin").value();
   m_minRHad = parameter("RHadMin").value();
   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::eTauSelect::sort(const InputTOBArray & input, TOBArray & output) {

   const eTauTOBArray & clusters = dynamic_cast<const eTauTOBArray&>(input);

   // fill output array with GenericTOB buildt from clusters
   for(eTauTOBArray::const_iterator etau = clusters.begin(); etau!= clusters.end(); ++etau ) {

      if( parType_t((*etau)->Et()) <= m_et ) continue; // ET cut

      // Isolation cut
      if ( !isocut(m_minRCore, (*etau)-> rCore()) ) {continue;}
      if ( !isocut(m_minRHad, (*etau)-> rHad()) ) {continue;}

      const GenericTOB gtob(**etau);
      output.push_back( gtob );
   }


   // keep only max number of clusters
   int par = m_numberOfeTaus ;
   unsigned int maxNumberOfeTaus = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfeTaus>0) {

     if (output.size()> maxNumberOfeTaus) {setOverflow(true);}

     while( output.size()> maxNumberOfeTaus ) {
         output.pop_back();
     }
   }
   return TCS::StatusCode::SUCCESS;
}

