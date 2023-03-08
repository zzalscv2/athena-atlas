/* emacs: this is -*- c++ -*- */
/**
 **     @file    Analysis_Distribution.h
 **
 **     @author  mark sutton
 **     @date    Fri 11 Jan 2019 07:06:37 CET 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TrigInDetAnalysisExample_Analysis_Distribution_H
#define TrigInDetAnalysisExample_Analysis_Distribution_H


#include <iostream>

#include "TH1.h"

#include "TrigInDetAnalysis/TrackAnalysis.h"


class Analysis_Distribution : public TrackAnalysis { 
  
 public:

  using TrackAnalysis::execute;

 public:
  
  Analysis_Distribution(const std::string& name);
  //Analysis_Distribution(const std::string& name, double pTCut, double etaCut, double d0Cut, double z0Cut);  
  
  virtual void initialise();
  
  virtual void execute(const std::vector<TIDA::Track*>& referenceTracks,
		       const std::vector<TIDA::Track*>& testTracks,
		       TrackAssociator* associator);
  
  virtual void finalise();
  
 private:

  double deltaR(double eta1, double eta2, double phi1, double phi2);
  double phi(double p);

 private:
  
  // Track parameter histograms
  TH1* m_h_eta;
  TH1* m_h_phi;
  TH1* m_h_d0;
  TH1* m_h_z0;
  TH1* m_h_pT;
  
  // Track hit histograms
  TH1* m_h_blay;
  TH1* m_h_pixel;
  TH1* m_h_sct;
  TH1* m_h_silicon;
  TH1* m_h_silicon_match;
  TH1* m_h_straw;
  TH1* m_h_tr;

};


#endif // TrigInDetAnalysisExample_Analysis_Distribution_H
