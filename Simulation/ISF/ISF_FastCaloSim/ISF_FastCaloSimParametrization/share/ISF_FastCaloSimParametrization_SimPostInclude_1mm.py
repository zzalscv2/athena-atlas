from AthenaCommon.CfgGetter import getPublicTool
stepInfoSDTool = getPublicTool("SensitiveDetectorMasterTool").SensitiveDetectors['FCS_StepInfoSensitiveDetector']
stepInfoSDTool.shift_lar_subhit = True #default
stepInfoSDTool.shorten_lar_step = True
stepInfoSDTool.maxRadiusFine = 1. #default (for EMB1 and EME1)
stepInfoSDTool.maxRadius = 25. #default
stepInfoSDTool.maxRadiusTile = 25. #default
stepInfoSDTool.maxTime = 25. #default
stepInfoSDTool.maxTimeTile = 100. #default


topSeq.BeamEffectsAlg.GenEventManipulators = [getPublicTool("GenEventValidityChecker")]
validTruthStrat = getPublicTool("ISF_ValidationTruthStrategy")
validTruthStrat.Regions = [3]
validTruthStrat.ParentMinP = 150
ServiceMgr.ISF_MC15aPlusTruthService.TruthStrategies = [ validTruthStrat ]
from AthenaCommon.AppMgr import ToolSvc
ToolSvc.ISF_EntryLayerToolMT.ParticleFilters = []
