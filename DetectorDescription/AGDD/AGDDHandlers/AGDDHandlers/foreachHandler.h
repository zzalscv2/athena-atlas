/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef foreachHandler_H
#define foreachHandler_H

#include "AGDDControl/XMLHandler.h"
#include <string>

class foreachHandler:public XMLHandler {
public:
	foreachHandler(const std::string&,
                       AGDDController& c);
	virtual void ElementHandle(AGDDController& c,
                                   xercesc::DOMNode *t) override;
};

#endif
