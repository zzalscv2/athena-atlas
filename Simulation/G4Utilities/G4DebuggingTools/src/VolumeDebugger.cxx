/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "VolumeDebugger.h"

#include "G4GDMLParser.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4TransportationManager.hh"
#include "G4Navigator.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"

#include <iostream>
#include <vector>

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"


namespace
{
  std::once_flag VolumeDebugger_DumpGeometryOnce;
}

namespace G4UA
{

  VolumeDebugger::VolumeDebugger(const Config& config)
    : AthMessaging(Gaudi::svcLocator()->service< IMessageSvc >( "MessageSvc" ),
                   "VolumeDebugger"),
      m_config(config)
  {}

  // Pull the EMEC
  void VolumeDebugger::PullVolumes( G4LogicalVolume* v ) const
  {
    if (v==0) return;
    std::vector<G4VPhysicalVolume*> pv_to_remove;
    for (unsigned int i=0;i<v->GetNoDaughters();++i){
      G4VPhysicalVolume * n_v = v->GetDaughter(i);
      if ( n_v->GetName() == "LArMgr::LAr::EMEC::Pos::InnerWheel" ||
	   n_v->GetName() == "LArMgr::LAr::EMEC::Neg::InnerWheel" ||
	   n_v->GetName() == "LArMgr::LAr::EMEC::Pos::OuterWheel" ||
	   n_v->GetName() == "LArMgr::LAr::EMEC::Neg::OuterWheel" ){
	// This is one to remove
	pv_to_remove.push_back(n_v);
      } else {
	// Recurse
	PullVolumes( n_v->GetLogicalVolume() );
      }
    }
    for (unsigned int j=0;j<pv_to_remove.size();++j){
      v->RemoveDaughter( pv_to_remove[j] );
    }
  }

  void VolumeDebugger::DumpGeometry() const
  {
    G4VPhysicalVolume* W =
      G4TransportationManager::GetTransportationManager()->
      GetNavigatorForTracking()->GetWorldVolume();

   
    // Clear out the EMEC if necessary
    PullVolumes( W->GetLogicalVolume() );

    if(m_config.targetVolume!=""){
      G4LogicalVolumeStore *lvs = G4LogicalVolumeStore::GetInstance();
      for (unsigned int i=0;i<lvs->size();++i){
        for (unsigned int j=0;j<(*lvs)[i]->GetNoDaughters();++j){
          if ( (*lvs)[i]->GetDaughter(j)->GetName().c_str()==m_config.targetVolume ){
             //Create a World volume for the sub-tree that will be dumped in the GDML file
             //NB: without a world volume, the dumped geometry will not be properly defined
             //in Geant4 and in simulation this will lead to segfaults
             G4NistManager* nist = G4NistManager::Instance();
             G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
             G4VSolid* vBox = W->GetLogicalVolume()->GetSolid();
             G4LogicalVolume* logicWorld = new G4LogicalVolume(vBox,world_mat,"World");
             W  = new G4PVPlacement(0,G4ThreeVector(),logicWorld,"World",0,false,0,false);
             new G4PVPlacement(0,G4ThreeVector(),(*lvs)[i]->GetDaughter(j)->GetLogicalVolume(),m_config.targetVolume,logicWorld,false,0,false);
            // FIXME: Remove this goto!!!
            goto exitLoop;
          } // If we found the volume
        } // Loop over PVs in the LV
      } // Loop over volumes

      // Did not find the volume
      ATH_MSG_FATAL("Did not find the volume named " << m_config.targetVolume << ". Please set parameter TargetVolume to one of:\n\n");

      for (unsigned int i = 0; i < lvs->size(); ++i) {
	for (unsigned int j = 0; j < (*lvs)[i]->GetNoDaughters(); ++j) {
	  ATH_MSG_FATAL( (*lvs)[i]->GetDaughter(j)->GetName());
	} // Loop over PVs in the LV
      } // Loop over volumes
      ATH_MSG_FATAL("\n\n ================= E N D   O F   L I S T ========================\n\n");

      return; // Really no point in doing anything on the entire Atlas volume

    } // Requested a volume

  exitLoop:
    
    if (m_config.printGeo) {
        ATH_MSG_INFO("Dump of the Geant4 world "<<std::endl<<printVolume(W));
    }
    if (m_config.dumpGDML) {
      ATH_MSG_INFO( "Writing to GDML volume " << W->GetName() << " to path " << m_config.path );
      G4GDMLParser parser;
      parser.SetRegionExport(m_config.dumpPhysicsRegions);
      parser.Write(m_config.path, W, true);
    }
    if(m_config.volumeCheck){
      ATH_MSG_INFO( "Running overlap test with parameters " << m_config.res << " " << m_config.tol << " " << m_config.verbose );

      bool hasOverlaps = recursiveCheck(W);
      if (hasOverlaps) {
	ATH_MSG_INFO("Overlap check: there were problems with the geometry.");
      }
      else {
	ATH_MSG_INFO("Overlap check: All looks good!");
      }
    }

  }

