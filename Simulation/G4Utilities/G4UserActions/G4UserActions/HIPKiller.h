/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIPKiller_H
#define HIPKiller_H

#include "FadsActions/UserAction.h"
#include <string>

class HIPKiller: public FADS::UserAction {

public:
	HIPKiller(std::string s): UserAction(s) {}
	void BeginOfEventAction(const G4Event*);
        void EndOfEventAction(const G4Event*);
        void BeginOfRunAction(const G4Run*);
        void EndOfRunAction(const G4Run*);
        void SteppingAction(const G4Step*);

};


#endif
