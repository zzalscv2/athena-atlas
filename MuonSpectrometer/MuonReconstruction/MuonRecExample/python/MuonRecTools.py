# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s", __name__)

from AthenaCommon.GlobalFlags import globalflags
from AthenaCommon.DetFlags import DetFlags
from AthenaCommon import CfgMgr
from AthenaCommon.BeamFlags import jobproperties
beamFlags = jobproperties.Beam

from MuonCnvExample.MuonCnvUtils import mdtCalibWindowNumber
from MuonRecExample.MuonRecUtils import logMuon,ConfiguredBase,ExtraFlags

from MuonRecExample.MuonRecFlags import muonRecFlags
muonRecFlags.setDefaults()

from MuonRecExample.MuonStandaloneFlags import muonStandaloneFlags
muonStandaloneFlags.setDefaults()

from RecExConfig.RecFlags import rec

from AthenaCommon.CfgGetter import getPrivateTool, getPrivateToolClone, getPublicTool, getService
from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from InDetRecExample import TrackingCommon

#--------------------------------------------------------------------------------
# Hit-on-track creation tools
#--------------------------------------------------------------------------------
# set up default ROT creator
def MuonClusterOnTrackCreator(name="MuonClusterOnTrackCreator",**kwargs):
    if globalflags.DataSource() == 'data': # collisions real data or simulated first data
        # scale TGC eta hit errors as long as TGC eta are not well aligned
        kwargs.setdefault("DoFixedErrorTgcEta", True)
        kwargs.setdefault("FixedErrorTgcEta", 15.)

    reco_stgcs = muonRecFlags.dosTGCs() and MuonGeometryFlags.hasSTGC()
    reco_mm = muonRecFlags.doMMs() and MuonGeometryFlags.hasMM()
    if reco_stgcs or reco_mm:
        kwargs.setdefault("NSWCalibTool", "NSWCalibTool")

    return CfgMgr.Muon__MuonClusterOnTrackCreator(name,**kwargs)

def getMuonRIO_OnTrackErrorScalingCondAlg() :
    error_scaling_def=["CSCRIO_OnTrackErrorScaling:/MUON/TrkErrorScalingCSC"]
    from InDetRecExample.TrackingCommon import getRIO_OnTrackErrorScalingCondAlg
    return getRIO_OnTrackErrorScalingCondAlg( name                = "MuonRIO_OnTrackErrorScalingCondAlg",
                                              ReadKey             = "/MUON/TrkErrorScaling",
                                              CondDataAssociation = error_scaling_def)

def CscClusterOnTrackCreator(name="CscClusterOnTrackCreator",**kwargs):
    kwargs.setdefault("CscStripFitter", getPrivateTool("CalibCscStripFitter") )
    kwargs.setdefault("CscClusterFitter", getPrivateTool("QratCscClusterFitter") )
    kwargs.setdefault("CscClusterUtilTool", getPrivateTool("CscClusterUtilTool") )
    if False  : # enable CscClusterOnTrack error scaling :
        TrackingCommon.createAndAddCondAlg(getMuonRIO_OnTrackErrorScalingCondAlg,'RIO_OnTrackErrorScalingCondAlg')

        kwargs.setdefault("CSCErrorScalingKey","/MUON/TrkErrorScalingCSC")

    if globalflags.DataSource() == 'data': # collisions real data or simulated first data
        # scale CSC and hit errors
        kwargs.setdefault("ErrorScalerBeta", 0.070 )

    return CfgMgr.Muon__CscClusterOnTrackCreator(name,**kwargs)

def CscBroadClusterOnTrackCreator(name="CscBroadClusterOnTrackCreator",**kwargs):
    kwargs["ErrorScalerBeta"] = 0.200
    return CscClusterOnTrackCreator(name,**kwargs)


