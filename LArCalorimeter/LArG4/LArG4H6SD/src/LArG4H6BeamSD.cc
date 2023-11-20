/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4H6BeamSD.h"

#include "G4NavigationHistory.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4TouchableHistory.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "G4ios.hh"
#include <iomanip>
#include <utility>

#undef DEBUG_HITS

const float LArG4H6BeamSD::WDIM = 6.4*CLHEP::cm;
const float LArG4H6BeamSD::DW = 0.1*CLHEP::cm;
const float LArG4H6BeamSD::BPC_WDIM = 7.5*CLHEP::cm;
const float LArG4H6BeamSD::BPC_DW = 0.6*CLHEP::mm;
const float LArG4H6BeamSD::BPCO_WDIM = 6.0*CLHEP::cm;
const float LArG4H6BeamSD::BPCO_DW = 0.6*CLHEP::mm;

LArG4H6BeamSD::LArG4H6BeamSD(G4String name, const G4String& colname)
  : G4VSensitiveDetector(std::move(name))
  , m_FrontCollection(colname)
{
}

void LArG4H6BeamSD::Initialize(G4HCofThisEvent* /*HCE*/)
{
#ifdef DEBUG_HITS
   std::cout<<"**** LArG4H6BeamSD::Initialize  colname: "<<collectionName[0]<<std::endl;
#endif
  if (!m_FrontCollection.isValid()) m_FrontCollection = std::make_unique<LArG4H6FrontHitCollection>();
}

G4bool LArG4H6BeamSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*Touchable*/)
{
  LArG4H6FrontHit* theFrontHit = nullptr;

  G4double edep  = aStep->GetTotalEnergyDeposit() * aStep->GetTrack()->GetWeight();

 if(edep == 0) return true;

  G4StepPoint* preStep = aStep->GetPreStepPoint();
  const G4TouchableHistory* theTouchable = dynamic_cast<const G4TouchableHistory*>(preStep->GetTouchable());
  G4ThreeVector xyz = preStep->GetPosition();
  G4VPhysicalVolume* physVol1 = theTouchable->GetVolume();
//
  std::string ime1=physVol1->GetName();
//
  //dubina=theTouchable->GetHistoryDepth();
//
  G4String lname(ime1);
  G4int kk = lname.rfind(':');
  lname.erase(0,kk+1);
#ifdef DEBUG_HITS
  std::cout<<"**** LArG4H6BeamSD: "<<SensitiveDetectorName.data()<<" :ProcessHits: lname: "<<lname.data()<<std::endl;
#endif
//
   short num = physVol1->GetCopyNo();
   short h_num = -1;
   if((!strcmp(lname.data(),"XDiv")) || (!strcmp(lname.data(),"YDiv")))
    {
     h_num = theTouchable->GetVolume(2)->GetCopyNo();
    } else if ((!strcmp(lname.data(),"bpc_xdiv")) ||
               (!strcmp(lname.data(),"bpc_ydiv"))) {
     h_num = theTouchable->GetVolume(2)->GetCopyNo();
    } else if (!strcmp(lname.data(),"bpco_div")) {
      h_num = (theTouchable->GetVolume(2)->GetCopyNo()%10) + 7;
    } else if((!strcmp(lname.data(),"Scintillator"))  ) {
      if(num<=0 || num>=9) {
         std::cerr<<"Problems with hit in Beam chambers !! num:"<<num<<std::endl;
          return false;
      }
      theFrontHit = new LArG4H6FrontHit(num,edep);
    } else {
       std::cerr<<"Unknown volume: "<<lname.data()<<" in LArG4H6BeamSD::ProcessHits !!"<<std::endl;
      return false;
    }
    if(h_num != -1) {
      if((!strcmp(lname.data(),"xdiv"))
            || (!strcmp(lname.data(),"bpc_xdiv")))
         theFrontHit = new LArG4H6FrontHit(h_num,num,-1);
      else if (!strcmp(lname.data(),"bpco_div")) {
         switch (h_num) {
            case 7: case 9: {theFrontHit = new LArG4H6FrontHit(h_num,num,-1); break;}
            case 8: case 10: {theFrontHit = new LArG4H6FrontHit(h_num,-1,num); break;}
            default: {std::cerr<<"Problems with middle stations SD !! "<<h_num<<" "<<num<<std::endl;break;}
         }
      } else theFrontHit = new LArG4H6FrontHit(h_num,-1,num);
    }
#ifdef DEBUG_HITS
        std::cout<<"**** LArG4H6BeamSD::ProcessHits:  num: "<<num<<" edep: "<<edep<<std::endl;
        std::cout<<"**** LArG4H6BeamSD::ProcessHits: h_num: "<<h_num<<std::endl;
#endif
    theFrontHit->SetEdep(edep);
    theFrontHit->SetPos(xyz);
    theFrontHit->SetTrackID(aStep->GetTrack()->GetTrackID());
    theFrontHit->SetPcode(aStep->GetTrack()->GetDefinition()->GetPDGEncoding());
#ifdef DEBUG_HITS
    std::cout<<"**** LArG4H6BeamSD::ProcessHits:m_FrontCollection: "<<m_FrontCollection<<std::endl;
#endif
    //m_FrontCollection->insert(theFrontHit);
    m_FrontCollection->push_back(theFrontHit);
   return true;
}
