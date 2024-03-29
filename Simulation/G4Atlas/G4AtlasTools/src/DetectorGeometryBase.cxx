/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// Base class
#include "G4AtlasTools/DetectorGeometryBase.h"

// Geant4 includes used in functions
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4LogicalVolumeStore.hh"

DetectorGeometryBase::DetectorGeometryBase(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type,name,parent)
{
}

// Athena method, called at initialization time
StatusCode DetectorGeometryBase::initialize()
{
  ATH_MSG_VERBOSE( name() << "::initialize(): starting." );
  if(m_detectorName.empty())
    {
      m_detectorName = this->name();
      // re-initialize m_detectorName in order to take the real detector name rather than the path to it
      size_t ipos=m_detectorName.value().find_last_of('.');
      size_t length=m_detectorName.value().size();
      if (ipos<length)
        {
          ATH_MSG_VERBOSE( "m_detectorName: " << m_detectorName.value() << " needs to be reset.");
          m_detectorName=m_detectorName.value().substr(ipos+1,length-ipos-1);
          ATH_MSG_VERBOSE( "m_detectorName default value reset to " << m_detectorName.value());
        }
    }
  ATH_MSG_DEBUG( name() << "::initialize() (Base class method): Detector name = " << m_detectorName.value() );
  CHECK(m_notifierSvc.retrieve());

  //  This fires initialize() for each of those tools
  if (m_subDetTools.size())
    {
      ATH_MSG_DEBUG( name() << "::initialize(): Initializing list of " << m_subDetTools.size() << " detectors." );
      CHECK( m_subDetTools.retrieve() );
    }
  else
    {
      ATH_MSG_DEBUG( name() << "::initialize(): no sub-detectors to initialize." );
    }

  ATH_MSG_VERBOSE( name() << "::initialize(): finished." );
  return StatusCode::SUCCESS;
}

void DetectorGeometryBase::Build()
{
  ATH_MSG_VERBOSE( name() << "::Build() (Base class method): Starting. Number of registered volumes "<<G4LogicalVolumeStore::GetInstance()->size() );
  
  SetEnvelope();
  ATH_MSG_VERBOSE( name() << "::Build() - Envelope set. Number of registered volumes "<<G4LogicalVolumeStore::GetInstance()->size() );
 
  m_notifierSvc->SetCurrentDetectorName(m_detectorName.value());
  BuildGeometry();
  ATH_MSG_VERBOSE( name() << "::Build() - Geometry built. Number of registered volumes "<<G4LogicalVolumeStore::GetInstance()->size() );
 
  SetRotationAndOffset();
  ATH_MSG_VERBOSE( name() << "::Build() - Volume moved around. Number of registered volumes "<<G4LogicalVolumeStore::GetInstance()->size() );
 
  PositionInParent();
  ATH_MSG_VERBOSE( name() << "::Build() - Connected with parent. Number of registered volumes "<<G4LogicalVolumeStore::GetInstance()->size() );
 
  BuildSubDetectors();
  ATH_MSG_VERBOSE( name() << "::Build() (Base class method): Finished. Number of registered volumes "<<G4LogicalVolumeStore::GetInstance()->size() );
  return;
}

void DetectorGeometryBase::BuildGeometry()
{
  ATH_MSG_VERBOSE( "DetectorGeometryBase::BuildGeometry(): Using base-class method. Anything going wrong?");
}

