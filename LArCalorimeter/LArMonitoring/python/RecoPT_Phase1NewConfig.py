# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LArDTMonitoringConfig(ConfigFlags,STREAM):
    
    acc=ComponentAccumulator()
    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(ConfigFlags))

    from LArCabling.LArCablingConfig import LArLATOMEMappingCfg
    acc.merge(LArLATOMEMappingCfg(ConfigFlags))

    LArRawSCDataReadingAlg=CompFactory.LArRawSCDataReadingAlg
    acc.addEventAlgo(LArRawSCDataReadingAlg(LATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder",ProtectSourceId = True)))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(ConfigFlags))
    acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg')) 

    larLATOMEBuilderAlg=CompFactory.LArLATOMEBuilderAlg("LArLATOMEBuilderAlg",LArDigitKey="SC", isADCBas=False)
    acc.addEventAlgo(larLATOMEBuilderAlg)

    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'RecoPT_Phase1' )

    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    mlog.info("Run number: "+str(ConfigFlags.Input.RunNumber[0]))

    try:
        runinfo=getLArDTInfoForRun(ConfigFlags.Input.RunNumber[0], connstring="COOLONL_LAR/CONDBR2")
        streams=runinfo.streamTypes()
        nsamples=int(runinfo.recipe().split("_bc")[1].split("-")[0])
    except Exception:
        mlog.warning("Could not get DT run info, using defaults !")
        streams=["SelectedEnergy","ADC"]
        nsamples=32

    from LArMonitoring.LArDigitalTriggMonAlg import LArDigitalTriggMonConfig
    acc.merge(LArDigitalTriggMonConfig(ConfigFlags, larLATOMEBuilderAlg, nsamples, streams))

    return acc
