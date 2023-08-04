/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  jLJetSort.cxx
//  TopoCore
//  algorithm to make sorted jLJets lists
//
#include "L1TopoAlgorithms/jLJetSort.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jLJetTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(jLJetSort)

bool SortByEtLargestjLJet(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2)
{
   return tob1->Et() > tob2->Et();
}

// constructor
TCS::jLJetSort::jLJetSort(const std::string & name) :
   SortingAlg(name)
{
   defineParameter( "InputWidth", 64 ); // for FW
   defineParameter( "InputWidth1stStage", 16 ); // for FW
   defineParameter( "OutputWidth", 10 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
}


TCS::jLJetSort::~jLJetSort()
{}



TCS::StatusCode
TCS::jLJetSort::initialize() {
   m_numberOfJets = parameter("OutputWidth").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   return TCS::StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::jLJetSort::sort(const InputTOBArray & input, TOBArray & output) {
  
  const jLJetTOBArray & jets = dynamic_cast<const jLJetTOBArray&>(input);
   
  // fill output array with GenericTOBs builds from jets
  for(jLJetTOBArray::const_iterator jet = jets.begin(); jet!= jets.end(); ++jet ) {
    if ( parType_t(std::abs((*jet)-> eta())) < m_minEta) continue; 
    if ( parType_t(std::abs((*jet)-> eta())) > m_maxEta) continue;      	
    output.push_back( GenericTOB(**jet)  );
  }

   // sort
   output.sort(SortByEtLargestjLJet);
   
   // keep only max number of jets
   int par = m_numberOfJets;
   unsigned int maxNumberOfJets = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfJets>0) {
      while( output.size()> maxNumberOfJets ) {
         output.pop_back();
      }
   }   
   return TCS::StatusCode::SUCCESS;
}

