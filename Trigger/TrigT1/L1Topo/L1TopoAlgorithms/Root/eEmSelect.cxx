/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
//  eEmSelect.cxx
//  TopoCore
//  Algorithm to select the abbreviated list of eEMs , no order is applied
//
#include "L1TopoAlgorithms/eEmSelect.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/eEmTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(eEmSelect)

// constructor
TCS::eEmSelect::eEmSelect(const std::string & name) : SortingAlg(name) {
   defineParameter( "InputWidth", 120 ); // for fw
   defineParameter( "InputWidth1stStage", 30 ); // for fw
   defineParameter( "OutputWidth", 6 );
   defineParameter( "MinET", 0 );
   defineParameter( "IsoMask", 0);
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 63);
   defineParameter( "DoIsoCut", 1);
}


// destructor
TCS::eEmSelect::~eEmSelect() {}

TCS::StatusCode
TCS::eEmSelect::initialize() {
   m_numberOfeEms = parameter("OutputWidth").value();
   m_et = parameter("MinET").value();
   m_iso = parameter("IsoMask").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   m_doIsoCut = parameter("DoIsoCut").value();
   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::eEmSelect::sort(const InputTOBArray & input, TOBArray & output) {

   const eEmTOBArray & eems = dynamic_cast<const eEmTOBArray&>(input);

   // fill output array with GenericTOB buildt from eEms
   for(eEmTOBArray::const_iterator eem = eems.begin(); eem!= eems.end(); ++eem ) {
      const GenericTOB gtob(**eem);

      if( parType_t((*eem)->Et()) <= m_et ) continue; // ET cut
      // isolation cut
      if (m_doIsoCut && (m_iso != 0 )) {
          if((parType_t((*eem)->isolation()) & m_iso) != m_iso ) continue;
      }
      // eta cut
      if (parType_t(std::abs((*eem)-> eta())) < m_minEta) continue; 
      if (parType_t(std::abs((*eem)-> eta())) > m_maxEta) continue;  
      
      output.push_back( gtob );
   }


   // keep only max number of eEms
   int par = m_numberOfeEms ;
   unsigned int maxNumberOfeEms = (unsigned int)(par<0?0:par);
   if(maxNumberOfeEms>0) {
      while( output.size()> maxNumberOfeEms ) {
         output.pop_back();
      }
   }
   return TCS::StatusCode::SUCCESS;
}

