# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from ViewAlgs.ViewAlgsConf import EventViewCreatorAlgorithm
from DecisionHandling.DecisionHandlingConf import ViewCreatorInitialROITool
from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

def getTrigCosmicInDetTracking(flags):
    dataPrepInputMaker = EventViewCreatorAlgorithm("IMCosmicTrkEventViewCreator")
    dataPrepInputMaker.ViewFallThrough = True
    dataPrepInputMaker.RoITool = ViewCreatorInitialROITool()
    dataPrepInputMaker.InViewRoIs = "InputRoI" # contract with the consumer
    dataPrepInputMaker.Views = "CosmicViewRoIs"
    dataPrepInputMaker.RequireParentView = False

    idTrigConfig = getInDetTrigConfig('cosmics')
    from AthenaCommon.Logging import logging 
    log = logging.getLogger("CosmicInDetTracking")
    
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    cosmicflags = getFlagsForActiveConfig(flags, 'cosmics', log)
    
    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking
    dataPrepAlgs, verifier = makeInDetTrigFastTracking(cosmicflags, config=idTrigConfig,
                                     rois=dataPrepInputMaker.InViewRoIs, 
                                     viewVerifier='VDVCosmicsIDTracking', 
                                     doFTF=False) # no fast tracking, just data prep
    verifier.DataObjects += [('TrigRoiDescriptorCollection', 'StoreGateSvc+InputRoI')]

    from TrigInDetConfig.EFIDTracking import makeInDetPatternRecognition
    efidAlgs, verifierForEF = makeInDetPatternRecognition(cosmicflags, idTrigConfig, verifier='VDVCosmicsIDTracking')
    return   dataPrepInputMaker, [verifier,verifierForEF] + dataPrepAlgs + efidAlgs
