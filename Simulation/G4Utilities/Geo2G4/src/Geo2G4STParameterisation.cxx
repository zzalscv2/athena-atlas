/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "Geo2G4STParameterisation.h"
#include "G4AutoDelete.hh"
#include "G4VPhysicalVolume.hh"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "CLHEP/Geometry/Transform3D.h"

Geo2G4STParameterisation::Geo2G4STParameterisation(const GeoXF::Function* func,
                                                   unsigned int copies):
  m_function(func->clone()),
  m_nCopies(copies)
{
}

void Geo2G4STParameterisation::ComputeTransformation(const G4int copyNo,
                                                     G4VPhysicalVolume* physVol) const
{
  HepGeom::Transform3D transform = Amg::EigenTransformToCLHEP((*m_function)(copyNo));
  G4ThreeVector translation = transform.getTranslation();

  // keep thread-local rotation matrix (see discussion on atlas/athena!58732)
  static G4ThreadLocal G4RotationMatrix* rotation = nullptr;
  if (!rotation) {
    rotation = new G4RotationMatrix();
    G4AutoDelete::Register(rotation);
  }
  *rotation = transform.getRotation().inverse();

  physVol->SetTranslation(translation);
  physVol->SetRotation(rotation);
}
