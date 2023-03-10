# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from TrigTauRec.TrigTauRecConf import TrigTauRecMerged
from TrigTauRec.TrigTauRecMonitoring import tauMonitoringCaloOnlyMVA,  tauMonitoringPrecisionMVA


class TrigTauRecMerged_TauCaloOnlyMVA (TrigTauRecMerged) :

        def __init__(self, flags, name = "TrigTauRecMerged_TauCaloOnlyMVA"):
            super( TrigTauRecMerged_TauCaloOnlyMVA , self ).__init__( name )
            self.MonTool = tauMonitoringCaloOnlyMVA(flags)
            self._mytools = [] 

            import TrigTauRec.TrigTauAlgorithmsHolder as taualgs
            tools = []

            taualgs.setPrefix("TrigTauCaloOnlyMVA_")

            # Only include tools needed for calo pre-selection

            # Set seedcalo energy scale (Full RoI)
            tools.append(taualgs.getJetSeedBuilder())
            # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)
            tools.append(taualgs.getTauAxis())
            # Decorate the clusters
            tools.append(taualgs.getTauClusterFinder())
            tools.append(taualgs.getTauVertexedClusterDecorator())
            # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
            tools.append(taualgs.getCellVariables(cellConeSize=0.2))
            # Compute MVA TES (ATR-17649), stores MVA TES as default tau pt()
            tools.append(taualgs.getMvaTESVariableDecorator())
            tools.append(taualgs.getMvaTESEvaluator(flags))

            for tool in tools:
                tool.inTrigger = True
                tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath

            self.Tools = tools

class TrigTauRecMerged_TauPrecisionMVA (TrigTauRecMerged) :

        def __init__(self, flags, name = "TrigTauRecMerged_TauPrecisionMVA", doTrackBDT=False, doLLP=False):
        
            super( TrigTauRecMerged_TauPrecisionMVA , self ).__init__( name )
            self.MonTool = tauMonitoringPrecisionMVA(flags)

            from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper, conf2toConfigurable
            import TrigTauRec.TrigTauAlgorithmsHolder as taualgs
            tools = []

            # using same prefix as in TauPrecision sequence should be safe if tools with different configurations have different names
            # e.g. TauTrackFinder in 2016 using dz0=2mm instead of 1mm in 2017
            taualgs.setPrefix("TrigTau_")

            # Include full set of tools

            # Associate RoI vertex or Beamspot to tau - don't use TJVA
            tools.append(taualgs.getTauVertexFinder(doUseTJVA=False)) #don't use TJVA by default
            # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)       

            from TrigTauRec.TrigTauToolsConfig import (tauAxisCfg, tauClusterFinderCfg, tauVertexedClusterDecoratorCfg, tauMvaTESVariableDecoratorCfg, tauMvaTESEvaluatorCfg)
            ca = CAtoGlobalWrapper(tauAxisCfg,flags,name="TrigTau_TauAxis")
            tools.append(conf2toConfigurable(ca.popPrivateTools()))
            
            # tightened to 0.75 mm for tracktwoMVA (until the track BDT can be used)
            tools.append(taualgs.getTauTrackFinder(applyZ0cut=True, maxDeltaZ0=0.75, prefix='TrigTauTightDZ_'))            
            
            # Decorate the clusters
            ca = CAtoGlobalWrapper(tauClusterFinderCfg,flags,name="TrigTau_TauClusterFinder")
            tools.append(conf2toConfigurable(ca.popPrivateTools()))

            ca = CAtoGlobalWrapper(tauVertexedClusterDecoratorCfg,flags,name="TrigTau_TauVertexedClusterDecorator")
            tools.append(conf2toConfigurable(ca.popPrivateTools()))

            from AthenaCommon.Logging import log
            if doTrackBDT:
                # BDT track classification is deprecated, RNN track classification feasibility under study
                log.warning( "BDT track classifier is deprecated and won't be scheduled")

            # Compute MVA TES (ATR-17649), stores MVA TES as default tau pt()
            from AthenaCommon.AppMgr import ToolSvc
            CAtoGlobalWrapper(tauMvaTESVariableDecoratorCfg,flags,name="TrigTau_MvaTESVariableDecorator")            
            tools.append(ToolSvc.TrigTau_MvaTESVariableDecorator)

            CAtoGlobalWrapper(tauMvaTESEvaluatorCfg,flags,name="TrigTau_MvaTESEvaluator")           
            tools.append(ToolSvc.TrigTau_MvaTESEvaluator)

            # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
            tools.append(taualgs.getCellVariables(cellConeSize=0.2))

            # Lifetime variables
            tools.append(taualgs.getTauVertexVariables())
            # Variables combining tracking and calorimeter information
            tools.append(taualgs.getTauCommonCalcVars())
            # Cluster-based sub-structure, with dRMax also
            tools.append(taualgs.getTauSubstructure())

            # RNN tau ID, either nominal or LLP
            tools.append(taualgs.getTauJetRNNEvaluator(flags, LLP = doLLP))
            # flattened RNN score and WP
            tools.append(taualgs.getTauWPDecoratorJetRNN(flags, LLP = doLLP))

            for tool in tools:
                tool.inTrigger = True
                tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath

            self.Tools = tools

