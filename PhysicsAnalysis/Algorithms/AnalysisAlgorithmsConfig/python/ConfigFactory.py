# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# This file defines a factory method that can create a configuration
# block sequence based on a passed in name.  This avoids having to
# import all the various config block sequence makers in the
# configuration code, and also would make it easier to create them
# from a text configuration file.

# This relies heavily on the blocks exposing all configurable
# parameters as options, since there is no other mechanism to
# configure them through this interface.

# The implementation itself is probably not the best possible, it
# lacks all extensibility, gathers all information in a single place,
# etc.  Still for now (08 Dec 22) this ought to be good enough.

from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence

def makeConfig (factoryName, groupName,
                *, jetCollection = None) :

    if jetCollection is not None and factoryName != 'Jets' :
        raise ValueError ('specifying jetCollection only allowed for Jets factory, not: ' + factoryName)

    configSeq = ConfigSequence ()

    if factoryName == 'Muons' :
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonCalibrationConfig
        makeMuonCalibrationConfig (configSeq, groupName)

    elif factoryName == 'Muons.Selection' :
        from MuonAnalysisAlgorithms.MuonAnalysisConfig import makeMuonWorkingPointConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makeMuonWorkingPointConfig (configSeq, groupSplit[0], workingPoint=None, selectionName=groupSplit[1])


    elif factoryName == 'Electrons' :
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import makeElectronCalibrationConfig
        makeElectronCalibrationConfig (configSeq, groupName)

    elif factoryName == 'Electrons.Selection' :
        from EgammaAnalysisAlgorithms.ElectronAnalysisConfig import makeElectronWorkingPointConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makeElectronWorkingPointConfig (configSeq, groupSplit[0], workingPoint=None, selectionName=groupSplit[1])


    elif factoryName == 'Photons' :
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import makePhotonCalibrationConfig
        makePhotonCalibrationConfig (configSeq, groupName)

    elif factoryName == 'Photons.Selection' :
        from EgammaAnalysisAlgorithms.PhotonAnalysisConfig import makePhotonWorkingPointConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makePhotonWorkingPointConfig (configSeq, groupSplit[0], workingPoint=None, selectionName=groupSplit[1])


    elif factoryName == 'TauJets' :
        from TauAnalysisAlgorithms.TauAnalysisConfig import makeTauCalibrationConfig
        makeTauCalibrationConfig (configSeq, groupName)

    elif factoryName == 'TauJets.Selection' :
        from TauAnalysisAlgorithms.TauAnalysisConfig import makeTauWorkingPointConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makeTauWorkingPointConfig (configSeq, groupSplit[0], workingPoint=None, selectionName=groupSplit[1])


    elif factoryName == 'Jets' :
        if jetCollection is None :
            raise ValueError ('need to specify jetCollection for Jets configuration')
        from JetAnalysisAlgorithms.JetAnalysisConfig import makeJetAnalysisConfig
        makeJetAnalysisConfig( configSeq, groupName, jetCollection)

    elif factoryName == 'Jets.Jvt' :
        from JetAnalysisAlgorithms.JetJvtAnalysisConfig import makeJetJvtAnalysisConfig
        makeJetJvtAnalysisConfig( configSeq, groupName )


    elif factoryName.startswith ('FlavorTagging') :
        raise ValueError ('You requested a FlavorTagging factory, but the name was changed to FlavourTagging')
    elif factoryName == 'FlavourTagging' :
        from FTagAnalysisAlgorithms.FTagAnalysisConfig import makeFTagAnalysisConfig
        groupSplit = groupName.split ('.')
        if len (groupSplit) != 2 or groupSplit[0] == '' or groupSplit[1] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        makeFTagAnalysisConfig( configSeq, groupSplit[0], selectionName = groupSplit[1])


    elif factoryName == 'MissingET' :
        from MetAnalysisAlgorithms.MetAnalysisConfig import makeMetAnalysisConfig

        makeMetAnalysisConfig (configSeq, containerName = groupName)


    elif factoryName == 'OverlapRemoval' :
        from AsgAnalysisAlgorithms.OverlapAnalysisConfig import \
            makeOverlapAnalysisConfig
        makeOverlapAnalysisConfig( configSeq )


    elif factoryName == 'Event.PileupReweighting' :
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import \
            makePileupReweightingConfig
        makePileupReweightingConfig (configSeq)

    elif factoryName == 'Event.Cleaning' :
        # Skip events with no primary vertex:
        from AsgAnalysisAlgorithms.EventCleaningConfig import \
            makeEventCleaningConfig
        makeEventCleaningConfig (configSeq)

    elif factoryName == 'Event.Generator' :
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import \
            makeGeneratorAnalysisConfig
        makeGeneratorAnalysisConfig( configSeq )


    elif factoryName == 'Trigger.Chains' :
        from TriggerAnalysisAlgorithms.TriggerAnalysisConfig import \
            makeTriggerAnalysisConfig
        makeTriggerAnalysisConfig( configSeq )


    elif factoryName == 'Selection.PtEta' :
        groupSplit = groupName.split ('.')
        if len (groupSplit) == 0 or len (groupSplit) > 2 or groupSplit[0] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)

        if len (groupSplit) == 2 :
            selection = groupSplit[1]
        else :
            selection = ''
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makePtEtaSelectionConfig
        makePtEtaSelectionConfig (configSeq, groupSplit[0],
                                  selectionName=selection)


    elif factoryName == 'Selection.ObjectCutFlow' :
        groupSplit = groupName.split ('.')
        if len (groupSplit) == 0 or len (groupSplit) > 2 or groupSplit[0] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)

        if len (groupSplit) == 2 :
            selection = groupSplit[1]
        else :
            selection = ''
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeObjectCutFlowConfig
        makeObjectCutFlowConfig (configSeq, groupSplit[0],
                                  selectionName=selection)


    elif factoryName == 'Output.Thinning' :
        groupSplit = groupName.split ('.')
        if len (groupSplit) == 0 or len (groupSplit) > 2 or groupSplit[0] == '' :
            raise ValueError ('invalid groupName for ' + factoryName + ': ' + groupName)
        if len (groupSplit) == 2 :
            configName = groupSplit[1]
        else :
            configName = ''
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeOutputThinningConfig
        makeOutputThinningConfig (configSeq, groupSplit[0], configName = configName)


    elif factoryName == 'Output.Simple' :
        if groupName == '' :
            groupName = 'Output'
        from AsgAnalysisAlgorithms.OutputAnalysisConfig import OutputAnalysisConfig
        config = OutputAnalysisConfig (groupName)
        configSeq.append (config)

    elif factoryName == 'CommonServices' :
        from AsgAnalysisAlgorithms.AsgAnalysisConfig import makeCommonServicesConfig
        makeCommonServicesConfig (configSeq)

    elif factoryName == 'SystObjectLink':
        try:
            groupSplit = groupName.split('.')
            assert len(groupSplit) == 2
            assert groupSplit[0] != ''
            containerName = groupSplit[1]
        except AssertionError:
            raise ValueError(f'invalid groupName for {factoryName}: {groupName}')
            
        from AsgAnalysisAlgorithms.SystObjectLinkConfig import makeSystObjectLinkConfig
        makeSystObjectLinkConfig(configSeq, containerName)

    elif factoryName == 'Bootstraps':
        from AsgAnalysisAlgorithms.BootstrapGeneratorConfig import makeBootstrapGeneratorConfig
        makeBootstrapGeneratorConfig(configSeq)

    else :
        raise ValueError ('unknown factory: ' + factoryName)


    if groupName is not None :
        for config in configSeq :
            if config.groupName() != groupName :
                raise Exception ("inconsistent group names: " + config.groupName() + " " + groupName)

    return configSeq
