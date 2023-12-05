#ifndef MUONG4R4_MmSensitiveDetector_H
#define MUONG4R4_MmSensitiveDetector_H
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/** @class MmSensitiveDetector
    @section MmSensitiveDetector Class methods and properties

The method MmSensitiveDetector::ProcessHits is executed by the G4 kernel each
time a particle crosses one of the Mm gas gaps.
Navigating with the touchableHistory method GetHistoryDepth()
through the hierarchy of volumes crossed by the particle,
the Sensitive Detector determines the correct set of Simulation Identifiers
to associate to each hit. The Mm SimIDs are 32-bit unsigned integers, built 
using the MuonSimEvent/MmHitIdHelper class
which inherits from the MuonHitIdHelper base class. 


*/

#include "G4VSensitiveDetector.hh"

#include <StoreGate/WriteHandle.h>
#include <AthenaBaseComps/AthMessaging.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonReadoutGeometryR4/MmReadoutElement.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>

namespace MuonG4R4 {

class MmSensitiveDetector : public G4VSensitiveDetector, public AthMessaging {


public:
     /** construction/destruction */
    MmSensitiveDetector(const std::string& name, const std::string& output_key,
                         const MuonGMR4::MuonDetectorManager* detMgr);
    ~MmSensitiveDetector()=default;
    
    /** member functions */
    void   Initialize(G4HCofThisEvent* HCE) override final;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;

    
private:
    /// Retrieves the matching readout element to a G4 hit
    const MuonGMR4::MmReadoutElement* getReadoutElement(const G4TouchableHistory* touchHist) const;
    Identifier getIdentifier(const MuonGMR4::MmReadoutElement* readOutEle, const Amg::Vector3D& hitAtGapPlane) const;
    /* For the moment use write handles because the sensitive detectors are 
     *  managed by a service which must not have a data dependency
    */
    SG::WriteHandle<xAOD::MuonSimHitContainer> m_writeHandle;
    /// Pointer to the acts Geometry context
    const ActsGeometryContext m_gctx{};
    /// Pointer to the underlying detector manager
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
   
};

}
#endif
