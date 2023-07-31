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

   static const SG::AuxElement::Accessor<unsigned short> acc_mcEventIndex{"mcEventIndex"};
   static const SG::AuxElement::Accessor<unsigned int> acc_mcBarcode{"mcBarcode"};
   static const SG::AuxElement::Accessor<char> acc_mcCollectionType{"mcCollectionType"};
}
namespace xAOD {

Identifier MuonSimHit_v1::identify() const { return Identifier{acc_Identifier(*this)}; }    
void MuonSimHit_v1::setIdentifier(const Identifier& id) { acc_Identifier(*this) = id.get_compact(); }

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, globalTime, setGlobalTime)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, int, pdgId, setPdgId)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, energyDeposit, setEnergyDeposit)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, kineticEnergy, setKineticEnergy)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonSimHit_v1, float, stepLength, setStepLength)

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
const HepMcParticleLink& MuonSimHit_v1::genParticleLink() const {
   if (!m_hepMCLink) {
      const unsigned short eventIndex = acc_mcEventIndex(*this);
      const HepMcParticleLink::PositionFlag flag =  eventIndex > 0 ? HepMcParticleLink::IS_INDEX :
                                                                     HepMcParticleLink::IS_POSITION;
      std::unique_ptr<HepMcParticleLink> link = std::make_unique<HepMcParticleLink>();
      
      auto collType = HepMcParticleLink::ExtendedBarCode::eventCollectionFromChar(acc_mcCollectionType(*this));
      HepMcParticleLink::ExtendedBarCode barcode {acc_mcBarcode(*this),
                                                  eventIndex,
                                                  collType,
                                                  flag};
      link->setExtendedBarCode(std::move(barcode));
      return *m_hepMCLink.set(std::move(link));
   }
   return (*m_hepMCLink);
}
void MuonSimHit_v1::setGenParticleLink(const HepMcParticleLink& link) {
   m_hepMCLink.release();
   acc_mcEventIndex(*this) = link.eventIndex();
   acc_mcBarcode(*this) = link.barcode();
   acc_mcCollectionType(*this) = link.getEventCollectionAsChar();
}

}
