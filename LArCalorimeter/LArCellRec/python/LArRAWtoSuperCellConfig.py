# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg

def LArRAWtoSuperCellCfg(configFlags,name='LArRAWtoSuperCell', mask=True,SCellContainerOut="SCell_ET",bcidShift=0):
    result=ComponentAccumulator()
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArRAWtoSuperCellCfg:' )
    result.merge(LArOnOffIdMappingSCCfg(configFlags))
    SCInput=""
    if ( len(configFlags.Input.RunNumber) > 0 ):
       from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
       runinfo=getLArDTInfoForRun(configFlags.Input.RunNumber[0], connstring="COOLONL_LAR/CONDBR2")
       for i in range(0,len(runinfo.streamTypes())):
         if runinfo.streamTypes()[i] ==  "SelectedEnergy":
           SCInput="SC_ET_ID"
    else :
       SCInput=configFlags.LAr.LATOME.DTInfoForL1
    if mask :
       LArBadChannelKey="LArBadChannelSC"
    else :
       LArBadChannelKey=""
    result.merge(LArBadChannelCfg(configFlags,isSC=True,tag="LARBadChannelsBadChannelsSC-RUN3-UPD1-00") )

    algo = CompFactory.LArRAWtoSuperCell(name,SCellContainerOut=SCellContainerOut,LArBadChannelKey=LArBadChannelKey,BCIDOffset=bcidShift)

    if mask and not configFlags.Input.isMC:
        # also setup to read OTF masked supercells if running on data
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        result.merge(addFolders(configFlags,"/LAR/BadChannels/MaskedSC","LAR_ONL",tag="LARBadChannelsMaskedSC-RUN3-UPD1-00",
                                className="CondAttrListCollection",extensible=configFlags.Common.isOnline )) # when run online, need folder to be extensible to force reload each event
        condAlgo = CompFactory.LArBadChannelCondAlg(name="MaskedSCCondAlg",ReadKey="/LAR/BadChannels/MaskedSC",isSC=True,
                                                    CablingKey="LArOnOffIdMapSC",WriteKey="LArMaskedSC",ReloadEveryEvent=configFlags.Common.isOnline)
        result.addCondAlgo(condAlgo)
        algo.LArMaskedChannelKey=condAlgo.WriteKey

    if ( SCInput == ""):
       mlog.info("Not setting SCInput container name")
    else :
       mlog.info("Setting SCInput container name to %s",SCInput)
       algo.SCellContainerIn = SCInput
    result.addEventAlgo(algo)
       
    return result
