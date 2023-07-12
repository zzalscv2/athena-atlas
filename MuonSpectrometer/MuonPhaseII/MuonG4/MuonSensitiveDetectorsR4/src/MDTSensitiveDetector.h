#ifndef MUONG4R4_MDTSENSITIVEDETECTOR_H
#define MUONG4R4_MDTSENSITIVEDETECTOR_H
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
    @section MDTSensitiveDetector Class methods and properties
 **   
The method MDTSensitiveDetector::ProcessHits is executed by the G4 kernel each
time a charged particle (or a geantino) crosses one of the MDT Sensitive Gas
volumes.

Once a G4Step is perfomed by the particle in the sensitive volume (both when
the particle leaves the tube or stops in it), Pre and
PostStepPositions are tranformed into local coordinates (chamber reference
system, with Z along the tube direction and XY tranversal plane) and used to
calculate the local direction of the track. 

Two cases are given:
1) the particle over-passed the wire: here the drift radius (the impact 
   parameter) is computed as the local direction distance from the wire;  
2) the step doesn't allow the particle to over-pass the wire: here the shortest 
   distance to the wire, which is one of the two
   end-points, is calculated for each step which occurs inside the sensitive 
   volumes and only the closer one is kept; this case includes also the hard
   scattering case inside the sensitive volume.

Navigating with the touchableHistory method GetHistoryDepth()
through the hierarchy of the volumes crossed by the particles, the 
MDTSensitiveDetector determinates the
correct set of geometry parameters to be folded in the Simulation Identifier
associated to each hit.

We describe in the following, how each field of the identifier is retrieved.

1) stationName, stationEta, stationPhi: when a volume is found in the hierarchy,
   whose name contains the substring "station", the stationName is extracted from
   the volume's name; stationPhi and stationEta are calculated starting from the
   volume copy number, assigned by MuonGeoModel.

2) multilayer: when a volume is found in the hierarchy,
   whose name contains the substring "component", the multilayer is set to 1 or to
   2, according to the component number (multilayer=1 if the component is 1, 5 or
   8; multilayer=2 if the component is 3, 7, 8, 10, 11 or 14).

3) tubeLayer and tube: when a volume is found in the hierarchy,
   whose name contains the substring "Drift", tubeLayer and tube are calculated 
   starting from the Drift volume copy number.

    @section Some notes:

1) this implementation of the MDT Sensitive Detectors can handle different
   situations: hard scattering of the muon on the sensitive volume (its direction
   changes), soft secondary particles completely included in the sensitive volume,
   muon hits masked by secondaries, like delta rays.

2) for each hit, the time of flight (the G4 globalTime), is recorded and
   associated to the hit.

3) more than none MDT hit can occur in the same tube. The hit selection is done
   at the level of the digitization procedure.

4) the MDTHit object contains: the SimID, the drift radius and the globalTime.


*/

#include <GeoPrimitives/GeoPrimitives.h>

#include <StoreGate/WriteHandle.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <AthenaBaseComps/AthMessaging.h>

#include <G4VSensitiveDetector.hh>



class G4TouchableHistory;


namespace MuonG4R4 {


class MDTSensitiveDetector : public G4VSensitiveDetector, public AthMessaging {

public:
    /** construction/destruction */
    MDTSensitiveDetector(const std::string& name, const std::string& output_key,
                         const MuonGMR4::MuonDetectorManager* detMgr);
    ~MDTSensitiveDetector()=default;
    
    /** member functions */
    void   Initialize(G4HCofThisEvent* HCE) override final;
    G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override final;
    
private:
   Identifier getIdentifier(const G4TouchableHistory* touchHist);
    /* For the moment use write handles because the sensitive detectors are 
     *  managed by a service which must not have a data dependency
    */
    SG::WriteHandle<xAOD::MuonSimHitContainer> m_writeHandle;
    /// Pointer to the underlying detector manager
    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    
    double m_driftR{std::numeric_limits<double>::max()};
    double m_globalTime{0.};
    Amg::Vector3D m_locPos{Amg::Vector3D::Zero()};
    Amg::Vector3D m_locDir{Amg::Vector3D::Zero()};

   

};

}
#endif
