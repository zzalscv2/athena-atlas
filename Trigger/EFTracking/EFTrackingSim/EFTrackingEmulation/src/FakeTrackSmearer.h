// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

// Stolen from A.Cerri

#ifndef EFTRACKINGEMULATION_FAKETRACKSMEARER_H
#define EFTRACKINGEMULATION_FAKETRACKSMEARER_H

#include "TRandom3.h"
#include "TCanvas.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TF2.h"
#include "TH1F.h"
#include "TROOT.h"

#include "FTS_Track.h"
namespace FitFunctions {
  #include "FitFunctions/L1TT/d0Fitparam_N.C"
  #include "FitFunctions/L1TT/z0Fitparam_N.C"
  #include "FitFunctions/L1TT/MoreFitParam_N.C"
}
 

class FakeTrackSmearer
{
 public:
  FakeTrackSmearer(std::string InstanceName, long long randomseed=0, bool verbose=false)
    {
      m_useCoinToss=false;
      m_includeFakesInResolutionCalculation=false;
      m_fakeKillerEnable=false;
      m_produceFakes=true;
      m_nominalEfficiency=0.95;
      m_outPtCut=2.0;
      m_inPtCut=1.0; 
      m_useTrackingTruth=true;      
      m_baseName=InstanceName;
      Prepare();
      m_myRandom=new TRandom3(randomseed);
      m_ntracks=0; m_nfakes=0; Tracks.clear(); 
      m_verbose=verbose;   
      
      // Set these in order to define the scenario:
      //SetSigmaScaleFactor(1.0);
      //SetResolutionPtCutOff(5.0);
      //UseResolutionPtCutOff(true);  
      
    }

// prepare the functions to use
  void Prepare()
  {
    printf("Entering Prepare\n");    
    std::string name;

    name="d0res_eta"+m_baseName;
    d0res_eta=new TF1(name.c_str(),[&](double*x,double*p){return d0ResFunc(x[0],p[0],0); },0.0,4.0,1);
    name="z0res_eta"+m_baseName;
    z0res_eta=new TF1(name.c_str(),[&](double*x,double*p){return z0ResFunc(x[0],p[0],0); },0.0,4.0,1);
    name="d0ref_eta"+m_baseName;
    d0ref_eta=new TF1(name.c_str(),[&](double*x,double*p){return d0RefFunc(x[0],p[0],0); },0.0,4.0,1);
    name="z0ref_eta"+m_baseName;
    z0ref_eta=new TF1(name.c_str(),[&](double*x,double*p){return z0RefFunc(x[0],p[0],0); },0.0,4.0,1);


    name="d0res_pt"+m_baseName;
    d0res_pt=new TF1(name.c_str(),[&](double*x,double*p){return d0ResFunc(p[0],x[0],0); },4.0,200.0,1);
    name="z0res_pt"+m_baseName;
    z0res_pt=new TF1(name.c_str(),[&](double*x,double*p){return z0ResFunc(p[0],x[0],0); },4.0,200.0,1);
    name="d0ref_pt"+m_baseName;
    d0ref_pt=new TF1(name.c_str(),[&](double*x,double*p){return d0RefFunc(p[0],x[0],0); },4.0,200.0,1);
    name="z0ref_pt"+m_baseName;
    z0ref_pt=new TF1(name.c_str(),[&](double*x,double*p){return z0RefFunc(p[0],x[0],0); },4.0,200.0,1);
  }


  double d0ResFunc(double eta,double pt,int verbose)
  {
    // add here other d0 res functions
    return FitFunctions::getD0ResParam_N(eta,pt,verbose)/1000.0;
  }

  double z0ResFunc(double eta,double pt,int verbose)
  {
    // add here other z0 res functions
    return FitFunctions::getZ0ResParam_N(eta,pt,verbose)/1000.0;
  }
   
  double etaResFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused))) 
  {
    // add here other eta res functions
    return 0.;
  }

  double phiResFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    // add here other phi res functions
    return 0.;
  }

  double curvResFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    // add here other curv res functions
    return 0.;
  }

// Reference functions, in case needed ////

 double d0RefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.; // no reference to scale
  }

  double z0RefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.; // no reference to scale
  }

double etaRefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.;// no reference to scale
  }

double phiRefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.;// no reference to scale
  }

