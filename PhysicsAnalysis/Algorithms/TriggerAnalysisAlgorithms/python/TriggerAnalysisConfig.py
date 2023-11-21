# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign


class TriggerAnalysisBlock (ConfigBlock):
    """the ConfigBlock for trigger analysis"""

    def __init__ (self, configName) :
        super (TriggerAnalysisBlock, self).__init__ (configName)
        self.addOption ('triggerChainsPerYear', {}, type=None)
        self.addOption ('triggerChainsForSelection', [], type=None)
        self.addOption ('prescaleLumiCalcFiles', [], type=None)
        self.addOption ('noFilter', False, type=bool)
        self.addOption ('electronID', '', type=str)
        self.addOption ('electronIsol', '', type=str)
        self.addOption ('photonIsol', '', type=str)
        self.addOption ('muonID', '', type=str)
        self.addOption ('electrons', '', type=str)
        self.addOption ('muons', '', type=str)
        self.addOption ('photons', '', type=str)
        self.addOption ('noEffSF', False, type=bool)

    def makeTriggerDecisionAlg(self, config):

        # Create public trigger tools
        xAODConfTool = config.createPublicTool( 'TrigConf::xAODConfigTool', 'xAODConfigTool' )
        decisionTool = config.createPublicTool( 'Trig::TrigDecisionTool', 'TrigDecisionTool' )
        decisionTool.ConfigTool = '%s/%s' % \
            ( xAODConfTool.getType(), xAODConfTool.getName() )
        if config.geometry() == LHCPeriod.Run3:
            decisionTool.NavigationFormat = 'TrigComposite' # Read Run 3 navigation (options are "TrigComposite" for R3 or "TriggElement" for R2, R2 navigation is not kept in most DAODs)
            decisionTool.HLTSummary = 'HLTNav_Summary_DAODSlimmed' # Name of R3 navigation container (if reading from AOD, then "HLTNav_Summary_AODSlimmed" instead)

        return decisionTool

    def makeTriggerMatchingAlg(self, config, decisionTool):

        # Create public trigger tools
        drScoringTool = config.createPublicTool( 'Trig::DRScoringTool', 'DRScoringTool' )
        if config.geometry() == LHCPeriod.Run3:
            matchingTool = config.createPublicTool( 'Trig::R3MatchingTool', 'MatchingTool' )
            matchingTool.ScoringTool = '%s/%s' % \
                    ( drScoringTool.getType(), drScoringTool.getName() )
            matchingTool.TrigDecisionTool = '%s/%s' % \
                    ( decisionTool.getType(), decisionTool.getName() )
        else:
            matchingTool = config.createPublicTool( 'Trig::MatchFromCompositeTool', 'MatchingTool' )
            if config.isPhyslite():
                matchingTool.InputPrefix = "AnalysisTrigMatch_"

        return matchingTool

    def makeTriggerSelectionAlg(self, config, decisionTool):

        # Set up the trigger selection:
        alg = config.createAlgorithm( 'CP::TrigEventSelectionAlg', 'TrigEventSelectorAlg' )
        alg.tool = '%s/%s' % \
            ( decisionTool.getType(), decisionTool.getName() )
        alg.triggers = self.triggerChainsForSelection
        alg.selectionDecoration = 'trigPassed'
        alg.noFilter = self.noFilter

        for t in self.triggerChainsForSelection :
            config.addOutputVar ('EventInfo', 'trigPassed_' + t, 'trigPassed_' + t, noSys=True)

        # Calculate trigger prescales
        if config.dataType() is DataType.Data and self.prescaleLumiCalcFiles:
            alg = config.createAlgorithm( 'CP::TrigPrescalesAlg', 'TrigPrescalesAlg' )
            config.addPrivateTool( 'pileupReweightingTool', 'CP::PileupReweightingTool' )
            alg.pileupReweightingTool.LumiCalcFiles = self.prescaleLumiCalcFiles
            alg.pileupReweightingTool.TrigDecisionTool = '%s/%s' % \
                    ( decisionTool.getType(), decisionTool.getName() )
            alg.triggers = [lumicalc.split(':')[-1] for lumicalc in self.prescaleLumiCalcFiles if ':' in lumicalc]
            alg.triggersAll = self.triggerChainsForSelection
            alg.prescaleDecoration = 'prescale'

        return

    def makeTriggerGlobalEffCorrAlg(self, config, decisionTool, matchingTool, noSF):

        alg = config.createAlgorithm( 'CP::TrigGlobalEfficiencyAlg', 'TrigGlobalSFAlg' )
        if config.geometry() == LHCPeriod.Run3:
            alg.triggers_2022 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2022',[])]
            alg.triggers_2023 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2023',[])]
            alg.triggers_2024 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2024',[])]
            alg.triggers_2025 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2025',[])]
            if config.campaign() in [Campaign.MC21a, Campaign.MC23a]:
                if not alg.triggers_2022:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2022!' )
            elif config.campaign() is Campaign.MC23c:
                if not alg.triggers_2023:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2023!' )
        else:
            alg.triggers_2015 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2015',[])]
            alg.triggers_2016 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2016',[])]
            alg.triggers_2017 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2017',[])]
            alg.triggers_2018 = [trig.replace("HLT_","").replace(" || ", "_OR_") for trig in self.triggerChainsPerYear.get('2018',[])]
            if config.campaign() is Campaign.MC20a:
                if not (alg.triggers_2015 and alg.triggers_2016):
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the years 2015 and 2016!' )
            elif config.campaign() is Campaign.MC20d:
                if not alg.triggers_2017:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2017!' )
            elif config.campaign() is Campaign.MC20e:
                if not alg.triggers_2018:
                    raise ValueError( 'TriggerAnalysisConfig: you must provide a set of triggers for the year 2018!' )
        alg.decisionTool = '%s/%s' % ( decisionTool.getType(), decisionTool.getName() )
        alg.matchingTool = '%s/%s' % ( matchingTool.getType(), matchingTool.getName() )
        alg.isRun3Geo = config.geometry() == LHCPeriod.Run3
        alg.scaleFactorDecoration = 'globalTriggerEffSF_%SYS%'
        alg.matchingDecoration = 'globalTriggerMatch_%SYS%'
        alg.eventDecisionOutputDecoration = 'dontsave_%SYS%'
        alg.doMatchingOnly = config.dataType() is DataType.Data or noSF
        alg.noFilter = self.noFilter
        alg.electronID = self.electronID
        alg.electronIsol = self.electronIsol
        alg.photonIsol = self.photonIsol
        alg.muonID = self.muonID
        if self.electrons:
            alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        if self.muons:
            alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        if self.photons:
            alg.photons, alg.photonSelection = config.readNameAndSelection(self.photons)
        if not (self.electrons or self.muons or self.photons):
            raise ValueError ('TriggerAnalysisConfig: at least one object collection must be provided! (electrons, muons, photons)' )

        if config.dataType() != DataType.Data and not alg.doMatchingOnly:
            config.addOutputVar ('EventInfo', alg.scaleFactorDecoration, 'globalTriggerEffSF')
        config.addOutputVar ('EventInfo', alg.matchingDecoration, 'globalTriggerMatch', noSys=True)

        return

    def makeAlgs (self, config) :

        # if we are only given the trigger dictionary, we fill the selection list automatically
        if self.triggerChainsPerYear and not self.triggerChainsForSelection:
            triggers = set()
            for chain_list in self.triggerChainsPerYear.values():
                for chain in chain_list:
                    if '||' in chain:
                        chains = chain.split('||')
                        triggers.update(map(str.strip, chains))
                    else:
                        triggers.add(chain.strip())
            self.triggerChainsForSelection = list(triggers)

        # Create the decision algorithm, keeping track of the decision tool for later
        decisionTool = self.makeTriggerDecisionAlg(config)

        # Now pass it to the matching algorithm, keeping track of the matching tool for later
        matchingTool = self.makeTriggerMatchingAlg(config, decisionTool)

        if self.triggerChainsForSelection:
            self.makeTriggerSelectionAlg(config, decisionTool)

        # Calculate multi-lepton (electron/muon/photon) trigger efficiencies and SFs
        if self.triggerChainsPerYear:
            self.makeTriggerGlobalEffCorrAlg(config, decisionTool, matchingTool, self.noEffSF)

        return


