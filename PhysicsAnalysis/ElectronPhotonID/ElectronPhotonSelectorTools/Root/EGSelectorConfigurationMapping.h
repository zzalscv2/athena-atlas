/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __EGSelectorConfigurationMapping__
#define __EGSelectorConfigurationMapping__
////////////////////////////////////////////////////////////////
#include "ElectronPhotonSelectorTools/egammaPIDdefs.h"
#include <map>
#include <string>

namespace EgammaSelectors {
// These should be the current best-supported ID menus. Typically picked up by
// analyzers using the "WorkingPoint" option.
// Note: keep this conf file up to date with the PhotonIsEMTightSelectorConfig
// function in python/PhotonIsEMTightSelectorCutDefs.py

// This is the internal part , We need to  map string to latest recommendations
const std::map<std::string, std::string> LHPointToConfFile = {
  { "VeryLooseLHElectron",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf" },
  { "LooseLHElectron",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodLooseOfflineConfig2017_Smooth.conf" },
  { "LooseBLLHElectron",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodLooseOfflineConfig2017_CutBL_Smooth.conf" },
  { "MediumLHElectron",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodMediumOfflineConfig2017_Smooth.conf" },
  { "TightLHElectron",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodTightOfflineConfig2017_Smooth.conf" },
  { "VeryLooseLHElectron_Run2",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth.conf" },
  { "LooseLHElectron_Run2",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodLooseOfflineConfig2017_Smooth.conf" },
  { "LooseBLLHElectron_Run2",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodLooseOfflineConfig2017_CutBL_Smooth.conf" },
  { "MediumLHElectron_Run2",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodMediumOfflineConfig2017_Smooth.conf" },
  { "TightLHElectron_Run2",
    "ElectronPhotonSelectorTools/offline/mc20_20210514/"
    "ElectronLikelihoodTightOfflineConfig2017_Smooth.conf" },
  { "VeryLooseLHElectron_LLP",
    "ElectronPhotonSelectorTools/offline/mc20_20230321/"
    "ElectronLikelihoodVeryLooseOfflineConfig2017_Smooth_NoD0_NoPix.conf" },
  { "LooseLHElectron_LLP",
    "ElectronPhotonSelectorTools/offline/mc20_20230321/"
    "ElectronLikelihoodLooseOfflineConfig2017_Smooth_NoD0_NoPix.conf" },
  { "MediumLHElectron_LLP",
    "ElectronPhotonSelectorTools/offline/mc20_20230321/"
    "ElectronLikelihoodMediumOfflineConfig2017_Smooth_NoD0_NoPix.conf" },
  { "TightLHElectron_LLP",
    "ElectronPhotonSelectorTools/offline/mc20_20230321/"
    "ElectronLikelihoodTightOfflineConfig2017_Smooth_NoD0_NoPix.conf" },
  { "VeryLooseLHElectron_HI",
    "ElectronPhotonSelectorTools/offline/mc23_20230728_HI/"
    "ElectronLikelihoodVeryLooseOfflineConfig2023_HI_Smooth.conf" },
  { "LooseLHElectron_HI",
    "ElectronPhotonSelectorTools/offline/mc23_20230728_HI/"
    "ElectronLikelihoodLooseOfflineConfig2023_HI_Smooth.conf" },
  { "MediumLHElectron_HI",
    "ElectronPhotonSelectorTools/offline/mc23_20230728_HI/"
    "ElectronLikelihoodMediumOfflineConfig2023_HI_Smooth.conf" },
  { "TightLHElectron_HI",
    "ElectronPhotonSelectorTools/offline/mc23_20230728_HI/"
    "ElectronLikelihoodTightOfflineConfig2023_HI_Smooth.conf" },
};
const std::map<std::string, std::string> PhotonCutPointToConfFile = {
  { "LoosePhoton",
    "ElectronPhotonSelectorTools/offline/mc15_20150712/"
    "PhotonIsEMLooseSelectorCutDefs.conf" },
  { "MediumPhoton",
    "ElectronPhotonSelectorTools/offline/mc15_20160512/"
    "PhotonIsEMMediumSelectorCutDefs.conf" },
  { "TightPhoton",
    "ElectronPhotonSelectorTools/offline/20180825/"
    "PhotonIsEMTightSelectorCutDefs.conf" },
  { "TightPhotonWithMu",
    "ElectronPhotonSelectorTools/offline/mc16_20220621/"
    "PhotonIsEMTightSelectorCutDefs.conf" },
};
const std::map<std::string, std::string> ForwardLHPointToConfFile = {
  { "LooseLHForwardElectron",
    "ElectronPhotonSelectorTools/offline/mc16_20180822/FwdLHLooseConf.conf" },
  { "MediumLHForwardElectron",
    "ElectronPhotonSelectorTools/offline/mc16_20180822/FwdLHMediumConf.conf" },
  { "TightLHForwardElectron",
    "ElectronPhotonSelectorTools/offline/mc16_20180822/FwdLHTightConf.conf" },
};
const std::map<std::string, std::string> ForwardElectronCutPointToConfFile = {
  { "LooseForwardElectron",
    "ElectronPhotonSelectorTools/offline/mc15_20150812/"
    "ForwardElectronIsEMLooseSelectorCutDefs.conf" },
  { "MediumForwardElectron",
    "ElectronPhotonSelectorTools/offline/mc15_20150812/"
    "ForwardElectronIsEMMediumSelectorCutDefs.conf" },
  { "TightForwardElectron",
    "ElectronPhotonSelectorTools/offline/mc15_20150812/"
    "ForwardElectronIsEMTightSelectorCutDefs.conf" },
};
const std::map<std::string, std::string> ElectronCutPointToConfFile = {
  { "LooseElectron",
    "ElectronPhotonSelectorTools/offline/mc15_20150712/"
    "ElectronIsEMLooseSelectorCutDefs.conf" },
  { "MediumElectron",
    "ElectronPhotonSelectorTools/offline/mc15_20150712/"
    "ElectronIsEMMediumSelectorCutDefs.conf" },
  { "TightElectron",
    "ElectronPhotonSelectorTools/offline/mc15_20150712/"
    "ElectronIsEMTightSelectorCutDefs.conf" }
};
//----------------------------------------------------------
// Map enums to masks , needed for photons
const std::map<std::string, unsigned int> PhotonCutPointToMask = {
  { "LoosePhoton", egammaPID::PhotonLoose },
  { "MediumPhoton", egammaPID::PhotonMedium },
  { "TightPhoton", egammaPID::PhotonTight },
  { "TightPhotonWithMu", egammaPID::PhotonTight },
};
const std::map<std::string, std::string> ElectronDNNPointToConfFile = {
  { "LooseDNNElectron",
    "ElectronPhotonSelectorTools/offline/mc16_20210430/"
    "ElectronDNNMulticlassLoose.conf"},
  { "MediumDNNElectron",
    "ElectronPhotonSelectorTools/offline/mc16_20210430/"
    "ElectronDNNMulticlassMedium.conf"},
  { "TightDNNElectron",
    "ElectronPhotonSelectorTools/offline/mc16_20210430/"
    "ElectronDNNMulticlassTight.conf"}
};
}
////////////////////////////////////////////
#endif
