# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class PileupReweightingBlock (ConfigBlock):
    """the ConfigBlock for pileup reweighting"""

    def __init__ (self) :
        super (PileupReweightingBlock, self).__init__ ()
        self.campaign=None
        self.files=None
        self.useDefaultConfig=False
        self.userLumicalcFiles=None
        self.userPileupConfigs=None


    def makeAlgs (self, config) :

        from Campaigns.Utils import Campaign

        try:
            from AthenaCommon.Logging import logging
        except ImportError:
            import logging
        log = logging.getLogger('makePileupAnalysisSequence')

        # TODO: support per-campaign config

        toolConfigFiles = []
        toolLumicalcFiles = []
        campaign = self.campaign
        if self.files is not None and (campaign is None or campaign is Campaign.Unknown or self.userPileupConfigs is None):
            if campaign is None or campaign is Campaign.Unknown:
                from Campaigns.Utils import getMCCampaign
                campaign = getMCCampaign(self.files)
                if campaign:
                    log.info(f'Autoconfiguring PRW with campaign: {campaign}')
                else:
                    log.info('Campaign could not be determined.')

            if campaign:
                if self.userPileupConfigs is None:
                    from PileupReweighting.AutoconfigurePRW import getConfigurationFiles
                    toolConfigFiles = getConfigurationFiles(campaign=campaign, files=self.files, useDefaultConfig=self.useDefaultConfig)
                    log.info('Setting PRW configuration based on input files')

                    if toolConfigFiles:
                        log.info(f'Using PRW configuration: {", ".join(toolConfigFiles)}')
                else:
                    log.info('Using user provided PRW configuration')

        if self.userPileupConfigs is not None:
            toolConfigFiles = self.userPileupConfigs[:]

        if self.userLumicalcFiles is not None:
            log.info('Using user-provided lumicalc files')
            toolLumicalcFiles = self.userLumicalcFiles[:]
        else:
            from PileupReweighting.AutoconfigurePRW import getLumicalcFiles
            toolLumicalcFiles = getLumicalcFiles(campaign)

        # Set up the only algorithm of the sequence:
        alg = config.createAlgorithm( 'CP::PileupReweightingAlg', 'PileupReweightingAlg' )
        config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
        alg.pileupReweightingTool.ConfigFiles = toolConfigFiles
        if not toolConfigFiles and dataType != "data":
            log.info("No PRW config files provided. Disabling reweighting")
            # Setting the weight decoration to the empty string disables the reweighting
            alg.pileupWeightDecoration = ""
        alg.pileupReweightingTool.LumiCalcFiles = toolLumicalcFiles



class PtEtaSelectionBlock (ConfigBlock):
    """the ConfigBlock for a pt-eta selection"""

    def __init__ (self, containerName, *, postfix, minPt, maxEta,
                  selectionDecoration) :
        super (PtEtaSelectionBlock, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        self.minPt = minPt
        self.maxEta = maxEta
        self.selectionDecoration = selectionDecoration

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        alg = config.createAlgorithm( 'CP::AsgSelectionAlg', 'PtEtaSelectionAlg' + self.containerName + postfix )
        config.addPrivateTool( 'selectionTool', 'CP::AsgPtEtaSelectionTool' )
        bits = 0
        if self.minPt is not None :
            alg.selectionTool.minPt = self.minPt
            bits += 1
        if self.maxEta is not None :
            alg.selectionTool.maxEta = self.maxEta
            bits += 1
        if not self.selectionDecoration.find (',as_bits') :
            bits = 1
        alg.selectionDecoration = self.selectionDecoration
        alg.particles = config.readName (self.containerName)
        alg.preselection = config.getPreselection (self.containerName, '')
        config.addSelection (self.containerName, '', alg.selectionDecoration,
                             bits=bits)



class OutputThinningBlock (ConfigBlock):
    """the ConfigBlock for output thinning"""

    def __init__ (self, containerName, *, postfix) :
        super (OutputThinningBlock, self).__init__ ()
        self.containerName = containerName
        self.postfix = postfix
        self.selection = None
        self.outputName = None

    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        selection = config.getPreselection (self.containerName, '')
        if selection == '' :
            selection = self.selection
        elif self.selection != '' :
            selection = selection + '&&' + self.selection

        if selection != '' :
            alg = config.createAlgorithm( 'CP::AsgUnionSelectionAlg', 'UnionSelectionAlg' + self.containerName + postfix)
            alg.preselection = selection
            alg.particles = config.readName (self.containerName)
            alg.selectionDecoration = 'outputSelect' + postfix

        alg = config.createAlgorithm( 'CP::AsgViewFromSelectionAlg', 'DeepCopyAlg' + self.containerName + postfix )
        alg.input = config.readName (self.containerName)
        if self.outputName is not None :
            alg.output = self.outputName + '_%SYS%'
        else :
            alg.output = config.copyName (self.containerName)
        if selection != '' :
            alg.selection = ['outputSelect' + postfix]
        else :
            alg.selection = []
        alg.deepCopy = False



def makePileupReweightingConfig( seq, campaign=None, files=None, useDefaultConfig=False, userLumicalcFiles=None, userPileupConfigs=None ):
    """Create a PRW analysis config

    Keyword arguments:
    """
    # TO DO: add explanation of the keyword arguments, left to experts

    config = PileupReweightingBlock ()
    config.campaign = campaign
    config.files = files
    config.useDefaultConfig = useDefaultConfig
    config.userLumicalcFiles = userLumicalcFiles
    config.userPileupConfigs = userPileupConfigs
    seq.append (config)



def makePtEtaSelectionConfig( seq, containerName,
                              *, postfix = '', minPt = None, maxEta = None,
                              selectionDecoration):
    """Create a pt-eta kinematic selection config

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      minPt -- minimum pt value
      maxEta -- maximum eta value
      selectionDecoration -- the name of the decoration to set
    """

    config = PtEtaSelectionBlock (containerName, postfix=postfix,
                                  minPt=minPt,maxEta=maxEta,
                                  selectionDecoration=selectionDecoration)
    seq.append (config)



def makeOutputThinningConfig( seq, containerName,
                              *, postfix = '', selection = None, outputName = None):
    """Create an output thinning config

    This will do a consistent selection of output containers (if there
    is a preselection or a selection specified) and then creates a set
    of view containers (or deep copies) based on that selection.

    Keyword arguments:
      containerName -- name of the container
      postfix -- a postfix to apply to decorations and algorithm
                 names.  this is mostly used/needed when using this
                 sequence with multiple working points to ensure all
                 names are unique.
      selection -- the name of an optional selection decoration to use
      outputName -- an optional name for the output container

    """

    config = OutputThinningBlock (containerName, postfix=postfix)
    config.selection = selection
    config.outputName = outputName
    seq.append (config)
