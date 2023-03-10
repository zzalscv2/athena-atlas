#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

def LArMonitoringConfig(inputFlags):

    from AthenaMonitoring.DQConfigFlags import DQDataType
    from LArMonitoring.LArCollisionTimeMonAlg import LArCollisionTimeMonConfig
    from LArMonitoring.LArAffectedRegionsAlg import LArAffectedRegionsConfig
    from LArMonitoring.LArDigitMonAlg import LArDigitMonConfig
    from LArMonitoring.LArRODMonAlg import LArRODMonConfig
    from LArMonitoring.LArNoisyROMonAlg import LArNoisyROMonConfig
    from LArMonitoring.LArFEBMonAlg import LArFEBMonConfig
    from LArMonitoring.LArHVCorrMonAlg import LArHVCorrMonConfig
    from LArMonitoring.LArCoverageAlg import LArCoverageConfig
    from LArMonitoring.LArNoiseCorrelationMonAlg import LArNoiseCorrelationMonConfig

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    
    # algos which can run in ESD but not AOD:
    if inputFlags.DQ.Environment != 'AOD':
        if inputFlags.DQ.DataType is not DQDataType.Cosmics:
            from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
            acc.merge(BunchCrossingCondAlgCfg(inputFlags))
            acc.merge(LArCollisionTimeMonConfig(inputFlags))
        if not inputFlags.Input.isMC:
            acc.merge(LArNoisyROMonConfig(inputFlags))
            if 'online' not in inputFlags.DQ.Environment:
                acc.merge(LArAffectedRegionsConfig(inputFlags))
                acc.merge(LArHVCorrMonConfig(inputFlags))


    # and others on RAW data only
    if inputFlags.DQ.Environment in ('online', 'tier0', 'tier0Raw'):
       if not inputFlags.Input.isMC:
          acc.merge(LArFEBMonConfig(inputFlags))
          acc.merge(LArDigitMonConfig(inputFlags))
          from LArConfiguration.LArConfigFlags import RawChannelSource
          if inputFlags.LAr.RawChannelSource != RawChannelSource.Calculated:
              acc.merge(LArRODMonConfig(inputFlags))
          acc.merge(LArCoverageConfig(inputFlags))
          acc.merge(LArNoiseCorrelationMonConfig(inputFlags))

    return acc



if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags=initConfigFlags()
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)


   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RAW_RUN2

   flags.Output.HISTFileName = 'LArMonitoringOutput.root'
   flags.DQ.enableLumiAccess = True
   flags.DQ.useTrigger = True
   flags.lock()

   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

   #from CaloD3PDMaker.CaloD3PDConfig import CaloD3PDCfg,CaloD3PDAlg
   #cfg.merge(CaloD3PDCfg(flags, filename=flags.Output.HISTFileName, streamname='CombinedMonitoring'))

   acc = LArMonitoringConfig(flags)
   cfg.merge(acc)

   cfg.printConfig()

   flags.dump()
   f=open("LArMonitoring.pkl","wb")
   cfg.store(f)
   f.close()

   #cfg.run(100)
