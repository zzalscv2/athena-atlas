# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class JetJvtAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the JVT sequence"""

    def __init__ (self, containerName) :
        super (JetJvtAnalysisConfig, self).__init__ (containerName)
        self.containerName = containerName
        self.addOption ('postfix', '', type=str)
        self.addOption ('enableFJvt', False, type=bool)
        self.addOption ('globalSF', True, type=bool)
        self.addOption ('runSelection', True, type=bool)


    def makeAlgs (self, config) :

        postfix = self.postfix
        if postfix != '' and postfix[0] != '_' :
            postfix = '_' + postfix

        if self.runSelection and not self.globalSF :
            raise ValueError ("per-event scale factors needs to be computed when doing a JVT selection")

        # Set up the per-event jet efficiency scale factor calculation algorithm
        if config.dataType() != 'data' and self.globalSF:
            alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'JvtEventScaleFactorAlg' + postfix )
            preselection = config.getPreselection (self.containerName, '')
            alg.preselection = preselection + '&&no_jvt' if preselection else 'no_jvt'
            alg.scaleFactorInputDecoration = 'jvt_effSF_%SYS%'
            alg.scaleFactorOutputDecoration = 'jvt_effSF_%SYS%'
            alg.particles = config.readName (self.containerName)

            config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_jvt_effSF')

            if self.enableFJvt:
                alg = config.createAlgorithm( 'CP::AsgEventScaleFactorAlg', 'ForwardJvtEventScaleFactorAlg' )
                preselection = config.getPreselection (self.containerName, '')
                alg.preselection = preselection + '&&no_fjvt' if preselection else 'no_fjvt'
                alg.scaleFactorInputDecoration = 'fjvt_effSF_%SYS%'
                alg.scaleFactorOutputDecoration = 'fjvt_effSF_%SYS%'
                alg.particles = config.readName (self.containerName)

                config.addOutputVar('EventInfo', alg.scaleFactorOutputDecoration, 'weight_fjvt_effSF')

        if self.runSelection:
            config.addSelection (self.containerName, 'jvt', 'jvt_selection',
                                 bits=1, preselection=False)
            if self.enableFJvt :
                config.addSelection (self.containerName, 'jvt', 'fjvt_selection',
                                     bits=1, preselection=False)


def makeJetJvtAnalysisConfig( seq, containerName,
                              postfix = None,
                              enableFJvt = None,
                              globalSF = None,
                              runSelection = None ):
    """Create a jet JVT analysis algorithm config

    Keyword arguments:
      enableFJvt -- Whether to enable forward JVT calculations
      globalSF -- Whether to calculate per event scale factors
      runSelection -- Whether to run selection
    """

    config = JetJvtAnalysisConfig (containerName)
    if postfix is not None :
        config.setOptionValue ('postfix', postfix)
    if enableFJvt is not None :
        config.setOptionValue ('enableFJvt', enableFJvt)
    if globalSF is not None :
        config.setOptionValue ('globalSF', globalSF)
    if runSelection is not None :
        config.setOptionValue ('runSelection', runSelection)

    seq.append (config)
