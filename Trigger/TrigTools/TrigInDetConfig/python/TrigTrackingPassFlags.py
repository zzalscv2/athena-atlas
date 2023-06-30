# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
import AthenaCommon.SystemOfUnits as Units

from TrkConfig.TrackingPassFlags import createTrackingPassFlags,createITkTrackingPassFlags
from TrigEDMConfig.TriggerEDMRun3 import recordable

def signatureSpecificSettingOfFlags(flags,mode):
  
  #temporary - to be reworked
  if mode=="InDet":
    flags.minPT = flags.pTmin   #hack to sync pT threshold used in offline and trigger
    
    flags.minClusters         = 7   #hardcoded to preserve trigger settings (not used for FTF config)
    flags.minSiNotShared      = 5
    flags.maxShared           = 2
    flags.Xi2max              = 9. if flags.input_name != "bjet" else 12.
    flags.Xi2maxNoAdd         = 25.
    flags.nHolesMax           = 2
    flags.nHolesGapMax        = 2
    flags.nWeightedClustersMin= 6
    flags.roadWidth           =10.
    if flags.input_name in ['jet','jetSuper','fullScan','fullScanUTT']:
      flags.roadWidth =         5.
    elif flags.input_name == 'cosmics':
      flags.roadWidth =        75.
      
    flags.useNewParameterizationTRT = True
    flags.minTRTonTrk          =9

    #TODO - simple ambiguitues
    flags.useTIDE_Ambi = False  

    #2023fix - it should read 2
    flags.maxSiHoles           = 5
    flags.maxSCTHoles          = 5
    #end 
    
  else:                         #ITk specific settings can be done here while we rely on ConfigSettings
    flags.minPT = [flags.pTmin] #ITk flags have eta dependant settings
    flags.Xi2max              = [9.]
    flags.Xi2maxNoAdd         = [25.]
    flags.nHolesMax           = [2]
    flags.nHolesGapMax        = [2]
    flags.nWeightedClustersMin= [6]
  
  flags.seedFilterLevel     = 0
  
  if flags.isLRT:
    flags.minClusters         = 8
    flags.nHolesGapMax        = 1
    flags.nWeightedClustersMin= 8
    flags.maxSiHoles          = 2
    flags.maxSCTHoles         = 1
    flags.maxPixelHoles       = 1
    flags.maxDoubleHoles      = 0
    
  if flags.input_name=="cosmics":
    flags.nClustersMin        = 4
    flags.nHolesMax           = 3
    flags.Xi2max              = 60.  if mode=="InDet" else [60.]
    flags.Xi2maxNoAdd         = 100. if mode=="InDet" else [100.]
    flags.nWeightedClustersMin= 8
    flags.minTRTonTrk         = 20

  def collToRecordable(flags,name):
    ret = name
    signature = flags.input_name
    firstStage = True if "FTF" in name else False
    record = True
    if firstStage:
      if signature in ["tau","tauTau",
                       "minBias","bjetLRT",
                       "beamSpot","BeamSpot"]:
        record = False
    else:
      if signature in ["tauCore","tauIso","tauIsoBDT",
                       "jet","fullScan","FS","jetSuper",
                       "beamSpot", "BeamSpot","beamSpotFS",
                       "bjetLRT","DJetLRT","DVtxLRT"]:
        record = False

    if record:
      ret = recordable(name)
      
    return ret

  flags.addFlag("trkTracks_FTF",    f'HLT_IDTrkTrack_{flags.suffix}_FTF')
  flags.addFlag("trkTracks_IDTrig", f'HLT_IDTrkTrack_{flags.suffix}_IDTrig')
  flags.addFlag("tracks_FTF",    
                collToRecordable(flags, f'HLT_IDTrack_{flags.suffix}_FTF'))
  flags.addFlag("tracks_IDTrig", 
                collToRecordable(flags, "HLT_IDTrack_{}_IDTrig".format(flags.suffix if flags.input_name != "tauIso" else "Tau")))

  flags.addFlag("refitROT", True) 
  flags.addFlag("trtExtensionType", "xf") 
  flags.addFlag("doBremRecovery", False)  #setTrue for electron once validated

    
def createTrigTrackingPassFlags(mode="InDet"):
  def __flagsFromConfigSettings(settings, mode):
    if mode == "InDet":
      flags = createTrackingPassFlags()
    elif mode == "ITk":
      flags = createITkTrackingPassFlags()
    else:
      raise RuntimeError("createTrigTrackingPassFlags cannot create flags for detector not in InDet or ITk: {}".format(mode)) 
  
    for setting, value in settings.__dict__.items():
        setting = setting.lstrip("_")
        if setting in flags._flagdict:
            if value is not None: 
                flags._flagdict[setting].set(value)
        else:
            if value is None: 
                flags.addFlag(setting, lambda pf: None)
            else:
                flags.addFlag(setting, value)

    signatureSpecificSettingOfFlags(flags,mode)

    return flags
    
  #hide instantiation of flags in a function that can be consumed by addFlagsCategory
  def flagsFactory(settings,mode):
    def hidearg():
        return __flagsFromConfigSettings(settings,mode)
    return hidearg

  from AthenaConfiguration.AthConfigFlags import AthConfigFlags
  flags = AthConfigFlags()
  from TrigInDetConfig.ConfigSettings import ConfigSettingsInstances,getInDetTrigConfig
  category = 'Trigger.InDetTracking' if mode=="InDet" else 'Trigger.ITkTracking'
  
  for i in ConfigSettingsInstances.keys():
    signatureCategory = "{}.{}".format(category,i)
    factory = flagsFactory(getInDetTrigConfig(i),mode)     
    flags.addFlagsCategory(signatureCategory,factory,prefix=True)

  #TBD make a function for global settings too
  flags.addFlag(f'{category}.RoiZedWidthDefault', 180.0 * Units.mm)

  return flags


import unittest

class FlagsCopiedTest(unittest.TestCase):
    def setUp(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Trigger.doID
        flags.Trigger.InDetTracking.Muon
        flags.Trigger.InDetTracking.Electron.minPT = 2.0 * Units.GeV
        self.newflags = flags.cloneAndReplace('Tracking.ActiveConfig', 'Trigger.InDetTracking.Electron')

        self.newflags.dump(".*InDet")

    def runTest(self):
        self.assertEqual(self.newflags.Tracking.ActiveConfig.minPT, 2.0 * Units.GeV, msg="Flags are not copied")



class UnsetFlagsTest(FlagsCopiedTest):
    def runTest(self):
        self.assertEqual(self.newflags.Tracking.ActiveConfig.vertex_jet, None)


if __name__ == "__main__":
    unittest.main()
