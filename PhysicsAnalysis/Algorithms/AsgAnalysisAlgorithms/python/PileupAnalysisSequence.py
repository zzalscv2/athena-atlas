# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool
from Campaigns.Utils import Campaign

try:
    from AthenaCommon.Logging import logging
except ImportError:
    import logging
log = logging.getLogger('makePileupAnalysisSequence')

def makePileupAnalysisSequence( dataType, campaign=None, files=None, useDefaultConfig=False, userLumicalcFiles=None, userPileupConfigs=None ):
    """Create a PRW analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
    """

    if dataType not in ["data", "mc", "afii"] :
        raise ValueError ("invalid data type: " + dataType)

    # Create the analysis algorithm sequence object:
    seq = AnaAlgSequence( "PileupAnalysisSequence" )

    # TODO: support per-campaign config

    toolConfigFiles = []
    toolLumicalcFiles = []
    if files is not None and (campaign is None or campaign is Campaign.Unknown or userPileupConfigs is None):
        if campaign is None or campaign is Campaign.Unknown:
            from Campaigns.Utils import getMCCampaign
            campaign = getMCCampaign(files)
            if campaign:
                log.info(f'Autoconfiguring PRW with campaign: {campaign}')
            else:
                log.info('Campaign could not be determined.')

        if campaign:
            if userPileupConfigs is None:
                from PileupReweighting.AutoconfigurePRW import getConfigurationFiles
                toolConfigFiles = getConfigurationFiles(campaign=campaign, files=files, useDefaultConfig=useDefaultConfig)
                log.info('Setting PRW configuration based on input files')

                if toolConfigFiles:
                    log.info(f'Using PRW configuration: {", ".join(toolConfigFiles)}')
            else:
                log.info('Using user provided PRW configuration')

    if userPileupConfigs is not None:
        toolConfigFiles = userPileupConfigs[:]

    if userLumicalcFiles is not None:
        log.info('Using user-provided lumicalc files')
        toolLumicalcFiles = userLumicalcFiles[:]
    else:
        from PileupReweighting.AutoconfigurePRW import getLumicalcFiles
        toolLumicalcFiles = getLumicalcFiles(campaign)

    # Set up the only algorithm of the sequence:
    alg = createAlgorithm( 'CP::PileupReweightingAlg', 'PileupReweightingAlg' )
    addPrivateTool( alg, 'pileupReweightingTool', 'CP::PileupReweightingTool' )
    alg.pileupReweightingTool.ConfigFiles = toolConfigFiles
    if not toolConfigFiles and dataType != "data":
        log.info("No PRW config files provided. Disabling reweighting")
        # Setting the weight decoration to the empty string disables the reweighting
        alg.pileupWeightDecoration = ""
    alg.pileupReweightingTool.LumiCalcFiles = toolLumicalcFiles

    seq.append( alg, inputPropName = {} )

    # Return the sequence:
    return seq
