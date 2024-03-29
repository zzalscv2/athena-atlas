/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4TOOLS_PHYSICSVALIDATIONUSERACTION_H
#define ISF_GEANT4TOOLS_PHYSICSVALIDATIONUSERACTION_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "ISF_Interfaces/IGeoIDSvc.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#ifndef MAXCHILDREN
#define MAXCHILDREN 40
#endif

#include <string>

// ROOT forward declarations
class TTree;

#include "G4UserEventAction.hh"
#include "G4UserRunAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserTrackingAction.hh"
#include "AthenaBaseComps/AthMessaging.h"

namespace G4UA{
  namespace iGeant4 {
    class PhysicsValidationUserAction: public G4UserEventAction, public G4UserRunAction, public G4UserSteppingAction, public G4UserTrackingAction, public AthMessaging
    {

    public:

      struct Config
      {
        MSG::Level verboseLevel=MSG::INFO;
        bool validationOutput = true;
        std::string validationStream="ISFG4SimKernel";
        ServiceHandle<ITHistSvc> thistSvc=ServiceHandle<ITHistSvc>("THistSvc", "PhysicsValidationUserAction");
        ServiceHandle<ISF::IGeoIDSvc>        geoIDSvc=
          ServiceHandle<ISF::IGeoIDSvc>("ISF::GeoIDSvc/ISF_GeoIDSvc", "PhysicsValidationUserAction");

        double idR=1150.-1.e-5;
        double idZ=3490.;
        double caloRmean=0.5*(40.+4250.);
        double caloZmean=0.5*(3490.+6740.);
        double muonRmean=0.5*(60.+30000.);
        double muonZmean=0.5*(6740.+30000.);
        double cavernRmean=300000.0;
        double cavernZmean=300000.0;

      };

      PhysicsValidationUserAction(const Config& config);
      virtual void BeginOfEventAction(const G4Event*) override final;
      virtual void EndOfEventAction(const G4Event*) override final;
      virtual void BeginOfRunAction(const G4Run*) override final;
      virtual void UserSteppingAction(const G4Step*) override final;
      virtual void PreUserTrackingAction(const G4Track*) override final;
    private:

      Config m_config;

      /** access to the central ISF GeoID serice*/
      ISF::IGeoIDSvc                      *m_geoIDSvcQuick; //!< quickaccess avoiding gaudi ovehead
      
      TTree                                                        *m_particles;    //!< ROOT tree containing track info
      int                                                           m_pdg;
      int                                                           m_scIn;
      int                                                           m_scEnd;
      int                                                           m_gen;
      int                                                           m_geoID;
      float                                                         m_pth;
      float                                                         m_pph;
      float                                                         m_p;
      float                                                         m_eloss;
      float                                                         m_radloss;
      float                                                         m_ionloss;
      float                                                         m_wzOaTr;
      float                                                         m_thIn;
      float                                                         m_phIn;
      float                                                         m_dIn;
      float                                                         m_thEnd;
      float                                                         m_phEnd;
      float                                                         m_dEnd;
      float                                                         m_X0;
      float                                                         m_L0;
      float                                                         m_wZ;
      float                                                         m_dt;
      
      TTree                                                        *m_interactions;   //!< ROOT tree containing vertex info
      int                                                           m_process;
      int                                                           m_pdg_mother;
      int                                                           m_gen_mother;
      int                                                           m_nChild;
      float                                                         m_vtx_dist;
      float                                                         m_vtx_theta;
      float                                                         m_vtx_phi;
      float                                                         m_vtx_e_diff;
      float                                                         m_vtx_p_diff;
      float                                                         m_vtx_plong_diff;
      float                                                         m_vtx_pperp_diff;
      float                                                         m_p_mother;
      float                                                         m_radLength;
      int                                                           m_pdg_child[MAXCHILDREN] ={};     // decay product pdg code
      float                                                         m_fp_child[MAXCHILDREN] ={};      // fraction of incoming momentum
      float                                                         m_oa_child[MAXCHILDREN] ={};      // opening angle wrt the mother
      
      
      int m_volumeOffset;
      int m_minHistoryDepth;
      
      int m_currentTrack;
      std::map<int, int> m_trackGenMap;
      
    }; // class PhysicsValidationUserAction

  } // namespace iGeant4

} // namespace G4UA

#endif // ISF_GEANT4TOOLS_PHYSICSVALIDATIONUSERACTION_H
