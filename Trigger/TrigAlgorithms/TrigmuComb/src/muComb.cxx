/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     muComb.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigmuComb
// VERSION:  V3 (MT Version)
//
// AUTHOR:   S.Giagu <stefano.giagu@cern.ch>
//
// ********************************************************************
#include <sstream>
#include <math.h>

#include "muComb.h"
#include "muCombUtil.h"
#include "xAODTrigMuon/TrigMuonDefs.h"
#include "xAODTrigMuon/L2StandAloneMuonContainer.h"
#include "xAODTrigMuon/L2StandAloneMuonAuxContainer.h"
#include "xAODTrigMuon/L2CombinedMuonAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "GaudiKernel/ThreadLocalContext.h"

class ISvcLocator;

muComb::muComb(const std::string& name, ISvcLocator* pSvcLocator):
   AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode muComb::initialize()
{
   ATH_MSG_DEBUG("Initialization:");

   //Filed service
   ATH_CHECK( m_fieldCacheCondObjInputKey.initialize() );

   if (!m_monTool.empty()) {
      ATH_MSG_DEBUG("Retrieving monTool");
      CHECK(m_monTool.retrieve());
   } else {
      ATH_MSG_INFO("No monTool configured => NO MONITORING");
   }

   // BackExtrapolator services
   if ( m_backExtrapolatorG4.retrieve().isFailure() ) {  
        ATH_MSG_ERROR("Unable to locate G4 BackExtrapolator tool ");
        return StatusCode::FAILURE; 
   }

   ATH_CHECK( m_muonCollKey.initialize() );
   //ATH_CHECK( m_muonROICollKey.initialize() );
   ATH_CHECK( m_TrackParticleContainerKey.initialize() );
   ATH_CHECK( m_outputCBmuonCollKey.initialize() );

   return StatusCode::SUCCESS;
}

// muon-trk match based on angular distance
// return 0 --> match,  1 --> no match
int muComb::drptMatch(const xAOD::L2StandAloneMuon* feature, double id_pt, double id_eta, double id_phi, int algo,
                      double& combPtInv, double& combPtRes, double& deta, double& dphi, double& dr) const
{
   double pt     = feature->pt() * Gaudi::Units::GeV;
   double phi    = feature->phiMS();
   double eta    = feature->etaMS();
   return muComb::drptMatch(pt, eta, phi, id_pt, id_eta, id_phi, algo, combPtInv, combPtRes, deta, dphi, dr);
}

int muComb::drptMatch(double pt, double eta, double phi, double id_pt, double id_eta, double id_phi, int algo,
                      double& combPtInv, double& combPtRes, double& deta, double& dphi, double& dr) const
{

   // algo: 1 --> R (+ pt), combined pt
   // algo: 2 --> R match, Id pt
   // algo: 3 --> R match, MS pt
   // algo: 4 --> R match, infinite pt
   if (algo < 1 || algo > 4) {
      ATH_MSG_DEBUG(" muComb::drptMatch wrong algo parameter, it is: " << algo
            << " while must be in the range [1,4], match failed!!!" );
      return 1;
   }

   double winDR = m_winDR;
   double winPt = m_winPt;

   combPtRes = 0.0;
   if (algo == 1) combPtInv = ((1. / pt) + (1. / id_pt)) * 0.5;
   if (algo == 2) combPtInv = 1. / id_pt;
   if (algo == 3) combPtInv = 1. / pt;
   if (algo == 4) combPtInv = 1.e-33;

   double tmp_deta = std::max(eta, id_eta) - std::min(eta, id_eta);
   double tmp_dphi = std::max(phi, id_phi) - std::min(phi, id_phi);
   if (tmp_dphi >= M_PI) tmp_dphi = 2 * M_PI - tmp_dphi;
   double tmp_dr = sqrt(tmp_deta * tmp_deta + tmp_dphi * tmp_dphi);

   dphi      = tmp_dphi;
   deta      = tmp_deta;
   dr        = tmp_dr;

   bool passDR = true;
   bool passPt = true;

   if (tmp_dr > winDR)  passDR = false;

   ATH_MSG_DEBUG(" REGTEST Angular MU-ID match / dR / threshold / result:"
         << " / " << tmp_dr
         << " / " << winDR
         << " / " << (passDR ? "true" : "false"));

   if (algo == 1 && winPt > 0) {
      double tmp_dpt = fabs(fabs(pt) - fabs(id_pt)) / Gaudi::Units::GeV; //don't use charge info
      if (tmp_dpt > winPt) passPt = false;
      ATH_MSG_DEBUG( " REGTEST MU-ID match / dpt (GeV) / threshold (GeV) / result:"
            << " / " << tmp_dpt
            << " / " << winPt
            << " / " << (passPt ? "true" : "false"));
   }

   if (passDR && passPt) return 0;
   else                  return 1;
}

