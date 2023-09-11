/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  jEmSort.cxx
//  TopoCore
//  algorithm to make sorted jEms lists
//
#include "L1TopoAlgorithms/jEmSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jEmTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(jEmSort)

bool SortByEtLargestjEm(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2)
{
   return tob1->Et() > tob2->Et();
}

// constructor
TCS::jEmSort::jEmSort(const std::string & name) :
   SortingAlg(name)
{
   defineParameter( "InputWidth", 64 ); // for FW
   defineParameter( "InputWidth1stStage", 16 ); // for FW
   defineParameter( "OutputWidth", 10 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
   defineParameter( "IsoMin", 0 );
   defineParameter( "Frac1Min", 0 );
   defineParameter( "Frac2Min", 0 );
}


TCS::jEmSort::~jEmSort()
{}



TCS::StatusCode
TCS::jEmSort::initialize() {
   m_numberOfJets = parameter("OutputWidth").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   m_iso = parameter("IsoMin").value();
   m_frac1 = parameter("Frac1Min").value();
   m_frac2 = parameter("Frac2Min").value();
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::jEmSort::sort(const InputTOBArray & input, TOBArray & output) {
  
   const jEmTOBArray & jets = dynamic_cast<const jEmTOBArray&>(input);
   
   // fill output array with GenericTOBs builds from jets
   for(jEmTOBArray::const_iterator jet = jets.begin(); jet!= jets.end(); ++jet ) {
     // Isolation cuts
     if ( !isocut(m_iso, (*jet)-> isolation()) ) continue;
     if ( !isocut(m_frac1, (*jet)-> frac1()) ) continue;
     if ( !isocut(m_frac2, (*jet)-> frac2()) ) continue;
     // Eta cut
     if ( parType_t(std::abs((*jet)-> eta())) < m_minEta ) continue; 
     if ( parType_t(std::abs((*jet)-> eta())) > m_maxEta ) continue;      	

     output.push_back( GenericTOB(**jet)  );
   }

   // sort
   output.sort(SortByEtLargestjEm);
   
   // keep only max number of jets
   int par = m_numberOfJets;
   unsigned int maxNumberOfJets = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfJets>0) {
      while( output.size()> maxNumberOfJets ) {
         if (output.size() == (maxNumberOfJets+1)) {
            bool isAmbiguous = output[maxNumberOfJets-1].EtDouble() == output[maxNumberOfJets].EtDouble();
            if (isAmbiguous) { output.setAmbiguityFlag(true); }
         }
         output.pop_back();
      }
   }   
   return TCS::StatusCode::SUCCESS;
}