void DetectorGeometryBase::SetRotationAndOffset()
{
  ATH_MSG_VERBOSE( name() << "::SetRotationAndOffset() (Base class method)");
  // Firstly do the rotation
  if (!m_envelope.theRotation)
    {
      // m_envelope.theRotation is null, so create an identity
      // rotation first.
      // FIXME probably a neater way to do this part.
      m_envelope.theRotation=new G4RotationMatrix;
      // add the extra rotations.
      m_envelope.theRotation->rotateX(m_rotateX);
      m_envelope.theRotation->rotateY(m_rotateY);
      m_envelope.theRotation->rotateZ(m_rotateZ);
      if (m_envelope.thePositionedVolume)
        {
          // Override the rotation for m_envelope.thePositionedVolume.
          m_envelope.thePositionedVolume->SetRotation(m_envelope.theRotation);
        }
    }
  else
    {
      // m_envelope.theRotation already exists, so just add the
      // extra rotations.
      m_envelope.theRotation->rotateX(m_rotateX);
      m_envelope.theRotation->rotateY(m_rotateY);
      m_envelope.theRotation->rotateZ(m_rotateZ);
    }
  // Secondly add the additional position offset to the existing
  // m_envelope.thePosition vector.
  m_envelope.thePosition+=G4ThreeVector(m_offsetX,m_offsetY,m_offsetZ);
  if (m_envelope.thePositionedVolume)
    {
      // Override the translation for m_envelope.thePositionedVolume.
      m_envelope.thePositionedVolume->SetTranslation(m_envelope.thePosition);
    }

  ATH_MSG_VERBOSE( name() << "::SetRotationAndOffset() (Base class method): Finished" );
  return;
}
void DetectorGeometryBase::PositionInParent()
{
  ATH_MSG_VERBOSE( name() << "::PositionInParent() (Base class method)");
  if (m_isWorld)
    {
      // check that the detector is built
      if (m_envelope.IsBuilt())
        {
          G4VPhysicalVolume* physWorld= new G4PVPlacement(0,G4ThreeVector(),
                                                          m_envelope.theEnvelope,m_envelope.theEnvelope->GetName(),0,false,0,false);
          m_envelope.thePositionedVolume=physWorld;
        }
    }
  else
    {
      // check that there is a parent
      if (!m_theParent)
        {
          ATH_MSG_ERROR("Parent not set for "<<m_detectorName.value()<<"!!!!!!!!!!");
        }
      else
        {
          if (m_theParent->GetEnvelope().IsBuilt())
            {
              // G4VPhysicalVolume *physVol = new G4PVPlacement(0,G4ThreeVector(),
              //                                                m_envelope.theEnvelope,m_envelope.theEnvelope->GetName(),m_theParent->GetEnvelope().theEnvelope,false,0,false);
              // TODO: implement a possible rotation/displacement - something like this based on the old code?
              G4VPhysicalVolume *physVol = new G4PVPlacement(m_envelope.theRotation,m_envelope.thePosition,
                                                             m_envelope.theEnvelope,m_envelope.theEnvelope->GetName(),m_theParent->GetEnvelope().theEnvelope,false,0);
              m_envelope.thePositionedVolume=physVol;
            }
        }
    }
}

void DetectorGeometryBase::BuildSubDetectors()
{
  ATH_MSG_VERBOSE( name() << "::BuildSubDetectors() (Base class method): Starting");
  for (auto& subDetTool: m_subDetTools)
    {
      ATH_MSG_VERBOSE(name() << "::BuildSubDetectors() (Base class method):  Positioning "<<subDetTool->GetDetectorName()<<" within "<<m_detectorName.value());
      subDetTool->SetParent(this);
      subDetTool->Build();
    }
  ATH_MSG_VERBOSE( name() << "::BuildSubDetectors() (Base class method): Finished");
}

void DetectorGeometryBase::SetEnvelope()
{
}

void DetectorGeometryBase::ResetEnvelope()
{
}

void DetectorGeometryBase::SetDetectorName(const std::string& s)
{
  m_detectorName=s;
}

std::string DetectorGeometryBase::GetDetectorName() const
{
  return m_detectorName.value();
}

void DetectorGeometryBase::SetAsWorld()
{
  m_isWorld=true;
}

void DetectorGeometryBase::SetParent(IDetectorGeometryTool* p)
{
  m_theParent=p;
}

Envelope& DetectorGeometryBase::GetEnvelope()
{
  return m_envelope;
}

G4VPhysicalVolume* DetectorGeometryBase::GetWorldVolume()
{
  if (m_isWorld)
    return m_envelope.thePositionedVolume;
  else
    {
      ATH_MSG_ERROR("trying to get World from a DetectorTool which World is not!");
      return 0;
    }
}