  void VolumeDebugger::BeginOfRunAction(const G4Run*)
  {
    std::call_once(VolumeDebugger_DumpGeometryOnce,
                   &G4UA::VolumeDebugger::DumpGeometry, this);
  }

  bool VolumeDebugger::recursiveCheck(G4VPhysicalVolume *topPV) const
  {
    bool somethingOverlapped = false;
    bool hasOverlaps = topPV->CheckOverlaps(m_config.res, m_config.tol, m_config.verbose);
    if (hasOverlaps && m_config.verbose) ATH_MSG_ERROR("Volume " << topPV->GetName() << " has overlaps.");
    somethingOverlapped |= hasOverlaps;
    //
    //    Make a list of PVs keyed by their LVs
    //
    std::multimap<G4LogicalVolume *, G4VPhysicalVolume *> lv2pvMap;
    G4LogicalVolume *lv = topPV->GetLogicalVolume();
    unsigned int nDaughters = lv->GetNoDaughters();
    for (unsigned int i = 0; i < nDaughters; ++i) {
      G4VPhysicalVolume *daughterPV = lv->GetDaughter(i);
      G4LogicalVolume *daughterLV = daughterPV->GetLogicalVolume();
      lv2pvMap.insert(std::pair<G4LogicalVolume *, G4VPhysicalVolume *>(daughterLV, daughterPV));
    }

    for (std::multimap<G4LogicalVolume *, G4VPhysicalVolume *>::iterator mapEl = lv2pvMap.begin(); mapEl != lv2pvMap.end(); ) {
      //
      //  The first of each LV gets checked externally and internally (recursively).
      //
      G4VPhysicalVolume *daughterPV = mapEl->second;
      hasOverlaps = recursiveCheck(daughterPV);
      somethingOverlapped |= hasOverlaps;
      if (hasOverlaps && m_config.verbose) ATH_MSG_ERROR("Volume " << daughterPV->GetName() << " has overlaps.");
      //
      //  Subsequent PVs with the same LV get only external checks
      //
      std::pair <std::multimap<G4LogicalVolume *, G4VPhysicalVolume *>::iterator,
	std::multimap<G4LogicalVolume *, G4VPhysicalVolume *>::iterator> range = lv2pvMap.equal_range(mapEl->first);
      ++mapEl;
      //
      // Sometimes there are huge numbers of the same item, in the same juxtaposition with nearby elements.
      // Takes too long to check them all, and in typical geometries it is a waste of time.
      // So we skip some, controlled by m_targetMaxCopiesToCheck
      //
      int n = std::distance(range.first, range.second); // n is total number in this group.
      int checkEveryNth = int(n / m_config.targetMaxCopiesToCheck + 0.5);
      if (checkEveryNth <= 0) checkEveryNth = 1;
      for (int i = 1; i < n; ++i) { // "i = 0" already done
	if (i % checkEveryNth == 0) {
	  hasOverlaps = mapEl->second->CheckOverlaps(m_config.res, m_config.tol, m_config.verbose);
	  somethingOverlapped |= hasOverlaps;
	  if (hasOverlaps && m_config.verbose) ATH_MSG_ERROR("Volume " << mapEl->second->GetLogicalVolume()->GetName() << " has overlaps.");
	}
	++mapEl;
      }
    }
    return somethingOverlapped;
  }
  
  std::string VolumeDebugger::printVolume(const G4VPhysicalVolume *pv, const std::string& childDelim) const {
      std::stringstream sstr{};
      const G4ThreeVector trans = pv->GetFrameTranslation();
      const G4RotationMatrix* rot = pv->GetFrameRotation();
    
      sstr<<"Volume: "<<pv->GetName()<<", location: "<<Amg::toString(trans, 2)<<", ";
      if (rot) {
        sstr<<"orientation: {"<<Amg::toString(rot->colX(), 3)<<", ";
        sstr<<Amg::toString(rot->colY(), 3)<<", ";
        sstr<<Amg::toString(rot->colZ(), 3)<<"}";        
      }
      sstr<<std::endl;    
      G4LogicalVolume* log = pv->GetLogicalVolume();
      for (size_t d= 0; d <log->GetNoDaughters(); ++d){         
         sstr<<childDelim<<(d+1)<<": "<< printVolume(log->GetDaughter(d),
                                                     childDelim + "    ");
      }      
      return sstr.str();
   }


} // namespace G4UA
