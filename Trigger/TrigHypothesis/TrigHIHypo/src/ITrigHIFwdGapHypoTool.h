/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHIHYPO_ITRIGHIFWDGAPHYPOTOOL_H
#define TRIGHIHYPO_ITRIGHIFWDGAPHYPOTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "xAODHIEvent/HIEventShapeContainer.h"

class ITrigHIFwdGapHypoTool : virtual public::IAlgTool {

  public:

    DeclareInterfaceID(ITrigHIFwdGapHypoTool, 1, 0);

    virtual ~ITrigHIFwdGapHypoTool() {};

    virtual StatusCode decide(const xAOD::HIEventShapeContainer*, bool&) const = 0;
    virtual const HLT::Identifier& getId() const = 0;

};

#endif

