# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class TriggerAnalysisBlock (ConfigBlock):
    """the ConfigBlock for trigger analysis"""

    def __init__ (self, configName) :
        super (TriggerAnalysisBlock, self).__init__ (configName)
        self.addOption ('triggerChains', [], type=None)
        self.addOption ('prescaleLumiCalcFiles', [], type=None)
        self.addOption ('noFilter', False, type=bool)


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
            for t in self.triggerChains :
                config.addOutputVar ('EventInfo', 'trigPassed_' + t, 'trigPassed_' + t, isEventLevel=True)

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
                               triggerChains = None,
                               prescaleLumiCalcFiles = None,
                               noFilter = None,
                               configName = 'Trigger'):
    """Create a basic trigger analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type to run on ("data", "mc" or "afii")
      triggerChains -- a list of trigger chains
      prescaleLumiCalcFiles -- a list of lumicalc files to calculate trigger prescales
      noFilter -- do not apply an event filter (i.e. don't skip any events)
    """

    config = TriggerAnalysisBlock (configName)
    config.setOptionValue ('triggerChains', triggerChains, noneAction='ignore')
    config.setOptionValue ('prescaleLumiCalcFiles', prescaleLumiCalcFiles, noneAction='ignore')
    config.setOptionValue ('noFilter', noFilter, noneAction='ignore')
    seq.append (config)
