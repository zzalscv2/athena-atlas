/*
	Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_CALIBRATION_AFP_DEADPIXELTOOL_H
#define AFP_CALIBRATION_AFP_DEADPIXELTOOL_H

// Package includes
#include "AFP_Calibration/IAFP_GenericPixelTool.h"

// ROOT includes
#include "TH2F.h"

// STL includes
#include <string>
#include <vector>
#include <memory>
#include <utility>
 
class AFP_DeadPixelTool : public IAFP_GenericPixelTool {
public:
	AFP_DeadPixelTool() : m_range(0.0) {}
	
	int Identify(std::shared_ptr<const TH2F> input, std::vector<TH2F>& output) const override;

	void setRange(float r) {m_range=r;}
	float getRange() const {return m_range;}

private:
	
	float m_range; // To prevent pixels be identified as dead when there is low statistics
	
	double getNeighbours(std::shared_ptr<const TH2F> input, int row_ID, int col_ID) const;
	std::vector<std::pair<int,int>> getLegitPixels(std::shared_ptr<const TH2F> input, const int col_ID, const int row_ID) const;
};

#endif // AFP_CALIBRATION_AFP_DEADPIXELTOOL_H
