# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
import copy

class OutputAnalysisConfig (ConfigBlock):
    """the ConfigBlock for the MET configuration"""

    def __init__ (self, configName) :
        super (OutputAnalysisConfig, self).__init__ (configName)
        self.addOption ('postfix', '', type=str)
        self.addOption ('vars', [], type=None)
        self.addOption ('metVars', [], type=None)
        self.addOption ('containers', {}, type=None)
        self.addOption ('treeName', 'analysis', type=str)
        self.addOption ('metTermName', 'Final', type=str)


    def makeAlgs (self, config) :

        outputConfigs = {}
        for prefix in self.containers.keys() :
            containerName = self.containers[prefix]
            outputDict = config.getOutputVars (containerName)
            for outputName in outputDict :
                outputConfig = copy.deepcopy (outputDict[outputName])
                if containerName == 'EventInfo' :
                    outputConfig.outputContainerName = outputConfig.origContainerName
                elif outputConfig.outputContainerName != outputConfig.origContainerName :
                    outputConfig.outputContainerName = containerName + '_%SYS%'
                else :
                    outputConfig.outputContainerName = config.readName (containerName)
                outputConfigs[prefix + outputName] = outputConfig
        autoVars = []
        autoMetVars = []
        for outputName in outputConfigs :
            outputConfig = outputConfigs[outputName]
            if outputConfig.enabled :
                if config.isMetContainer (outputConfig.origContainerName) :
                    myVars = autoMetVars
                else :
                    myVars = autoVars
                if outputConfig.noSys :
                    outputConfig.outputContainerName = outputConfig.outputContainerName.replace ('%SYS%', 'NOSYS')
                    outputConfig.variableName = outputConfig.variableName.replace ('%SYS%', 'NOSYS')
                else :
                    outputName += '_%SYS%'
                myVars += [outputConfig.outputContainerName + '.' + outputConfig.variableName + ' -> ' + outputName]

        postfix = self.postfix

        # Add an ntuple dumper algorithm:
        treeMaker = config.createAlgorithm( 'CP::TreeMakerAlg', 'TreeMaker' + postfix )
        treeMaker.TreeName = self.treeName
        # the auto-flush setting still needs to be figured out
        #treeMaker.TreeAutoFlush = 0

        if len (self.vars) + len (autoVars) :
            ntupleMaker = config.createAlgorithm( 'CP::AsgxAODNTupleMakerAlg', 'NTupleMaker' + postfix )
            ntupleMaker.TreeName = self.treeName
            ntupleMaker.Branches = self.vars + autoVars
            # ntupleMaker.OutputLevel = 2  # For output validation

        if len (self.metVars) + len (autoMetVars) > 0:
            ntupleMaker = config.createAlgorithm( 'CP::AsgxAODMetNTupleMakerAlg', 'MetNTupleMaker' + postfix )
            ntupleMaker.TreeName = self.treeName
            ntupleMaker.Branches = self.metVars + autoMetVars
            ntupleMaker.termName = self.metTermName
            #ntupleMaker.OutputLevel = 2  # For output validation

        treeFiller = config.createAlgorithm( 'CP::TreeFillerAlg', 'TreeFiller' + postfix )
        treeFiller.TreeName = self.treeName
