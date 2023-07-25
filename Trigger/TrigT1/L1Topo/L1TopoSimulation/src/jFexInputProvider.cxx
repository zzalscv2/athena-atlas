/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <math.h>

#include "jFexInputProvider.h"
#include "TrigT1CaloEvent/JetROI_ClassDEF.h"
#include "L1TopoEvent/TopoInputEvent.h"
#include "L1TopoSimulationUtils/Conversions.h"
#include "TrigT1CaloEvent/EnergyRoI_ClassDEF.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

#include "GaudiKernel/PhysicalConstants.h"

using namespace std;
using namespace LVL1;


// jFex to L1Topo conversion factors
const int jFexInputProvider::m_Et_conversion = 2;            // 200 MeV to 100 MeV
const double jFexInputProvider::m_sumEt_conversion = 0.01;   // 1 MeV to 100 MeV
const int jFexInputProvider::m_phi_conversion = 2;           // 10 x phi to 20 x phi
const int jFexInputProvider::m_eta_conversion = 4;           // 10 x eta to 40 x eta

const double jFexInputProvider::m_EtDouble_conversion = 0.1;       // 100 MeV to GeV
const double jFexInputProvider::m_sumEtDouble_conversion = 0.1;    // 100 MeV to GeV
const double jFexInputProvider::m_phiDouble_conversion = 0.05;     // 20 x phi to phi
const double jFexInputProvider::m_etaDouble_conversion = 0.025;    // 40 x eta to eta


jFexInputProvider::jFexInputProvider(const std::string& type, const std::string& name, 
                                   const IInterface* parent) :
   base_class(type, name, parent)
{
   declareInterface<LVL1::IInputTOBConverter>( this );
}

jFexInputProvider::~jFexInputProvider()
{}

StatusCode
jFexInputProvider::initialize() {

   CHECK(m_jJet_EDMKey.initialize(SG::AllowEmpty));
   CHECK(m_jLJet_EDMKey.initialize(SG::AllowEmpty));
   CHECK(m_jEM_EDMKey.initialize(SG::AllowEmpty));
   CHECK(m_jTau_EDMKey.initialize(SG::AllowEmpty));
   CHECK(m_jXE_EDMKey.initialize(SG::AllowEmpty));
   CHECK(m_jTE_EDMKey.initialize(SG::AllowEmpty));

   if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

   return StatusCode::SUCCESS;
}

StatusCode
jFexInputProvider::fillEM(TCS::TopoInputEvent& inputEvent) const {
  if (m_jEM_EDMKey.empty()) {
    ATH_MSG_DEBUG("jFex EM input disabled, skip filling");
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::jFexFwdElRoIContainer> jEM_EDM(m_jEM_EDMKey);
  ATH_CHECK(jEM_EDM.isValid());

  for(const xAOD::jFexFwdElRoI* jFexRoI : * jEM_EDM) {

    ATH_MSG_DEBUG( "EDM jFex Em Number: " 
           << +jFexRoI->jFexNumber() // returns an 8 bit unsigned integer referring to the jFEX number 
           << " et: " 
           << jFexRoI->et() // returns the et value of the jet in MeV unit
           << " tobEt: " 
           << jFexRoI->tobEt() // returns the et value of the jet in units of 200 MeV
           << " globalEta: "
           << jFexRoI->globalEta() // returns simplified global eta in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
           << " globalPhi: "
           << jFexRoI->globalPhi() // returns simplified global phi in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
           << " tobEMIso: "
           << +jFexRoI->tobEMIso() // returns isolation bits
           << " tobEMf1: "
           << +jFexRoI->tobEMf1() // returns isolation bits
           << " tobEMf2: "
           << +jFexRoI->tobEMf2() // returns isolation bits
           );
       
    unsigned int EtTopo = jFexRoI->tobEt()*m_Et_conversion;
    unsigned int phiTopo = TSU::toTopoPhi(jFexRoI->phi());
    int etaTopo = TSU::toTopoEta(jFexRoI->eta());
    unsigned int isolation = jFexRoI->tobEMIso();
    unsigned int frac1 = jFexRoI->tobEMf2();
    unsigned int frac2 = jFexRoI->tobEMf1();
   
    // Avoid the events with 0 Et (events below threshold)
    if (EtTopo==0) continue;

    TCS::jEmTOB jem( EtTopo, etaTopo, phiTopo );
    jem.setEtDouble( static_cast<double>(EtTopo*m_EtDouble_conversion) );
    jem.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
    jem.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );
    jem.setIsolation( isolation );
    jem.setFrac1( frac1 );
    jem.setFrac2( frac2 );

    inputEvent.addjEm( jem );

    auto mon_h_jEmPt = Monitored::Scalar("jEmTOBPt", jem.EtDouble());
    auto mon_h_jEmIsolation = Monitored::Scalar("jEmTOBIsolation", jem.isolation());
    auto mon_h_jEmFrac1 = Monitored::Scalar("jEmTOBFrac1", jem.frac1());
    auto mon_h_jEmFrac2 = Monitored::Scalar("jEmTOBFrac2", jem.frac2());
    auto mon_h_jEmPhi = Monitored::Scalar("jEmTOBPhi", jem.phi());
    auto mon_h_jEmEta = Monitored::Scalar("jEmTOBEta", jem.eta());
    Monitored::Group(m_monTool, mon_h_jEmPt, mon_h_jEmIsolation, mon_h_jEmFrac1, mon_h_jEmFrac2, mon_h_jEmPhi, mon_h_jEmEta); 
  }

  return StatusCode::SUCCESS;
}


