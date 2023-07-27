/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include <xAODCore/AuxStoreAccessorMacros.h>

#include <xAODMuonSimHit/versions/MuonSimHit_v1.h>

namespace {
   static const SG::AuxElement::Accessor<Identifier::value_type> acc_Identifier{"identifier"};
   static const xAOD::PosAccessor<3> acc_localPos{"localPositionDim3"};
   static const xAOD::PosAccessor<3> acc_localDir{"localDirectionDim3"};
}
namespace xAOD {

Identifier MuonSimHit_v1::identify() const { return Identifier{acc_Identifier(*this)}; }    
void MuonSimHit_v1::setIdentifier(const Identifier& id) { acc_Identifier(*this) = id.get_compact(); }

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, globalTime, setGlobalTime)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, int, pdgId, setPdgId)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, energyDeposit, setEnergyDeposit)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, kineticEnergy, setKineticEnergy)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, stepLength, setStepLength)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, ElementLink<McEventCollection>, genParticleLink, setGenParticleLink)



void MuonSimHit_v1::setLocalPosition(MeasVector<3> vec) {
   VectorMap<3> lPos{acc_localPos(*this).data()};
   lPos = vec;   
}
ConstVectorMap<3> MuonSimHit_v1::localPosition() const { return ConstVectorMap<3>{acc_localPos(*this).data()};}

void MuonSimHit_v1::setLocalDirection(MeasVector<3> vec) {
   VectorMap<3> lPos{acc_localDir(*this).data()};
   lPos = vec;   
}
ConstVectorMap<3> MuonSimHit_v1::localDirection() const { return ConstVectorMap<3>{acc_localDir(*this).data()};}

}