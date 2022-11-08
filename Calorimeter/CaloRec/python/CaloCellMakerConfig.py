# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from LArCellRec.LArCellBuilderConfig import LArCellBuilderCfg,LArCellCorrectorCfg
from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg, CaloCellNeighborsAverageCorrCfg, CaloCellTimeCorrCfg, CaloEnergyRescalerCfg

def CaloCellMakerCfg(configFlags):
    result=ComponentAccumulator()
   
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    
    result.merge(LArGMCfg(configFlags))
    result.merge(TileGMCfg(configFlags))

    larCellBuilder     = result.popToolsAndMerge(LArCellBuilderCfg(configFlags))
    larCellCorrectors  = result.popToolsAndMerge(LArCellCorrectorCfg(configFlags))
    tileCellBuilder = result.popToolsAndMerge(TileCellBuilderCfg(configFlags))
    cellFinalizer  = CompFactory.CaloCellContainerFinalizerTool()

    cellMakerTools=[larCellBuilder,tileCellBuilder,cellFinalizer]+larCellCorrectors

    #Add corrections tools that are not LAr or Tile specific:
    if configFlags.Calo.Cell.doPileupOffsetBCIDCorr or configFlags.Calo.Cell.doPedestalCorr:
        theCaloCellPedestalCorr=CaloCellPedestalCorrCfg(configFlags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloCellPedestalCorr))

    #LAr HV scale corr must come after pedestal corr
    if configFlags.LAr.doHVCorr:
        from LArCellRec.LArCellBuilderConfig import LArHVCellContCorrCfg
        theLArHVCellContCorr=LArHVCellContCorrCfg(configFlags)
        cellMakerTools.append(result.popToolsAndMerge(theLArHVCellContCorr))


    if configFlags.Calo.Cell.doDeadCellCorr:
        theCaloCellNeighborAvg=CaloCellNeighborsAverageCorrCfg(configFlags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloCellNeighborAvg))

    if configFlags.Calo.Cell.doEnergyCorr:
        theCaloCellEnergyRescaler=CaloEnergyRescalerCfg(configFlags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloCellEnergyRescaler))
    if configFlags.Calo.Cell.doTimeCorr:
        theCaloTimeCorr=CaloCellTimeCorrCfg(configFlags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloTimeCorr))

    cellAlgo = CompFactory.CaloCellMaker(CaloCellMakerToolNames=cellMakerTools,
                                         CaloCellsOutputName="AllCalo")

    result.addEventAlgo(cellAlgo, primary=True)

    outputContainers = ["CaloCellContainer#AllCalo"]
    if configFlags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2, LHCPeriod.Run3]:
        outputContainers += ["TileCellContainer#MBTSContainer"]
    if configFlags.GeoModel.Run is LHCPeriod.Run2:
        outputContainers += ["TileCellContainer#E4prContainer"]
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    result.merge(addToESD(configFlags, outputContainers))
    result.merge(addToAOD(configFlags, outputContainers))

    return result

 
                                      
if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))
    cfg.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(),sequenceName="AthAlgSeq")


    acc=CaloCellMakerCfg(ConfigFlags)
    acc.getPrimary().CaloCellsOutputName="AllCaloNew"
    cfg.merge(acc)
    
    from AthenaCommon.SystemOfUnits import GeV
    cfg.addEventAlgo(CompFactory.CaloCellDumper(InputContainer="AllCaloNew",EnergyCut=2*GeV))

    #cfg.getService("StoreGateSvc").Dump=True
    cfg.run(5)

    #f=open("CaloCellMaker.pkl","wb")
    #cfg.store(f)
    #f.close()

    #ConfigFlags.dump()
