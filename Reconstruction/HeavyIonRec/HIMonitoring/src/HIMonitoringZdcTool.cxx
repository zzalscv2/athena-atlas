/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */


#include "AthenaMonitoring/AthenaMonManager.h"
#include "HIMonitoringZdcTool.h"
#include <xAODForward/ZdcModule.h>
#include <xAODForward/ZdcModuleContainer.h>

#include "TStyle.h"

#include "LWHists/TH2D_LW.h"

HIMonitoringZdcTool::
 HIMonitoringZdcTool(const std::string& type, const std::string& name,
                     const IInterface* parent) : ManagedMonitorToolBase(type, name, parent) {
}

HIMonitoringZdcTool::~HIMonitoringZdcTool() {
}

// Description: Used for rebooking unmanaged histograms
StatusCode HIMonitoringZdcTool::bookHistogramsRecurrent( ) {
  return StatusCode::SUCCESS;
}

// Description: Used for re-booking managed histograms
StatusCode HIMonitoringZdcTool::bookHistograms( ) {
  if (m_environment == AthenaMonManager::online) {
    // book histograms that are only made in the online environment...
  }

  if (m_dataType == AthenaMonManager::cosmics) {
    // book histograms that are only relevant for cosmics data...
  }

  book_hist();

  return StatusCode::SUCCESS;
}

StatusCode HIMonitoringZdcTool::fillHistograms() {
  StatusCode sc;

  const xAOD::ZdcModuleContainer* zdcs = 0;

  sc = evtStore()->retrieve(zdcs, "ZdcModules");
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Could not find Zdc");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO("Zdcs retrieved from StoreGate");
//		std::cout << "Number of zdcs " << zdcs->size() << std::endl;
  }

  xAOD::ZdcModuleContainer::const_iterator zdc_itr = zdcs->begin();
  xAOD::ZdcModuleContainer::const_iterator zdc_end = zdcs->end();

  for (; zdc_itr != zdc_end; ++zdc_itr) {
    // removed since using unused amplitudes
  }

  return StatusCode::SUCCESS;
}

StatusCode HIMonitoringZdcTool::procHistograms( ) 
{
  //functionality removed
  return StatusCode::SUCCESS;
}