def MdtDriftCircleOnTrackCreator(name="MdtDriftCircleOnTrackCreator",**kwargs):
    # setup dependencies missing in C++. TODO: fix in C++
    from MuonRecExample import MuonAlignConfig  # noqa: F401
    from MuonCnvExample import MuonCalibConfig  # noqa: F401
    MuonCalibConfig.setupMdtCondDB()
    from MuonCnvExample.MuonCalibFlags import mdtCalibFlags
    mdtCalibFlags.setDefaults()

    margs = dict()
    margs.setdefault("DoMagneticFieldCorrection", mdtCalibFlags.correctMdtRtForBField())
    margs.setdefault("DoWireSagCorrection", muonRecFlags.useWireSagCorrections())
    margs.setdefault("DoSlewingCorrection", mdtCalibFlags.correctMdtRtForTimeSlewing())


    if beamFlags.beamType() == 'cosmics' or beamFlags.beamType() == 'singlebeam':
        margs.setdefault("DoTofCorrection", False)
        kwargs.setdefault("DoFixedError", True)
        kwargs.setdefault("TimingMode", 1)
        kwargs.setdefault("UseParametrisedError", True)
        kwargs.setdefault("ApplyToF", False)

    else: # collisions simulation/data settings
        margs.setdefault("DoTofCorrection", True)
        kwargs.setdefault("DoFixedError", False)
        kwargs.setdefault("DoErrorScaling", False)
        margs.setdefault("TimeWindowSetting", mdtCalibWindowNumber('Collision_data'))  # MJW: should this be Collision_G4 ???
        kwargs.setdefault("UseParametrisedError", False)

        if globalflags.DataSource() == 'data': # collisions real data or simulated first data
            kwargs.setdefault("CreateTubeHit", True)  # BroadErrors
            kwargs.setdefault("UseLooseErrors", muonRecFlags.useLooseErrorTuning())  # LooseErrors on data
       
    calibSettings = ["DoTofCorrection", "TimeWindowSetting"]
    for cSett in calibSettings:
        if cSett in kwargs: 
          margs[cSett] =  kwargs.pop(cSett)

    kwargs.setdefault("CalibrationTool",    getPrivateToolClone (name+"MdtCalibTool", 
                                                                 MuonCalibConfig.MdtCalibrationTool(), **margs))
 
 
    kwargs.setdefault("IsMC", globalflags.DataSource() != 'data')
    return CfgMgr.Muon__MdtDriftCircleOnTrackCreator(name, WasConfigured=True, **kwargs)
# end of factory function MdtDriftCircleOnTrackCreator


def MdtTubeHitOnTrackCreator(name="MdtTubeHitOnTrackCreator",**kwargs):
    kwargs["CreateTubeHit"] = True
    return MdtDriftCircleOnTrackCreator(name,**kwargs)

def MdtDriftCircleOnTrackCreatorStau(name="MdtDriftCircleOnTrackCreatorStau",**kwargs ):
    kwargs.setdefault("TimingMode", 3 )
    kwargs.setdefault("TimeWindowSetting", mdtCalibWindowNumber('Collision_t0fit') )
    return MdtDriftCircleOnTrackCreator(name,**kwargs)


# For segment fitting with variable t0. Note separate instance names below for Moore and Muonboy
# DoTof must be consistent with the one in MdtDriftCircleOnTrackCreator
def AdjustableT0Tool(name="AdjustableT0Tool",**kwargs):
    # NB: the following 'ifs' are the same as in the MdtDriftCircleOnTrackCreator, so that the ToF setting is the same
    if muonStandaloneFlags.reconstructionMode() == 'cosmics':
        kwargs.setdefault("DoTof", 0)
    else: # collisions simulation final precise cuts
        kwargs.setdefault("DoTof", 1)

    from MdtDriftCircleOnTrackCreator.MdtDriftCircleOnTrackCreatorConf import AdjT0__AdjustableT0Tool
    return AdjT0__AdjustableT0Tool(name,**kwargs)
# end of factory function AdjustableT0ToolMboy


def MdtDriftCircleOnTrackCreatorAdjustableT0(name="MdtDriftCircleOnTrackCreatorAdjustableT0",**kwargs):
    kwargs.setdefault("TimingMode", 3)
    kwargs.setdefault("DoTofCorrection", True)
    kwargs.setdefault("TimeWindowSetting", mdtCalibWindowNumber('Collision_data'))
    return MdtDriftCircleOnTrackCreator(name,**kwargs)

# default RIO_OnTrackCreator for muons
# make a dedicated class to delay instantiation of the muon RIO_OnTrack creators members
from TrkRIO_OnTrackCreator.TrkRIO_OnTrackCreatorConf import Trk__RIO_OnTrackCreator
class MuonRotCreator(Trk__RIO_OnTrackCreator,ConfiguredBase):
    __slots__ = ()

    def __init__(self,name="MuonRotCreator",**kwargs):
        self.applyUserDefaults(kwargs,name)                  
        kwargs.setdefault("ToolMuonDriftCircle", getPublicTool("MdtDriftCircleOnTrackCreator"))
        kwargs.setdefault("ToolMuonCluster", getPublicTool("MuonClusterOnTrackCreator"))
        kwargs.setdefault("Mode", 'muon' )
        super(MuonRotCreator,self).__init__(name,**kwargs)
