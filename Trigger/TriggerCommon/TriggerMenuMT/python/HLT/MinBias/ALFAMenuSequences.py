#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigStreamerHypo.TrigStreamerHypoConf import TrigStreamerHypoTool
from TriggerMenuMT.HLT.Config.MenuComponents import InEventRecoCA, SelectionCA, MenuSequenceCA


@AccumulatorCache
def ALFAROBMonitorCfg(flags):
    acc = ComponentAccumulator()

    from TrigOnlineMonitor.TrigOnlineMonitorConfig import TrigALFAROBMonitor
    mon = TrigALFAROBMonitor(flags)
    acc.addEventAlgo(mon)

    return acc


@AccumulatorCache
def ALFAPerfSequence(flags):
    def alwaysRejectHypoToolGen(chainDict):
        return TrigStreamerHypoTool(chainDict['chainName'], Pass = False)

    recoAcc = InEventRecoCA(name='ALFAPerfReco')

    mon = ALFAROBMonitorCfg(flags)
    recoAcc.mergeReco(mon)

    hypo = CompFactory.TrigStreamerHypoAlg('ALFAPerfHypo', RuntimeValidation=False)

    selAcc = SelectionCA('ALFAPerfRecoView')
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequenceCA(flags, selAcc, HypoToolGen=alwaysRejectHypoToolGen)


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.lock()

    alfa = ALFAPerfSequence(flags)
    alfa.ca.printConfig(withDetails=True)
    from TriggerMenuMT.HLT.Config.MenuComponents import menuSequenceCAToGlobalWrapper
    alfa_gw = menuSequenceCAToGlobalWrapper(ALFAPerfSequence, flags)
