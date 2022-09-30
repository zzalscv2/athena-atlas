/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_CALIBRATION_AFP_PIXELHISTOFILLER_H
#define AFP_CALIBRATION_AFP_PIXELHISTOFILLER_H

// Framework includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODForward/AFPSiHitContainer.h"
#include "xAODForward/AFPSiHit.h"
#include "AFP_Geometry/AFP_constants.h"

// STL includes
#include <string>
#include <vector>
#include <memory>

// ROOT includes
#include "TFile.h"
#include "TH2F.h"

/**
 * @class AFP_PixelHistoFiller
 **/

class AFP_PixelHistoFiller : public AthAlgorithm {
public:
  AFP_PixelHistoFiller(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~AFP_PixelHistoFiller() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:	
	static const int m_nStations=4;
	static const int m_nLayers=4;
	int m_nPixelsX;
	int m_nPixelsY;
	
	std::vector<TH2F> m_pixelHits[m_nStations][m_nLayers];
	
	Gaudi::Property<int> m_LBRangeLength{this, "LBRangeLength",5, "How many lumiblocks should be merged together to have reasonable statistics"};
	
	SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey { this, "EventInfoKey", "EventInfo", "name of EventInfo container" };
	SG::ReadHandleKey<xAOD::AFPSiHitContainer> m_afpHitContainerKey{ this ,"AFPHitContainerKey", "AFPSiHitContainer", "name of AFPSiHitContainer" };
};

#ifndef __CINT__
  CLASS_DEF( AFP_PixelHistoFiller , 161453345 , 1 )
#endif

#endif // AFP_CALIBRATION_AFP_PIXELHISTOFILLER_H
