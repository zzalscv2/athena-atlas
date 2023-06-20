/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HIEVENTUTILS_IHITOWERWEIGHTTOOL_H
#define HIEVENTUTILS_IHITOWERWEIGHTTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

class IHITowerWeightTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IHITowerWeightTool, 1, 0);

    virtual float getEtaPhiResponse(float eta, float phi, int runIndex) const = 0;
    virtual float getEtaPhiOffset(float eta, float phi, int runIndex) const = 0;
    virtual float getWeight(float eta, float phi, int sampling) const = 0;
    virtual float getWeightEta(float eta, float phi, int sampling) const = 0;
    virtual float getWeightPhi(float eta, float phi, int sampling) const = 0;
    virtual float getWeightMag(float eta, float phi, int sampling) const = 0;
    virtual int getRunIndex(const EventContext& ctx) const = 0;
};
#endif
