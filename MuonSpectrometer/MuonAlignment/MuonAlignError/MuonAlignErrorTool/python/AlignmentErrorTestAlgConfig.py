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

    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--errorClobFileOverride', help='Optional path to an error clob file to use as override')

    # Set flags defaults
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/main/q442/v32/myESD.pool.root']
    flags.Exec.MaxEvents = 10
    flags.Common.ShowMsgStats = True

    args, _ = parser.parse_known_args()

    flags.fillFromArgs(parser=parser)
    flags.lock()

    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(AlignmentErrorTestAlgCfg(flags))

    if args.errorClobFileOverride:
        cfg.getCondAlgo("MuonAlignmentErrorDbAlg").clobFileOverride = args.errorClobFileOverride

    if args.postExec:
        print('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig()

    cfg.run()
