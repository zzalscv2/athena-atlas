/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4TOOLS_G4RUNMANAGERHELPER_H
#define ISF_GEANT4TOOLS_G4RUNMANAGERHELPER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ISF_Geant4Tools/IG4RunManagerHelper.h"

#include "G4AtlasAlg/G4AtlasRunManager.h"

namespace iGeant4 {

  class G4RunManagerHelper: public extends<AthAlgTool, ISF::IG4RunManagerHelper> {

  public:
    G4RunManagerHelper(const std::string& type,
                       const std::string& name,
                       const IInterface* parent);
    virtual ~G4RunManagerHelper();

    virtual StatusCode initialize() override;

    virtual G4AtlasRunManager* g4RunManager ATLAS_NOT_THREAD_SAFE () override;
    virtual G4RunManager*      fastG4RunManager ATLAS_NOT_THREAD_SAFE () override;

  private:

    G4AtlasRunManager* m_g4RunManager;
    G4RunManager*      m_fastG4RunManager;

  };

}

#endif // ISF_GEANT4TOOLS_G4RUNMANAGERHELPER_H
