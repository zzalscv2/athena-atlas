# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock


class EventCleaningBlock (ConfigBlock):
    """the ConfigBlock for event cleaning"""

    def __init__ (self) :
        super (EventCleaningBlock, self).__init__ ('Trigger')
        self.addOption ('runPrimaryVertexSelection', True, type=bool)
        self.addOption ('runEventCleaning', False, type=bool)
        self.addOption ('userGRLFiles', [], type=None)


    def makeAlgs (self, config) :

        if config.dataType() == 'data' :
            grlFiles = self.userGRLFiles[:]

            # Set up the GRL selection:
            alg = config.createAlgorithm( 'GRLSelectorAlg', 'GRLSelectorAlg' )
            config.addPrivateTool( 'Tool', 'GoodRunsListSelectionTool' )
            alg.Tool.GoodRunsListVec = grlFiles

        # Skip events with no primary vertex:
        if self.runPrimaryVertexSelection:
            alg = config.createAlgorithm( 'CP::VertexSelectionAlg',
                                          'PrimaryVertexSelectorAlg' )
            alg.VertexContainer = 'PrimaryVertices'
            alg.MinVertices = 1

        # Set up the event cleaning selection:
        if self.runEventCleaning:
            alg = config.createAlgorithm( 'CP::EventFlagSelectionAlg', 'EventFlagSelectorAlg' )
            alg.FilterKey = 'JetCleaning'
            alg.FilterDescription = 'selecting events passing DFCommonJets_eventClean_LooseBad'
            alg.selectionFlags = ['DFCommonJets_eventClean_LooseBad,as_char']




def makeEventCleaningConfig( seq,
                             runPrimaryVertexSelection = None,
                             runEventCleaning = None,
                             userGRLFiles = None):
    """Create a basic event cleaning analysis algorithm sequence

    Keyword arguments:
      runPrimaryVertexSelection -- whether to run primary vertex selection
      runEventCleaning -- wether to run event cleaning
      userGRLFiles -- a list of GRL files to select data from
    """

    config = EventCleaningBlock ()
    config.setOptionValue ('runPrimaryVertexSelection', runPrimaryVertexSelection, noneAction='ignore')
    config.setOptionValue ('runEventCleaning', runEventCleaning, noneAction='ignore')
    config.setOptionValue ('userGRLFiles', userGRLFiles, noneAction='ignore')
    seq.append (config)