// muon-trk match based on Geant4 backextrapolated SA Muon matched with ID track
// return 0 --> match,  1/2/3/4/5/6 --> no match (muon pt zero, muon angle zero, ID pt zero, fail eta match, fail phi match, fail chi2 match)
int muComb::g4Match(const xAOD::L2StandAloneMuon* feature,
                    double id_eta, double id_phi, double id_pt, double id_charge, double id_eeta, double id_ephi, double id_eipt,
                    double& combPtInv, double& combPtRes, double& deta, double& dphi, double& chi2, int& ndof) const
{

   const EventContext& ctx = Gaudi::Hive::currentContext();
   chi2 = 1.0e30;
   ndof = 0;

   //muFast parameters (in MeV!)
   double phi    = feature->phiMS();
   //double eta    = feature->etaMS();
   double theta  = 2.*atan(exp(-feature->etaMS()));
   double p      = 0.0;
   if (sin(theta) != 0) {
      p = (feature->pt() * Gaudi::Units::GeV) / sin(theta);
   } else {
      return 2; //No match if muon angle is zero
   }
   double charge = 1.0;
   double q_over_p = 0.;
   if (p != 0.)  {
      q_over_p = 1. / p;
      charge = p / fabs(p);
   } else {
      return 1; //No match if muon Pt is zero
   }
   double pt = feature->pt() * Gaudi::Units::GeV;
   //double ptinv  = 1/pt;
   double eptinv = feature->deltaPt() * Gaudi::Units::GeV / pt / pt;

   bool   isBarrel = ((feature->sAddress() != -1) ? true : false);
   double etaShift = (isBarrel ? 0 : charge * 0.01);
   bool   doFix = kFALSE;

   //Superpoints
   int inner  = (feature->sAddress() == -1) ? xAOD::L2MuonParameters::Chamber::EndcapInner : xAOD::L2MuonParameters::Chamber::BarrelInner;
   int middle = (feature->sAddress() == -1) ? xAOD::L2MuonParameters::Chamber::EndcapMiddle : xAOD::L2MuonParameters::Chamber::BarrelMiddle;
   //int outer  = (feature->sAddress() == -1) ? xAOD::L2MuonParameters::Chamber::EndcapOuter : xAOD::L2MuonParameters::Chamber::BarrelOuter;

   double sp1_z = feature->superPointZ(inner);
   double sp1_R = feature->superPointR(inner);
   double sp2_z = feature->superPointZ(middle);
   double sp2_R = feature->superPointR(middle);

   if ((fabs(sp1_R) < 1000.)) {
      sp1_z *= 10.;
      sp1_R *= 10.;
   }
   if ((fabs(sp2_R) < 1300.)) {
      sp2_z *= 10.;
   }

   double R = sp1_R;
   double z = sp1_z;

   if (R == 0. && z == 0.) { //treat patological endcap cases
      doFix = kTRUE;
      if (fabs(sp2_R) > 1300.) {
         z = sp2_z;
         if (z == 0.) z = 7600.;
         R = z * tan(theta);
         theta  = 2.*atan(exp(-(feature->etaMS() - etaShift)));
      }
   }

   double x = R * cos(phi);
   double y = R * sin(phi);

   Amg::Vector3D vertex(x, y, z);
   Trk::PerigeeSurface beamSurface;
   Trk::PerigeeSurface pgsf(vertex);
   Trk::Perigee perigeeMS(0., 0., phi, theta, q_over_p, pgsf);

   //ID parameters
   double id_eptinv_OLD = muCombUtil::getIDSCANRes(m_IDSCANRes_barrel,  m_IDSCANRes_endcap1, m_IDSCANRes_endcap2,
                                               m_IDSCANRes_endcap3, m_IDSCANRes_endcap4, id_pt, id_eta);
   double id_ptinv  = 1.0e33;
   if (id_pt != 0) {
      id_ptinv  = 1. / id_pt;
   } else {
      return 3; //no match if ID track Pt zero
   }

   double id_eptinv = id_eipt; //now taken from Track itself ...

   std::unique_ptr<const Trk::TrackParameters> muonPerigee
     (m_backExtrapolatorG4->extrapolate(ctx,perigeeMS, beamSurface, Trk::oppositeMomentum, true, Trk::muon));

   //Protection against failing extrapolation
   double extr_eta;
   double extr_phi;
   if (!muonPerigee) { //G4 probably failed, getting LUT extrapolated values
      extr_eta    = feature->eta();
      extr_phi    = feature->phi();
   } else {
      double extr_theta    = muonPerigee -> parameters()[Trk::theta];
      extr_phi      = muonPerigee -> parameters()[Trk::phi0];
      extr_eta      = -log(tan(extr_theta / 2.));
      if (doFix) extr_eta = -log(tan(theta / 2.));
   }

   double extr_eeta = muCombUtil::getG4ExtEtaRes(feature->pt(), feature->etaMS());
   double extr_ephi = muCombUtil::getG4ExtPhiRes(feature->pt(), feature->etaMS());
   double extr_ptinv  = 1.0e33;
   if (pt != 0) extr_ptinv = 1. / pt;
   double extr_eptinv = eptinv;

   //Combined muon parameters
   combPtInv = muCombUtil::getCombinedAverage(extr_ptinv, extr_eptinv, id_ptinv, id_eptinv);
   combPtRes = muCombUtil::getCombinedAverageSigma(extr_eptinv, id_eptinv);
   double q_tmp = charge;
   if (m_ChargeStrategy == 1) q_tmp = id_charge;
   else if (m_ChargeStrategy == 2) {
      if (1. / combPtInv > 50000.) q_tmp = charge;
      else                         q_tmp = id_charge;
   }
   combPtInv *= q_tmp;

   //Masaki/Kunihiro treatment of TGC/RPC readout problems
   ATH_MSG_DEBUG( " Enlarge phi matching error in case TGC/RPC readout failed. : " << feature->isRpcFailure() << " / " << feature->isTgcFailure() );

   if (feature->isTgcFailure() || feature->isRpcFailure()) extr_ephi *= 2.0;

   //Match
   deta = muCombUtil::getDeltaEta(extr_eta, id_eta);
   dphi = muCombUtil::getDeltaPhi(extr_phi, id_phi);
   int ndof_OLD = 0;
   double chi2_OLD = muCombUtil::getChi2(ndof_OLD, combPtInv, 
                                         extr_eta, extr_eeta, extr_phi, extr_ephi, extr_ptinv, extr_eptinv,
                                         id_eta, 0.0, id_phi, 0.0, id_ptinv, id_eptinv, true);
   chi2 = muCombUtil::getStdChi2(ndof, extr_eta, extr_eeta, extr_phi, extr_ephi, extr_ptinv, extr_eptinv,
                              id_eta, id_eeta, id_phi, id_ephi, id_ptinv, id_eptinv, m_UseAbsPt);


   ATH_MSG_DEBUG(" REGTEST Resolution / OLDIdRes / IdRes / muFastRes / combRes:"
         << " / " << std::setw(11) << id_eptinv_OLD / Gaudi::Units::GeV
         << " / " << std::setw(11) << id_eptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << extr_eptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << combPtRes / Gaudi::Units::GeV );

   ATH_MSG_DEBUG(" REGTEST Momentum / IdPt / muFastPt  / CombPt :"
         << " / " << std::setw(11) << 1. / id_ptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << 1. / extr_ptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << 1. / combPtInv / Gaudi::Units::GeV );

   ATH_MSG_DEBUG(" REGTEST Chi2 / ndof // Chi2OLD / ndofOLD :"
         << " / " << std::setw(11) << chi2
         << " / " << std::setw(11) << ndof
         << " // " << std::setw(11) << chi2_OLD
         << " / " << std::setw(11) << ndof_OLD );

   //Cuts
   double winEtaSigma = m_WinEta_g4;
   double winPhiSigma = m_WinPhi_g4;
   double maxChi2     = m_Chi2Max_g4;
   if (!isBarrel) {//EC
      winEtaSigma = m_WinEta_EC_g4;
      winPhiSigma = m_WinPhi_EC_g4;
      maxChi2     = m_Chi2Max_EC_g4;
   }

   ATH_MSG_DEBUG(" REGTEST DeltaEta / DeltaPhi / WinEta / WinPhi:"
         << " / " << std::setw(11) << fabs(deta)
         << " / " << std::setw(11) << fabs(dphi)
         << " / " << std::setw(11) << m_WeightEta_g4*winEtaSigma*sqrt(extr_eeta * extr_eeta)
         << " / " << std::setw(11) << m_WeightPhi_g4*winPhiSigma*sqrt(extr_ephi * extr_ephi) );

   if (fabs(deta) > m_WeightEta_g4 * winEtaSigma * sqrt(extr_eeta * extr_eeta)) {
      return 4;
   }
   if (fabs(dphi) > m_WeightPhi_g4 * winPhiSigma * sqrt(extr_ephi * extr_ephi)) {
      return 5;
   }
   if (ndof >= m_NdofMin) {
      if (chi2 > maxChi2) return 6;
   }

   return 0; //match OK
}

