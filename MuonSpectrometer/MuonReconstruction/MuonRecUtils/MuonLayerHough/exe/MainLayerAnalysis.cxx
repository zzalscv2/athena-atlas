/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonLayerHough/LayerAnalysis.h"

#include "TChain.h"
#include "TString.h"
#include "TFile.h"
#include "TSystem.h"
#include <string>
#include <iostream>
#include <TApplication.h> 

int main( int argc, char* argv[]){
	 
  
  TApplication theApp("App", &argc, argv);
	
  int data = 0;
  if( argc > 1 ) {
    TString str = argv[1];
    data = str.Atoi();
  }

  std::cout << " opening dataset " << data << std::endl;
  TChain* ntupleToRead = new TChain("data") ;
  ntupleToRead->SetMakeClass(1);
  TString outName = "LayerAnalysis";
  TString inName = "HitNtuple2";
  TString postFix = "";
  if( data == 0 )      postFix += "Cav";
  else if( data == 1 ) postFix += "Zmumu";
  else if( data == 2 ) postFix += "Overlay1";
  else if( data == 3 ) postFix += "Overlay10";
  else if( data == 4 ) postFix += "Overlay20";
  else if( data == 5 ) postFix += "CavNSW";
  else if( data == 6 ) postFix += "SingleNSW";
 
  ntupleToRead->Add("HitNtuple.root"); 
//  ntupleToRead->Add(inName+postFix+".root");
  outName += postFix;
  outName += ".root";
  TFile* output = new TFile(outName,"RECREATE");
  std::cout << " creating analysis " << outName << std::endl;
  MuonHough::LayerAnalysis analysis(*ntupleToRead);
  analysis.analyse();
  std::cout << " done, closing file " << std::endl;

  output->Write();
  output->Close();
}
