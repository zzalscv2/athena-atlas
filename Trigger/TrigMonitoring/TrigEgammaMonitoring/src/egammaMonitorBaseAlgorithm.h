/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef egammaMonitorBaseAlgorithm_h 
#define egammaMonitorBaseAlgorithm_h 

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "egammaRecEvent/egammaRec.h"
#include "egammaRecEvent/egammaRecContainer.h"


class egammaMonitorBaseAlgorithm : public AthReentrantAlgorithm  {
  public:
    
    egammaMonitorBaseAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
        
    virtual ~egammaMonitorBaseAlgorithm();
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& /*ctx*/) const override {return StatusCode::SUCCESS;};



  protected:

  /*! C Macros for plotting */
    #define GETTER(_name_) float getShowerShape_##_name_(const xAOD::Egamma* eg) const;
      GETTER(e011)
      GETTER(e132)
      GETTER(e237)
      GETTER(e277)
      GETTER(ethad)
      GETTER(ethad1)
      GETTER(weta1)
      GETTER(weta2)
      GETTER(f1)
      GETTER(e2tsts1)
      GETTER(emins1)
      GETTER(emaxs1)
      GETTER(wtots1)
      GETTER(fracs1)
      GETTER(Reta)
      GETTER(Rphi)
      GETTER(f3)
      GETTER(f3core)
      GETTER(Eratio)
      GETTER(Rhad)
      GETTER(Rhad1)
      GETTER(DeltaE)
#undef GETTER


      // GETTER for Isolation monitoring
#define GETTER(_name_) float getIsolation_##_name_(const xAOD::Egamma* eg) const;
      GETTER(ptcone20)
      GETTER(ptcone30)
      GETTER(ptcone40)
      GETTER(ptvarcone20)
      GETTER(ptvarcone30)
      GETTER(ptvarcone40)
#undef GETTER
#define GETTER(_name_) float getIsolation_##_name_(const xAOD::Egamma* eg) const;
      GETTER(etcone20)
      GETTER(etcone30)
      GETTER(etcone40)
      GETTER(topoetcone20)
      GETTER(topoetcone30)
      GETTER(topoetcone40)
#undef GETTER
      // GETTERs for CaloCluster monitoring
#define GETTER(_name_) float getCluster_##_name_(const xAOD::Egamma* eg) const;
      GETTER(et)
      GETTER(phi)
      GETTER(eta)
#undef GETTER

      // GETTERs for Track monitoring
#define GETTER(_name_) float getTrack_##_name_(const xAOD::Electron* eg) const;
      GETTER(pt)
      GETTER(phi)
      GETTER(eta)
      GETTER(d0)
      GETTER(z0)
#undef GETTER


      // GETTERs for Calo-Track monitoring
#define GETTER(_name_) float getCaloTrackMatch_##_name_(const xAOD::Electron* eg) const;
      GETTER(deltaEta0)
      GETTER(deltaPhi0)
      GETTER(deltaPhiRescaled0)
      GETTER(deltaEta1)
      GETTER(deltaPhi1)
      GETTER(deltaPhiRescaled1)
      GETTER(deltaEta2)
      GETTER(deltaPhi2)
      GETTER(deltaPhiRescaled2)
      GETTER(deltaEta3)
      GETTER(deltaPhi3)
      GETTER(deltaPhiRescaled3)
      GETTER(deltaPhiFromLastMeasurement)
#undef GETTER


};
#endif

