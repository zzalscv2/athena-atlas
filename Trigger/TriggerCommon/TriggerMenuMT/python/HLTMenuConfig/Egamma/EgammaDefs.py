# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#----------------------------------------------------------------
# Static classes to configure photon chain container names
#----------------------------------------------------------------

from TrigEDMConfig.TriggerEDMRun3 import recordable
from AthenaCommon.Logging import logging
from AthenaCommon import CfgMgr
from ROOT import egammaPID
from ElectronPhotonSelectorTools.ConfiguredAsgPhotonIsEMSelectors import ConfiguredAsgPhotonIsEMSelector

log = logging.getLogger(__name__)

class TrigEgammaKeys(object):
      """Static class to collect all string manipulation in Electron sequences """
      SuperElectronRecCollectionName = 'HLT_ElectronSuperRecCollection'
      outputElectronKey = recordable('HLT_egamma_Electrons')
      SuperPhotonRecCollectionName = 'HLT_PhotonSuperRecCollection'
      EgammaRecKey = 'HLT_egammaRecCollection'
      outputPhotonKey = recordable('HLT_egamma_Photons')
      outputClusterKey = 'HLT_egammaClusters'
      outputTopoSeededClusterKey = 'HLT_egammaTopoSeededClusters'
      TrigEMClusterToolOutputContainer = 'HLT_TrigEMClusterOutput'
      TrigElectronTracksCollectionName = recordable('HLT_IDTrack_Electron_IDTrig')
      pidVersion = 'rel22_20210611'
      dnnVersion = 'mc16_20210430'

class TrigEgammaKeys_LRT(object):
      """Static class to collect all string manipulation in Electron_LRT sequences """
      outputElectronKey_LRT = recordable('HLT_egamma_Electrons_LRT')
      TrigElectronTracksCollectionName_LRT = recordable('HLT_IDTrack_ElecLRT_IDTrig')

class TrigEgammaKeys_GSF(object):
      """Static class to collect all string manipulation in Electron sequences """
      outputElectronKey_GSF = recordable('HLT_egamma_Electrons_GSF')
      outputTrackKey_GSF = 'HLT_IDTrkTrack_Electron_GSF'
      outputTrackParticleKey_GSF = recordable('HLT_IDTrack_Electron_GSF')
      


#
# Electron DNN Selectors
#
def createTrigEgammaPrecisionElectronDNNSelectors(ConfigFilePath=None):
# We should include the DNN here
    if not ConfigFilePath:
      ConfigFilePath = 'ElectronPhotonSelectorTools/offline/'+TrigEgammaKeys.dnnVersion
  
    import collections
    SelectorNames = collections.OrderedDict({
          'dnntight':'AsgElectronDNNTightSelector',
          'dnnmedium':'AsgElectronDNNMediumSelector',
          'dnnloose':'AsgElectronDNNLooseSelector',
          })

    ElectronToolConfigFile = collections.OrderedDict({
          'dnntight':'ElectronDNNMulticlassTight.conf',
          'dnnmedium':'ElectronDNNMulticlassMedium.conf',
          'dnnloose':'ElectronDNNMulticlassLoose.conf',
          })

    selectors = []
    log.debug('Configuring electron DNN' )
    for dnnname, name in SelectorNames.items():
      SelectorTool = CfgMgr.AsgElectronSelectorTool(name)
      SelectorTool.ConfigFile = ConfigFilePath + '/' + ElectronToolConfigFile[dnnname]
      SelectorTool.skipDeltaPoverP = True
      selectors.append(SelectorTool)

    return selectors