# end of class MuonRotCreator


# configure the pattern recognition

def MuonCombinePatternTool(name="MuonCombinePatternTool",**kwargs):
    kwargs.setdefault("UseTightAssociation", muonStandaloneFlags.reconstructionMode() == 'collisions')
    kwargs.setdefault("UseCosmics", beamFlags.beamType() != 'collisions'   )
    return CfgMgr.MuonCombinePatternTool(name,**kwargs)
# end of factory function MuonCombinePatternTool

def MuonHoughPatternTool(name="MuonHoughPatternTool",**kwargs):
    if muonStandaloneFlags.reconstructionMode() == 'collisions':
        kwargs.setdefault("UseCosmics", True)
        kwargs.setdefault("NumberOfMaximaPerIterations", 1)
    return CfgMgr.MuonHoughPatternTool(name,**kwargs)
# end of factory function MuonHoughPatternTool

def MuonHoughPatternFinderTool(name="MuonHoughPatternFinderTool",**kwargs):
    kwargs.setdefault("muonCombinePatternTool", getPublicTool("MuonCombinePatternTool"))
    kwargs.setdefault("muonHoughPatternTool", getPublicTool("MuonHoughPatternTool"))
    if muonStandaloneFlags.reconstructionMode() == 'collisions':
        kwargs.setdefault("MDT_TDC_cut", False)
    if muonStandaloneFlags.reconstructionMode() == 'collisions' or ConfigFlags.Muon.MuonTrigger:
        kwargs.setdefault("RecordAll",False)
    
    return CfgMgr.Muon__MuonHoughPatternFinderTool(name,**kwargs)

#--------------------------------------------------------------------------------
# Tracking geometry
#--------------------------------------------------------------------------------

# combined tracking geometry service

def TrackingVolumesSvc(name="TrackingVolumesSvc",**kwargs):
    from TrkDetDescrSvc.TrkDetDescrSvcConf import Trk__TrackingVolumesSvc
    return Trk__TrackingVolumesSvc("TrackingVolumesSvc")


# set the propagator
def MuonRK_Propagator(name="MuonRK_Propagator",**kwargs):
    from TrkExRungeKuttaPropagator.TrkExRungeKuttaPropagatorConf import Trk__RungeKuttaPropagator
    kwargs.setdefault("AccuracyParameter", 0.0002 )
    return Trk__RungeKuttaPropagator(name,**kwargs)

def MuonSTEP_Propagator(name="MuonSTEP_Propagator",**kwargs):
    from TrkExSTEP_Propagator.TrkExSTEP_PropagatorConf import Trk__STEP_Propagator
    kwargs.setdefault("Tolerance", 0.00001 )
    kwargs.setdefault("MaterialEffects", True  )
    kwargs.setdefault("IncludeBgradients", True  )
    return Trk__STEP_Propagator(name, **kwargs)


def MuonExtrapolator(name='MuonExtrapolator',**kwargs):
    kwargs.setdefault("Propagators", ["MuonPropagator"])
    kwargs.setdefault("MaterialEffectsUpdators", ["MuonMaterialEffectsUpdator"])
    kwargs.setdefault("Navigator", TrackingCommon.getInDetNavigator())
    kwargs.setdefault("ResolveMuonStation", True)
    kwargs.setdefault("Tolerance", 0.0011)  # must be > 1um to avoid missing MTG intersections

    return CfgMgr.Trk__Extrapolator(name,**kwargs)
# end of factory function MuonExtrapolator
def MuonStraightLineExtrapolator(name="MuonStraightLineExtrapolator",**kwargs):
    kwargs.setdefault("Propagators",["Trk::STEP_Propagator/MuonStraightLinePropagator"])
    kwargs.setdefault("STEP_Propagator","Trk::STEP_Propagator/MuonStraightLinePropagator")
    return MuonExtrapolator(name,**kwargs)

