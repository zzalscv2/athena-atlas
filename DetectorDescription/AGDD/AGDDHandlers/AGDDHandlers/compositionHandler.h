/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef compositionHandler_H
#define compositionHandler_H

#include "AGDDControl/XMLHandler.h"
#include <string>

class compositionHandler:public XMLHandler {
public:
	compositionHandler(const std::string&,
                           AGDDController& c);
        virtual void ElementHandle(AGDDController& c,
                                   xercesc::DOMNode *t) override;
};

#endif
