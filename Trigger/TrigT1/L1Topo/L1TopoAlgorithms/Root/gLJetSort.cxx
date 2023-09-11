/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  gLJetSort.cxx
//  TopoCore
//  algorithm to make sorted gLJets lists
//
#include "L1TopoAlgorithms/gLJetSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/gLJetTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(gLJetSort)

bool SortByEtLargestgLJet(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2)
{
   return tob1->Et() > tob2->Et();
}

// constructor
TCS::gLJetSort::gLJetSort(const std::string & name) :
   SortingAlg(name)
{
   defineParameter( "InputWidth", 64 ); // for FW
   defineParameter( "InputWidth1stStage", 16 ); // for FW
   defineParameter( "OutputWidth", 10 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
}


TCS::gLJetSort::~gLJetSort()
{}



TCS::StatusCode
TCS::gLJetSort::initialize() {
   m_numberOfgLJets = parameter("OutputWidth").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::gLJetSort::sort(const InputTOBArray & input, TOBArray & output) {
  
  const gLJetTOBArray & jets = dynamic_cast<const gLJetTOBArray&>(input);
   
  // fill output array with GenericTOBs builds from jets
  for(gLJetTOBArray::const_iterator jet = jets.begin(); jet!= jets.end(); ++jet ) {
    if ( parType_t(std::abs((*jet)-> eta())) < m_minEta) continue; 
    if ( parType_t(std::abs((*jet)-> eta())) > m_maxEta) continue;      	
    output.push_back( GenericTOB(**jet)  );
  }

   // sort
   output.sort(SortByEtLargestgLJet);
   
   // keep only max number of jets
   int par = m_numberOfgLJets;
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

