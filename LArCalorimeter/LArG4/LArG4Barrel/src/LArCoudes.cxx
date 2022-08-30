/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCoudes.h"


const PhysicalVolumeAccessor& LArCoudes::theCoudes(const std::string& strDetector)
{
 static const PhysicalVolumeAccessor pva = [&]() {
   const std::string prefix = strDetector.empty() ? "" : strDetector+"::";
   PhysicalVolumeAccessor pva(prefix+"LAr::EMB::STAC",
                              prefix+"LAr::EMB::Electrode::CornerDownFold");
   pva.SetPhysicalVolumeList(prefix+"LAr::EMB::Electrode::CornerUpFold");
   return pva;
 }();

 return pva;
}


LArCoudes::LArCoudes(const std::string& strDetector)
{
  // initialize singleton
  theCoudes(strDetector);
}

double LArCoudes::XCentCoude(int stackid, int cellid) const
{
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theCoudes().GetPhysicalVolume(id);
  if (!pv) std::abort();
  const G4ThreeVector& tv=pv->GetTranslation();
  return tv.x();
}
double LArCoudes::YCentCoude(int stackid, int cellid) const
{
  const int id=cellid+stackid*10000;
  const G4VPhysicalVolume *pv=theCoudes().GetPhysicalVolume(id);
  if (!pv) std::abort();
  const G4ThreeVector& tv=pv->GetTranslation();
  return tv.y();
}