def MuonEDMHelperSvc(name='MuonEDMHelperSvc',**kwargs):
    # configure some tools that are used but are not declared as properties (they should be!)
    getService("MuonIdHelperSvc")
    getPublicTool("MCTBExtrapolator")

    from MuonRecHelperTools.MuonRecHelperToolsConf import Muon__MuonEDMHelperSvc
    return Muon__MuonEDMHelperSvc(name,**kwargs)
# end of factory function MuonEDMHelperSvc

from MuonRecHelperTools.MuonRecHelperToolsConf import Muon__MuonEDMPrinterTool
class MuonEDMPrinterTool(Muon__MuonEDMPrinterTool,ConfiguredBase):
    __slots__ = ()

    def __init__(self,name='MuonEDMPrinterTool',**kwargs):
        self.applyUserDefaults(kwargs,name)
        super(MuonEDMPrinterTool,self).__init__(name,**kwargs)
        getService("MuonEDMHelperSvc")
# end of class MuonEDMPrinterTool


def MuonTrackSummaryHelperTool(name="MuonTrackSummaryHelperTool",**kwargs):
    kwargs.setdefault("CalculateCloseHits", True)

    from MuonTrackSummaryHelperTool.MuonTrackSummaryHelperToolConf import Muon__MuonTrackSummaryHelperTool
    return Muon__MuonTrackSummaryHelperTool(name,**kwargs)

# end of factory function MuonTrackSummaryHelper

def MuonPRDSelectionTool(name="MuonPRDSelectionTool", **kwargs):
    kwargs.setdefault("MdtDriftCircleOnTrackCreator", getPublicTool("MdtDriftCircleOnTrackCreator"))
    kwargs.setdefault("MuonClusterOnTrackCreator", getPublicTool("MuonClusterOnTrackCreator"))
    return CfgMgr.Muon__MuonPRDSelectionTool(name,**kwargs)

from TrkTrackSummaryTool.TrkTrackSummaryToolConf import Trk__TrackSummaryTool
class MuonTrackSummaryTool(Trk__TrackSummaryTool,ConfiguredBase):
    __slots__ = ()

    def __init__(self,name="MuonTrackSummaryTool",**kwargs):
        self.applyUserDefaults(kwargs,name)
        kwargs.setdefault("MuonSummaryHelperTool", "MuonTrackSummaryHelperTool" )
        kwargs.setdefault("AddDetailedMuonSummary", True )
        super(MuonTrackSummaryTool,self).__init__(name,**kwargs)
# end of class MuonTrackSummaryTool


def MuonParticleCreatorTool(name="MuonParticleCreatorTool",**kwargs):
    from TrkParticleCreator.TrkParticleCreatorConf import Trk__TrackParticleCreatorTool
    kwargs.setdefault("TrackSummaryTool", "MuonTrackSummaryTool" )
    kwargs.setdefault("KeepAllPerigee", True )
    from MuonTrackSummaryHelperTool.MuonTrackSummaryHelperToolConf import Muon__MuonHitSummaryTool
    kwargs.setdefault("MuonSummaryTool", Muon__MuonHitSummaryTool("MuonHitSummaryTool"))
    kwargs.setdefault("PerigeeExpression", "Origin" )
    return Trk__TrackParticleCreatorTool(name, **kwargs)
# end of class MuonParticleCreatorTool

def MuonChi2TrackFitter(name='MuonChi2TrackFitter',**kwargs):
    from TrkGlobalChi2Fitter.TrkGlobalChi2FitterConf import Trk__GlobalChi2Fitter
    kwargs.setdefault("ExtrapolationTool"    , getPublicTool("MCTBExtrapolator"))
    kwargs.setdefault("RotCreatorTool"       , getPublicTool("MuonRotCreator"))
    kwargs.setdefault("MeasurementUpdateTool", getPublicTool("MuonMeasUpdator"))
    kwargs.setdefault("StraightLine"         , False)
    kwargs.setdefault("OutlierCut"           , 3.0)
    kwargs.setdefault("GetMaterialFromTrack" , False)
    kwargs.setdefault("RejectLargeNScat"     , True)

    # take propagator and navigator from the extrapolator
    Extrapolator = getPublicTool(kwargs["ExtrapolationTool"])
    kwargs.setdefault("PropagatorTool",Extrapolator.Propagators[0].getName())
    kwargs.setdefault("NavigatorTool",TrackingCommon.getInDetNavigator())
    
    cond_alg = TrackingCommon.createAndAddCondAlg(TrackingCommon.getTrackingGeometryCondAlg, "AtlasTrackingGeometryCondAlg", name="AtlasTrackingGeometryCondAlg")
    kwargs.setdefault("TrackingGeometryReadKey",cond_alg.TrackingGeometryWriteKey)
    return Trk__GlobalChi2Fitter(name,**kwargs)

