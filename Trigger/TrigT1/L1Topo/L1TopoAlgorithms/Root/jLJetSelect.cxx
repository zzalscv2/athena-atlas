/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//  jLJetSelect.cxx
//  TopoCore
//  algorithm to create abbreviated jLJet lists
//
#include "L1TopoAlgorithms/jLJetSelect.h"
#include "L1TopoEvent/TOBArray.h"
#include "L1TopoEvent/jLJetTOBArray.h"
#include "L1TopoEvent/GenericTOB.h"
#include <algorithm>

REGISTER_ALG_TCS(jLJetSelect)


// constructor
TCS::jLJetSelect::jLJetSelect(const std::string & name) :
   SortingAlg(name)
{
   defineParameter( "InputWidth", 64 ); // for FW
   defineParameter( "InputWidth1stStage", 16 ); // for FW
   defineParameter( "OutputWidth", 10 );
   defineParameter( "MinET", 0 );
   defineParameter( "MinEta", 0 );
   defineParameter( "MaxEta", 196 );
}


TCS::jLJetSelect::~jLJetSelect()
{}

TCS::StatusCode
TCS::jLJetSelect::initialize() {
   m_numberOfJets = parameter("OutputWidth").value();
   m_et = parameter("MinET").value();
   m_minEta = parameter("MinEta").value();
   m_maxEta = parameter("MaxEta").value();
   return TCS::StatusCode::SUCCESS;
}




TCS::StatusCode
TCS::jLJetSelect::sort(const InputTOBArray & input, TOBArray & output) {

  const jLJetTOBArray & jets = dynamic_cast<const jLJetTOBArray&>(input);

  // fill output array with GenericTOBs builds from jets
  for(jLJetTOBArray::const_iterator jet = jets.begin(); jet!= jets.end(); ++jet ) {
    unsigned int Et = parType_t((*jet)->Et()); 
    if( Et <= m_et ) continue; // ET cut
    if ( parType_t(std::abs((*jet)-> eta())) < m_minEta) continue; 
    if ( parType_t(std::abs((*jet)-> eta())) > m_maxEta) continue;      	

    output.push_back( GenericTOB(**jet) );
  }

   // keep only max number of jets
   int par = m_numberOfJets ;
   unsigned int maxNumberOfJets = std::clamp(par, 0, std::abs(par));
   if(maxNumberOfJets>0) {

     if (output.size()> maxNumberOfJets) {setOverflow(true);}

      while( output.size()> maxNumberOfJets ) {
         output.pop_back();
      }
   }   
   return TCS::StatusCode::SUCCESS;
}