#
# Electron LH Selectors
#
def createTrigEgammaPrecisionElectronLHSelectors(ConfigFilePath=None):

    # Configure the LH selectors
    #TrigEgammaKeys.pidVersion.set_On()
    if not ConfigFilePath:
      ConfigFilePath = 'ElectronPhotonSelectorTools/trigger/'+TrigEgammaKeys.pidVersion

    import collections
    SelectorNames = collections.OrderedDict({
          'lhtight'       :'AsgElectronLHTightSelector',
          'lhmedium'      :'AsgElectronLHMediumSelector',
          'lhloose'       :'AsgElectronLHLooseSelector',
          'lhvloose'      :'AsgElectronLHVLooseSelector',
          'lhtight_nopix' :'AsgElectronLHTightSelectorNoPix',
          'lhmedium_nopix':'AsgElectronLHMediumSelectorNoPix',
          'lhloose_nopix' :'AsgElectronLHLooseSelectorNoPix',
          'lhvloose_nopix':'AsgElectronLHVLooseSelectorNoPix',
          })
     
    ElectronToolConfigFile = collections.OrderedDict({
          'lhtight'         :'ElectronLikelihoodTightTriggerConfig.conf',
          'lhmedium'        :'ElectronLikelihoodMediumTriggerConfig.conf',
          'lhloose'         :'ElectronLikelihoodLooseTriggerConfig.conf',
          'lhvloose'        :'ElectronLikelihoodVeryLooseTriggerConfig.conf',
          'lhtight_nopix'   :'ElectronLikelihoodTightTriggerConfig_NoPix.conf',
          'lhmedium_nopix'  :'ElectronLikelihoodMediumTriggerConfig_NoPix.conf',
          'lhloose_nopix'   :'ElectronLikelihoodLooseTriggerConfig_NoPix.conf',
          'lhvloose_nopix'  :'ElectronLikelihoodVeryLooseTriggerConfig_NoPix.conf',
          })

    selectors = []
    log.debug('Configuring electron PID' )
    for pidname, name in SelectorNames.items():
      SelectorTool = CfgMgr.AsgElectronLikelihoodTool(name)
      SelectorTool.ConfigFile = ConfigFilePath + '/' + ElectronToolConfigFile[pidname]
      SelectorTool.usePVContainer = False 
      SelectorTool.skipDeltaPoverP = True
      selectors.append(SelectorTool)

    return selectors


#
# Electron CB Selectors
#
def createTrigEgammaPrecisionElectronCBSelectors(ConfigFilePath=None):
    from ElectronPhotonSelectorTools.TrigEGammaPIDdefs import BitDefElectron

    ElectronLooseHI = (0
        | 1 << BitDefElectron.ClusterEtaRange_Electron
        | 1 << BitDefElectron.ClusterHadronicLeakage_Electron
        | 1 << BitDefElectron.ClusterMiddleEnergy_Electron
        | 1 << BitDefElectron.ClusterMiddleEratio37_Electron
        | 1 << BitDefElectron.ClusterMiddleWidth_Electron
        | 1 << BitDefElectron.ClusterStripsWtot_Electron
    )

    ElectronMediumHI = (ElectronLooseHI
        | 1 << BitDefElectron.ClusterMiddleEratio33_Electron
        | 1 << BitDefElectron.ClusterBackEnergyFraction_Electron
        | 1 << BitDefElectron.ClusterStripsEratio_Electron
        | 1 << BitDefElectron.ClusterStripsDeltaEmax2_Electron
        | 1 << BitDefElectron.ClusterStripsDeltaE_Electron
        | 1 << BitDefElectron.ClusterStripsFracm_Electron
        | 1 << BitDefElectron.ClusterStripsWeta1c_Electron
    )

    if not ConfigFilePath:
        ConfigFilePath = 'ElectronPhotonSelectorTools/trigger/'+TrigEgammaKeys.pidVersion

    from collections import OrderedDict
    SelectorNames = OrderedDict({
        'medium': 'AsgElectronIsEMSelectorHIMedium',
        'loose': 'AsgElectronIsEMSelectorHILoose',
        'mergedtight'  : 'AsgElectronIsEMSelectorMergedTight',
    })

    ElectronToolConfigFile = {
        'medium': 'ElectronIsEMMediumSelectorCutDefs.conf',
        'loose': 'ElectronIsEMLooseSelectorCutDefs.conf',
        'mergedtight'  : 'ElectronIsEMMergedTightSelectorCutDefs.conf',
    }

    ElectronMaskBits = {
        'medium': ElectronMediumHI,
        'loose': ElectronLooseHI,
        'mergedtight'  : egammaPID.ElectronTightHLT,
    }

    selectors = []
    for sel, name in SelectorNames.items():
        SelectorTool = CfgMgr.AsgElectronIsEMSelector(name)
        SelectorTool.ConfigFile = ConfigFilePath + '/' + ElectronToolConfigFile[sel]
        SelectorTool.isEMMask = ElectronMaskBits[sel]
        selectors.append(SelectorTool)

    return selectors


