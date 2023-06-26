/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_STRIP_SPACE_POINT_VALIDATION_PLOTS_H
#define ACTS_STRIP_SPACE_POINT_VALIDATION_PLOTS_H

#include "TrkValHistUtils/PlotBase.h" 
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "InDetIdentifier/SCT_ID.h"

#include <string>
#include <mutex>

namespace ActsTrk {

  class StripSpacePointValidationPlots :
    public PlotBase {
  public:
    StripSpacePointValidationPlots(PlotBase* pParent, const std::string& sDir,
				   const std::string& type = "Strip");
    virtual ~StripSpacePointValidationPlots() = default;

    void fill(const xAOD::SpacePoint* spacePoint,
	      float beamSpotWeight,
	      const SCT_ID*);
    
  private:
    std::string m_type;

  private:
    std::mutex m_mutex;

    TH1* m_barrelEndcap {};

    TH1* m_layerDisk_barrel {};
    TH1* m_layerDisk_endcap {};

    TH1* m_phiModule_barrel {};
    TH1* m_phiModule_endcap {};

    TH1* m_etaModule_barrel {};
    TH1* m_etaModule_endcap {};

    TH1* m_sideModule_barrel {};
    TH1* m_sideModule_endcap {};

    TH1* m_eta_barrel {};
    TH1* m_eta_endcap {};

    TH1* m_perp_barrel {};
    TH1* m_perp_endcap {};

    TH1* m_global_x_barrel {};
    TH1* m_global_x_endcap {};

    TH1* m_global_y_barrel {};
    TH1* m_global_y_endcap {};

    TH1* m_global_z_barrel {};
    TH1* m_global_z_endcap {};

    TH1* m_globalCovR_barrel {};
    TH1* m_globalCovR_endcap {};

    TH1* m_globalCovZ_barrel {};
    TH1* m_globalCovZ_endcap {};

    TH2* m_global_xy_barrel {};
    TH2* m_global_xy_endcap {};

    TH2* m_global_zr_barrel {};
    TH2* m_global_zr_endcap {};

    TH1* m_topHalfStripLength_barrel {};
    TH1* m_topHalfStripLength_endcap {};

    TH1* m_bottomHalfStripLength_barrel {};    
    TH1* m_bottomHalfStripLength_endcap {};

    TH1* m_topStripDirection_barrel {};
    TH1* m_topStripDirection_endcap {};

    TH1* m_bottomStripDirection_barrel {};
    TH1* m_bottomStripDirection_endcap {};

    TH1* m_stripCenterDistance_barrel {};
    TH1* m_stripCenterDistance_endcap {};

    TH1* m_topStripCenter_barrel {};
    TH1* m_topStripCenter_endcap {};
  };

}

#endif
