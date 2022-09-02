# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class JetJvtAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the small-r jet sequence"""

    def __init__ (self, containerName, jetCollection, postfix = '') :
        super (JetJvtAnalysisConfig, self).__init__ ()
        self.containerName = containerName
        self.jetCollection = jetCollection
        self.postfix = postfix
        if self.postfix != '' and self.postfix[0] != '_' :
            self.postfix = '_' + self.postfix
        self.enableFJvt = False
        self.globalSF = True
        self.runSelection = True


    def makeAlgs (self, config) :

        if self.runSelection and not self.globalSF :
            raise ValueError ("per-event scale factors needs to be computed when doing a JVT selection")

        # Set up the per-event jet efficiency scale factor calculation algorithm
        if config.dataType() != 'data' and self.globalSF:
            alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'JvtEventScaleFactorAlg' + self.postfix )
            preselection = config.getPreselection (self.containerName, '')
            alg.preselection = preselection + '&&no_jvt' if preselection else 'no_jvt'
            alg.scaleFactorInputDecoration = 'jvt_effSF_%SYS%'
            alg.scaleFactorOutputDecoration = 'jvt_effSF_%SYS%'
            alg.particles = config.readName (self.containerName)

            if self.enableFJvt:
                alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'ForwardJvtEventScaleFactorAlg' )
                preselection = config.getPreselection (self.containerName, '')
                alg.preselection = preselection + '&&no_fjvt' if preselection else 'no_fjvt'
                alg.scaleFactorInputDecoration = 'fjvt_effSF_%SYS%'
                alg.scaleFactorOutputDecoration = 'fjvt_effSF_%SYS%'
                alg.particles = config.readName (self.containerName)

        if self.runSelection:
            config.addSelection (self.containerName, '', 'jvt_selection',
                                 bits=1)
            if self.enableFJvt :
                config.addSelection (self.containerName, '', 'fjvt_selection',
                                     bits=1)


def makeJetJvtAnalysisConfig( seq, containerName, jetCollection,
                              postfix = '',
                              enableFJvt = False,
                              globalSF = True,
                              runSelection = True ):
    """Create a jet JVT analysis algorithm config

    Keyword arguments:
      jetCollection -- The jet container to run on
      enableFJvt -- Whether to enable forward JVT calculations
      globalSF -- Whether to calculate per event scale factors
      runSelection -- Whether to run selection
    """

    config = JetJvtAnalysisConfig (containerName, jetCollection, postfix)
    config.enableFJvt = enableFJvt
    config.globalSF = globalSF
    config.runSelection = runSelection

    seq.append (config)
