/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMonitoring/JetSubStructureHistos.h"
#include "TString.h"
#include <map>

#define toGeV 1/1000.

JetSubStructureHistos::JetSubStructureHistos(const std::string &t) : JetHistoBase(t) 
                                                             ,m_jetScale("JetAssignedScaleMomentum")
{
    declareProperty("JetScale", m_jetScale);
}

int JetSubStructureHistos::buildHistos(){

  // -------------- Modify histo names/titles --------
  //  if we're plotting a defined scale, modify the Histo titles and names
  
  std::map<std::string, std::string> scale2str( { 
      { "JetEMScaleMomentum" , "EMScale" } , 
      { "JetConstitScaleMomentum" , "ConstitScale" } } );
  TString scaleTag = scale2str[ m_jetScale ] ; // defaults to ""

  TString prefixn = scaleTag;
  if(prefixn != "") prefixn +="_";
  // -------------- 

  // Build and register the histos in this group : 
  TH1::AddDirectory(kFALSE); // Turn off automatic addition to gDirectory to avoid Warnings. Histos are anyway placed in their own dir later.
  m_tau21     = bookHisto( new TH1F(prefixn+"Tau21"  ,  "Jet Tau21 ;Entries", 100, 0, 1) );
  m_tau32     = bookHisto( new TH1F(prefixn+"Tau32"  ,  "Jet Tau32 ;Entries", 100, 0, 1) );
  m_tau21_wta = bookHisto( new TH1F(prefixn+"Tau21_wta"  ,  "Jet Tau21_wta ;Entries", 100, 0, 1) );
  m_tau32_wta = bookHisto( new TH1F(prefixn+"Tau32_wta"  ,  "Jet Tau32_wta ;Entries", 100, 0, 1) );
  m_C1        = bookHisto( new TH1F(prefixn+"C1"  ,  "Jet C1;Entries", 100, -1, 1) );
  m_C2        = bookHisto( new TH1F(prefixn+"C2"  ,  "Jet C2;Entries", 100, -1, 1) );
  m_D2        = bookHisto( new TH1F(prefixn+"D2"  ,  "Jet D2;Entries", 100, 0, 10) );
  


  TH1::AddDirectory(kTRUE); // Turn on automatic addition to gDirectory in case others needs it.

  // -------------- Modify histo titles.
  if(prefixn != "" ){

    // build a qualifier in the form "(EMScale, Leading Jet, ...)"
    TString qualif = "(";
    TString tags[] = { scaleTag};
    for(const auto& t : tags ) { if(qualif != "(") qualif+=",";qualif += t; }
    qualif += ")";
    // reset all titles :
    for(auto& hdata : m_vBookedHistograms ){
      TString t = hdata.hist->GetTitle(); t+=" "+qualif;
      hdata.hist->SetTitle(t );
    }
  }

  
  return 0;
}



int JetSubStructureHistos::fillHistosFromJet(const xAOD::Jet &j, float weight){
  //For definitions see JetSubStructureMomentTools

  if( j.isAvailable<float>("Tau1") && j.isAvailable<float>("Tau2") && j.isAvailable<float>("Tau3")){
    if( j.getAttribute<float>("Tau1") > 1e-8 ) m_tau21->Fill( j.getAttribute<float>("Tau2") / j.getAttribute<float>("Tau1"), weight );
    if( j.getAttribute<float>("Tau2") > 1e-8 ) m_tau32->Fill( j.getAttribute<float>("Tau3") / j.getAttribute<float>("Tau2"), weight );
  }
  if( j.isAvailable<float>("Tau1_wta") && j.isAvailable<float>("Tau2_wta") && j.isAvailable<float>("Tau3_wta")){
    if( j.getAttribute<float>("Tau1_wta") > 1e-8 ) m_tau21_wta->Fill( j.getAttribute<float>("Tau2_wta") / j.getAttribute<float>("Tau1_wta"), weight );
    if( j.getAttribute<float>("Tau2_wta") > 1e-8 ) m_tau32_wta->Fill( j.getAttribute<float>("Tau3_wta") / j.getAttribute<float>("Tau2_wta"), weight );
  }
  if( j.isAvailable<float>("ECF1") && j.isAvailable<float>("ECF2") && j.isAvailable<float>("ECF3")){
    if( j.getAttribute<float>("ECF1") > 1e-8 ) m_C1->Fill( j.getAttribute<float>("ECF2") / pow( j.getAttribute<float>("ECF1"), 2.0), weight );
    if( j.getAttribute<float>("ECF2") > 1e-8 ) {
      m_C2->Fill( ( j.getAttribute<float>("ECF3") * j.getAttribute<float>("ECF1") ) / pow( j.getAttribute<float>("ECF2"), 2.0), weight );
      m_D2->Fill( ( j.getAttribute<float>("ECF3") * pow( j.getAttribute<float>("ECF1"), 3.0 ) ) / pow( j.getAttribute<float>("ECF2"), 3.0), weight );
    }
  }

  return 0;
}

