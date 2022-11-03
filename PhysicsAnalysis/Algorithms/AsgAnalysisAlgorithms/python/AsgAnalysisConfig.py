# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class PileupReweightingBlock (ConfigBlock):
    """the ConfigBlock for pileup reweighting"""

    def __init__ (self) :
        super (PileupReweightingBlock, self).__init__ ()
        self.userPileupConfigs=[]
        self.userLumicalcFiles=[]
        self.autoConfig=False


    def makeAlgs (self, config) :

        try:
            from AthenaCommon.Logging import logging
        except ImportError:
            import logging
        prwlog = logging.getLogger('makePileupAnalysisSequence')

        muMcFiles = self.userPileupConfigs[:]
        userLumicalcFiles = self.userLumicalcFiles
        if self.autoConfig:
            from PileupReweighting.AutoconfigurePRW import getLumiCalcFiles,getMCMuFiles
            userLumicalcFiles = getLumiCalcFiles()
            if len(muMcFiles)==0:
                muMcFiles = getMCMuFiles()
            else:
                prwlog.warning('Sent autoconfig and self.userPileupConfigs='+str(self.userPileupConfigs))
                prwlog.warning('Ignoring autoconfig and keeping user-specified files')

        if userLumicalcFiles==[]:
            muDataFiles = ["GoodRunsLists/data15_13TeV/20170619/PHYS_StandardGRL_All_Good_25ns_276262-284484_OflLumi-13TeV-008.root",
                           "GoodRunsLists/data16_13TeV/20180129/PHYS_StandardGRL_All_Good_25ns_297730-311481_OflLumi-13TeV-009.root",
                           "GoodRunsLists/data17_13TeV/20180619/physics_25ns_Triggerno17e33prim.lumicalc.OflLumi-13TeV-010.root",
                           "GoodRunsLists/data18_13TeV/20190708/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root" ]
        else:
            muDataFiles = userLumicalcFiles[:]

        # Set up the only algorithm of the sequence:
        alg = config.createAlgorithm( 'CP::PileupReweightingAlg', 'PileupReweightingAlg' )
        config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
        alg.pileupReweightingTool.ConfigFiles = muMcFiles
        if not muMcFiles and config.dataType() != "data":
            prwlog.info("No PRW config files provided. Disabling reweighting")
            # Setting the weight decoration to the empty string disables the reweighting
            alg.pileupWeightDecoration = ""
        alg.pileupReweightingTool.LumiCalcFiles = muDataFiles



class GeneratorAnalysisBlock (ConfigBlock):
    """the ConfigBlock for generator algorithms"""

    def __init__ (self) :
        super (GeneratorAnalysisBlock, self).__init__ ()
        self.saveCutBookkeepers = False
        self.runNumber = 0
        self.cutBookkeepersSystematics = False

    def makeAlgs (self, config) :

        if config.dataType() not in ["mc", "afii"] :
            raise ValueError ("invalid data type: " + config.dataType())

        if self.saveCutBookkeepers and not self.runNumber:
            raise ValueError ("invalid run number: " + 0)

        # Set up the CutBookkeepers algorithm:
        if self.saveCutBookkeepers:
          alg = config.createAlgorithm('CP::AsgCutBookkeeperAlg', 'CutBookkeeperAlg')
          alg.runNumber = self.runNumber
          alg.enableSystematics = self.cutBookkeepersSystematics
          config.addPrivateTool( 'truthWeightTool', 'PMGTools::PMGTruthWeightTool' )

        # Set up the weights algorithm:
        alg = config.createAlgorithm( 'CP::PMGTruthWeightAlg', 'PMGTruthWeightAlg' )
        config.addPrivateTool( 'truthWeightTool', 'PMGTools::PMGTruthWeightTool' )
        alg.decoration = 'generatorWeight_%SYS%'



class PrimaryVertexBlock (ConfigBlock):
    """the ConfigBlock for requiring primary vertices"""

    def __init__ (self) :
        super (PrimaryVertexBlock, self).__init__ ()


    def makeAlgs (self, config) :

        config.createAlgorithm( 'CP::VertexSelectionAlg',
                                'PrimaryVertexSelectorAlg' )
        config.VertexContainer = 'PrimaryVertices'
        config.MinVertices = 1



class PtEtaSelectionBlock (ConfigBlock):
    """the ConfigBlock for a pt-eta selection"""

    def __init__ (self, containerName, postfix, minPt, maxEta,
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

    def __init__ (self, containerName, postfix) :
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



def makePileupReweightingConfig( seq, userPileupConfigs=[], userLumicalcFiles=[] , autoConfig=False ):
    """Create a PRW analysis config

    Keyword arguments:
    """
    # TO DO: add explanation of the keyword arguments, left to experts

    config = PileupReweightingBlock ()
    config.userPileupConfigs = userPileupConfigs
    config.userLumicalcFiles = userLumicalcFiles
    config.autoConfig = autoConfig
    seq.append (config)



def makeGeneratorAnalysisConfig( seq,
                                 saveCutBookkeepers=False,
                                 runNumber=0,
                                 cutBookkeepersSystematics=False ):
    """Create a generator analysis algorithm sequence

    Keyword arguments:
      saveCutBookkeepers -- save cut bokkeepers information into output file
      runNumber -- MC run number
      cutBookkeepersSystematics -- store CutBookkeepers systematics
    """

    config = GeneratorAnalysisBlock ()
    config.saveCutBookkeepers = saveCutBookkeepers
    config.runNumber = runNumber
    config.cutBookkeepersSystematics = cutBookkeepersSystematics
    seq.append (config)



def makePrimaryVertexConfig( seq ) :
    """Create config block that requires a primary vertex
    """

    config = PrimaryVertexBlock ()
    seq.append (config)



def makePtEtaSelectionConfig( seq, containerName,
                              selectionDecoration,
                              postfix = '', minPt = None, maxEta = None):
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
                              postfix = '', selection = None, outputName = None):
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