try:
    from MuonSegmentMomentum.MuonSegmentMomentumConf import MuonSegmentMomentumFromField as MuonSegMomField
    class MuonSegmentMomentumFromField(MuonSegMomField,ConfiguredBase):
        __slots__ = ()

        def __init__(self,name="MuonSegmentMomentumFromField",**kwargs):
            self.applyUserDefaults(kwargs,name)
            super(MuonSegmentMomentumFromField,self).__init__(name,**kwargs)

    MuonSegmentMomentumFromField.setDefaultProperties(
        PropagatorTool = "MuonPropagator",
        NavigatorTool  = TrackingCommon.getInDetNavigator()
        )
except ImportError:
    pass


def MuonPhiHitSelector(name="MuonPhiHitSelector",**kwargs):
    kwargs.setdefault("MakeClusters", True)
    kwargs.setdefault("CompetingRios", True)
    kwargs.setdefault("DoCosmics", beamFlags.beamType() != 'collisions'  )

    return CfgMgr.MuonPhiHitSelector(name,**kwargs)
# end of factory function MuonPhiHitSelector

def MuonSegmentMomentum(name="MuonSegmentMomentum",**kwargs):
    kwargs.setdefault("DoCosmics", beamFlags.beamType() != 'collisions'  )
    return CfgMgr.MuonSegmentMomentum(name,**kwargs)
# end of factory function MuonSegmentMomentum

def MdtSegmentT0Fitter(name="MdtSegmentT0Fitter",**kwargs):
    # setup dependencies missing in C++. TODO: fix in C++
    from MuonRecExample import MuonAlignConfig  # noqa: F401
    from MuonCnvExample import MuonCalibConfig  # noqa: F401
    MuonCalibConfig.setupMdtCondDB()
    ### Enable floating of segments in the case of cosmic reconstruction
    kwargs.setdefault("FloatSegDirection", beamFlags.beamType() != 'collisions')
    return CfgMgr.TrkDriftCircleMath__MdtSegmentT0Fitter(name,**kwargs)

def MdtMathSegmentFinder(name="MdtMathSegmentFinder",extraFlags=None,**kwargs):
    beamType       = getattr(extraFlags,"beamType", beamFlags.beamType())
    doSegmentT0Fit = getattr(extraFlags,"doSegmentT0Fit",muonRecFlags.doSegmentT0Fit())
    enableCurvedSegmentFinding = getattr(extraFlags,"enableCurvedSegmentFinding", muonStandaloneFlags.enableCurvedSegmentFinding())

    if doSegmentT0Fit and not (ConfigFlags.Muon.MuonTrigger and (beamType =="cosmics" or globalflags.DataSource()=="data")):
        kwargs.setdefault("AssociationRoadWidth", 3.)
        kwargs.setdefault("MDTAssocationPullcut", 3.)
        kwargs.setdefault("RecoverMdtOutliers", False)
        kwargs.setdefault("DCFitProvider", "MdtSegmentT0Fitter" )

    if beamType == 'singlebeam' or beamType == 'cosmics' or globalflags.DataSource() == 'data':
        kwargs.setdefault("AssociationRoadWidth", 2.)
        kwargs.setdefault("MDTAssocationPullcut", 4.)
        kwargs.setdefault("RecoverMdtOutliers", True )


    if enableCurvedSegmentFinding:
        kwargs.setdefault("DoCurvedSegmentFinder",True)

    return CfgMgr.Muon__MdtMathSegmentFinder(name,**kwargs)

def MdtMathT0FitSegmentFinder(name="MdtMathT0FitSegmentFinder",extraFlags=None,**kwargs):
    if extraFlags is None: extraFlags = ExtraFlags()
    extraFlags.setFlagDefault('doSegmentT0Fit',True)
    return MdtMathSegmentFinder(name,extraFlags,**kwargs)

def MuonClusterSegmentFinder(name="MuonClusterSegmentFinder", extraFlags=None,**kwargs):
    kwargs.setdefault("AmbiguityProcessor",getPublicTool("MuonAmbiProcessor"))
    kwargs.setdefault("TrackToSegmentTool", getPublicTool("MuonTrackToSegmentTool") )
    kwargs.setdefault("MuonPRDSelectionTool", getPublicTool("MuonPRDSelectionTool") )
    return CfgMgr.Muon__MuonClusterSegmentFinder(name,**kwargs)

