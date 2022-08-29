/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCoudeElectrodes.h"
#include "PhysicalVolumeAccessor.h"


const LArCoudeElectrodes* LArCoudeElectrodes::GetInstance(const std::string& strDetector)
{
  static const LArCoudeElectrodes instance(strDetector);
  return &instance;
}


LArCoudeElectrodes::LArCoudeElectrodes(const std::string& strDetector)
{
  static const PhysicalVolumeAccessor theCoudes = [&]() {
    const std::string prefix = strDetector.empty() ? "" : strDetector+"::";
    PhysicalVolumeAccessor pva(prefix+"LAr::EMB::STAC",
                               prefix+"LAr::EMB::Electrode::CornerDownFold");
    pva.SetPhysicalVolumeList(prefix+"LAr::EMB::Electrode::CornerUpFold");
    return pva;
  }();

  for (int stackid=0; stackid<15; stackid++) {
    for (int cellid=0; cellid<1024; cellid++) {
      const int id=cellid+stackid*10000;
      const G4VPhysicalVolume *pv=theCoudes.GetPhysicalVolume(id);
      if (pv) {
        const G4ThreeVector& tv=pv->GetTranslation();
        m_xcent[cellid][stackid] = tv.x();
        m_ycent[cellid][stackid] = tv.y();

        // Calculate phirot
        const G4RotationMatrix *rm=pv->GetRotation();
        double alpha;
        if (!rm) alpha=0.;
        else alpha = rm->phiX();
        // for down fold
        if (pv->GetName().find("DownFold") != std::string::npos) alpha=alpha-3.14159;
        // old way was assuming we start with a down fold if (stackid%2==0) alpha=alpha-3.14159;
        m_phirot[cellid][stackid] = alpha;
      }
    }
  }
}
