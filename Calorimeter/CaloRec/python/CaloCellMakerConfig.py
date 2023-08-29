# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from LArCellRec.LArCellBuilderConfig import LArCellBuilderCfg,LArCellCorrectorCfg
from TileRecUtils.TileCellBuilderConfig import TileCellBuilderCfg
from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg, CaloCellNeighborsAverageCorrCfg, CaloCellTimeCorrCfg, CaloEnergyRescalerCfg

def CaloCellMakerCfg(flags):
    result=ComponentAccumulator()
   
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    
    result.merge(LArGMCfg(flags))
    result.merge(TileGMCfg(flags))

    larCellBuilder     = result.popToolsAndMerge(LArCellBuilderCfg(flags))
    larCellCorrectors  = result.popToolsAndMerge(LArCellCorrectorCfg(flags))
    tileCellBuilder = result.popToolsAndMerge(TileCellBuilderCfg(flags))
    cellFinalizer  = CompFactory.CaloCellContainerFinalizerTool()

    cellMakerTools=[larCellBuilder,tileCellBuilder,cellFinalizer]+larCellCorrectors

    #Add corrections tools that are not LAr or Tile specific:
    if flags.Calo.Cell.doPileupOffsetBCIDCorr or flags.Calo.Cell.doPedestalCorr:
        theCaloCellPedestalCorr=CaloCellPedestalCorrCfg(flags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloCellPedestalCorr))

    #LAr HV scale corr must come after pedestal corr
    if flags.LAr.doHVCorr:
        from LArCellRec.LArCellBuilderConfig import LArHVCellContCorrCfg
        theLArHVCellContCorr=LArHVCellContCorrCfg(flags)
        cellMakerTools.append(result.popToolsAndMerge(theLArHVCellContCorr))


    if flags.Calo.Cell.doDeadCellCorr:
        theCaloCellNeighborAvg=CaloCellNeighborsAverageCorrCfg(flags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloCellNeighborAvg))

    if flags.Calo.Cell.doEnergyCorr:
        theCaloCellEnergyRescaler=CaloEnergyRescalerCfg(flags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloCellEnergyRescaler))
    if flags.Calo.Cell.doTimeCorr:
        theCaloTimeCorr=CaloCellTimeCorrCfg(flags)
        cellMakerTools.append(result.popToolsAndMerge(theCaloTimeCorr))

    cellAlgo = CompFactory.CaloCellMaker(CaloCellMakerToolNames=cellMakerTools,
                                         CaloCellsOutputName="AllCalo",
                                         EnableChronoStat=(flags.Concurrency.NumThreads == 0))

    result.addEventAlgo(cellAlgo, primary=True)

    outputContainers = ["CaloCellContainer#AllCalo"]
    if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2, LHCPeriod.Run3]:
        outputContainers += ["TileCellContainer#MBTSContainer"]
    if flags.GeoModel.Run is LHCPeriod.Run2:
        outputContainers += ["TileCellContainer#E4prContainer"]
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    result.merge(addToESD(flags, outputContainers))
    result.merge(addToAOD(flags, outputContainers))

    # Add a SuperCell container creation, if asked by flags
    if flags.LAr.DT.storeET_ID or flags.LAr.DT.storeET_additional:
       from LArConfiguration.LArSuperCellConfig import LArSuperCellCfg
       result.merge(LArSuperCellCfg(flags))

    return result

 
                                      
if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg=MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.addEventAlgo(CompFactory.xAODMaker.EventInfoCnvAlg(),sequenceName="AthAlgSeq")


    acc=CaloCellMakerCfg(flags)
    acc.getPrimary().CaloCellsOutputName="AllCaloNew"
    cfg.merge(acc)

    import sys, os
    thisdir = os.path.dirname (sys.argv[0])
    from AthenaCommon.SystemOfUnits import GeV
    cfg.addEventAlgo(CompFactory.CaloCellDumper(InputContainer="AllCaloNew",EnergyCut=2*GeV,
                                                RefName=os.path.join (thisdir, '../share/CaloCells.txt.ref')),sequenceName="AthAlgSeq")

    cfg.run(5)

