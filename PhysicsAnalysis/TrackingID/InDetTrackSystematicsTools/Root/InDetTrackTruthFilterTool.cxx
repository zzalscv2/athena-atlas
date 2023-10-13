/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "PathResolver/PathResolver.h"

#include <TH2.h>
#include <TRandom3.h>
#include <TFile.h>

namespace InDet {

  static const CP::SystematicSet FilterSystematics = 
    {
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_TIGHT),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_GLOBAL),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_IBL),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_PP0),
      InDet::TrackSystematicMap.at(TRK_EFF_LOOSE_PHYSMODEL),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_GLOBAL),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_IBL),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_PP0),
      InDet::TrackSystematicMap.at(TRK_EFF_TIGHT_PHYSMODEL),
      InDet::TrackSystematicMap.at(TRK_FAKE_RATE_LOOSE_ROBUST),
      InDet::TrackSystematicMap.at(TRK_EFF_LARGED0_GLOBAL),
      InDet::TrackSystematicMap.at(TRK_EFF_LARGED0_IBL),
      InDet::TrackSystematicMap.at(TRK_EFF_LARGED0_PP0),
      InDet::TrackSystematicMap.at(TRK_EFF_LARGED0_PHYSMODEL),
    };

  InDetTrackTruthFilterTool::InDetTrackTruthFilterTool(const std::string& name) :
    InDetTrackSystematicsTool(name),
    m_trackOriginTool("InDet::InDetTrackTruthOriginTool", this)
  {

#ifndef XAOD_STANDALONE
    declareInterface<IInDetTrackTruthFilterTool>(this);
#endif

    declareProperty("trackOriginTool", m_trackOriginTool);

    declareProperty("Seed", m_seed);

    declareProperty("fPrim", m_fPrim);
    declareProperty("fSec", m_fSec);
    declareProperty("fFakeLoose", m_fFakeLoose);
    declareProperty("fFakeTight", m_fFakeTight);
    declareProperty("fPU", m_fPU);
    declareProperty("fFrag", m_fFrag);
    declareProperty("fFromC", m_fFromC);
    declareProperty("fFromB", m_fFromB);
    declareProperty("trkEffSystScale", m_trkEffSystScale);
    declareProperty("doLRTSystematics", m_doLRTSystematics);

    declareProperty("calibFileNomEff", m_calibFileNomEff = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root");
    declareProperty("calibFileLRTEff", m_calibFileLRTEff = "InDetTrackSystematicsTools/CalibData_22.0_2022-v00/TrackingRecommendations_prelim_rel22.root");
  }

  StatusCode InDetTrackTruthFilterTool::initialize() {

    m_rnd = std::make_unique<TRandom3>(m_seed);

    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLooseGlobal,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSOverall_5_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLooseIBL,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSIBL_10_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLoosePP0,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSPP0_25_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistLoosePhysModel,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSQGSP_BIC_Loose") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightGlobal,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSOverall_5_TightPrimary") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightIBL,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSIBL_10_TightPrimary") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightPP0,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSPP0_25_TightPrimary") );
    ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
           m_trkEffHistTightPhysModel,
           m_calibFileNomEff,
           "OneMinusRatioEfficiencyVSEtaPt_AfterRebinning_NominalVSQGSP_BIC_TightPrimary") );

    ATH_MSG_INFO( "Using for nominal track efficiency the calibration file " << PathResolverFindCalibFile(m_calibFileNomEff) );

    if(m_doLRTSystematics) {
      ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
             m_trkEffHistLRTGlobal,
             m_calibFileLRTEff,
             "OneMinusRatioEfficiencyVSEtaProdR_AfterRebinning_NominalVSp5Overall_LRT") );
      ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
             m_trkEffHistLRTIBL,
             m_calibFileLRTEff,
             "OneMinusRatioEfficiencyVSEtaProdR_AfterRebinning_NominalVSp10IBL_LRT") );
      ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
             m_trkEffHistLRTPP0,
             m_calibFileLRTEff,
             "OneMinusRatioEfficiencyVSEtaProdR_AfterRebinning_NominalVSp25PP0_LRT") );
      ATH_CHECK ( initTrkEffSystHistogram( m_trkEffSystScale,
             m_trkEffHistLRTPhysModel,
             m_calibFileLRTEff,
             "OneMinusRatioEfficiencyVSEtaProdR_AfterRebinning_NominalVSQGSP_LRT") );
      ATH_MSG_INFO( "Using for LRT track efficiency the calibration file " << PathResolverFindCalibFile(m_calibFileLRTEff) );
    }

    ATH_CHECK ( m_trackOriginTool.retrieve() );

    ATH_CHECK ( InDetTrackSystematicsTool::initialize() );

    return StatusCode::SUCCESS;
  }


  InDetTrackTruthFilterTool::~InDetTrackTruthFilterTool() {

    delete m_fPrimHistogram;
    delete m_fSecHistogram;
    //delete m_fFakeHistogram;
    delete m_fPUHistogram;
    delete m_fFragHistogram;
    delete m_fFromCHistogram;
    delete m_fFromBHistogram;
    delete m_trkEffHistLooseGlobal;
    delete m_trkEffHistLooseIBL;
    delete m_trkEffHistLoosePP0;
    delete m_trkEffHistLoosePhysModel;
    delete m_trkEffHistTightGlobal;
    delete m_trkEffHistTightIBL;
    delete m_trkEffHistTightPP0;
    delete m_trkEffHistTightPhysModel;
    if(m_doLRTSystematics) {
      delete m_trkEffHistLRTGlobal;
      delete m_trkEffHistLRTIBL;
      delete m_trkEffHistLRTPP0;
      delete m_trkEffHistLRTPhysModel;
    }

    m_fPrimHistogram = nullptr;
    m_fSecHistogram = nullptr;
    // m_fFakeHistogram = nullptr;
    m_fPUHistogram = nullptr;
    m_fFragHistogram = nullptr;
    m_fFromCHistogram = nullptr;
    m_fFromBHistogram = nullptr;
    m_trkEffHistLooseGlobal = nullptr;
    m_trkEffHistLooseIBL = nullptr;
    m_trkEffHistLoosePP0 = nullptr;
    m_trkEffHistLoosePhysModel = nullptr;
    m_trkEffHistTightGlobal = nullptr;
    m_trkEffHistTightIBL = nullptr;
    m_trkEffHistTightPP0 = nullptr;
    m_trkEffHistTightPhysModel = nullptr;
    m_trkEffHistLRTGlobal = nullptr;
    m_trkEffHistLRTIBL = nullptr;
    m_trkEffHistLRTPP0 = nullptr;
    m_trkEffHistLRTPhysModel = nullptr;
  }

  bool InDetTrackTruthFilterTool::accept(const xAOD::TrackParticle* track, float mu) const {

    // this is only really useful if you are using the "robust" systematic version, otherwise the mu set can't do anything
    if(isActive( TRK_FAKE_RATE_LOOSE_ROBUST )){

      // calcuate probability for this to be a fake, and then roll random numbers to decide if it is one, and if it should not be accepted
      float fakeProb = pseudoFakeProbability(track,mu);
      if(dropPseudoFake(fakeProb)) return false;

      // otherwise, pass the track
      return true;

    } else {
      ATH_MSG_ERROR("User-specified mu value cannot be applied due to using wrong systematic version - select TRK_FAKE_RATE_LOOSE_ROBUST is you want to use this");
      return true; // if you're not using the robust version, do nothing
    }
  }

  bool InDetTrackTruthFilterTool::accept(const xAOD::TrackParticle* track) const {

    float pt = track->pt();
    float eta = track->eta();

    // Do robust version without using truth first if selected, as this is relatively decoupled from the rest
    if(isActive( TRK_FAKE_RATE_LOOSE_ROBUST )) {
      const xAOD::EventInfo* ei = 0;
      float mu = 20; // sensible mu default
      if ( ! evtStore()->retrieve( ei , "EventInfo" ).isSuccess() ) { // this will check data vs. MC and run number.
        throw std::runtime_error("Error in InDetTrackTruthFilterTool::accept - failed to retrieve EventInfo.");
      }
      mu = ei->averageInteractionsPerCrossing();

      if (not accept(track,mu)) return false;
    }
    // now, back to using truth...

    int origin = m_trackOriginTool->getTrackOrigin(track);

    // we unimplemented histograms for these, so only flat defaults will be used.
    float fPrim = getFractionDropped(m_fPrim, m_fPrimHistogram, pt, eta);
    if(InDet::TrkOrigin::isPrimary(origin) && m_rnd->Uniform(0, 1) > fPrim) return false;
    float fSec = getFractionDropped(m_fSec, m_fSecHistogram, pt, eta);
    if(InDet::TrkOrigin::isSecondary(origin) && m_rnd->Uniform(0, 1) > fSec) return false;
    float fPU = getFractionDropped(m_fPU, m_fPUHistogram, pt, eta);
    if(InDet::TrkOrigin::isPileup(origin) && m_rnd->Uniform(0, 1) > fPU) return false;
    float fFrag = getFractionDropped(m_fFrag, m_fFragHistogram, pt, eta);
    if(InDet::TrkOrigin::isFragmentation(origin) && m_rnd->Uniform(0, 1) > fFrag) return false;
    float fFromC = getFractionDropped(m_fFromC, m_fFromCHistogram, pt, eta);
    if(InDet::TrkOrigin::isFromD(origin) && m_rnd->Uniform(0, 1) > fFromC) return false;
    float fFromB = getFractionDropped(m_fFromB, m_fFromBHistogram, pt, eta);
    if(InDet::TrkOrigin::isFromB(origin) && m_rnd->Uniform(0, 1) > fFromB) return false;

    if ( InDet::TrkOrigin::isFake(origin) ) {
      bool isActiveLoose = isActive( TRK_FAKE_RATE_LOOSE );
      bool isActiveTight = isActive( TRK_FAKE_RATE_TIGHT );
      if (isActiveLoose && isActiveTight) {
       throw std::runtime_error( "Both Loose and TightPrimary versions of fake rate systematic are set." );
      }
      if ( isActiveLoose ) {
        //float fFake = getFractionDropped(m_fFake, m_fFakeHistogram, pt, eta); // there is no fake-rate histogram - just a flat uncertainty
        if(m_rnd->Uniform(0, 1) < m_fFakeLoose) return false;
      }
      if ( isActiveTight ) {
        if(m_rnd->Uniform(0, 1) < m_fFakeTight) return false;
      }
    }

    if ( InDet::TrkOrigin::isPrimary(origin) ) {
      if ( isActive( TRK_EFF_LOOSE_GLOBAL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLooseGlobal, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_LOOSE_IBL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLooseIBL, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_LOOSE_PP0 ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLoosePP0, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_LOOSE_PHYSMODEL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistLoosePhysModel, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_GLOBAL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightGlobal, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_IBL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightIBL, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_PP0 ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightPP0, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
      if ( isActive( TRK_EFF_TIGHT_PHYSMODEL ) ) {
        float fTrkEffSyst = getFractionDropped(1, m_trkEffHistTightPhysModel, pt, eta);
        if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
      }
    }

    if(m_doLRTSystematics) {
      const ElementLink< xAOD::TruthParticleContainer > &truthParticleLink = track->auxdata< ElementLink< xAOD::TruthParticleContainer > >("truthParticleLink");
      if(truthParticleLink.isValid()) {
        const xAOD::TruthParticle *truthParticle = *truthParticleLink;
        double eta = truthParticle->eta();

        const ElementLink< xAOD::TruthVertexContainer > &truthVertexLink = truthParticle->auxdata< ElementLink< xAOD::TruthVertexContainer > >("prodVtxLink");
        if(truthVertexLink.isValid()) {
          const xAOD::TruthVertex *truthVertex = *truthVertexLink;
          double prodR = truthVertex->perp();
          if ( isActive( TRK_EFF_LARGED0_GLOBAL ) ) {
            float fTrkEffSyst = getFractionDropped(1.0, m_trkEffHistLRTGlobal, eta, prodR, false);
            if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
          }

          if ( isActive( TRK_EFF_LARGED0_IBL ) ) {
            float fTrkEffSyst = getFractionDropped(1.0, m_trkEffHistLRTIBL, eta, prodR, false);
            if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
          }

          if ( isActive( TRK_EFF_LARGED0_PP0 ) ) {
            float fTrkEffSyst = getFractionDropped(1.0, m_trkEffHistLRTPP0, eta, prodR, false);
            if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
          }

          if ( isActive( TRK_EFF_LARGED0_PHYSMODEL ) ) {
            float fTrkEffSyst = getFractionDropped(1.0, m_trkEffHistLRTPhysModel, eta, prodR, false);
            if(m_rnd->Uniform(0, 1) < fTrkEffSyst) return false;
          }
        }
      }
    }

    return true;
  }

  StatusCode InDetTrackTruthFilterTool::initTrkEffSystHistogram(float scale, TH2 *&histogram, std::string rootFileName, std::string histogramName) const {

    ATH_CHECK( initObject<TH2>(histogram, rootFileName, histogramName) );

    for(int binx=1; binx<=histogram->GetNbinsX(); binx++) {
      for(int biny=1; biny<=histogram->GetNbinsY(); biny++) {
        // now using a histogram which is 1 - eff1/eff2, so we don't apply the "1 - content" correction ourselves
        float content = histogram->GetBinContent(binx, biny); // get the bin content
        content = fabs(content); // retrieve uncertainty and symmetrize (should be positive in histogram anyway)
        content *= scale; // scale systematic
        if(content > 1) content = 1; // protection: no larger than 100% uncertainty

        histogram->SetBinContent(binx, biny, content);
      }
    }

    return StatusCode::SUCCESS;
  }

  float InDetTrackTruthFilterTool::getFractionDropped(float fDefault, const TH2 *histogram, float x, float y, bool xAxisIspT) const {

    if(histogram==nullptr) {
      return fDefault;
    }

    if(xAxisIspT)
      {
        x *= 1.e-3; // unit conversion to GeV
        if( x >= histogram->GetXaxis()->GetXmax() ) x = histogram->GetXaxis()->GetXmax() - 0.001;
      }

    float frac = histogram->GetBinContent(histogram->FindFixBin(x, y));
    if( frac > 1. ) {
      ATH_MSG_WARNING( "Fraction from histogram " << histogram->GetName()
           << " is greater than 1. Setting fraction to 1." );
      frac = 1.;
    }
    if( frac < 0. ) {
      ATH_MSG_WARNING( "Fraction from histogram " << histogram->GetName()
           << " is less than 0. Setting fraction to 0." );
      frac = 0.;
    }
    return frac;
  }

  // this is where the calculation of the fake probability is done if you are not using truth info to determine whether a track is fake

  float InDetTrackTruthFilterTool::pseudoFakeProbability(const xAOD::TrackParticle* track, float mu) const {

    float pt = track->pt();
    float d0 = track->d0();   

    // make a function to determine which tracks we will classify as fake, parameters determined from fit to full-truth MC16e ttbar
    float fakeProb = 0.01;
    // this is the mu dependence part
    if(mu>20) {
      float p0 = 0.008645;
      float p1 = 0.0001114;
      float p2 = 9.299e-6;
      fakeProb = p0 + (p1*mu) + (p2*mu*mu);
    }

    // now we add the pT term
    float pTcorr = 0.02;
    if(pt<50000) {
      float p0 = 0.02;
      float p1 = -3.564;
      float p2 = -0.0005982;
      float param = p1 + (p2 * pt);
      pTcorr = p0 + std::exp(param);
    }

    fakeProb*=pTcorr;

    float d0corr = 1;
    d0corr = std::tanh(std::abs(d0));

    fakeProb*=d0corr;

    // multiply by empirical ad hoc rescaling factor 
    // to give correct overall probability
    fakeProb*=309.602;

    return fakeProb;

  }

  bool InDetTrackTruthFilterTool::dropPseudoFake(float prob) const {

    bool isFake = false;
    if(m_rnd->Uniform(0, 1) < prob) isFake=true;

    // Now, if we've decided if this is a fake, we apply the uncertainty
    if(isFake){
      if(m_rnd->Uniform(0, 1) < m_fFakeLoose) return true; // drop this track
    }

    return false;

  }

  bool InDetTrackTruthFilterTool::isAffectedBySystematic( const CP::SystematicVariation& syst ) const
  {
    return InDetTrackSystematicsTool::isAffectedBySystematic( syst );
  }

  CP::SystematicSet InDetTrackTruthFilterTool::affectingSystematics() const
  {
    return FilterSystematics;
  }

  CP::SystematicSet InDetTrackTruthFilterTool::recommendedSystematics() const
  {
    return InDetTrackSystematicsTool::recommendedSystematics();
  }

  StatusCode InDetTrackTruthFilterTool::applySystematicVariation( const CP::SystematicSet& systs )
  {
    return InDetTrackSystematicsTool::applySystematicVariation(systs);
  }

} // namespace InDet