StatusCode
jFexInputProvider::fillTau(TCS::TopoInputEvent& inputEvent) const {
  if (m_jTau_EDMKey.empty()) {
    ATH_MSG_DEBUG("jFex Tau input disabled, skip filling");
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::jFexTauRoIContainer> jTau_EDM(m_jTau_EDMKey);
  ATH_CHECK(jTau_EDM.isValid());

  for(const xAOD::jFexTauRoI* jFexRoI : * jTau_EDM) {

    ATH_MSG_DEBUG( "EDM jFex Tau Number: " 
		   << +jFexRoI->jFexNumber() // returns an 8 bit unsigned integer referring to the jFEX number 
		   << " et: " 
		   << jFexRoI->et() // returns the et value of the jet in MeV unit
		   << " tobEt: " 
		   << jFexRoI->tobEt() // returns the et value of the jet in units of 200 MeV
		   << " globalEta: "
		   << jFexRoI->globalEta() // returns simplified global eta in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
		   << " globalPhi: "
		   << jFexRoI->globalPhi() // returns simplified global phi in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
		   << " isolation: "
		   << jFexRoI->tobIso() // returns isolation value in units of 200 MeV
		   );
       
    unsigned int EtTopo = jFexRoI->tobEt()*m_Et_conversion;
    unsigned int phiTopo = TSU::toTopoPhi(jFexRoI->phi());
    int etaTopo = TSU::toTopoEta(jFexRoI->eta());
    unsigned int isolation = jFexRoI->tobIso()*m_Et_conversion;
   
    // Avoid the events with 0 Et (events below threshold)
    if (EtTopo==0) continue;

    TCS::jTauTOB jtau( EtTopo, etaTopo, phiTopo );
    jtau.setEtDouble( static_cast<double>(EtTopo*m_EtDouble_conversion) );
    jtau.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
    jtau.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );
    jtau.setEtIso( isolation );

    inputEvent.addjTau( jtau );
    inputEvent.addcTau( jtau );

    auto mon_h_jTauPt = Monitored::Scalar("jTauTOBPt", jtau.EtDouble());
    auto mon_h_jTauIsolation = Monitored::Scalar("jTauTOBIsolation", jtau.EtIso()*m_EtDouble_conversion);
    auto mon_h_jTauPhi = Monitored::Scalar("jTauTOBPhi", jtau.phi());
    auto mon_h_jTauEta = Monitored::Scalar("jTauTOBEta", jtau.eta());
    Monitored::Group(m_monTool, mon_h_jTauPt, mon_h_jTauIsolation, mon_h_jTauPhi, mon_h_jTauEta); 
  }

  return StatusCode::SUCCESS;
}


