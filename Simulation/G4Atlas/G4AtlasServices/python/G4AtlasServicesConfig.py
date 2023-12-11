# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from ExtraParticles.ExtraParticlesConfig import ExtraParticlesPhysicsToolCfg
from SimulationConfig.SimEnums import CavernBackground
from G4AtlasTools.G4GeometryToolConfig import G4AtlasDetectorConstructionToolCfg
from G4ExtraProcesses.G4ExtraProcessesConfig import G4EMProcessesPhysicsToolCfg
from G4StepLimitation.G4StepLimitationConfig import G4StepLimitationToolCfg
from TRT_TR_Process.TRT_TR_ProcessConfig import TRTPhysicsToolCfg


def DetectorGeometrySvcCfg(flags, name="DetectorGeometrySvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorConstruction", result.addPublicTool(result.popToolsAndMerge(G4AtlasDetectorConstructionToolCfg(flags))))
    result.addService(CompFactory.DetectorGeometrySvc(name, **kwargs), primary = True)
    return result


@AccumulatorCache
def PhysicsListSvcCfg(flags, name="PhysicsListSvc", **kwargs):
    result = ComponentAccumulator()
    PhysOptionList = [ result.popToolsAndMerge(G4StepLimitationToolCfg(flags)) ]
    if flags.Sim.ISF.Simulator.isQuasiStable():
        #Quasi stable particle simulation
        PhysOptionList += [ result.popToolsAndMerge(ExtraParticlesPhysicsToolCfg(flags)) ] # FIXME more configuration required in this method
        PhysOptionList += [ result.popToolsAndMerge(G4EMProcessesPhysicsToolCfg(flags)) ]
    #PhysOptionList += flags.Sim.PhysicsOptions # FIXME Missing functionality
    if flags.Detector.GeometryTRT:
        PhysOptionList +=[ result.popToolsAndMerge(TRTPhysicsToolCfg(flags)) ]
    if flags.Detector.GeometryLucid or flags.Detector.GeometryAFP or flags.Detector.GeometryZDC:
        LucidPhysicsTool = CompFactory.LucidPhysicsTool
        PhysOptionList +=[LucidPhysicsTool("LucidPhysicsTool")]
    kwargs.setdefault("PhysOption", PhysOptionList)
    PhysDecaysList = []
    kwargs.setdefault("PhysicsDecay", PhysDecaysList)
    kwargs.setdefault("PhysicsList", flags.Sim.PhysicsList)
    if 'PhysicsList' in kwargs:
        if kwargs['PhysicsList'].endswith('_EMV') or kwargs['PhysicsList'].endswith('_EMX'):
            raise RuntimeError( 'PhysicsList not allowed: '+kwargs['PhysicsList'] )

    kwargs.setdefault("GeneralCut", 1.)
    if flags.Sim.CavernBackground not in [CavernBackground.Read, CavernBackground.Write]:
        kwargs.setdefault("NeutronTimeCut", flags.Sim.NeutronTimeCut)
    kwargs.setdefault("NeutronEnergyCut", flags.Sim.NeutronEnergyCut)
    kwargs.setdefault("ApplyEMCuts", flags.Sim.ApplyEMCuts)
    ## from AthenaCommon.SystemOfUnits import eV, TeV
    ## kwargs.setdefault("EMMaxEnergy"     , 7*TeV)
    ## kwargs.setdefault("EMMinEnergy"     , 100*eV)
    """ --- ATLASSIM-3967 ---
        these two options are replaced by SetNumberOfBinsPerDecade
        which now controls both values.
    """
    ## kwargs.setdefault("EMDEDXBinning"   , 77)
    ## kwargs.setdefault("EMLambdaBinning" , 77)
    if flags.Sim.ISF.Simulator.usesFatras():
        kwargs.setdefault("UnstableAntiNeutrons", True) # Fix for ATLASSIM-6634 - consider fixing for FullG4 also
    result.addService(CompFactory.PhysicsListSvc(name, **kwargs), primary = True)
    return result
