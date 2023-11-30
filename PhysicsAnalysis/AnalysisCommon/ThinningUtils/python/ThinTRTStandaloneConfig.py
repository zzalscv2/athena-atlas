# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Instantiate the TRT Standalone Thinning
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def ThinTRTStandaloneCfg(flags, name="ThinTRTStandaloneAlg", **kwargs):

    mlog = logging.getLogger(name)
    mlog.info("Starting TRT standalone Thinning configuration")
    acc = ComponentAccumulator()
    kwargs.setdefault("StreamName", "StreamAOD")
    kwargs.setdefault(
        "doElectron", flags.Reco.EnableEgamma and flags.Egamma.doTracking
    )
    kwargs.setdefault(
        "doPhoton", flags.Reco.EnableEgamma and flags.Egamma.doTracking
    )
    kwargs.setdefault("doTau", flags.Reco.EnableTau)
    kwargs.setdefault("doTauEleRM", flags.Reco.EnableTau and flags.Tau.doTauEleRMRec)
    kwargs.setdefault("doMuon", flags.Reco.EnableCombinedMuon)
    acc.addEventAlgo(CompFactory.ThinTRTStandaloneTrackAlg(name, **kwargs))
    mlog.info("TRT Alone Thinning configured")
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Output.doWriteAOD = True  # To test the AOD parts
    flags.lock()
    acc = MainServicesCfg(flags)
    acc.merge(ThinTRTStandaloneCfg(flags))
    mlog = logging.getLogger("ThinTRTStandaloneConfigTest")
    printProperties(
        mlog,
        acc.getEventAlgo("ThinTRTStandaloneAlg"),
        nestLevel=1,
        printDefaults=True,
    )

    with open("thintrtstandalonecfg.pkl", "wb") as f:
        acc.store(f)