StatusCode
jFexInputProvider::fillLRJet(TCS::TopoInputEvent& inputEvent) const {
  if (m_jLJet_EDMKey.empty()) {
    ATH_MSG_DEBUG("jFex LJet input disabled, skip filling");
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::jFexLRJetRoIContainer> jLJet_EDM(m_jLJet_EDMKey);
  ATH_CHECK(jLJet_EDM.isValid());
  
  for(const xAOD::jFexLRJetRoI* jFexRoI : * jLJet_EDM) {

    ATH_MSG_DEBUG( "EDM jFex LJet Number: " 
		   << jFexRoI->jFexNumber() // returns an 8 bit unsigned integer referring to the jFEX number
	           << " et: " 
		   << jFexRoI->et() // returns the et value of the jet in MeV
	           << " tobEt: " 
		   << jFexRoI->tobEt() // returns the et value of the jet in units of 200 MeV
		   << " globalEta: "
		   << jFexRoI->globalEta() // returns simplified global eta in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
		   << " globalPhi: "
		   << jFexRoI->globalPhi() // returns simplified global phi in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
		   );
    
    unsigned int EtTopo = jFexRoI->tobEt()*m_Et_conversion;
    unsigned int phiTopo = TSU::toTopoPhi(jFexRoI->phi());
    int etaTopo = TSU::toTopoEta(jFexRoI->eta());

    // Avoid the events with 0 Et (events below threshold)
    if (EtTopo==0) continue;

    TCS::jLJetTOB jet( EtTopo, etaTopo, phiTopo );
    jet.setEtDouble( static_cast<double>(EtTopo*m_EtDouble_conversion) );
    jet.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
    jet.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );

    inputEvent.addjLJet( jet );

    auto mon_h_jLJetPt = Monitored::Scalar("jLJetTOBPt", jet.EtDouble());
    auto mon_h_jLJetPhi = Monitored::Scalar("jLJetTOBPhi", jet.phi());
    auto mon_h_jLJetEta = Monitored::Scalar("jLJetTOBEta", jet.eta());
    Monitored::Group(m_monTool, mon_h_jLJetPt, mon_h_jLJetPhi, mon_h_jLJetEta); 
  }

  return StatusCode::SUCCESS;
}


StatusCode
jFexInputProvider::fillSRJet(TCS::TopoInputEvent& inputEvent) const {
  if (m_jJet_EDMKey.empty()) {
    ATH_MSG_DEBUG("jFex Jet input disabled, skip filling");
    return StatusCode::SUCCESS;
  }
  SG::ReadHandle<xAOD::jFexSRJetRoIContainer> jJet_EDM(m_jJet_EDMKey);
  ATH_CHECK(jJet_EDM.isValid());
  
  for(const xAOD::jFexSRJetRoI* jFexRoI : * jJet_EDM){

    ATH_MSG_DEBUG( "EDM jFex Jet Number: " 
		   << +jFexRoI->jFexNumber() // returns an 8 bit unsigned integer referring to the jFEX number 
		   << " et: " 
		   << jFexRoI->et() // returns the et value of the jet in MeV
		   << " tobEt: " 
		   << jFexRoI->tobEt() // returns the et value of the jet in units of 200 MeV
		   << " globalEta: "
		   << jFexRoI->globalEta() // returns simplified global eta in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
		   << " globalPhi: "
		   << jFexRoI->globalPhi() // returns simplified global phi in units of 0.1 (fcal straightened out, not suitable for use in L1TopoSim)
		   );

    unsigned int EtTopo = jFexRoI->tobEt()*m_Et_conversion;
    unsigned int phiTopo = TSU::toTopoPhi(jFexRoI->phi());
    int etaTopo = TSU::toTopoEta(jFexRoI->eta());

    // Avoid the events with 0 Et (events below threshold)
    if (EtTopo==0) continue;

    TCS::jJetTOB jet( EtTopo, etaTopo, phiTopo );
    jet.setEtDouble( static_cast<double>(EtTopo*m_EtDouble_conversion) );
    jet.setEtaDouble( static_cast<double>(etaTopo*m_etaDouble_conversion) );
    jet.setPhiDouble( static_cast<double>(phiTopo*m_phiDouble_conversion) );
 
    inputEvent.addjJet( jet );

    auto mon_h_jJetPt = Monitored::Scalar("jJetTOBPt", jet.EtDouble());
    auto mon_h_jJetPhi = Monitored::Scalar("jJetTOBPhi", jet.phi());
    auto mon_h_jJetEta = Monitored::Scalar("jJetTOBEta", jet.eta());
    Monitored::Group(m_monTool, mon_h_jJetPt, mon_h_jJetPhi, mon_h_jJetEta); 
    
  }

  return StatusCode::SUCCESS;
}


