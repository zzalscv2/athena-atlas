# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from LArBadChannelTool.LArBadChannelConfig import LArBadFebCfg
from AthenaCommon.Logging import logging
from OutputStreamAthenaPool.OutputStreamConfig import addToESD

def LArFebErrorSummaryMakerCfg(flags):

    log = logging.getLogger('LArFebErrorSummaryMakerConfig')
    projectName=flags.Input.ProjectName

    bCheckAllFEB=True
    streamName=flags.Input.ProcessingTags
    if len(streamName) > 0 and len(streamName[0])>4 and streamName[0].endswith("PEB"):
        log.info("StreamName %s suggests partial event building. Do not check for FEB completeness",str(streamName))
        bCheckAllFEB=False


    if projectName == "data_test":
        from datetime import date
        yearNumber=date.today().year-2000
        log.info("Found project name data_test, assume year number to be %d",yearNumber)
    else:
        try:
            yearNumber=int(projectName[4:6])
        except ValueError:
            from datetime import date
            yearNumber=date.today().year-2000
            log.warning("Failed to extract year from project tag "+ projectName+". Guessing %d",yearNumber)

    if yearNumber > 20:
       lMaskFebScacStatus = [0x38680000,0x38720000]
       lMaskFebEvtId      = [0x38680000]
    else:
       lMaskFebScacStatus = [0x38080000]
       lMaskFebEvtId      = [0x38080000]   
    # needed only if it is not in DB.
    #lMaskFebZeroSample = [0x39618000,0x39930000,0x3b1b0000,0x38db0000,0x38f60000,0x39ae8000,0x3bb90000]

    acc = LArBadFebCfg(flags)

    minErrFeb=1
    # In online or express processing, EventInfo::LArError is triggered if >=4 FEB with data corrupted
    if flags.Common.isOnline or flags.Common.doExpressProcessing:
       minErrFeb=4

    acc.addEventAlgo(CompFactory.LArFebErrorSummaryMaker("LArFebErrorSummaryMaker",CheckAllFEB=bCheckAllFEB,
                                         WriteKey="StoreGateSvc+LArFebErrorSummary",
                                         MaskFebScacStatus = lMaskFebScacStatus, MaskFebEvtId = lMaskFebEvtId,
                                         minFebInError=minErrFeb
                                         #MaskFebZeroSample = lMaskFebZeroSample,
                                         )
                    )

    acc.merge(addToESD(flags,["LArFebErrorSummary#LArFebErrorSummary",]))

    return acc


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.lock()


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg    

    acc=MainServicesCfg(flags)
    acc.merge(LArRawDataReadingCfg(flags))
    acc.merge(LArFebErrorSummaryMakerCfg(flags))
    
    acc.run(3)
