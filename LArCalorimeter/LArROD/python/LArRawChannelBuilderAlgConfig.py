# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
from LArRecUtils.LArRecUtilsConfig import LArOFCCondAlgCfg
from LArConfiguration.LArConfigFlags import RawChannelSource

def LArRawChannelBuilderAlgCfg(configFlags, **kwargs):

    acc = LArADC2MeVCondAlgCfg(configFlags)

    kwargs.setdefault("name", "LArRawChannelBuilder")
    kwargs.setdefault("firstSample", configFlags.LAr.ROD.nPreceedingSamples if configFlags.LAr.ROD.nPreceedingSamples!=0 else configFlags.LAr.ROD.FirstSample)
    obj = "AthenaAttributeList"
    dspkey = 'Run2DSPThresholdsKey'
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    if configFlags.Input.isMC:
        # need OFC configuration, which includes appropriate ElecCalibDb
        acc.merge(LArOFCCondAlgCfg(configFlags))
        kwargs.setdefault("LArRawChannelKey", "LArRawChannels")
        kwargs.setdefault("ShapeKey", "LArShapeSym")
        if configFlags.GeoModel.Run is LHCPeriod.Run1:  # back to flat threshold
           kwargs.setdefault("useDB", False)
           dspkey = ''
        else:
           fld="/LAR/NoiseOfl/DSPThresholds"
           sgkey=fld
           dbString="OFLP200"
           dbInstance="LAR_OFL"
           acc.merge(addFolders(configFlags,fld, dbInstance, className=obj, db=dbString))

        if configFlags.Common.ProductionStep is ProductionStep.PileUpPresampling:
            kwargs.setdefault("LArDigitKey", configFlags.Overlay.BkgPrefix + "LArDigitContainer_MC")
        else:
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")
    else:
        acc.merge(LArElecCalibDBCfg(configFlags,("OFC","Shape","Pedestal")))
        if configFlags.Overlay.DataOverlay:
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")
            kwargs.setdefault("LArRawChannelKey", "LArRawChannels")
        else:
            kwargs.setdefault("LArRawChannelKey", "LArRawChannels_FromDigits")
        if 'COMP200' in configFlags.IOVDb.DatabaseInstance:
            fld='/LAR/Configuration/DSPThreshold/Thresholds'
            obj='LArDSPThresholdsComplete'
            dspkey = 'Run1DSPThresholdsKey'
            sgkey='LArDSPThresholds'
            dbString = 'COMP200'
        else:
            fld="/LAR/Configuration/DSPThresholdFlat/Thresholds"
            sgkey=fld
            dbString="CONDBR2"
        dbInstance="LAR_ONL"
        acc.merge(addFolders(configFlags,fld, dbInstance, className=obj, db=dbString))

    if len (dspkey) > 0:
        kwargs.setdefault(dspkey, sgkey)

    if configFlags.LAr.ROD.forceIter or configFlags.LAr.RawChannelSource is RawChannelSource.Calculated:
       # iterative OFC procedure
       kwargs.setdefault('minSample',2)
       kwargs.setdefault('maxSample',12)
       kwargs.setdefault('minADCforIterInSigma',4)
       kwargs.setdefault('minADCforIter',15)
       kwargs.setdefault('defaultPhase',12)
       nominalPeakSample=2
       from LArConditionsCommon.LArRunFormat import getLArFormatForRun
       larformat=getLArFormatForRun(configFlags.Input.RunNumbers[0],connstring="COOLONL_LAR/"+configFlags.IOVDb.DatabaseInstance)
       if larformat is not None:
          nominalPeakSample = larformat.firstSample()
       else:
          print("WARNING: larformat not found, use nominalPeakSample = 2")
          nominalPeakSample = 2
       if (nominalPeakSample > 1) :
          kwargs.setdefault('DefaultShiftTimeSample',nominalPeakSample-2)
       else :
          kwargs.setdefault('DefaultShiftTimeSample',0)   

       acc.addEventAlgo(CompFactory.LArRawChannelBuilderIterAlg(**kwargs))
    else:
       #fixed OFC, as in DSP
       acc.addEventAlgo(CompFactory.LArRawChannelBuilderAlg(**kwargs))

    return acc


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RAW_RUN2
    # in case of testing iterative OFC:
    #ConfigFlags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data15_1beam/data15_1beam.00260466.physics_L1Calo.merge.RAW._lb1380._SFO-ALL._0001.1']
    ConfigFlags.Input.isMC = False
    ConfigFlags.Detector.GeometryTile = False
    ConfigFlags.lock()


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg    

    acc=MainServicesCfg(ConfigFlags)
    acc.merge(LArRawDataReadingCfg(ConfigFlags))
    acc.merge(LArRawChannelBuilderAlgCfg(ConfigFlags))
    
    DumpLArRawChannels=CompFactory.DumpLArRawChannels
    acc.addEventAlgo(DumpLArRawChannels(LArRawChannelContainerName="LArRawChannels_FromDigits",),sequenceName="AthAlgSeq")
    
    acc.run(3)
