/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef AFP_CALIBRATION_IAFP_GENERICPIXELTOOL_H
#define AFP_CALIBRATION_IAFP_GENERICPIXELTOOL_H

#include <vector>
#include <memory>

#include "TH2F.h"

/**
 * @class IAFP_GenericPixelTool
 * @brief Base class for all pixel identifier tool
 **/
 
class IAFP_GenericPixelTool{
public:
	virtual int Identify(std::shared_ptr<const TH2F> input, std::vector<TH2F>& output) const = 0;
}; 

#endif // AFP_CALIBRATION_IAFP_GENERICPIXELTOOL_H
