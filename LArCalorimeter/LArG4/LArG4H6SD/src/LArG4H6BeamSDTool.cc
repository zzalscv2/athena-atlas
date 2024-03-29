/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LArG4H6BeamSDTool.h"
#include "LArG4H6BeamSD.h"

#include "G4Version.hh"
#include "G4String.hh"

LArG4H6BeamSDTool::LArG4H6BeamSDTool(const std::string& type, const std::string& name, const IInterface *parent)
  : SensitiveDetectorBase(type,name,parent)
{
}

G4VSensitiveDetector* LArG4H6BeamSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  //FIXME better to use a configurable property here?

  // be smart, and take only last part of the name
#if G4VERSION_NUMBER < 1100
  G4String colname = G4String(name()).strip(G4String::both,':');
  if ( colname.contains(':') )
#else
  G4String colname = G4StrUtil::strip_copy(G4String(name()),':');
  if ( G4StrUtil::contains(colname, ':') )
#endif
  {
    G4int last = colname.rfind(':');
    colname.erase(0,last+1);
  }
  colname += "::Hits";
  return new LArG4H6BeamSD(name(),colname);
}
