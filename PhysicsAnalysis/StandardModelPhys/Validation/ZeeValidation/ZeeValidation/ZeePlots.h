/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZEEVALIDATION_ZEEPLOTS_H
#define ZEEVALIDATION_ZEEPLOTS_H
	
#include "TrkValHistUtils/PlotBase.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "xAODEgamma/Electron.h"
#include "TLorentzVector.h"

namespace ZeeValidation{
  
  class ZeePlots:public PlotBase {
  public:
    ZeePlots(PlotBase* pParent, std::string sDir, std::string sParticleType);

    void fillZPlots( TLorentzVector& z, int level );
    void fillElectronPlots( TLorentzVector& elec1, TLorentzVector& elec2, TLorentzVector& cluster1, TLorentzVector& cluster2, TLorentzVector& track1, TLorentzVector& track2,int charge1, int charge2, int level );
    void fillGenPlots( int nel );

    
    //nLevels = 5*2 ( Loose, Medium, Tight, OQ, Reco * OS/SS)
    static const int nLevels = 10;
    static const std::string cLevelLabel[nLevels];

    std::string m_sParticleType;

    TH1* n_el;

    TH1* z_mass[nLevels];
    TH1* z_pt[nLevels];
    TH1* z_y[nLevels];

    TH1* dr_electrons[nLevels];

    TH1* ptclust_electron1[nLevels];
    TH1* ptclust_electron2[nLevels];
    TH1* etaclust_electron1[nLevels];
    TH1* etaclust_electron2[nLevels];

    TH1* ptclust_electrons[nLevels];
    TH1* etaclust_electrons[nLevels];
    TH1* pt_electrons[nLevels];
    TH1* eta_electrons[nLevels];
    TH1* phi_electrons[nLevels];
    TH1* eoverp_electrons[nLevels];
    TH1* eoverpq_electrons[nLevels];
  
  private:
 
    virtual void initializePlots();
    
  };
}

#endif
