#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#           Setup of precision tracking

from AthenaCommon.Logging import logging
log = logging.getLogger("InDetTrigPrecisionTracking")

from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPublicToolWrapper
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper

def makeInDetTrigPrecisionTracking( inflags, config=None, verifier=False, rois='EMViewRoIs', prefix="InDetTrigMT" ) :
    
    log.info( "makeInDetTrigPrecisionTracking:: {} {} doTRT: {} ".format(  config.input_name, config.name, config.doTRT ) )

    ptAlgs = [] # List containing all the precision tracking algorithms hence every new added alg has to be appended to the list
    
    # Expects configuration  
    if config is None:
      raise ValueError('PrecisionTracking No configuration provided!')

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flags = getFlagsForActiveConfig(inflags, config.input_name, log)

    from InDetTrigRecExample import InDetTrigCA
    InDetTrigCA.InDetTrigConfigFlags = flags
    
    from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg
    summaryTool = CAtoLegacyPublicToolWrapper(InDetTrigTrackSummaryToolCfg)

    doTRT = flags.Tracking.ActiveConfig.doTRT

    # Add suffix to the algorithms
    signature =  "_{}".format( flags.Tracking.ActiveConfig.input_name )
    
    # Name settings for output Tracks/TrackParticles
    outTrkTracks        = flags.Tracking.ActiveConfig.trkTracks_IDTrig # Final output Track collection
    outTrackParticles   = flags.Tracking.ActiveConfig.tracks_IDTrig # Final output xAOD::TrackParticle
    ambiTrackCollection = outTrkTracks+"_Amb"  # Ambiguity solver tracks
    
    #  Verifying input data for the algorithms
  
    # If run in views need to check data dependancies!
    # NOTE: this seems necessary only when PT is called from a different view than FTF otherwise causes stalls
    if verifier:
        from .InDetTrigCollectionKeys import TrigPixelKeys
        verifier.DataObjects += [( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+' + TrigPixelKeys.PixelClusterAmbiguitiesMap ),
                                 ( 'TrackCollection' , 'StoreGateSvc+' + flags.Tracking.ActiveConfig.trkTracks_FTF )]

    
    ambiSolvingAlgs = ambiguitySolver_builder( signature, config, summaryTool, outputTrackName=ambiTrackCollection, prefix=prefix+"Trk" )

    #Loading the alg to the sequence
    ptAlgs.extend( ambiSolvingAlgs )

    finalTrackCollection = ambiTrackCollection
    if doTRT:
        # do the TRT extension if requested
        finalTrackCollection = outTrkTracks
        trtAlgs = trtExtension_builder(flags, signature, config, rois, summaryTool, inputTracks=ambiTrackCollection, outputTracks=outTrkTracks, prefix=prefix ) 
        ptAlgs.extend( trtAlgs )

        
    #  Track particle conversion algorithm

    from .InDetTrigCommon import trackParticleCnv_builder
    from InDetTrigRecExample.InDetTrigConfigRecLoadToolsPost import InDetTrigParticleCreatorTool, \
        InDetTrigParticleCreatorToolTRTPid

    creatorTool = InDetTrigParticleCreatorTool
    if config.electronPID:
      creatorTool = InDetTrigParticleCreatorToolTRTPid
    
    trackParticleCnvAlg = trackParticleCnv_builder( 
        flags,
        name                 = prefix+'xAODParticleCreatorAlg'+flags.Tracking.ActiveConfig.input_name+'_IDTrig',
        config               = config,
        inTrackCollectionKey = finalTrackCollection,
        outTrackParticlesKey = outTrackParticles,
        trackParticleCreatorTool     =  creatorTool)
    
    log.debug(trackParticleCnvAlg)
    ptAlgs.append(trackParticleCnvAlg)
    
    # Potentialy other algs with more collections? 
    # Might Drop the list in the end and keep just one output key
    nameTrackCollections =[ outTrkTracks ]
    nameTrackParticles =  [ outTrackParticles ]
    
    # Return list of Track keys, TrackParticle keys, and PT algs
    return  nameTrackCollections, nameTrackParticles, ptAlgs




# top level alg

def ambiguitySolver_builder( signature, config, summaryTool, outputTrackName=None, prefix="InDetTrigMT" ) :

    log.info( "Precision tracking using new configuration: {} {} {} {}".format(  signature, config.input_name, config.name, prefix ) )

    scoreMap        = 'ScoreMap'+config.input_name
    if config.usePixelNN:
        ambiguityScore  = ambiguityScoreNN_builder( signature, config, scoreMap, prefix )
    else:
        ambiguityScore  = ambiguityScore_builder( signature, config, scoreMap, prefix )
    ambiguitySolver = ambiguitySolverInternal_builder( signature, config, summaryTool, scoreMap, outputTrackName, prefix )

    return [ ambiguityScore, ambiguitySolver ]



# next level alg

def ambiguityScore_builder( signature, config, scoreMap, prefix=None ):
    
    from TrkAmbiguitySolver.TrkAmbiguitySolverConf import Trk__TrkAmbiguityScore
    ambiguityScore = Trk__TrkAmbiguityScore( name                    = '%sAmbiguityScore_%s'%(prefix, config.input_name),
                                             TrackInput              = [ config.trkTracks_FTF() ],
                                             TrackOutput             = scoreMap,
                                             AmbiguityScoreProcessor = None ) 
         
    log.info(ambiguityScore)
    
    return ambiguityScore 



def ambiguityScoreNN_builder( signature, config, scoreMap, prefix=None ):
    MultiplicityContent = [1 , 1 , 1]
    from AthenaCommon.CfgGetter import getPublicTool
    from InDetRecExample import TrackingCommon as TrackingCommon
    from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags
    from AthenaCommon.AppMgr import ToolSvc
    from InDetRecExample.TrackingCommon import createAndAddCondAlg,getPixelClusterNnCondAlg,getPixelClusterNnWithTrackCondAlg
    createAndAddCondAlg( getPixelClusterNnCondAlg,         'PixelClusterNnCondAlg',          GetInputsInfo = False)
    createAndAddCondAlg( getPixelClusterNnWithTrackCondAlg,'PixelClusterNnWithTrackCondAlg', GetInputsInfo = False)
    TrigPixelLorentzAngleTool = getPublicTool("PixelLorentzAngleTool")
    TrigNnClusterizationFactory = TrackingCommon.getNnClusterizationFactory( name                  = "%sTrigNnClusterizationFactory_%s"%(prefix, config.input_name),
                                                                      PixelLorentzAngleTool        = TrigPixelLorentzAngleTool,
                                                                      useToT                       = InDetTrigFlags.doNNToTCalibration(),
                                                                      NnCollectionReadKey          = 'PixelClusterNN',
                                                                      NnCollectionWithTrackReadKey = 'PixelClusterNNWithTrack',
                                                                      useTTrainedNetworks          = True)
    from SiClusterizationTool.SiClusterizationToolConf import InDet__NnPixelClusterSplitProbTool as PixelClusterSplitProbTool
    TrigNnPixelClusterSplitProbTool=PixelClusterSplitProbTool(name       = "%sTrigNnPixelClusterSplitProbTool_%s"%(prefix, config.input_name),
                                                                PriorMultiplicityContent = MultiplicityContent,
                                                                NnClusterizationFactory  = TrigNnClusterizationFactory,
                                                                useBeamSpotInfo          = True)
    ToolSvc += TrigNnPixelClusterSplitProbTool
    from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__DenseEnvironmentsAmbiguityScoreProcessorTool
    trackMapTool = TrackingCommon.getInDetTrigPRDtoTrackMapToolGangedPixels()
    scoringTool = scoringTool_builder( signature, config, prefix=None )
    ambiguityScoreProcessor = Trk__DenseEnvironmentsAmbiguityScoreProcessorTool( name               = "%sInDetTrigMT_AmbiguityScoreProcessorTool_%s"%(prefix, config.input_name),
                                                                 ScoringTool        = scoringTool,
                                                                 AssociationTool    = trackMapTool,
                                                                 SplitProbTool      = TrigNnPixelClusterSplitProbTool
                                                                )
    from TrkAmbiguitySolver.TrkAmbiguitySolverConf import Trk__TrkAmbiguityScore
    ambiguityScore = Trk__TrkAmbiguityScore( name                    = '%sAmbiguityScore_%s'%(prefix, config.input_name),
                                             TrackInput              = [ config.trkTracks_FTF() ],
                                             TrackOutput             = scoreMap,
                                             AmbiguityScoreProcessor = ambiguityScoreProcessor )
    log.info(ambiguityScore)
    return ambiguityScore
# next level alg

def ambiguitySolverInternal_builder( signature, config, summaryTool, scoreMap, outputTrackName=None, prefix=None ):
    if config.usePixelNN: 
        ambiguityProcessorTool = ambiguityProcessorToolNN_builder( signature, config, summaryTool, prefix )
    else:
        ambiguityProcessorTool = ambiguityProcessorTool_builder( signature, config, summaryTool, prefix )
    
    from TrkAmbiguitySolver.TrkAmbiguitySolverConf import Trk__TrkAmbiguitySolver
    ambiguitySolver = Trk__TrkAmbiguitySolver( name               = '%sAmbiguitySolver_%s'%(prefix,config.input_name),
                                               TrackInput         = scoreMap,
                                               TrackOutput        = outputTrackName, 
                                               AmbiguityProcessor = ambiguityProcessorTool )
    
    
    return ambiguitySolver




def ambiguityProcessorTool_builder( signature, config, summaryTool ,prefix=None ) : 

    from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTrackFitter
    from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigAmbiTrackSelectionTool

    from InDetRecExample import TrackingCommon as TrackingCommon
    trackMapTool = TrackingCommon.getInDetTrigPRDtoTrackMapToolGangedPixels()

    scoringTool = scoringTool_builder( signature, config, prefix )
    
    from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__SimpleAmbiguityProcessorTool 
    ambiguityProcessorTool = Trk__SimpleAmbiguityProcessorTool( name             = '%sAmbiguityProcessor_%s'%(prefix,config.input_name),
                                                                Fitter           = InDetTrigTrackFitter,
                                                                ScoringTool      = scoringTool,
                                                                AssociationTool  = trackMapTool,
                                                                TrackSummaryTool = summaryTool,
                                                                SelectionTool    = InDetTrigAmbiTrackSelectionTool )
    
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += ambiguityProcessorTool
    
    return ambiguityProcessorTool

def ambiguityProcessorToolNN_builder( signature, config, summaryTool ,prefix=None ) : 

    from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTrackFitter
    from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigAmbiTrackSelectionTool

    from InDetRecExample import TrackingCommon as TrackingCommon
    trackMapTool = TrackingCommon.getInDetTrigPRDtoTrackMapToolGangedPixels()

    scoringTool = scoringTool_builder( signature, config, prefix )
   
    InDetTrigAmbiTrackSelectionTool.doPixelSplitting=True 
    from TrkAmbiguityProcessor.TrkAmbiguityProcessorConf import Trk__DenseEnvironmentsAmbiguityProcessorTool as ProcessorTool
    ambiguityProcessorTool = ProcessorTool( name             = '%sDenseEnvironmentsAmbiguityProcessor_%s'%(prefix,config.input_name),
                                                                Fitter           = [InDetTrigTrackFitter],
                                                                ScoringTool      = scoringTool,
                                                                AssociationTool  = trackMapTool,
                                                                TrackSummaryTool = summaryTool,
                                                                SelectionTool    = InDetTrigAmbiTrackSelectionTool,
                                                                SuppressHoleSearch = False,
                                                                tryBremFit         = False,
                                                                caloSeededBrem     = False,
                                                                RefitPrds          = True,
                                                                pTminBrem          = 1000.0 )
    
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += ambiguityProcessorTool
    
    return ambiguityProcessorTool



def scoringTool_builder( signature, config, prefix=None, SiOnly=True ):

  from InDetRecExample.TrackingCommon import setDefaults

  kwargs = {}

  if SiOnly:
      scoringToolName = '%sScoringTool_%s'%( prefix, config.input_name)
      InDetTrigTRTDriftCircleCut = None
  else:
      scoringToolName = '%sExtScoringTool_%s'%( prefix, config.input_name)
      from InDetTrigRecExample.InDetTrigConfigRecLoadTools import InDetTrigTRTDriftCircleCut

  kwargs = setDefaults(kwargs, 
                       name = scoringToolName,
                       minPt        = config.pTmin, 
                       doEmCaloSeed = False,
                       minTRTonTrk        = 0,          #2023fix  (switch to flags)
                       DriftCircleCutTool = InDetTrigTRTDriftCircleCut)
  
  if(config.maxRPhiImpact is not None):
    kwargs = setDefaults(kwargs, maxRPhiImp = config.maxRPhiImpact)

  if(config.maxRPhiImpactEM is not None):
    kwargs = setDefaults(kwargs, maxRPhiImpEM = config.maxRPhiImpactEM)

  if(config.maxZImpact is not None):
    kwargs = setDefaults(kwargs, maxZImp = config.maxZImpact)
    
  if(config.maxEta is not None):
    kwargs = setDefaults(kwargs, maxEta = config.maxEta)

  if(config.minSiClusters is not None):
    kwargs = setDefaults(kwargs, minSiClusters = config.minSiClusters)

  if(config.maxSiHoles is not None):
    kwargs = setDefaults(kwargs, maxSiHoles = config.maxSiHoles)

  if(config.maxPixelHoles is not None):
    kwargs = setDefaults(kwargs, maxPixelHoles = config.maxPixelHoles)

  if(config.maxSCTHoles is not None):
    kwargs = setDefaults(kwargs, maxSCTHoles = config.maxSCTHoles)

  if(config.maxDoubleHoles is not None):
    kwargs = setDefaults(kwargs, maxDoubleHoles = config.maxDoubleHoles)

  if(config.usePixel is not None):
    kwargs = setDefaults(kwargs, usePixel = config.usePixel)

  if(config.useSCT is not None):
    kwargs = setDefaults(kwargs, useSCT = config.useSCT)

  if(config.doEmCaloSeed is not None):
    kwargs = setDefaults(kwargs, doEmCaloSeed = config.doEmCaloSeed)

  from InDetConfig.InDetTrackScoringToolsConfig import InDetAmbiScoringToolCfg
  scoringTool = CAtoLegacyPublicToolWrapper(InDetAmbiScoringToolCfg, **kwargs)
  return scoringTool


def trtExtension_builder(flags, signature, config, rois, summaryTool, inputTracks, outputTracks, prefix="InDetTrigMT" ): 

        
    trtRIOMaker           = trtRIOMaker_builder(flags, signature, config, rois, prefix  )
    trtExtensionAlg       = trtExtensionAlg_builder(flags, signature, config, inputTracks, prefix =prefix)
    trtExtensionProcessor = trtExtensionProcessor_builder(flags, signature, config, summaryTool, inputTracks, outputTracks, prefix )

    return [ trtRIOMaker, trtExtensionAlg, trtExtensionProcessor]
    

def trtRIOMaker_builder(inflags, signature, config, rois, prefix="InDetTrigMT" ): 
    
    log.info( "trtRIOMaker_builder: {} {}".format( signature, prefix ) )

    algs = []

    TRT_RDO_Key = "TRT_RDOs"

    from .InDetTrigCollectionKeys import TrigTRTKeys
    from AthenaCommon.GlobalFlags import globalflags

    #TODO invoke as photon from the menu
    if signature=="electrontrt":
      flags = inflags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking."+"photon")
    else:
      flags = inflags

    # Only add raw data decoders if we're running over raw data
    if globalflags.InputFormat.is_bytestream():
        #Global keys/names for collections 
        TRT_RDO_Key = TrigTRTKeys.RDOs
        trtDataProvider = trtDataProvider_builder(flags, signature, config, TRT_RDO_Key, rois )
        algs.append( trtDataProvider )
        

    from InDetConfig.InDetPrepRawDataFormationConfig import TrigTRTRIOMakerCfg
    trtRIOMaker = algorithmCAToGlobalWrapper(TrigTRTRIOMakerCfg,
                                             flags, RoIs = rois)
    algs.extend( trtRIOMaker )
    return algs
    


def trtDataProvider_builder(flags, signature, config, TRT_RDO_Key, rois, prefix="InDetTrigMT" ) :

    
    from TrigInDetConfig.TrigInDetConfig import TRTDataProviderCfg
    trtRawDataProvider = algorithmCAToGlobalWrapper(TRTDataProviderCfg,
                                                    flags, rois, signature)
    return trtRawDataProvider[0]

    
def trtExtensionAlg_builder(flags, signature, config, inputTracks, prefix="InDetTrigMT" ): 

    from InDetConfig.TRT_TrackExtensionAlgConfig import Trig_TRT_TrackExtensionAlgCfg
    extensionAlg = algorithmCAToGlobalWrapper(Trig_TRT_TrackExtensionAlgCfg,
                                              flags, inputTracks, name="%sTrackExtensionAlg%s"%(prefix, signature),
                                              )
    return extensionAlg[0]


def trtExtensionProcessor_builder(flags, signature, config, summaryTool, inputTracks, outputTracks, prefix="InDetTrigMT" ):   


    from InDetConfig.InDetExtensionProcessorConfig import TrigInDetExtensionProcessorCfg
    trtExtensionProcessor = algorithmCAToGlobalWrapper(TrigInDetExtensionProcessorCfg,
                                                       flags, name="%sExtensionProcessor%s"%(prefix, signature),
                                                       )
    return trtExtensionProcessor[0]





