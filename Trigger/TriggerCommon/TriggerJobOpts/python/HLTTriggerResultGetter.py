# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from AthenaConfiguration.Enums import Format

from RecExConfig.RecFlags import rec
from RecExConfig.ObjKeyStore import objKeyStore

_log = logging.getLogger("HLTTriggerResultGetter.py")

def HLTTriggerResultGetter(flags=None):
    if flags is None:
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        flags = ConfigFlags

    if flags.Input.Format is Format.BS and flags.Trigger.DecodeHLT:
        _log.info("Configuring BS unpacking")
        if flags.Trigger.EDMVersion in [1, 2]:
            from TriggerJobOpts.TriggerRecoConfig import Run1Run2BSExtractionCfg
            CAtoGlobalWrapper(Run1Run2BSExtractionCfg, flags)
        elif flags.Trigger.EDMVersion >= 3:
            from TriggerJobOpts.TriggerRecoConfig import Run3TriggerBSUnpackingCfg
            CAtoGlobalWrapper(Run3TriggerBSUnpackingCfg, flags)
        else:
            raise RuntimeError("Invalid EDMVersion=%s " % flags.Trigger.EDMVersion)

    if flags.Trigger.EDMVersion in [1, 2]:
        if rec.doTrigger() and ( (rec.doWriteESD() or rec.doWriteAOD() or rec.doESD() or rec.doAOD()) and
                                    (not (rec.readAOD() or rec.readESD() or rec.doWriteBS())) ):
            from TrigDecisionMaker.TrigDecisionMakerConfig import Run1Run2DecisionMakerCfg
            CAtoGlobalWrapper(Run1Run2DecisionMakerCfg, flags)
            _log.info('Run-1&2 xTrigDecision writing enabled')

    elif flags.Trigger.EDMVersion >= 3:
        if flags.Input.Format is Format.BS:
            from TrigDecisionMaker.TrigDecisionMakerConfig import Run3DecisionMakerCfg
            CAtoGlobalWrapper( Run3DecisionMakerCfg, flags)
            _log.info('Run-3 xTrigDecision writing enabled')
    else:
        raise RuntimeError("Invalid EDMVersion=%s " % flags.Trigger.EDMVersion)

    # TrigJetRec additions
    if rec.doWriteESD():
        objKeyStore.addStreamESD("JetKeyDescriptor","JetKeyMap")
        objKeyStore.addStreamESD("JetMomentMap","TrigJetRecMomentMap")

    if rec.doWriteAOD():
        objKeyStore.addStreamAOD("JetKeyDescriptor","JetKeyMap")
        objKeyStore.addStreamAOD("JetMomentMap","TrigJetRecMomentMap")

    # schedule the RoiDescriptorStore conversion
    if rec.doAOD() or rec.doWriteAOD():
        from TrigRoiConversion.TrigRoiConversionConfig import RoiWriterCfg
        CAtoGlobalWrapper(RoiWriterCfg, flags)

        # output needs to be configured explicitly even if done in CA already
        from TrigEDMConfig.TriggerEDMRun2 import TriggerRoiList
        objKeyStore.addManyTypesStreamAOD( TriggerRoiList )


    # ESD objects definitions
    # for Run-1 we are storing the converted Run-2 xAOD types
    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    edmVersion = max(2, flags.Trigger.EDMVersion)
    _TriggerESDList = getTriggerEDMList(flags.Trigger.ESDEDMSet, edmVersion)

    _log.info("ESD content set according to the ESDEDMSet flag: %s and EDM version %d", flags.Trigger.ESDEDMSet, flags.Trigger.EDMVersion)

    # AOD objects choice
    _TriggerAODList = getTriggerEDMList(flags.Trigger.AODEDMSet,  flags.Trigger.EDMVersion)

    _log.info("AOD content set according to the AODEDMSet flag: %s and EDM version %d", flags.Trigger.AODEDMSet, flags.Trigger.EDMVersion)

    _log.debug("ESD EDM list: %s", _TriggerESDList)
    _log.debug("AOD EDM list: %s", _TriggerAODList)
    
    # Highlight what is in AOD list but not in ESD list, as this can cause
    # the "different number of entries in branch" problem, when it is in the
    # AOD list but the empty container per event is not created
    # Just compares keys of dicts, which are the class names, not their string keys in StoreGate
    not_in = [ element for element in  _TriggerAODList if element not in _TriggerESDList ]
    if (len(not_in)>0):
        _log.warning("In AOD list but not in ESD list: ")
        _log.warning(not_in)
    else:
        _log.info("AOD list is subset of ESD list - good.")


    def _addSlimmingRun2(stream, edm):
        from TrigNavTools.TrigNavToolsConfig import navigationThinningSvc

        edmlist = list(y.split('-')[0] for x in edm.values() for y in x) #flatten names
        
        # TimM Sep 2021: In MT the 'reload' slimming option in the R2 navigation thinning service was found to be creating
        # AODs which would crash when trying to return features. We therefore remove this option by using the added 'cleanup_noreload'
        # configuration, see ATR-24141 for details. 
        svc = navigationThinningSvc ({'name':'HLTNav_%s'%stream, 'mode':'cleanup_noreload', 
                                        'result':'HLTResult_HLT',
                                        'features':edmlist})

        from OutputStreamAthenaPool.CreateOutputStreams import registerTrigNavThinningSvc
        registerTrigNavThinningSvc (stream, svc)

        _log.info("Configured slimming of HLT for %s with %s", stream, svc)


    if flags.Trigger.EDMVersion in [1, 2]:

        # Run 1, 2 slimming
        if flags.Trigger.doNavigationSlimming and rec.readRDO() and rec.doWriteAOD():
            _addSlimmingRun2('StreamAOD', _TriggerESDList ) #Use ESD item list also for AOD!
            _log.info("configured navigation slimming for AOD output")
            
        if flags.Trigger.doNavigationSlimming and rec.readRDO() and rec.doWriteESD():
            _addSlimmingRun2('StreamESD', _TriggerESDList )                
            _log.info("configured navigation slimming for ESD output")
        if flags.Trigger.doEDMVersionConversion and 'HLTNav_R2ToR3Summary' not in flags.Input.Collections:
            from TrigNavTools.NavConverterConfig import NavConverterCfg
            CAtoGlobalWrapper( NavConverterCfg, flags)
            _log.info("Configured Run 1/3 -> Run3 Navigation conversion")

    if flags.Trigger.EDMVersion >= 3:
        # Change in the future to 'if EDMVersion >= 3 or doEDMVersionConversion:'

        # Run 3 slimming
        if flags.Trigger.doNavigationSlimming and flags.Trigger.DecodeHLT: 
            from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import TrigNavSlimmingMTCfg
            CAtoGlobalWrapper(TrigNavSlimmingMTCfg, flags)
        else:
            _log.info("doNavigationSlimming or DecodeHLT is False, won't schedule run 3 trigger HLT navigation slimming")

    # This is the relevant ItemList if we are running in mixed old/new-style job options.
    # While some of the CA-fragments above do call addToESD/AOD as well, these calls are no-ops
    # because flags.Output.doWriteESD/AOD is always False when running in this sceneario.
    # So for the moment we need to maintain the list of output conainters both here and in the CA-framents.
    # See discussion on ATR-25220.
    objKeyStore.addManyTypesStreamESD( _TriggerESDList )
    objKeyStore.addManyTypesStreamAOD( _TriggerAODList )        
        
    return True
