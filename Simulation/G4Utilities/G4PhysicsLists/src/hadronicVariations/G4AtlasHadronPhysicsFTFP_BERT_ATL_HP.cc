//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4AtlasHadronPhysicsFTFP_BERT_ATL_HP.cc 83699 2014-09-10 07:18:25Z gcosmo $
//
//---------------------------------------------------------------------------
// Author: Alberto Ribon
// Date:   October 2017
//
// Hadron physics for the new physics list FTFP_BERT_ATL_HP.
// This is a modified version of the FTFP_BERT_HP hadron physics for ATLAS.
// The hadron physics of FTFP_BERT_ATL_HP has the transition between Bertini
// (BERT) intra-nuclear cascade model and Fritiof (FTF) string model in the
// energy region [9, 12] GeV (instead of [4, 5] GeV as in FTFP_BERT).
//----------------------------------------------------------------------------
//
#include <iomanip>   

#include "G4AtlasHadronPhysicsFTFP_BERT_ATL_HP.hh"

#include "G4Version.hh"
#include "globals.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4ChipsKaonMinusInelasticXS.hh"
#include "G4ChipsKaonPlusInelasticXS.hh"
#include "G4ChipsKaonZeroInelasticXS.hh"
#include "G4CrossSectionDataSetRegistry.hh"

#if G4VERSION_NUMBER < 1100
#include "G4HadronCaptureProcess.hh"
#include "G4HadronFissionProcess.hh"
#else
#include "G4NeutronCaptureProcess.hh"
#include "G4NeutronFissionProcess.hh"
#endif
#include "G4NeutronRadCapture.hh"
#include "G4NeutronCaptureXS.hh"
#include "G4NeutronHPCaptureData.hh"
#include "G4LFission.hh"

#include "G4PhysListUtil.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(G4AtlasHadronPhysicsFTFP_BERT_ATL_HP);

G4ThreadLocal G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::ThreadPrivate* G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::tpdata=0;

G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::G4AtlasHadronPhysicsFTFP_BERT_ATL_HP(G4int)
    :  G4VPhysicsConstructor("hInelastic FTFP_BERT_ATL_HP")
/*    , theNeutrons(0)
    , theBertiniNeutron(0)
    , theFTFPNeutron(0)
    , theHPNeutron(0)
    , thePiK(0)
    , theBertiniPiK(0)
    , theFTFPPiK(0)
    , thePro(0)
    , theBertiniPro(0)
    , theFTFPPro(0)
    , theHyperon(0)
    , theAntiBaryon(0)
    , theFTFPAntiBaryon(0) */
    , QuasiElastic(false)
  /*    , ChipsKaonMinus(0)
    , ChipsKaonPlus(0)
    , ChipsKaonZero(0)
    , xsNeutronCaptureXS(0)*/
{}

G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::G4AtlasHadronPhysicsFTFP_BERT_ATL_HP(const G4String& name, G4bool quasiElastic)
    :  G4VPhysicsConstructor(name) 
/*    , theNeutrons(0)
    , theBertiniNeutron(0)
    , theFTFPNeutron(0)
    , theHPNeutron(0)
    , thePiK(0)
    , theBertiniPiK(0)
    , theFTFPPiK(0)
    , thePro(0)
    , theBertiniPro(0)
    , theFTFPPro(0)
    , theHyperon(0)
    , theAntiBaryon(0)
    , theFTFPAntiBaryon(0)*/
    , QuasiElastic(quasiElastic)
  /*    , ChipsKaonMinus(0)
    , ChipsKaonPlus(0)
    , ChipsKaonZero(0)
    , xsNeutronCaptureXS(0)*/
{}

void G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::CreateModels()
{

  G4double minFTFP =  9.0 * GeV;
  G4double maxBERT = 12.0 * GeV;
  G4cout << " FTFP_BERT_ATL_HP : new threshold between BERT and FTFP" 
         << " is over the interval " << minFTFP/GeV << " to " << maxBERT/GeV 
         << " GeV." << G4endl;

  tpdata->theNeutrons=new G4NeutronBuilder;
  tpdata->theFTFPNeutron=new G4FTFPNeutronBuilder(QuasiElastic);
  tpdata->theNeutrons->RegisterMe(tpdata->theFTFPNeutron);
  tpdata->theFTFPNeutron->SetMinEnergy(minFTFP);
  tpdata->theNeutrons->RegisterMe(tpdata->theBertiniNeutron=new G4BertiniNeutronBuilder);
  tpdata->theBertiniNeutron->SetMinEnergy(19.9*MeV);
  tpdata->theBertiniNeutron->SetMaxEnergy(maxBERT);
  tpdata->theNeutrons->RegisterMe(tpdata->theHPNeutron=new G4NeutronHPBuilder);

  tpdata->thePro=new G4ProtonBuilder;
  tpdata->theFTFPPro=new G4FTFPProtonBuilder(QuasiElastic);
  tpdata->thePro->RegisterMe(tpdata->theFTFPPro);
  tpdata->theFTFPPro->SetMinEnergy(minFTFP);
  tpdata->thePro->RegisterMe(tpdata->theBertiniPro=new G4BertiniProtonBuilder);
  tpdata->theBertiniPro->SetMaxEnergy(maxBERT);

  tpdata->thePiK=new G4PiKBuilder;
  tpdata->theFTFPPiK=new G4FTFPPiKBuilder(QuasiElastic);
  tpdata->thePiK->RegisterMe(tpdata->theFTFPPiK);
  tpdata->theFTFPPiK->SetMinEnergy(minFTFP);
  tpdata->thePiK->RegisterMe(tpdata->theBertiniPiK=new G4BertiniPiKBuilder);
  tpdata->theBertiniPiK->SetMaxEnergy(maxBERT);
  
  tpdata->theHyperon=new G4HyperonFTFPBuilder;
    
  tpdata->theAntiBaryon=new G4AntiBarionBuilder;
  tpdata->theAntiBaryon->RegisterMe(tpdata->theFTFPAntiBaryon=new  G4FTFPAntiBarionBuilder(QuasiElastic));
}

