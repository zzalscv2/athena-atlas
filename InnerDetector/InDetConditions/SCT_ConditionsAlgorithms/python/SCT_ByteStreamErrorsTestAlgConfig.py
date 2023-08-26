"""Define method to configure and test SCT_ByteStreamErrorsTestAlg

Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def SCT_ByteStreamErrorsTestAlgCfg(flags, name="SCT_ByteStreamErrorsTestAlg", **kwargs):
    """Return a configured SCT_ByteStreamErrorsTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ByteStreamErrorsToolCfg
    kwargs.setdefault("ByteStreamErrorsTool", acc.popToolsAndMerge(SCT_ByteStreamErrorsToolCfg(flags)))
    acc.addEventAlgo(CompFactory.SCT_ByteStreamErrorsTestAlg(name, **kwargs))
    return acc

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.isMC = False
    flags.Input.Files = ["./myESD.pool.root"]
    flags.Input.ProjectName = "data17_13TeV" # q431 input
    flags.Input.RunNumber = 330470 # q431 input
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2018-03" # q431 setup
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    algkwargs = {}
    algkwargs["OutputLevel"] = INFO
    cfg.merge(SCT_ByteStreamErrorsTestAlgCfg(flags, **algkwargs))

    cfg.run(maxEvents=25)