def makeTriggerAnalysisConfig( seq,
                               triggerChainsPerYear = None,
                               triggerChainsForSelection = None,
                               prescaleLumiCalcFiles = None,
                               noFilter = None,
                               electronWorkingPoint = None,
                               muonWorkingPoint = None,
                               photonWorkingPoint = None,
                               electrons = None,
                               muons = None,
                               photons = None,
                               noEffSF = None,
                               configName = 'Trigger'):
    """Create a basic trigger analysis algorithm sequence

    Keyword arguments:
      triggerChainsPerYear -- a dictionary with key (str) the year and value (list of strings) the trigger chains
      triggerChainsForSelection -- a list of trigger chains to be used for trigger selection, only set it if you need a different setup than for SFs!
      prescaleLumiCalcFiles -- a list of lumicalc files to calculate trigger prescales
      noFilter -- do not apply an event filter (i.e. don't skip any events)
      noEffSF -- Disables the calculation of efficiencies and scale factors (just matching)
    """

    config = TriggerAnalysisBlock (configName)
    config.setOptionValue ('triggerChainsPerYear', triggerChainsPerYear, noneAction='ignore')
    config.setOptionValue ('triggerChainsForSelection', triggerChainsForSelection, noneAction='ignore')
    config.setOptionValue ('prescaleLumiCalcFiles', prescaleLumiCalcFiles, noneAction='ignore')
    config.setOptionValue ('noFilter', noFilter, noneAction='ignore')
    if electronWorkingPoint is not None:
        splitWP = electronWorkingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError (f'electron working point should be of format "likelihood.isolation", not {electronWorkingPoint}')
        config.setOptionValue ('electronID', splitWP[0], noneAction='ignore')
        config.setOptionValue ('electronIsol', splitWP[1], noneAction='ignore')
    if muonWorkingPoint is not None:
        splitWP = muonWorkingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError (f'muon working point should be of format "likelihood.isolation", not {muonWorkingPoint}')
        config.setOptionValue ('muonID', splitWP[0], noneAction='ignore')
    if photonWorkingPoint is not None:
        splitWP = photonWorkingPoint.split ('.')
        if len (splitWP) != 2 :
            raise ValueError (f'photon working point should be of format "likelihood.isolation", not {photonWorkingPoint}')
        config.setOptionValue ('photonIsol', splitWP[1], noneAction='ignore')
    config.setOptionValue('electrons', electrons, noneAction='ignore')
    config.setOptionValue('muons', muons, noneAction='ignore')
    config.setOptionValue('photons', photons, noneAction='ignore')
    config.setOptionValue('noEffSF', noEffSF, noneAction='ignore')
    seq.append (config)