double curvRefFunc(double eta __attribute__((unused)),double pt __attribute__((unused)),int verbose __attribute__((unused)))
  {
    return 1.;// no reference to scale
  }


  void InitArray(double *a,int n,const double in[])
  {
    for (int i=0;i<n;i++) a[i]=in[i];
  }



  void SetInputTracksPtCut(double ptcut) {m_inPtCut=ptcut;};
  void SetOutputTracksPtCut(double ptcut) {m_outPtCut=ptcut;};
  void SetMatchRatio(double m) { m_FMatches=m;}; // NMatch/NOffline  i.e. k
  void SetFakeFraction(double f) {m_FakeFraction=f; }; // i.e. f
  void SetSigmaScaleFactor(double s) {m_SigmaScaleFactor=s; };
  void SetResolutionPtCutOff(double cutoff) {m_resolutionPtCutOff=cutoff; };
  void UseResolutionPtCutOff(bool use ) {m_useResolutionPtCutOff=use ; };

  void Clear() {m_ntracks=0; m_nfakes=0; Tracks.clear(); };

  int GetNTrueTracks() {return m_ntracks;        };
  int GetNFakes()      {return m_nfakes;         };
  int GetNTracks()     {return m_ntracks+m_nfakes; };

  void AddTrackAndTruth(double d0,double z0,double curv,double eta,double phi)
  {
    m_useTrackingTruth=true; m_useInputSigmas=false;
    AddTrack(d0,z0,curv,eta,phi);
    m_useTrackingTruth=true; m_useInputSigmas=false;
  }


  void AddTrack(double d0,double z0,double curv,double eta,double phi)
  {
    // Adding one or more tracks to this input
    // input curv is in GeV    
    bool verbose=m_verbose;
      
    double abseta=TMath::Abs(eta);
    double abspt=TMath::Abs(1.0/curv); //GeV
    if (verbose) printf("Smearer::AddTrack : d0=%f, z0=%f, qOverPt=%f, eta=%f, phi=%f pt=%f\n", d0, z0, curv, eta,phi, TMath::Abs(1.0/curv));
 
    bool condition = (abspt>m_inPtCut) &&
      (d0RefFunc(abseta,abspt,verbose)>0.0) &&
      (d0ResFunc(abseta,abspt,verbose)>0.0) &&
      (z0RefFunc(abseta,abspt,verbose)>0.0) &&
      (z0ResFunc(abseta,abspt,verbose)>0.0);
    
    if (!condition)
      return;
    
    
    //Call here SetFakeFraction() and SetMatchRatio();
    
    double curvres = m_SigmaScaleFactor * curvResFunc(abseta,abspt,0);
    double phires = m_SigmaScaleFactor * phiResFunc(abseta,abspt,0);
    double etares = m_SigmaScaleFactor * etaResFunc(abseta,abspt,0);
    double d0res = m_SigmaScaleFactor * d0ResFunc(abseta,abspt,0);
    double z0res = m_SigmaScaleFactor * z0ResFunc(abseta,abspt,0);
    if (verbose) printf("Smearing parameters: curv = %f, phi=%f, eta=%f, d0=%f, z0=%f\n",curvres, phires, etares, d0res, z0res);

   
  #ifdef STANDALONE_FAKETRACKSMEARER
    d0Narrow->Fill(d0res);
    z0Narrow->Fill(z0res);
  #endif
    
    
  #ifdef STANDALONE_FAKETRACKSMEARER
    d0Sim->Fill(s_narrow_d0_model->Eval(abseta));
    z0Sim->Fill(s_narrow_z0_model->Eval(abseta));
  #endif



    double avgntracks=(m_nominalEfficiency> m_FMatches)?m_nominalEfficiency: m_FMatches;
    int ntracks=( m_myRandom->Rndm()<m_nominalEfficiency)?1:0;
    
    if (m_produceFakes)
      {
        if (m_useCoinToss) {
          if (avgntracks>m_nominalEfficiency)     ntracks+= m_myRandom->Poisson(avgntracks-m_nominalEfficiency);
          else if (avgntracks<m_nominalEfficiency) ntracks=( m_myRandom->Rndm()<avgntracks)?1:0;
        }
        else
          ntracks= m_myRandom->Poisson(avgntracks);
      }    

    if (verbose) printf("Now producing %d tracks\n", ntracks);
    for (int i=0;i<ntracks;i++)
      {
        // Creating close track
        double gend0,genz0, geneta, genphi, gencurv;
        gend0= m_myRandom->Gaus(d0,d0res);
        genz0= m_myRandom->Gaus(z0,z0res);
        geneta= m_myRandom->Gaus(eta,etares);
        genphi= m_myRandom->Gaus(phi,phires);
        gencurv= m_myRandom->Gaus(curv,curvres);

        EFTrackingSmearing::FTS_Track Track(
              1000./ gencurv, // convert to MeV
              geneta,
              genphi,
              gend0,
              genz0,
              curvres,
              etares,
              phires,
              d0res,
              z0res,
              false
        );
        if (verbose) printf("Producing this track: curv = %f, phi=%f, eta=%f, d0=%f, z0=%f pt=%f\n",gencurv, genphi, geneta, gend0, genz0, Track.pt()/1000.0);
        if (TMath::Abs(Track.pt())>=m_outPtCut)
          {
            Tracks.push_back(Track);
            m_ntracks++;
            if (verbose) printf("Adding track with pt=%f ==> track #%d\n",Track.pt(), m_ntracks);
          }
      }

        
    // AddTrack Done
    return;      
  };

  void PlotD0Res(double eta)
  {
    TCanvas *d0rescanvas=(TCanvas *)gROOT->FindObjectAny("d0rescanvas");
    if (d0rescanvas==NULL) d0rescanvas=new TCanvas("d0rescanvas");
    d0res_pt->SetParameter(0,eta);
    d0ref_pt->SetParameter(0,eta);
    d0res_pt->Draw();
    d0ref_pt->SetLineColor(4);
    d0ref_pt->Draw("same");
  }


  void EnableFakes(bool enable=true)
  {
    m_produceFakes=enable;
    if (Tracks.size()!=0)
      printf("Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer\n");
  }

  void FakeKillerEnable(bool enable=true)
  {
    m_fakeKillerEnable=enable;
    if (Tracks.size()!=0)
      printf("Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer (FakeKillerEnable)\n");
  }

  void IncludeFakesInResolutionCalculation(bool enable=true)
  {
    m_includeFakesInResolutionCalculation=enable;
    if (Tracks.size()!=0)
      printf("Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer (IncluldeFakesInResolutionCalculation)\n");
  }

  void UseCoinToss(bool enable=true)
  {
    m_useCoinToss=enable;
    if (Tracks.size()!=0)
      printf("Warning: you are reconfiguring fakes production but it seems you have already processed events with this instance of FakeTracksSmearer (UseCoinToss)\n");
  }

  void SetTrackingEfficiency(double epsilon=0.95)
  {
    m_nominalEfficiency=epsilon;
  }

  std::vector<EFTrackingSmearing::FTS_Track> Tracks;
  double z0(int idx)  {return Tracks[idx].z0();};
  double d0(int idx)  {return Tracks[idx].d0();};
  double phi(int idx) {return Tracks[idx].phi();};
  double pt(int idx)  {return Tracks[idx].pt();};
  double curv(int idx)  {return 1000.0/Tracks[idx].pt();};
  double eta(int idx) {return Tracks[idx].eta();};

  
  TH1F *d0Narrow,*z0Narrow;
  TH1F *d0Sim,*z0Sim;

  TF1 *d0res_eta,*d0ref_eta;
  TF1 *z0res_eta,*z0ref_eta;
  TF1 *d0res_pt,*d0ref_pt;
  TF1 *z0res_pt,*z0ref_pt;


 private:
  std::string m_baseName;
  int m_ntracks;
  int m_nfakes;
  bool m_verbose;
  TRandom3 *m_myRandom;

  double m_FMatches;
  double m_FakeFraction;
  double m_SigmaScaleFactor;
  double m_nominalEfficiency;

  bool m_includeFakesInResolutionCalculation;
  bool m_fakeKillerEnable;
  bool m_useCoinToss;
  bool m_useInputSigmas;
  bool m_produceFakes;
  bool m_useResolutionPtCutOff;
  bool m_useTrackingTruth;
  double m_resolutionPtCutOff=0.0;
  double m_inPtCut,m_outPtCut;

};

#endif