def MuonNSWSegmentFinderTool(name="MuonNSWSegmentFinderTool", extraFlags=None,**kwargs):
    kwargs.setdefault("SLFitter","Trk::GlobalChi2Fitter/MCTBSLFitterMaterialFromTrack")
    import MuonCombinedRecExample.CombinedMuonTrackSummary  # noqa: F401
    from AthenaCommon.AppMgr import ToolSvc
    kwargs.setdefault("TrackToSegmentTool", getPublicTool("MuonTrackToSegmentTool") )
    if ConfigFlags.Muon.MuonTrigger:
        kwargs.setdefault("TrackSummaryTool", "MuonTrackSummaryTool" )
    else:
        kwargs.setdefault("TrackSummaryTool", ToolSvc.CombinedMuonTrackSummary)

    kwargs.setdefault("MuonClusterCreator", getPublicTool("MuonClusterOnTrackCreator")) 
    return CfgMgr.Muon__MuonNSWSegmentFinderTool(name,**kwargs)

def DCMathSegmentMaker(name='DCMathSegmentMaker',extraFlags=None,**kwargs):
    beamType       = getattr(extraFlags,"beamType", beamFlags.beamType())
    doSegmentT0Fit = getattr(extraFlags,"doSegmentT0Fit", muonRecFlags.doSegmentT0Fit())
    updateSegmentSecondCoordinate = getattr(extraFlags,"updateSegmentSecondCoordinate", muonStandaloneFlags.updateSegmentSecondCoordinate())
    enableCurvedSegmentFinding = getattr(extraFlags,"enableCurvedSegmentFinding", muonStandaloneFlags.enableCurvedSegmentFinding())

    kwargs.setdefault("RefitSegment", True)
    kwargs.setdefault("AssumePointingPhi", beamType != 'cosmics')
    kwargs.setdefault("OutputFittedT0", True)
    kwargs.setdefault("DCFitProvider", "MdtSegmentT0Fitter")
    #kwargs.setdefault("CurvedErrorScaling", False)
    kwargs.setdefault("UsePreciseError", True)
    kwargs.setdefault("SinAngleCut", 0.4)

    if (beamType == 'singlebeam' or beamType == 'cosmics'):
        kwargs.setdefault("SinAngleCut", 0.9)
        kwargs.setdefault("AddUnassociatedPhiHits", True)
        kwargs.setdefault("RecoverBadRpcCabling", True)
        kwargs.setdefault("CurvedErrorScaling", False)
    elif globalflags.DataSource() == 'data': # collisions real data or simulation first data
        kwargs.setdefault("AddUnassociatedPhiHits", True)
        kwargs.setdefault("RecoverBadRpcCabling", True)

    if doSegmentT0Fit:
        kwargs.setdefault("MdtCreatorT0", getPrivateTool("MdtDriftCircleOnTrackCreatorAdjustableT0") )
        kwargs.setdefault("MdtSegmentFinder", getPrivateTool("MdtMathT0FitSegmentFinder") )
    else:
        kwargs.setdefault("MdtSegmentFinder", getPrivateTool("MdtMathSegmentFinder") )
    kwargs.setdefault("SegmentFitter", getPrivateTool("MuonSegmentFittingTool") )
    kwargs.setdefault("SegmentSelector", getPrivateTool("MuonSegmentSelectionTool") )

    if updateSegmentSecondCoordinate:
        kwargs.setdefault("UpdatePhiUsingPhiHits",True)

    if enableCurvedSegmentFinding:
        kwargs.setdefault("CurvedErrorScaling", False)
        kwargs.setdefault("PreciseErrorScale", 1)
        kwargs.setdefault("UsePreciseError", True)

    return CfgMgr.Muon__DCMathSegmentMaker(name,**kwargs)


def DCMathT0FitSegmentMaker(name='DCMathT0FitSegmentMaker',extraFlags=None,**kwargs):
    if extraFlags is None: extraFlags = ExtraFlags()
    extraFlags.setFlagDefault('doSegmentT0Fit',True)

    return DCMathSegmentMaker(name,extraFlags,**kwargs)

# end of factory function DCMathSegmentMaker

