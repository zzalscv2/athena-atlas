# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg

def LArRAWtoSuperCellCfg(configFlags,mask=True,SCellContainerIn="SC_ET_ID",SCellContainerOut="SCell_ET"):
    result=ComponentAccumulator()
    result.merge(LArOnOffIdMappingSCCfg(configFlags))
    if mask :
       LArBadChannelKey="LArBadChannelSC"
    else :
       LArBadChannelKey=""
    result.merge(LArBadChannelCfg(configFlags,isSC=True,tag="LARBadChannelsBadChannelsSC-RUN3-UPD1-00") )
    result.addEventAlgo(CompFactory.LArRAWtoSuperCell(SCellContainerIn=SCellContainerIn, SCellContainerOut=SCellContainerOut,LArBadChannelKey=LArBadChannelKey))
    return result