/// *** FCal sum Et *** ///
void HIMonitoringZdcTool::book_hist() {
  std::string path = "HeavyIon/ZDC";

  std::stringstream nameEM_HAD1;
  std::stringstream nameHAD1_HAD2;
  std::stringstream nameHAD2_HAD3;
  std::stringstream histnameamp;
  std::stringstream histnameampG0;
  std::stringstream histnameampG1;
  std::stringstream histnameg0d0;
  std::stringstream histnameg1d0;
  std::stringstream histnameg0d1;
  std::stringstream histnameg1d1;
  std::stringstream nameSumSideAmp;
  std::stringstream nameSumSideAmpG1;
  std::stringstream nameSumSideAmpG0;
  std::stringstream nameSideAC;

  nameSideAC.str("");
  nameSideAC << "hSideA_sideC";
  m_hSideAC = TH2D_LW::create(nameSideAC.str().c_str(), nameSideAC.str().c_str(), 4097, -5, 40965, 4097, -5, 40965);
  regHist(m_hSideAC, path, run).ignore();
/*
    nameSideAC.str("");
    nameSideAC<<"h_NEW_SideA_sideC";
    m_hSideAC_NEW = TH2D_LW::create(nameSideAC.str().c_str(),
       nameSideAC.str().c_str(),10241,-0.5,10240.5,10241,-0.5,10240.5);
    regHist(m_hSideAC_NEW, path, run).ignore();
 */
  for (int k = 0; k < s_Nside; k++) {
    nameEM_HAD1.str("");
    nameEM_HAD1 << "hEM_HAD1_side" << k;
    m_hEM_HAD1[k] = TH2D_LW::create(nameEM_HAD1.str().c_str(),
                                    nameEM_HAD1.str().c_str(), 1025, -5, 10245, 1025, -5, 10245);
    regHist(m_hEM_HAD1[k], path, run).ignore();
/*
        nameEM_HAD1.str("");
        nameEM_HAD1<<"h_NEW_EM_HAD1_side"<<k;
        m_hEM_HAD1_NEW[k] =
           TH2D_LW::create(nameEM_HAD1.str().c_str(),nameEM_HAD1.str().c_str(),10241,-0.5,10240.5,10241,-0.5,10240.5);
        regHist(m_hEM_HAD1_NEW[k], path, run).ignore();
 */
    nameHAD1_HAD2.str("");
    nameHAD1_HAD2 << "hHAD1_HAD2_side" << k;
    m_hHAD1_HAD2[k] = TH2D_LW::create(nameHAD1_HAD2.str().c_str(),
                                      nameHAD1_HAD2.str().c_str(), 1025, -5, 10245, 1025, -5, 10245);
    regHist(m_hHAD1_HAD2[k], path, run).ignore();
/*
        nameHAD1_HAD2.str("");
        nameHAD1_HAD2<<"h_NEW_HAD1_HAD2_side"<<k;
        m_hHAD1_HAD2_NEW[k] =
           TH2D_LW::create(nameHAD1_HAD2.str().c_str(),nameHAD1_HAD2.str().c_str(),10241,-0.5,10240.5,10241,-0.5,10240.5);
        regHist(m_hHAD1_HAD2_NEW[k], path, run).ignore();
 */
    nameHAD2_HAD3.str("");
    nameHAD2_HAD3 << "hHAD2_HAD3_side" << k;
    m_hHAD2_HAD3[k] = TH2D_LW::create(nameHAD2_HAD3.str().c_str(),
                                      nameHAD2_HAD3.str().c_str(), 1025, -5, 10245, 1025, -5, 10245);
    regHist(m_hHAD2_HAD3[k], path, run).ignore();
/*
        nameHAD2_HAD3.str("");
        nameHAD2_HAD3<<"h_NEW_HAD2_HAD3_side"<<k;
        m_hHAD2_HAD3_NEW[k] =
           TH2D_LW::create(nameHAD2_HAD3.str().c_str(),nameHAD2_HAD3.str().c_str(),10241,-0.5,10240.5,10241,-0.5,10240.5);
        regHist(m_hHAD2_HAD3_NEW[k], path, run).ignore();
 */
    nameSumSideAmp.str("");
    nameSumSideAmp << "hSumSideAmp_side" << k;
    m_hSumSideAmp[k] = new TH1D(nameSumSideAmp.str().c_str(), nameSumSideAmp.str().c_str(), 4097, -5, 40965);
    regHist(m_hSumSideAmp[k], path, run).ignore();
/*
        nameSumSideAmp.str("");
        nameSumSideAmp<<"h_NEW_SumSideAmp_side"<<k;
        hSumSideAmp_NEW[k] = new TH1D(nameSumSideAmp.str().c_str(), nameSumSideAmp.str().c_str(),40961,-0.5,40960.5);
        regHist(hSumSideAmp_NEW[k], path, run).ignore();
 */
    nameSumSideAmpG0.str("");
    nameSumSideAmpG0 << "hSumSideAmpG0_side" << k;
    m_hSumSideAmpG0[k] = new TH1D(nameSumSideAmpG0.str().c_str(), nameSumSideAmpG0.str().c_str(), 4097, -0.5, 4096.5);
    regHist(m_hSumSideAmpG0[k], path, run).ignore();

    nameSumSideAmpG1.str("");
    nameSumSideAmpG1 << "hSumSideAmpG1_side" << k;
    m_hSumSideAmpG1[k] = new TH1D(nameSumSideAmpG1.str().c_str(), nameSumSideAmpG1.str().c_str(), 4097, -0.5, 4096.5);
    regHist(m_hSumSideAmpG1[k], path, run).ignore();

    for (int i = 0; i < s_Nmod; i++) {
      histnameamp.str("");
      histnameamp << "h_amplitude_mod" << i << "_side" << k;
      m_hamp[i][k] = new TH1D(histnameamp.str().c_str(), histnameamp.str().c_str(), 1025, -5, 10245);
      regHist(m_hamp[i][k], path, run).ignore();
      /*
              histnameamp.str("");
              histnameamp<<"h_NEW_amplitude_mod"<<i<<"_side"<<k;
              m_hamp_NEW[i][k] = new TH1D(histnameamp.str().c_str(), histnameamp.str().c_str(), 10241,-0.5,10240.5);
              regHist(m_hamp_NEW[i][k], path, run).ignore();
       */
      histnameampG0.str("");
      histnameampG0 << "h_amplitudeG0_mod" << i << "_side" << k;
      m_hampG0[i][k] = new TH1D(histnameampG0.str().c_str(), histnameampG0.str().c_str(), 1025, -0.5, 1024.5);
      regHist(m_hampG0[i][k], path, run).ignore();

      histnameampG1.str("");
      histnameampG1 << "h_amplitudeG1_mod" << i << "_side" << k;
      m_hampG1[i][k] = new TH1D(histnameampG1.str().c_str(), histnameampG1.str().c_str(), 1025, -0.5, 1024.5);
      regHist(m_hampG1[i][k], path, run).ignore();
    }
  }
}