def MuonLayerHoughTool(name='MuonLayerHoughTool', **kwargs):
    if ConfigFlags.Muon.MuonTrigger:
        kwargs.setdefault("DoTruth", False)
    else:
        kwargs.setdefault("DoTruth", rec.doTruth())

    return CfgMgr.Muon__MuonLayerHoughTool(name,**kwargs)

def MuonSegmentFittingTool(name='MuonSegmentFittingTool',extraFlags=None,**kwargs):
    prop = getPublicTool('AtlasRungeKuttaPropagator')
    kwargs.setdefault("SLPropagator", getPrivateToolClone('SLPropagator',prop) )
    # Think I need to do it this way because AtlasRungeKuttaPropagator isn't known to ConfigurableFactory.
    # If I directly call getPrivateTool('AtlasRungeKuttaPropagator') then it fails with:
    # ConfigurationError: Private Tool <Trk::RungeKuttaPropagator/ToolSvc.AtlasRungeKuttaPropagator at 0x7f5811db3158> not found
    kwargs.setdefault("SLFitter",     getPrivateTool('MCTBSLFitter') )
    kwargs.setdefault("CurvedFitter", getPrivateTool('MCTBFitter') )
    kwargs.setdefault("TrackCleaner", getPrivateTool('MuonTrackCleaner')  )
    return CfgMgr.Muon__MuonSegmentFittingTool(name,**kwargs)

def getMuonToolSafe(name, isPublic):
    '''Helper function for creating a public tool, if it's available

    In order to make it possible to use this python module in projects in
    which not every single reconstruction tool is available, this function
    only instantiates the ones that can actually be instantiated.

    Parameter(s):
       name     -- The type (and instance name) of the public tool to create
       isPublic -- Flag specifying whether the tool is public (or private)
    '''

    # If the tool is not available, don't do anything.
    from AthenaCommon.ConfigurableDb import getConfigurable
    if not getConfigurable(name):
        return

    # If it *is* available, leave the heavy lifting to the "unsafe" functions.
    if isPublic:
        getPublicTool(name)
    else:
        getPrivateTool(name)
        pass
    return

if DetFlags.detdescr.Muon_on() and rec.doMuon():
    # until all clients explicitly get their tools and services, load some explicitly
    getMuonToolSafe("ResidualPullCalculator", True)
    getMuonToolSafe("MuonHoughPatternTool", True)
    getMuonToolSafe("MuonCombinePatternTool", True)
    getMuonToolSafe("MuonPhiHitSelector", True)
    getMuonToolSafe("MuonEDMPrinterTool", True)
    getMuonToolSafe("MuonSegmentMomentum", True)
    getMuonToolSafe("MuonClusterOnTrackCreator", True)
    if MuonGeometryFlags.hasCSC() and muonRecFlags.doCSCs():
        getMuonToolSafe("CscClusterOnTrackCreator", False)
        getMuonToolSafe("CscBroadClusterOnTrackCreator", False)
    getMuonToolSafe("MdtDriftCircleOnTrackCreator", True)
    getMuonToolSafe("MdtTubeHitOnTrackCreator", True)
else: # not (DetFlags.Muon_on() and rec.doMuon())
    logMuon.warning("Muon reconstruction tools only loaded on-demand because Muons")

def MuonLayerSegmentFinderTool(name='MuonLayerSegmentFinderTool',extraFlags=None,**kwargs):
    kwargs.setdefault("Csc2DSegmentMaker", getPublicTool("Csc2dSegmentMaker") if muonRecFlags.doCSCs() and MuonGeometryFlags.hasCSC() else "")
    kwargs.setdefault("Csc4DSegmentMaker", getPublicTool("Csc4dSegmentMaker") if muonRecFlags.doCSCs() and MuonGeometryFlags.hasCSC() else "")
    kwargs.setdefault("MuonPRDSelectionTool", getPublicTool("MuonPRDSelectionTool") )
    if ConfigFlags.Muon.MuonTrigger:
        kwargs.setdefault("InSegmentContainer", "")
    return CfgMgr.Muon__MuonLayerSegmentFinderTool(name,**kwargs)

def ExtraTreeTrackFillerTool(name="ExtraTreeTrackFillerTool",extraFlags=None,**kwargs):
    kwargs.setdefault("PullCalculator", getPublicTool("ResidualPullCalculator"))
    return CfgMgr.MuonCalib__ExtraTreeTrackFillerTool(name,**kwargs)