// muon-trk match based on LUT backextrapolated SA Muon matched with ID track
// return 0 --> match,  1/2/3/4/5/6 --> no match (muon pt zero, muon angle zero, ID pt zero, fail eta match, fail phi match, fail chi2 match)
int muComb::mfMatch(const xAOD::L2StandAloneMuon* feature,
                    double id_eta, double id_phi, double id_pt, double id_charge,
                    double& combPtInv, double& combPtRes, double& deta, double& dphi, double& chi2, int& ndof) const
{

   chi2 = 1.0e30;
   ndof = 0;
   //muFast parameters

   double    pt = feature->pt() * Gaudi::Units::GeV;
   if (pt == 0.)  {
      return 1; //No match if muFast Pt is zero
   }

   bool   isTS     = ((feature->rMS() <= 10.) ? true : false);
   bool   isBarrel = ((feature->sAddress() != -1) ? true : false);

   double charge = pt / fabs(pt);
   double ptinv  = 1. / pt;
   double eptinv = feature->deltaPt() * Gaudi::Units::GeV / pt / pt;

   //ID parameters
   double id_eptinv    = muCombUtil::getIDSCANRes(m_IDSCANRes_barrel, m_IDSCANRes_endcap1, m_IDSCANRes_endcap2,
                                                  m_IDSCANRes_endcap3, m_IDSCANRes_endcap4, id_pt, id_eta);
   double id_ptinv     = 1.0e33;
   //double id_ept       = 1.0e33;
   if (id_pt != 0) {
      id_ptinv  = 1. / id_pt;
      //id_ept    = id_eptinv * id_pt * id_pt;
   } else {
      return 3; //no match if ID track Pt zero
   }

   //Combined muon parameters
   combPtInv = muCombUtil::getCombinedAverage(ptinv, eptinv, id_ptinv, id_eptinv);
   combPtRes = muCombUtil::getCombinedAverageSigma(eptinv, id_eptinv);
   double q_tmp = charge;
   if (m_ChargeStrategy == 1) q_tmp = id_charge;
   else if (m_ChargeStrategy == 2) {
      if (1. / combPtInv > 50000.) q_tmp = charge;
      else                         q_tmp = id_charge;
   }
   combPtInv *= q_tmp;

   // Extrapolated (LUT) quantities (now stored in the xAOD::L2StandAloneMuon container)
   double extr_eta    = feature->eta();
   double extr_eeta   = feature->deltaEta();
   double extr_phi    = feature->phi();
   double extr_ephi   = feature->deltaPhi();
   double extr_ptinv  = ptinv;
   double extr_eptinv = eptinv;

   //Masaki/Kunihiro treatment of TGC/RPC readout problems
   ATH_MSG_DEBUG(" Enlarge phi matching error in case TGC/RPC readout failed. : " << feature->isRpcFailure() << " / " << feature->isTgcFailure() );

   if (feature->isTgcFailure() || feature->isRpcFailure()) extr_ephi *= 2.0;

   //Match
   deta = muCombUtil::getDeltaEta(extr_eta, id_eta);
   dphi = muCombUtil::getDeltaPhi(extr_phi, id_phi);
   chi2 = muCombUtil::getChi2(ndof, combPtInv, extr_eta, extr_eeta, extr_phi, extr_ephi, extr_ptinv, extr_eptinv,
                              id_eta, 0.0, id_phi, 0.0, id_ptinv, id_eptinv, m_UseAbsPt);


   ATH_MSG_DEBUG(" REGTEST Resolution / IdRes / muFastRes / combRes:"
         << " / " << std::setw(11) << id_eptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << extr_eptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << combPtRes / Gaudi::Units::GeV );

   ATH_MSG_DEBUG(" REGTEST Momentum / IdPt / muFastPt  / CombPt :"
         << " / " << std::setw(11) << 1. / id_ptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << 1. / ptinv / Gaudi::Units::GeV
         << " / " << std::setw(11) << 1. / combPtInv / Gaudi::Units::GeV );

   ATH_MSG_DEBUG(" REGTEST Chi2 / ndof :"
         << " / " << std::setw(11) << chi2
         << " / " << std::setw(11) << ndof );

   //Cuts
   double winEtaSigma = m_WinEta;
   double winPhiSigma = m_WinPhi;
   double maxChi2     = m_Chi2Max;
   if (isTS) { //Trigger Station (barrel/EC)
      winEtaSigma = m_WinEta_TS;
      winPhiSigma = m_WinPhi_TS;
      maxChi2     = m_Chi2Max_TS;
   } else { //if endcaps and not TS
      if (!isBarrel) {
         winEtaSigma = m_WinEta_EC;
         winPhiSigma = m_WinPhi_EC;
         maxChi2     = m_Chi2Max_EC;
      }
   }
   bool isFT = false;
   if (isBarrel)
      if ((id_phi >= -2.60 && id_phi <= -2.10) || (id_phi >= -1.10 && id_phi <= -0.50)) isFT = true;
   if (isFT) { //if MS-feet region
      winEtaSigma = m_WinEta_FE;
      winPhiSigma = m_WinPhi_FE;
      maxChi2     = m_Chi2Max_FE;
   }

   ATH_MSG_DEBUG(" REGTEST DeltaEta / DeltaPhi / WinEta / WinPhi:"
         << " / " << std::setw(11) << fabs(deta)
         << " / " << std::setw(11) << fabs(dphi)
         << " / " << std::setw(11) << m_WeightEta*winEtaSigma*sqrt(extr_eeta * extr_eeta)
         << " / " << std::setw(11) << m_WeightPhi*winPhiSigma*sqrt(extr_ephi * extr_ephi) );

   if (fabs(deta) > m_WeightEta * winEtaSigma * sqrt(extr_eeta * extr_eeta)) {
      return 4;
   }
   if (fabs(dphi) > m_WeightPhi * winPhiSigma * sqrt(extr_ephi * extr_ephi)) {
      return 5;
   }
   if (ndof >= m_NdofMin) {
      if (chi2 > maxChi2) return 6;
   }

   return 0; //match OK
}


