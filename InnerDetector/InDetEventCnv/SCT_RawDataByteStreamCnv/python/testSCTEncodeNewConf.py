#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.isMC = True
    flags.Input.Files = ["/afs/cern.ch/atlas/project/rig/referencefiles/RTTinputFiles/MC15_13TeV/valid1.110401.PowhegPythia_P2012_ttbar_nonallhad.recon.RDO.e3099_s2578_r6699_10evt.pool.root"]
    flags.IOVDb.GlobalTag = "OFLCOND-RUN12-SDR-31"
    flags.Detector.GeometrySCT = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    # For POOL file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # For ByteStream file writing
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
    writingAcc = ByteStreamWriteCfg(flags, [ "SCT_RDO_Container#SCT_RDOs" ] )
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamType = "EventStorage"
    writingAcc.getService("ByteStreamEventStorageOutputSvc").StreamName = "StreamBSFileOutput"
    acc.merge(writingAcc)

    # For SCT geometry and cabling
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))
    from SCT_Cabling.SCT_CablingConfig import SCT_CablingToolCfg
    acc.popToolsAndMerge(SCT_CablingToolCfg(flags))

    # For EventInfo necessary for ByteStream file writing
    from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
    acc.merge(EventInfoCnvAlgCfg(flags,
                                 inputKey="McEventInfo",
                                 outputKey="EventInfo"))

    acc.run(maxEvents=10)