# this is the newJO fragment
class TrigTauDefaultsKeys:
    VertexContainer = 'PrimaryVertices'
    TrackContainer ='InDetTrackParticles'
    LargeD0TrackContainer ='InDetLargeD0TrackParticles'

def trigTauRecMergedCaloOnlyMVACfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc = ComponentAccumulator()
    # prepare tools 
    tools = []
    # Set seedcalo energy scale (Full RoI)
    tools.append(CompFactory.JetSeedBuilder())

    # Set LC energy scale (0.2 cone) and intermediate axis (corrected for vertex: useless at trigger)
    tools.append(CompFactory.TauAxisSetter(ClusterCone = 0.2,
                                           VertexCorrection = False))
    # Decorate the clusters
    tools.append(CompFactory.TauClusterFinder(UseOriginalCluster = False)) # TODO use JetRec.doVertexCorrection once available

    tools.append(CompFactory.TauVertexedClusterDecorator(SeedJet = ''))

    # Calculate cell-based quantities: strip variables, EM and Had energies/radii, centFrac, isolFrac and ring energies
    from AthenaCommon.SystemOfUnits import GeV
    tools.append(CompFactory.TauCellVariables(StripEthreshold = 0.2*GeV,
                                                CellCone = 0.2,
                                                VertexCorrection = False))
    # Compute MVA TES (ATR-17649), stores MVA TES as default tau pt()
    tools.append(CompFactory.MvaTESVariableDecorator(Key_vertexInputContainer='',
                                                     EventShapeKey='',
                                                     VertexCorrection = False))
    acc.addPublicTool(tools[-1])
    tools.append(CompFactory.MvaTESEvaluator(WeightFileName = flags.Trigger.Offline.Tau.MvaTESConfig))
    acc.addPublicTool(tools[-1])

    for tool in tools:
        tool.inTrigger = True
        tool.calibFolder = flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath

    alg = CompFactory.TrigTauRecMerged("TrigTauRecMerged_TauCaloOnlyMVA",
                                        Tools=tools,
                                        MonTool = tauMonitoringCaloOnlyMVA(flags),
                                        Key_trackPartInputContainer = '',
                                        Key_trigJetSeedOutputKey = 'HLT_jet_seed',
                                        Key_trigTauJetInputContainer = '',
                                        Key_trigTauJetOutputContainer = 'HLT_TrigTauRecMerged_CaloMVAOnly',
                                        Key_trigTauTrackInputContainer = '',
                                        Key_trigTauTrackOutputContainer = 'HLT_tautrack_dummy',
                                        Key_vertexInputContainer = '',
                                        clustersKey = 'HLT_TopoCaloClustersLC',
                                        RoIInputKey = 'UpdatedCaloRoI')
    acc.addEventAlgo(alg)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()

    acc = trigTauRecMergedCaloOnlyMVACfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.wasMerged() # do not run, do not save, we just want to see the config
