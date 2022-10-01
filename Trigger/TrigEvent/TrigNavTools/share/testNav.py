from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaConfiguration import AllConfigFlags, TestDefaults, MainServicesConfig, ComponentFactory
from AthenaCommon import Configurable, Constants, Logging
from OutputStreamAthenaPool import OutputStreamConfig

flags = AllConfigFlags.ConfigFlags
flags.Trigger.EDMVersion = 3 # when reading DOAD force new format

flags.fillFromArgs()

cfg= MainServicesConfig.MainServicesCfg(flags)



from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
cfg.merge(TrigDecisionToolCfg(flags))

print("EDM ver", flags.Trigger.EDMVersion)


cfg.merge(PoolReadCfg(flags))
tester = ComponentFactory.CompFactory.Trig.NavigationTesterAlg(OutputLevel=2)
if flags.Trigger.EDMVersion == 3:
    tester.RetrievalTool1 = ComponentFactory.CompFactory.Trig.R3IParticleRetrievalTool()
    cfg.getPublicTool("TrigDecisionTool").HLTSummary = "HLTNav_R2ToR3Summary"
else:
    tester.RetrievalTool1 = ComponentFactory.CompFactory.Trig.IParticleRetrievalTool()

# example chains
#tester.Chains= ["HLT_mu4"]
tester.Chains= ['HLT_e26_lhtight_nod0_e15_etcut_L1EM7_Zee']
cfg.addEventAlgo(tester)

cfg.run()