StatusCode
jFexInputProvider::fillXE(TCS::TopoInputEvent& inputEvent) const {
  
  if (m_jXE_EDMKey.empty()) {
    ATH_MSG_DEBUG("jFex XE input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::jFexMETRoIContainer> jXE_EDM(m_jXE_EDMKey);
  ATH_CHECK(jXE_EDM.isValid());

  int global_ExTopo = 0;
  int global_EyTopo = 0;

  for(const xAOD::jFexMETRoI* jFexRoI : *jXE_EDM){

    // Get the XE components and convert to 100 MeV units
    int ExTopo = jFexRoI->tobEx()*m_Et_conversion;
    int EyTopo = jFexRoI->tobEy()*m_Et_conversion;
    int jFexNumber = jFexRoI->jFexNumber();
    int fpgaNumber = jFexRoI->fpgaNumber();  

    int hemisphere = fpgaNumber == 0 ? -1 : 1; //Note: flipped to produce the right sign

    ExTopo = hemisphere * ExTopo;
    EyTopo = hemisphere * EyTopo;

    global_ExTopo += ExTopo;
    global_EyTopo += EyTopo;

    ATH_MSG_DEBUG( "EDM jFex XE Number: "
                   << jFexNumber
                   << " FPGA Number: "
                   << fpgaNumber
                   << " Ex: "
                   << ExTopo
                   << " Ey: "
                   << EyTopo
                   );
  }

  unsigned long long global_ExTopoLong = static_cast<unsigned long long>(global_ExTopo);
  unsigned long long global_EyTopoLong = static_cast<unsigned long long>(global_EyTopo);

  unsigned long long Et2Topo = global_ExTopoLong*global_ExTopoLong + global_EyTopoLong*global_EyTopoLong;
  unsigned int EtTopo =  std::sqrt( Et2Topo );

  TCS::jXETOB jxe( -(global_ExTopo), -(global_EyTopo), EtTopo, TCS::JXE );

  jxe.setExDouble( static_cast<double>(-global_ExTopo*m_EtDouble_conversion) );
  jxe.setEyDouble( static_cast<double>(-global_EyTopo*m_EtDouble_conversion) );
  jxe.setEtDouble( static_cast<double>(EtTopo*m_EtDouble_conversion) );
  jxe.setEt2( Et2Topo );

  inputEvent.setjXE( jxe );
  auto mon_h_jXE_Pt = Monitored::Scalar("jXETOBPt", jxe.EtDouble());
  auto mon_h_jXE_Phi = Monitored::Scalar("jXETOBPhi", atan2(jxe.Ey(),jxe.Ex()));
  Monitored::Group(m_monTool, mon_h_jXE_Pt, mon_h_jXE_Phi); 

  return StatusCode::SUCCESS;
}


StatusCode
jFexInputProvider::fillTE(TCS::TopoInputEvent& inputEvent) const {
  
  if (m_jTE_EDMKey.empty()) {
    ATH_MSG_DEBUG("jFex TE input disabled, skip filling");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::jFexSumETRoIContainer> jTE_EDM(m_jTE_EDMKey);
  ATH_CHECK(jTE_EDM.isValid());

  int topoTE = 0;
  bool topoTE_sat = false;
  // jTE variations include jTEC, jTEFWD, jTEFWDA, jTEFWDC
  // These quantities are defined according to the jFex module number
  // FWDA = 5, FWDC = 0, C = 1,2,3,4
  int topoTEC = 0;
  int topoTEFWD = 0;
  int topoTEFWDA = 0;
  int topoTEFWDC = 0;

  for(const xAOD::jFexSumETRoI* jFexRoI : *jTE_EDM){

    // Get the TE components (upper and lower) and convert to 100 MeV units
    int EtLowerTopo = jFexRoI->Et_lower()*m_sumEt_conversion;
    int EtUpperTopo = jFexRoI->Et_upper()*m_sumEt_conversion;
    int jFexNumber = jFexRoI->jFexNumber();
    int fpgaNumber = jFexRoI->fpgaNumber(); 

    int Sat_lower = jFexRoI->tobSat_lower();
    int Sat_upper = jFexRoI->tobSat_upper();

    ATH_MSG_DEBUG( "EDM jFex TE Number: "
                   << jFexNumber
                   << " FPGA Number: "
                   << fpgaNumber
                   << " Et_lower: "
                   << EtLowerTopo
                   << " Et_upper: "
                   << EtUpperTopo
                   << " Sat_lower: "
                   << Sat_lower
                   << " Sat_upper: "
                   << Sat_upper
                   );

    // jTE
    topoTE += EtLowerTopo;
    topoTE += EtUpperTopo;
    topoTE_sat |= Sat_lower;
    topoTE_sat |= Sat_upper;

    // jTEC
    topoTEC += EtLowerTopo;
    if( jFexNumber!=0 && jFexNumber!=5 )
      {
	topoTEC += EtUpperTopo;
      }

    // jTEFWD
    if( jFexNumber==0 || jFexNumber==5 )
      {
	topoTEFWD += EtUpperTopo;
      }

    // jTEFWDA
    if( jFexNumber==5 )
      {
	topoTEFWDA += EtUpperTopo;
      }

    // jTEFWDC
    if( jFexNumber==0 )
      {
	topoTEFWDC += EtUpperTopo;
      }
  }

  TCS::jTETOB jte( static_cast<unsigned int>(topoTE), TCS::JTE );
  TCS::jTETOB jtec( static_cast<unsigned int>(topoTEC), TCS::JTEC );
  TCS::jTETOB jtefwd( static_cast<unsigned int>(topoTEFWD), TCS::JTEFWD );
  TCS::jTETOB jtefwda( static_cast<unsigned int>(topoTEFWDA), TCS::JTEFWDA );
  TCS::jTETOB jtefwdc( static_cast<unsigned int>(topoTEFWDC), TCS::JTEFWDC );

  jte.setSaturationFlag( static_cast<double>(topoTE_sat) );
  jte.setSumEtDouble( static_cast<double>(topoTE*m_sumEtDouble_conversion) );
  jtec.setSumEtDouble( static_cast<double>(topoTEC*m_sumEtDouble_conversion) );
  jtefwd.setSumEtDouble( static_cast<double>(topoTEFWD*m_sumEtDouble_conversion) );
  jtefwda.setSumEtDouble( static_cast<double>(topoTEFWDA*m_sumEtDouble_conversion) );
  jtefwdc.setSumEtDouble( static_cast<double>(topoTEFWDC*m_sumEtDouble_conversion) );

  inputEvent.setjTE( jte );
  inputEvent.setjTEC( jtec );
  inputEvent.setjTEFWD( jtefwd );
  inputEvent.setjTEFWDA( jtefwda );
  inputEvent.setjTEFWDC( jtefwdc );

  auto mon_h_jTE_saturation = Monitored::Scalar("jTETOBsaturation", jte.saturationFlag());
  auto mon_h_jTE_sumEt = Monitored::Scalar("jTETOBsumEt", jte.sumEtDouble());
  auto mon_h_jTEC_sumEt = Monitored::Scalar("jTECTOBsumEt", jtec.sumEtDouble());
  auto mon_h_jTEFWD_sumEt = Monitored::Scalar("jTEFWDTOBsumEt", jtefwd.sumEtDouble());
  auto mon_h_jTEFWDA_sumEt = Monitored::Scalar("jTEFWDATOBsumEt", jtefwda.sumEtDouble());
  auto mon_h_jTEFWDC_sumEt = Monitored::Scalar("jTEFWDCTOBsumEt", jtefwdc.sumEtDouble());
  Monitored::Group(m_monTool, mon_h_jTE_saturation, mon_h_jTE_sumEt, mon_h_jTEC_sumEt, mon_h_jTEFWD_sumEt, mon_h_jTEFWDA_sumEt, mon_h_jTEFWDC_sumEt); 

  return StatusCode::SUCCESS;
}


StatusCode
jFexInputProvider::fillTopoInputEvent(TCS::TopoInputEvent& inputEvent) const {
  ATH_CHECK(fillEM(inputEvent));
  ATH_CHECK(fillTau(inputEvent));
  ATH_CHECK(fillSRJet(inputEvent));
  ATH_CHECK(fillLRJet(inputEvent));
  ATH_CHECK(fillXE(inputEvent));
  ATH_CHECK(fillTE(inputEvent));
  return StatusCode::SUCCESS;
}