#
# Photon IsEM selectors
#
def createTrigEgammaPrecisionPhotonSelectors(ConfigFilePath=None):

    if not ConfigFilePath:
      ConfigFilePath = 'ElectronPhotonSelectorTools/trigger/'+TrigEgammaKeys.pidVersion

    import collections
    # Configure the IsEM selectors
    SelectorNames = collections.OrderedDict( {
            'tight'  : 'TightPhotonSelector',
            'medium' : 'MediumPhotonSelector',
            'loose'  : 'LoosePhotonSelector',
            } )
    SelectorPID = {
            'loose'  : egammaPID.PhotonIDLoose,
            'medium' : egammaPID.PhotonIDMedium,
            'tight'  : egammaPID.PhotonIDTight,
            }
    PhotonToolConfigFile = {
            'loose'  : 'PhotonIsEMLooseSelectorCutDefs.conf', 
            'medium' : 'PhotonIsEMMediumSelectorCutDefs.conf', 
            'tight'  : 'PhotonIsEMTightSelectorCutDefs.conf',
            } 
    PhotonIsEMBits = {
            'loose'  : egammaPID.PhotonLooseEF,
            'medium' : egammaPID.PhotonMediumEF,
            'tight'  : egammaPID.PhotonTight,
            }

    selectors = []
    for sel, name in SelectorNames.items():
        log.debug('Configuring photon PID for %s', sel)
        SelectorTool = ConfiguredAsgPhotonIsEMSelector(name, SelectorPID[sel])
        ConfigFile = ConfigFilePath + '/' + PhotonToolConfigFile[sel] 
        log.debug('Configuration file: %s', ConfigFile)
        SelectorTool.ConfigFile = ConfigFile
        SelectorTool.ForceConvertedPhotonPID = True
        SelectorTool.isEMMask = PhotonIsEMBits[sel] 
        selectors.append(SelectorTool)

    return selectors



#
# Electron/Photon ringer NN selectors
#
def createTrigEgammaFastCaloSelectors(doPhotons=False, ConfigFilePath='RingerSelectorTools/TrigL2_20210227_r3'):

    from RingerSelectorTools.RingerSelectorToolsConf import Ringer__AsgRingerSelectorTool 
    from AthOnnxruntimeService.AthOnnxruntimeServiceConf import AthONNX__ONNXRuntimeSvc
    from AthenaCommon.AppMgr import ServiceMgr
    import collections
    # add ONNX into app service mgr
    ServiceMgr += AthONNX__ONNXRuntimeSvc()
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    SelectorNames = collections.OrderedDict( {
        "Electrons": collections.OrderedDict({
            'tight'    : 'AsgElectronFastCaloRingerTightSelectorTool',
            'medium'   : 'AsgElectronFastCaloRingerMediumSelectorTool',
            'loose'    : 'AsgElectronFastCaloRingerLooseSelectorTool',
            'vloose'   : 'AsgElectronFastCaloRingerVeryLooseSelectorTool',
            }),
        "Photons": collections.OrderedDict({
            'tight'  : 'AsgPhotonFastCaloRingerTightSelectorTool',
            'medium' : 'AsgPhotonFastCaloRingerMediumSelectorTool',
            'loose'  : 'AsgPhotonFastCaloRingerLooseSelectorTool',
          })
    } )
    
    ToolConfigFile = {
        "Electrons" : collections.OrderedDict({
          'tight'   :'ElectronRingerTightTriggerConfig.conf',
          'loose'   :'ElectronRingerLooseTriggerConfig.conf',
          'medium'  :'ElectronRingerMediumTriggerConfig.conf',
          'vloose'  :'ElectronRingerVeryLooseTriggerConfig.conf',
          }),
        "Photons" : collections.OrderedDict({
          'tight' :'PhotonRingerTightTriggerConfig.conf',
          'medium':'PhotonRingerMediumTriggerConfig.conf',
          'loose' :'PhotonRingerLooseTriggerConfig.conf',
        })
    }
    cand = 'Photons' if doPhotons else 'Electrons'
    selectors = []
    for pidname , name in SelectorNames[cand].items():
      log.debug('Configuring electron ringer PID for %s', pidname)
      SelectorTool=Ringer__AsgRingerSelectorTool(name)
      SelectorTool.ConfigFile = ConfigFilePath + '/' + ToolConfigFile[cand][pidname]
      selectors.append(SelectorTool)
    return selectors
