/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef polyplaneHandler_H
#define polyplaneHandler_H

#include "AGDDControl/XMLHandler.h"
#include "AGDDHandlers/Polyplane.h"
#include <string>

class polyplaneHandler:public XMLHandler {
public:
	polyplaneHandler(const std::string&,
                         AGDDController& c);
	virtual void ElementHandle(AGDDController& c,
                                   xercesc::DOMNode *t) override;
	static Polyplane CurrentPolyplane() {return s_pPlane;}
private:
	static Polyplane s_pPlane;
};

#endif
