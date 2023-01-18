/* emacs: this is -*- c++ -*- */
/**
 **     @file    Analysis_Tier0.h
 **
 **     @author  mark sutton
 **     @date    $Id: Analysis_Tier0.h   Thu 18 May 2017 15:29:53 CEST 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TrigInDetAnalysisExample_Analysis_Tier0_H
#define TrigInDetAnalysisExample_Analysis_Tier0_H


#include <iostream>

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

#include "TrigInDetAnalysis/TrackAnalysis.h"
#include "TrigInDetAnalysis/TIDDirectory.h"

#include "TrigInDetAnalysisExample/VtxAnalysis.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

class Analysis_Tier0 : public TrackAnalysis { 

public:

  using TrackAnalysis::execute;
  
public:
  
  Analysis_Tier0(const std::string& name, double pTCut, double etaCut, double d0Cut, double z0Cut);
  
  virtual void initialise();
  
  virtual void execute(const std::vector<TIDA::Track*>& referenceTracks,
  		       const std::vector<TIDA::Track*>& testTracks,
  		       TrackAssociator* associator );
  
  virtual void execute_vtx(const std::vector<TIDA::Vertex*>& vtx0,
			   const std::vector<TIDA::Vertex*>& vtx1, 
			   const TIDA::Event* tevt=0 );
  
  virtual void finalise();

  void setvertices(int numvtx) {m_nVtx = numvtx;}
  
  std::map<std::string, TH1*>::const_iterator THbegin() const { return m_histos.begin(); }
  std::map<std::string, TH1*>::const_iterator THend()   const { return m_histos.end(); }

  std::map<std::string, TProfile*>::const_iterator TEffbegin() const { return m_effhistos.begin(); }
  std::map<std::string, TProfile*>::const_iterator TEffend()   const { return m_effhistos.end(); }


  void set_monTool( ToolHandle<GenericMonitoringTool>* m ) { m_monTool=m; }

  ToolHandle<GenericMonitoringTool>* monTool() { return m_monTool; }


  bool debug() const { return m_debug; }
  
public:

  void addHistogram( TH1* h ) {
    std::string name = h->GetName();
    m_histos.insert( std::map<std::string, TH1*>::value_type( name, h) );
  }

  void addHistogram( TProfile* h ) {
    std::string name = h->GetName();
    m_effhistos.insert( std::map<std::string, TProfile*>::value_type( name, h) );
  }

  double phi(double p);

private:

  std::map<std::string, TH1*> m_histos;
  std::map<std::string, TProfile*> m_effhistos;
  
  TProfile* m_h_total_efficiency;
  TProfile* m_h_pTeff;
  TProfile* m_h_etaeff;
  TProfile* m_h_phieff;
  TProfile* m_h_d0eff;
  TProfile* m_h_z0eff;
  TProfile* m_h_nVtxeff;
  TProfile* m_h_lbeff;

  TProfile* m_h_pTres;
  TProfile* m_h_ipTres;
  TProfile* m_h_etares;
  TProfile* m_h_phires;
  TProfile* m_h_d0res;
  TProfile* m_h_z0res;

  TProfile* m_h_trkvtx_x_lb;
  TProfile* m_h_trkvtx_y_lb;
  TProfile* m_h_trkvtx_z_lb;


  TProfile* m_h_npixvseta;
  TProfile* m_h_npixvsphi;
  TProfile* m_h_npixvsd0;
  TProfile* m_h_npixvspT;

  TProfile* m_h_nsctvseta;
  TProfile* m_h_nsctvsphi;
  TProfile* m_h_nsctvsd0;
  TProfile* m_h_nsctvspT;

  TProfile* m_h_ntrtvseta;
  TProfile* m_h_ntrtvsphi;

  TProfile* m_h_nsihits_lb;

  TProfile* m_h_npixvseta_rec;
  TProfile* m_h_npixvsphi_rec;
  TProfile* m_h_npixvsd0_rec;
  TProfile* m_h_npixvspT_rec;

  TProfile* m_h_nsctvseta_rec;
  TProfile* m_h_nsctvsphi_rec;
  TProfile* m_h_nsctvsd0_rec;
  TProfile* m_h_nsctvspT_rec;

  TProfile* m_h_ntrtvseta_rec;
  TProfile* m_h_ntrtvsphi_rec;

  TProfile* m_h_nsihits_lb_rec;

  TProfile* m_h_d0vsphi;
  TProfile* m_h_d0vsphi_rec;

  //No currently in use 
  //but retained in case 
  //TH2F*     h2d_d0vsphi;
  //TH2F*     h2d_d0vsphi_rec;


  TH1F* m_h_chain;

  TH1F* m_h_ntrk;

  TH1F* m_h_trkpT;
  TH1F* m_h_trketa;
  TH1F* m_h_trkphi;
  TH1F* m_h_trkd0;
  TH1F* m_h_trkz0;

  //No currently in use 
  //but retained in case 
  //TH1F* m_h_trketaroi;
  //TH1F* m_h_trketazroi;

  TH1F* m_h_trkdd0;
  TH1F* m_h_trkdz0;

  TH1F* m_h_trkd0sig;

  TH1F* m_h_npix;
  TH1F* m_h_nsct;
  TH1F* m_h_nsihits;
  TH1F* m_h_ntrt;

  TH1F* m_h_ntrk_rec;

  TH1F* m_h_layer;


  TH1F* m_h_trkpT_rec;
  TH1F* m_h_trketa_rec;
  TH1F* m_h_trkphi_rec;
  TH1F* m_h_trkd0_rec;
  TH1F* m_h_trkz0_rec;

  TH1F* m_h_trkdd0_rec;
  TH1F* m_h_trkdz0_rec;

  TH1F* m_h_trkd0sig_rec;

  TH1F* m_h_npix_rec;
  TH1F* m_h_nsct_rec;
  TH1F* m_h_nsihits_rec;
  TH1F* m_h_ntrt_rec;


  TH1F* m_h_layer_rec;


  TH1F* m_h_trkpT_residual;
  TH1F* m_h_trkipT_residual;
  TH1F* m_h_trketa_residual;
  TH1F* m_h_trkphi_residual;
  TH1F* m_h_trkd0_residual;
  TH1F* m_h_trkz0_residual;

  TH1F* m_h_trkdd0_residual;
  TH1F* m_h_trkdz0_residual;


  // Efficiency plateau cuts
  //No currently in use 
  //but retained in case 
  //double m_pTCut;
  //double m_etaCut;
  //double m_d0Cut;
  //double m_z0Cut;

  int      m_nVtx;

  bool     m_debug;

  unsigned long long m_eventid;

  VtxAnalysis* m_vtxanal;

  ToolHandle<GenericMonitoringTool>* m_monTool;

};


#endif // TrigInDetAnalysisExample_Analysis_Tier0_H