/** @fn bool muComb::execute()
 * @brief Execute() method.
 * @return bool
 */
StatusCode muComb::execute(const EventContext& ctx) const
{
   using namespace xAOD;

   // Monitoring variables
   //Timer
   auto timer = Monitored::Timer("TIME_execute");
   //Input
   auto ptMS = Monitored::Scalar("PtMS",        -9999.);
   auto etaMS = Monitored::Scalar("EtaMS",      -9999.);
   auto phiMS = Monitored::Scalar("PhiMS",      -9999.);
   auto zetaMS = Monitored::Scalar("ZetaMS",    -9999.);
   //ID
   auto ptID = Monitored::Scalar("PtID",        -9999.);
   auto etaID = Monitored::Scalar("EtaID",      -9999.);
   auto phiID = Monitored::Scalar("PhiID",      -9999.);
   auto zetaID = Monitored::Scalar("ZetaID",    -9999.);
   //Combined
   auto ptMC = Monitored::Scalar("PtMC",        -9999.);
   auto dEta = Monitored::Scalar("DEta",        -9999.);
   auto dPhi = Monitored::Scalar("DPhi",        -9999.);
   auto dZeta = Monitored::Scalar("DZeta",      -9999.);
   auto dR = Monitored::Scalar("DeltaR",        -9999.);
   //Failed
   auto ptFL = Monitored::Scalar("PtFL",        -9999.);
   auto etaFL = Monitored::Scalar("EtaFL",      -9999.);
   auto phiFL = Monitored::Scalar("PhiFL",      -9999.);
   //Info
   auto efficiency = Monitored::Scalar<int>("Efficiency",  -1);
   auto StrategyMC = Monitored::Scalar<int>("StrategyMC",  -1);
   auto ErrorFlagMC = Monitored::Scalar<int>("ErrorFlagMC", 0);
   auto MatchFlagMC = Monitored::Scalar<int>("MatchFlagMC", 0);


   auto mon = Monitored::Group(m_monTool, timer, ptMS, etaMS, phiMS, zetaMS, 
                                                        ptID, etaID, phiID, zetaID, 
                                                        ptMC, dEta, dPhi, dZeta, dR,
                                                        ptFL, etaFL, phiFL,
                                                        efficiency, StrategyMC, ErrorFlagMC, MatchFlagMC);


   StrategyMC  = m_AlgoStrategy;

   //Magnetic field status
   bool toroidOn   = !m_assumeToroidOff;
   bool solenoidOn = !m_assumeSolenoidOff;

   SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCacheCondObjInputKey, ctx};
   const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
   if (fieldCondObj == nullptr) {
       ATH_MSG_ERROR("execute: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
       return StatusCode::FAILURE;
   }
   MagField::AtlasFieldCache fieldCache;
   fieldCondObj->getInitializedCache (fieldCache);

   toroidOn   = fieldCache.toroidOn() && !m_assumeToroidOff;
   solenoidOn = fieldCache.solenoidOn() && !m_assumeSolenoidOff;
   ATH_MSG_DEBUG( "=========== Magnetic Field Status ========== " );
   ATH_MSG_DEBUG( " Assuming Toroid OFF is:                  " << (m_assumeToroidOff ? "TRUE" : "FALSE") );
   ATH_MSG_DEBUG( " Assuming Solenoid OFF is:                " << (m_assumeSolenoidOff ? "TRUE" : "FALSE") );
   ATH_MSG_DEBUG( " ---> Solenoid : " << ((solenoidOn) ? "ON" : "OFF") );
   ATH_MSG_DEBUG( " ---> Toroid   : " << ((toroidOn) ?   "ON" : "OFF") );

   // Algorithm strategy
   // select best matchinig strategy
   int usealgo = 0;                           //extrapolated muon - ID match (default)
   if (solenoidOn) {
      if (toroidOn) {
         if (m_AlgoStrategy == 0)  usealgo = 0;           //extrapolated muon - ID match
         else                      usealgo = 1;           //simple R-match w/o extrapolation (pt from combined MS-ID)
      } else {
         usealgo = 2;   //simple R-match w/o extrapolation (pt from ID)
      }
   } else {
      if (toroidOn) usealgo = 3;           //simple R-match w/o extrapolation (pt from MS)
      else          usealgo = 4;           //simple R-match w/o extrapolation (pt inf)
   }

   ATH_MSG_DEBUG( "MuCombStrategy: " << usealgo );

   ATH_MSG_DEBUG( "=========== Matching windows g4 ========== " );
   ATH_MSG_DEBUG( " WinEtaSigma g4: " << m_WinEta_g4 );
   ATH_MSG_DEBUG( " WinPhiSigma g4: " << m_WinPhi_g4 );
   ATH_MSG_DEBUG( " WinEtaSigma g4 EC: " << m_WinEta_EC_g4 );
   ATH_MSG_DEBUG( " WinPhiSigma g4 EC: " << m_WinPhi_EC_g4 );
   ATH_MSG_DEBUG( " WeightEta g4: " << m_WeightEta_g4 );
   ATH_MSG_DEBUG( " WeightPhi g4: " << m_WeightPhi_g4 );
   ATH_MSG_DEBUG( " Chi2Weight g4: " << m_Chi2Weight_g4 );
   ATH_MSG_DEBUG( " " );

   // Create Combined muon collection and record it with WriteHandle
   auto muonCBColl = SG::makeHandle (m_outputCBmuonCollKey, ctx);
   ATH_CHECK( muonCBColl.record (std::make_unique<xAOD::L2CombinedMuonContainer>(),
                                 std::make_unique<xAOD::L2CombinedMuonAuxContainer>()) );

   // Get input for seeding
   auto muonColl = SG::makeHandle(m_muonCollKey, ctx);  //SA muons

   // useL1 commented code to be removed once full muon chain with MT tested (SG)
   //Bool_t useL1 = false;
   if (muonColl->size() == 0) {
     //ATH_MSG_DEBUG(" L2 SA Muon collection size = 0");
     //if (usealgo == 2) {
     //    ATH_MSG_DEBUG(" L2StandAloneMuonContainer not found -> use L1 ROI " );
     //    useL1 = true;
     //} else {
     ATH_MSG_DEBUG(" L2StandAloneMuonContainer empty -> stop processing RoI, no match" );
     return StatusCode::SUCCESS;
     //}
   }

   // useL1 commented code to be removed once full muon chain with MT tested (SG)
   //xAOD::L2StandAloneMuonContainer* muonColl = const_cast<xAOD::L2StandAloneMuonContainer*>(const_muonColl);

   // retrieve L2StandAloneMuon (loop for L2 multi-track SA)
   for(uint i_muonSA = 0; i_muonSA < muonColl->size(); i_muonSA++){

     const xAOD::L2StandAloneMuon* muonSA = muonColl->at(i_muonSA);

     xAOD::L2CombinedMuon* muonCB = new xAOD::L2CombinedMuon();
     muonCB->makePrivateStore();
     muonCB->setPt(0.0);
     muonCB->setStrategy(usealgo);

     // Save SA muon EL into CB muon
     ElementLink<xAOD::L2StandAloneMuonContainer> muonSAEL(*muonColl, i_muonSA);
     muonCB->setMuSATrackLink(muonSAEL);

     // check muonSA pt != 0
     if (muonSA->pt() == 0.) {
       ErrorFlagMC = 1;
       if (usealgo == 2 || usealgo == 4) {
         ATH_MSG_DEBUG(" L2StandAloneMuon pt = 0 --> using angular match" );
         //if (usealgo == 2) useL1 = true;
       } else {
         ATH_MSG_DEBUG(" L2StandAloneMuon pt = 0 --> stop processing RoI, no match" );
         muonCB->setErrorFlag(ErrorFlagMC);
         muonCBColl->push_back(muonCB);
         return StatusCode::SUCCESS;
       }
     }

     // useL1 commented code to be removed once full muon chain with MT tested (SG)
     //double ptL1    = -999.;
     //double etaL1   = -999.;
     //double phiL1   = -999.;
     double pt      = -999.;
     double eta     = -999.;
     double phi     = -999.;
     double eta_ms  = -999.;
     double phi_ms  = -999.;
     double zeta_ms = -999.;
     // useL1 commented code to be removed once full muon chain with MT tested (SG)
     //if (useL1) {
     //   auto muonROIColl = SG::makeHandle(m_muonROICollKey, ctx); // L1 muon ROI
     //   if (muonROIColl->size() == 0) {
     //      ATH_MSG_DEBUG(" L1 Muon RoI collection size = 0");
     //      ATH_MSG_DEBUG(" L2StandAloneMuon pt == 0. && no L1 && torid=OFF --> no match" );
     //      ErrorFlagMC = 1;
     //      muonCB->setErrorFlag(ErrorFlagMC);
     //      return StatusCode::SUCCESS;
     //   } else {
     //      const TrigRoiDescriptor* muonROI = *(muonROIColl->begin());
     //      ptL1  = (muonRoI)->getThresholdValue();
     //      etaL1 = (muonRoI)->eta();
     //      phiL1 = (muonRoI)->phi();
     //      ATH_MSG_DEBUG( " Input L1 muon pt (GeV) = " << (muonROI)->getThresholdValue()
     //           <<  " / eta   = " << (muonROI)->eta()
     //           <<  " / phi   = " << (muonROI)->phi()
     //           <<  " / roiID = " << (muonROI)->roiId() );
     //   }
     //} else {

     pt       = muonSA->pt();
     eta      = muonSA->eta();
     phi      = muonSA->phi();
     eta_ms   = muonSA->etaMS();
     phi_ms   = muonSA->phiMS();
     zeta_ms  = muonSA->zMS();

     ATH_MSG_DEBUG(" Input L2StandaloneMuon pt (GeV) = " << pt
                << " / eta = "                    << eta
                << " / phi = "                    << phi
                << " / etaMS = "                  << eta_ms
                << " / phiMS = "                  << phi_ms
                << " / zMS = "                    << zeta_ms);
     //}

     // ID tracks Decoding

     // how to retrieve a specific colection name?
     SG::ReadHandle<xAOD::TrackParticleContainer> idTrackParticles(m_TrackParticleContainerKey, ctx); //L2 ID trks
     if (!idTrackParticles.isValid()){
       ATH_MSG_DEBUG(" Failed to get xAOD::TrackParticleContainer --> no match" );
       ErrorFlagMC = 2;
       muonCB->setErrorFlag(ErrorFlagMC);
       muonCBColl->push_back(muonCB);
       return StatusCode::SUCCESS;
     }
     if (idTrackParticles->size() < 1){
       ATH_MSG_DEBUG(" xAOD::TrackParticleContainer has 0 tracks --> no match" );
       ErrorFlagMC = 2;
       muonCB->setErrorFlag(ErrorFlagMC);
       muonCBColl->push_back(muonCB);
       return StatusCode::SUCCESS;
     }

     ATH_MSG_DEBUG( " Found xAOD::TrackParticleContainer with size: " << idTrackParticles->size() );

     // matching
     double ptinv_comb = 0.;
     double ptres_comb = 0.001;
     double chi2_comb  = 1.0e33;
     int    ndof_comb  = -1;
     double best_dr    = 1.0e33;
     double best_deta  = 1.0e33;
     double best_dphi  = 1.0e33;
     bool   has_match  = false;
     int    imatch     = -1;
     int    matched_trk_idx = -1;
     int    matched_trk_idx_tmp = -1;


     for(const auto trkit:(*idTrackParticles)) {

       matched_trk_idx_tmp++;
       double ptinv_tmp = 0.;
       double ptres_tmp = 0.001;
       double deta_tmp  = 0.;
       double dphi_tmp  = 0.;
       double dr_tmp    = 0.;
       double chi2_tmp  = 0.;
       int    ndof_tmp  = 0;
       bool   has_match_tmp = false;
       int    imatch_tmp    = -1;

       //Select tracks
       double phi_id    = (trkit)->phi();
       double eta_id    = (trkit)->eta();
       double qoverp_id = (trkit)->qOverP();
       double q_id      = ((trkit)->qOverP() > 0 ? 1.0 : -1.0);
       double pt_id     = (trkit)->pt() * q_id;

       double e_qoverp_id = std::sqrt((trkit)->definingParametersCovMatrix()(Trk::qOverP,Trk::qOverP));
       double e_phi_id = std::sqrt((trkit)->definingParametersCovMatrix()(Trk::phi0,Trk::phi0));
       double e_theta_id = std::sqrt((trkit)->definingParametersCovMatrix()(Trk::theta,Trk::theta));
       double tanthetaov2 = fabs(std::exp(-eta));
       double e_eta_id = fabs(0.5 * (1./tanthetaov2 + tanthetaov2) * e_theta_id);

       double e_qoverpt_id = e_qoverp_id;
       double theta_id = (trkit)->theta();
       if (sin(theta_id) != 0) e_qoverpt_id /= fabs(sin(theta_id)); //approximate

       ATH_MSG_DEBUG( " Found track: "
                   << "  with pt (GeV) = " << pt_id / Gaudi::Units::GeV
                   << ", q    = " << q_id
                   << ", eta  = " << eta_id
                   << ", phi  = " << phi_id
                   << ", th   = " << theta_id
                   << ", ephi = " << e_phi_id
                   << ", eth  = " << e_theta_id
                   << ", eeta = " << e_eta_id
                   << ", ip   = " << qoverp_id
                   << ", eip  = " << e_qoverp_id
                   << ", eipt = " << e_qoverpt_id );

       if (usealgo != 3) {
         if ((fabs(pt_id) / Gaudi::Units::GeV) < m_PtMinTrk)       continue;
       }
       if (fabs(eta_id)  > m_EtaMaxTrk)      continue;

       ATH_MSG_DEBUG(" Track selected " );

       if (usealgo > 0) {//DeltaR match
         // commented code (useL1) to be removed once full muon chain with MT tested
         //if (useL1) {
         //   imatch_tmp = drptMatch(ptL1, etaL1, phiL1, pt_id, eta_id, phi_id, usealgo, ptinv_tmp, ptres_tmp, deta_tmp, dphi_tmp, chi2_tmp);
         //} else {
         imatch_tmp = drptMatch(muonSA, pt_id, eta_id, phi_id, usealgo, ptinv_tmp, ptres_tmp, deta_tmp, dphi_tmp, chi2_tmp);
         //}
         if (imatch_tmp == 0) has_match_tmp = true;
       } else { //Std match
         if (!m_useBackExtrapolatorG4) {
           imatch_tmp = mfMatch(muonSA, eta_id, phi_id, pt_id, q_id, ptinv_tmp, ptres_tmp, deta_tmp, dphi_tmp, chi2_tmp, ndof_tmp);
           if (imatch_tmp == 0) has_match_tmp = true;
         } else { //G4 match
           imatch_tmp = g4Match(muonSA, eta_id, phi_id, pt_id, q_id, e_eta_id, e_phi_id, e_qoverpt_id, ptinv_tmp, ptres_tmp, deta_tmp, dphi_tmp, chi2_tmp, ndof_tmp);
           if (imatch_tmp == 0) has_match_tmp = true;
         }
       }

       imatch = imatch_tmp;
       if (!has_match_tmp) continue;

       // select nearest track
       dr_tmp = sqrt(deta_tmp * deta_tmp + dphi_tmp * dphi_tmp);

       if (chi2_tmp <= chi2_comb) {//Select nearest track
         has_match  = true;
         chi2_comb  = chi2_tmp;
         ndof_comb  = ndof_tmp;
         ptinv_comb = ptinv_tmp;
         ptres_comb = ptres_tmp;
         best_deta  = deta_tmp;
         best_dphi  = dphi_tmp;
         best_dr    = dr_tmp;
         imatch     = imatch_tmp;
         matched_trk_idx  = matched_trk_idx_tmp;
       }

     }//Tracks loop

     //Set monitored quantities (monitor only pt>6 GeV/c && standard matching)
     if (usealgo == 0 && fabs(pt) >= 6.) {
       ptMS       = pt;
       etaMS      = eta;
       phiMS      = phi;
       zetaMS     = zeta_ms;
       ptFL       = pt;
       etaFL      = eta;
       phiFL      = phi;
       if (ptMS >  100.) ptMS =  101.5;
       if (ptMS < -100.) ptMS = -101.5;
       if (ptFL >  100.) ptFL =  101.5;
       if (ptFL < -100.) ptFL = -101.5;
     }

     if (!has_match) {
       ErrorFlagMC = 3;
       MatchFlagMC = imatch;
       if (usealgo == 0 && fabs(pt) >= 6.)  efficiency = 0; //monitor only efficiency for mu6 && standard matching
       ATH_MSG_DEBUG( " No matched ID tracks --> no match" );
       muonCB->setErrorFlag(ErrorFlagMC);
       muonCB->setMatchFlag(MatchFlagMC);
       muonCBColl->push_back(muonCB);
       return StatusCode::SUCCESS;
     }

     //Save EL to ID trk into CB muon
     ElementLink<xAOD::TrackParticleContainer> idtrkEL(*idTrackParticles, matched_trk_idx);
     muonCB->setIdTrackLink(idtrkEL);

     double pt_id        = muonCB->idTrack()->pt();
     double q_id         = ((muonCB->idTrack()->qOverP()) > 0 ? 1.0 : -1.0);
     double phi_id       = muonCB->idTrack()->phi();
     double eta_id       = muonCB->idTrack()->eta();
     //const Trk::Perigee& idtrk_perigee = muonCB->idTrack()->perigeeParameters();
     double zPos_id      = muonCB->idTrack()->z0(); //idtrk_perigee.parameters()[Trk::z0];

     ATH_MSG_DEBUG(" SA muon macthed to ID track ..." );

     //Update monitored vars
     MatchFlagMC = imatch;
     ptFL        = -9999.;
     etaFL       = -9999.;
     phiFL       = -9999.;
     if (usealgo == 0 && fabs(pt) >= 6.) {
       efficiency = 1;
       ptID       = pt_id / Gaudi::Units::GeV; //in GeV/c
       etaID      = eta_id;
       phiID      = phi_id;
       zetaID     = zPos_id;
       ptMC       = 1. / (ptinv_comb * Gaudi::Units::GeV); //in GeV/c
       dZeta      = zeta_ms - zPos_id;
       dPhi       = best_dphi;
       dEta       = best_deta;
       dR         = best_dr;

       if (ptMC >  100.) ptMC =  101.5;
       if (ptMC < -100.) ptMC = -101.5;
       if (ptID >  100.) ptID =  101.5;
       if (ptID < -100.) ptID = -101.5;
     }

     double prt_pt = pt;
     // commented code (useL1) to be removed once full muon chain with MT tested
     //if (useL1) prt_pt = ptL1;
     ATH_MSG_DEBUG( " REGTEST Combination chosen: "
                 << " usealgo / IdPt (GeV) / muonPt (GeV) / CombPt (GeV) / chi2 / ndof: " << " / " << usealgo << " / " << pt_id*q_id / Gaudi::Units::GeV << " / " << prt_pt << " / " << 1. / ptinv_comb / Gaudi::Units::GeV << " / " << chi2_comb << " / " << ndof_comb );

     muonCB->setPt(fabs(1. / ptinv_comb));
     muonCB->setEta(eta_id);
     muonCB->setPhi(phi_id);

     float mcq = -1.0;
     if (ptinv_comb > 0) mcq = 1.0;

     muonCB->setCharge(mcq);

     float mcresu = fabs(ptres_comb / (ptinv_comb * ptinv_comb));
     ATH_MSG_DEBUG( " SigmaPt (GeV) is: " << mcresu / Gaudi::Units::GeV );
     muonCB->setSigmaPt(mcresu);

     muonCB->setErrorFlag(ErrorFlagMC);
     muonCB->setMatchFlag(MatchFlagMC);

     muonCBColl->push_back(muonCB);
   }
   return StatusCode::SUCCESS;
}
