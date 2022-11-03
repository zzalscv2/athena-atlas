# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class TriggerAnalysisBlock (ConfigBlock):
    """the ConfigBlock for trigger analysis"""

    def __init__ (self) :
        super (TriggerAnalysisBlock, self).__init__ ()
        self.triggerChains = []
        self.prescaleLumiCalcFiles = []
        self.noFilter = False


    def makeAlgs (self, config) :

        # Create public trigger tools
        xAODConfTool = config.createPublicTool( 'TrigConf::xAODConfigTool', 'xAODConfigTool' )
        decisionTool = config.createPublicTool( 'Trig::TrigDecisionTool', 'TrigDecisionTool')
        decisionTool.ConfigTool = '%s/%s' % \
            ( xAODConfTool.getType(), xAODConfTool.getName() )

        if self.triggerChains:
            # Set up the trigger selection:
            alg = config.createAlgorithm( 'CP::TrigEventSelectionAlg', 'TrigEventSelectorAlg' )
            alg.tool = '%s/%s' % \
                ( decisionTool.getType(), decisionTool.getName() )
            alg.triggers = list(self.triggerChains)
            alg.selectionDecoration = 'trigPassed'
            alg.noFilter = self.noFilter

            # Calculate trigger prescales
            if config.dataType() == 'data' and self.prescaleLumiCalcFiles:
                alg = config.createAlgorithm( 'CP::TrigPrescalesAlg', 'TrigPrescalesAlg' )
                config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
                alg.pileupReweightingTool.LumiCalcFiles = self.prescaleLumiCalcFiles
                alg.pileupReweightingTool.TrigDecisionTool = '%s/%s' % \
                    ( decisionTool.getType(), decisionTool.getName() )
                alg.triggers = [lumicalc.split(':')[-1] for lumicalc in self.prescaleLumiCalcFiles if ':' in lumicalc]
                alg.triggersAll = list(self.triggerChains)
                alg.prescaleDecoration = 'prescale'



def makeTriggerAnalysisConfig( seq,
                               triggerChains = [],
                               prescaleLumiCalcFiles = [],
                               noFilter = False):
    """Create a basic trigger analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      triggerChains -- a list of trigger chains
      prescaleLumiCalcFiles -- a list of lumicalc files to calculate trigger prescales
      noFilter -- do not apply an event filter (i.e. don't skip any events)
    """

    config = TriggerAnalysisBlock ()
    config.triggerChains = triggerChains
    config.prescaleLumiCalcFiles = prescaleLumiCalcFiles
    config.noFilter = noFilter
    seq.append (config)
