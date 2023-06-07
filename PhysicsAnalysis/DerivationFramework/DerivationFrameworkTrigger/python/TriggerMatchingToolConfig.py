# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def IParticleRetrievalToolCfg(ConfigFlags):
    """Configure the IParticle retrieval tool, depends on R1/R2 or R3 trigger"""
    acc = ComponentAccumulator()
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = acc.getPrimaryAndMerge(TrigDecisionToolCfg(ConfigFlags))
    if ConfigFlags.Trigger.EDMVersion == 3:
        IParticleRetrievalTool = CompFactory.Trig.R3IParticleRetrievalTool
    else: 
        IParticleRetrievalTool = CompFactory.Trig.IParticleRetrievalTool
    acc.addPublicTool(IParticleRetrievalTool("OnlineParticleTool", TrigDecisionTool = tdt),
                      primary = True)
    return(acc)

def TriggerMatchingToolCfg(ConfigFlags, name, UseTypedScoringTool=False, **kwargs):
    """Config fragment for the trigger matching tool used in DAOD production"""
    acc = ComponentAccumulator()

    # Option to use typed scoring tool
    if UseTypedScoringTool:
        from xAODBase.xAODType import xAODType
        drST = CompFactory.Trig.DRScoringTool("DRScoringTool")
        emST = CompFactory.Trig.EgammaDRScoringTool("EgammaDRScoringTool",
                                                    UseClusterDecorator = False)

        tst = CompFactory.Trig.TypedScoringTool("TypedScoringTool",
                              DefaultScoringTool = drST,
                              TypedScoringTools = [emST],
                              ToolTypes = [xAODType.Electron])
        kwargs["ScoringTool"] = tst

    OnlineParticleTool = acc.getPrimaryAndMerge(IParticleRetrievalToolCfg(ConfigFlags))
    kwargs['OnlineParticleTool'] = OnlineParticleTool
    TriggerMatchingTool = CompFactory.DerivationFramework.TriggerMatchingTool
    acc.addPublicTool(TriggerMatchingTool(name, **kwargs),
                      primary = True)
    return(acc)


