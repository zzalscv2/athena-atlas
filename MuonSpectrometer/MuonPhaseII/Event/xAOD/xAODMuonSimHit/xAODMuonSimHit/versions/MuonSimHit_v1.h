
/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONSIMHIT_VERSION_MUONSIMHIT_V1_H
#define XAODMUONSIMHIT_VERSION_MUONSIMHIT_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "AthLinks/ElementLink.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "GeneratorObjects/McEventCollection.h"
namespace xAOD {


class MuonSimHit_v1 : public SG::AuxElement {
    public:
    
    MuonSimHit_v1() = default;
    
    ///@brief Sets the local position of the traversing particle
    void setLocalPosition(MeasVector<3> vec);
    ///@brief Returns the local postion of the traversing particle
    ConstVectorMap<3> localPosition() const;

    ///@brief Sets the local direction of the traversing particle
    void setLocalDirection(MeasVector<3> vec);
    ///@brief Returns the local direction of the traversing particle
    ConstVectorMap<3> localDirection() const;

    ///@brief Returns the spatial length of the corresponding Geant4 simulation step
    float stepLength() const;
    ///@brief Sets the spatial length of the corresponding Geant4 simulation step
    void setStepLength(const float length);

    ///@brief Returns the time of the traversing particle
    float globalTime() const;
    ///@brief Sets the time of the traversing particle
    void setGlobalTime(const float time);

    ///@brief Returns the pdgID of the traversing particle
    int pdgId() const;
    ///@brief Sets the pdgID of the traversing particle
    void setPdgId(int id);

    ///@brief Returns the global ATLAS identifier of the SimHit
    Identifier identify() const;
    ///@brief Sets the global ATLAS identifier
    void setIdentifier(const Identifier& id);

    ///@brief Returns the energy deposited by the traversing particle inside the gas volume
    float energyDeposit() const;
    ///@brief Sets the energy deposited by the traversing particle inside the gas volume
    void setEnergyDeposit(const float deposit);

    ///@brief Returns the kinetic energy of the traversing particle
    float kineticEnergy() const;
    ///@brief Sets the kinetic energy of the traversing particle
    void setKineticEnergy(const float energy);
    
    ///@brief Returns the link to the HepMC particle producing this hit
    ElementLink<McEventCollection> genParticleLink() const;
    ///@brief Sets the link to the HepMC particle producing this hit
    void setGenParticleLink(const ElementLink<McEventCollection> link);
    
};
}
#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::MuonSimHit_v1, SG::AuxElement);
#endif