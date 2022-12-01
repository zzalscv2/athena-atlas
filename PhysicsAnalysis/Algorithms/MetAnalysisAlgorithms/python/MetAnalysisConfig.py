# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class MetAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self, containerName) :
        super (MetAnalysisConfig, self).__init__ ()
        self.containerName = containerName
        self.postfix = ''
        self.useFJVT = False
        self.treatPUJets = False
        self.jets = ""
        self.electrons = ""
        self.muons = ""
        self.photons = ""
        self.taus = ""
        self.invisible = ""

    def makeAlgs (self, config) :

        postfix = self.postfix

        if config.isPhyslite() :
            metSuffix = 'AnalysisMET'
        else :
            jetContainer = config.originalName (self.jets)
            metSuffix = jetContainer[:-4]

        if not self.useFJVT and self.treatPUJets:
            raise ValueError ("MET significance pile-up treatment requires fJVT")

        # Remove b-tagging calibration from the MET suffix name
        btIndex = metSuffix.find('_BTagging')
        if btIndex != -1:
            metSuffix = metSuffix[:btIndex]

        # Set up the met maker algorithm:
        alg = config.createAlgorithm( 'CP::MetMakerAlg', 'MetMakerAlg' + postfix)
        config.addPrivateTool( 'makerTool', 'met::METMaker' )

        alg.makerTool.DoPFlow = 'PFlow' in metSuffix or metSuffix=="AnalysisMET"

        if self.useFJVT:
            alg.makerTool.JetRejectionDec = 'passFJVT'
        if config.dataType() != "data" :
            config.addPrivateTool( 'systematicsTool', 'met::METSystematicsTool' )
        alg.metCore = 'MET_Core_' + metSuffix
        alg.metAssociation = 'METAssoc_' + metSuffix
        alg.jets = config.readName (self.jets)
        if self.muons != "" :
            alg.muons, alg.muonsSelection = config.readNameAndSelection (self.muons)
        if self.electrons != "" :
            alg.electrons, alg.electronsSelection = config.readNameAndSelection (self.electrons)
        if self.photons != "" :
            alg.photons, alg.photonsSelection = config.readNameAndSelection (self.photons)
        if self.taus != "" :
            alg.taus, alg.tausSelection = config.readNameAndSelection (self.taus)
        if self.invisible != "" :
            alg.invisible = config.readName (self.invisible)
        alg.met = config.writeName (self.containerName)


        # Set up the met builder algorithm:
        alg = config.createAlgorithm( 'CP::MetBuilderAlg', 'MetBuilderAlg' + postfix )
        alg.met = config.readName (self.containerName)


        # Set up the met significance algorithm:
        alg = config.createAlgorithm( 'CP::MetSignificanceAlg', 'MetSignificanceAlg' + postfix )
        config.addPrivateTool( 'significanceTool', 'met::METSignificance' )
        alg.significanceTool.SoftTermParam = 0
        alg.significanceTool.TreatPUJets = self.treatPUJets
        alg.significanceTool.IsAFII = config.dataType() == "afii"
        alg.met = config.readName (self.containerName)




def makeMetAnalysisConfig( seq, containerName,
                             postfix = '',
                             useFJVT = False,
                             treatPUJets = False,
                             jets = "",
                             electrons = "",
                             muons = "",
                             photons = "",
                             taus = ""):
    """Create a met analysis algorithm config

    Note that defining a jet container is mandatory, but all other input
    containers are optional.

    Selections on each container can also be defined

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      useFJVT -- Use FJVT decision for the calculation
      treatPUJets -- Treat pile-up jets in the MET significance calculation
    """

    config = MetAnalysisConfig (containerName)
    config.postfix = postfix
    config.useFJVT = useFJVT
    config.treatPUJets = treatPUJets
    config.jets = jets
    config.electrons = electrons
    config.muons = muons
    config.photons = photons
    config.taus = taus
    seq.append (config)
