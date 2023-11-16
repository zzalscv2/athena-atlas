#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def AlignmentErrorTestAlgCfg(flags, name="AlignmentErrorTestAlg", **kwargs):
    acc = ComponentAccumulator()

    from MuonAlignErrorTool.AlignmentErrorToolConfig import AlignmentErrorToolCfg
    kwargs.setdefault("alignmentErrorTool", acc.popToolsAndMerge(AlignmentErrorToolCfg(flags)))

    alg = CompFactory.MuonAlign.AlignmentErrorTestAlg(name, **kwargs)
    acc.addEventAlgo(alg)

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/main/q442/v32/myESD.pool.root']
    flags.Exec.MaxEvents = 10
    flags.Common.ShowMsgStats = True
    flags.addFlag("Muon.Align.ErrorClobFileOverride", '')

    flags.fillFromArgs()
    flags.lock()

    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(AlignmentErrorTestAlgCfg(flags))

    if flags.Muon.Align.ErrorClobFileOverride:
        cfg.getCondAlgo("MuonAlignmentErrorDbAlg").clobFileOverride = flags.Muon.Align.ErrorClobFileOverride

    cfg.printConfig()

    cfg.run()
