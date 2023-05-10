#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD


def ZdcRecCfg(flags, DoCalib=False, DoTimeCalib=False, DoTrigEff=False):    
    """Configure Zdc analysis alg
    Additional arguments are useful in calibration runs
    """
    #TODO a better way to find data taking period (not by looking at Geo tag)
    if int(flags.Input.ProjectName.strip("data").split("_")[0]) < 20:
        return ZdcRecRun2Cfg(flags, DoCalib=DoCalib, DoTimeCalib=DoTimeCalib, DoTrigEff=DoTrigEff)

def ZdcRecRun2Cfg(flags, DoCalib=False, DoTimeCalib=False, DoTrigEff=False):        
    acc = ComponentAccumulator()

    from AtlasGeoModel.ForDetGeoModelConfig import ForDetGeometryCfg
    acc.merge(ForDetGeometryCfg(flags))

    acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData())

    alg = CompFactory.ZdcRec()
    acc.addEventAlgo(alg)
    acc.merge(addToAOD(flags, ['ZdcRawChannelCollection#ZdcRawChannelCollection']))

    return acc

if __name__ == '__main__':
    """ This is selftest & Zdc analysis transform at the same time"""
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    flags = initConfigFlags()
    # if these are promoted to globally available flags should be removed from here
    flags.addFlag("DoCalib", False)
    flags.addFlag("DoTimeCalib", False)
    flags.addFlag("DoTrigEff", False)
    flags.Detector.GeometryZDC=True
    flags.Detector.GeometryAFP=False
    flags.Detector.GeometryALFA=False
    flags.Detector.GeometryLucid=False
    flags.Trigger.DecodeHLT=False
    flags.Trigger.enableL1MuonPhase1=False
    flags.Trigger.L1.doMuon=False
    flags.Trigger.L1.doCalo=False
    flags.Trigger.L1.doTopo=False
    flags.Trigger.EDMVersion=2
    flags.Output.AODFileName="calibAOD.pool.root"
    flags.Output.doWriteAOD=True
    flags.fillFromArgs()
    flags.lock()

    acc=MainServicesCfg(flags)

    # if need to read BS file
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    from TriggerJobOpts.TriggerRecoConfig import TriggerRecoCfgData
    acc.merge(TriggerRecoCfgData(flags))

    acc.merge(ZdcRecCfg(flags,
                        DoCalib=flags.DoCalib, 
                        DoTimeCalib=flags.DoTimeCalib,
                        DoTrigEff=flags.DoTrigEff))

    # example on how to enable logging for decoder
    # print(help(acc.getService("MessageSvc")))
    # acc.getService("MessageSvc").setDebug = ["ZdcLucrodDecoder"]
    
    acc.printConfig(withDetails=True)

    with open("config.pkl", "wb") as f:
        acc.store(f)
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