G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::~G4AtlasHadronPhysicsFTFP_BERT_ATL_HP()
{
  if (!tpdata) return;

  delete tpdata->theNeutrons;
  delete tpdata->theBertiniNeutron;
  delete tpdata->theFTFPNeutron;
  delete tpdata->theHPNeutron;

  delete tpdata->thePiK;
  delete tpdata->theBertiniPiK;
  delete tpdata->theFTFPPiK;

  delete tpdata->thePro;
  delete tpdata->theBertiniPro;
  delete tpdata->theFTFPPro;    

  delete tpdata->theHyperon;
  delete tpdata->theAntiBaryon;
  delete tpdata->theFTFPAntiBaryon;

  //Note that here we need to set to 0 the pointer
  //since tpdata is static and if thread are "reused"
  //it can be problematic
  delete tpdata; tpdata = 0;
}

void G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::ConstructParticle()
{
  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();  
}

#include "G4ProcessManager.hh"
void G4AtlasHadronPhysicsFTFP_BERT_ATL_HP::ConstructProcess()
{
  if ( tpdata == 0 ) tpdata = new ThreadPrivate;
  CreateModels();
  tpdata->theNeutrons->Build();
  tpdata->thePro->Build();
  tpdata->thePiK->Build();

  // --- Kaons ---
  tpdata->ChipsKaonMinus = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4ChipsKaonMinusInelasticXS::Default_Name());
  tpdata->ChipsKaonPlus = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4ChipsKaonPlusInelasticXS::Default_Name());
  tpdata->ChipsKaonZero = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4ChipsKaonZeroInelasticXS::Default_Name());
    
  G4PhysListUtil::FindInelasticProcess(G4KaonMinus::KaonMinus())->AddDataSet(tpdata->ChipsKaonMinus);
  G4PhysListUtil::FindInelasticProcess(G4KaonPlus::KaonPlus())->AddDataSet(tpdata->ChipsKaonPlus);
  G4PhysListUtil::FindInelasticProcess(G4KaonZeroShort::KaonZeroShort())->AddDataSet(tpdata->ChipsKaonZero );
  G4PhysListUtil::FindInelasticProcess(G4KaonZeroLong::KaonZeroLong())->AddDataSet(tpdata->ChipsKaonZero );
    
  tpdata->theHyperon->Build();
  tpdata->theAntiBaryon->Build();

  // --- Neutrons ---
  G4HadronicProcess* capture = 0;
  G4HadronicProcess* fission = 0;
  G4ProcessManager* pmanager = G4Neutron::Neutron()->GetProcessManager();
  G4ProcessVector*  pv = pmanager->GetProcessList();
  for ( size_t i=0; i < static_cast<size_t>(pv->size()); ++i ) {
    if ( fCapture == ((*pv)[i])->GetProcessSubType() ) {
      capture = static_cast<G4HadronicProcess*>((*pv)[i]);
    } else if ( fFission == ((*pv)[i])->GetProcessSubType() ) {
      fission = static_cast<G4HadronicProcess*>((*pv)[i]);
    }
  }
  if ( ! capture ) {
#if G4VERSION_NUMBER < 1100
    capture = new G4HadronCaptureProcess("nCapture");
#else
    capture = new G4NeutronCaptureProcess("nCapture");
#endif
    pmanager->AddDiscreteProcess(capture);
  }
  tpdata->xsNeutronCaptureXS = (G4NeutronCaptureXS*)G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet(G4NeutronCaptureXS::Default_Name());
  capture->AddDataSet(tpdata->xsNeutronCaptureXS);
  capture->AddDataSet( new G4NeutronHPCaptureData );
  G4NeutronRadCapture* theNeutronRadCapture = new G4NeutronRadCapture(); 
  theNeutronRadCapture->SetMinEnergy( 19.9*MeV ); 
  capture->RegisterMe( theNeutronRadCapture );
  if ( ! fission ) {
#if G4VERSION_NUMBER < 1100
    fission = new G4HadronFissionProcess("nFission");
#else
    fission = new G4NeutronFissionProcess("nFission");
#endif
    pmanager->AddDiscreteProcess(fission);
  }
  G4LFission* theNeutronLEPFission = new G4LFission();
  theNeutronLEPFission->SetMinEnergy( 19.9*MeV );
  fission->RegisterMe( theNeutronLEPFission );
}
