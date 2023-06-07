#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

def LArSuperCellCfg(inputFlags):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    acc = ComponentAccumulator()

    # should we create bcid'ed container ?
    if inputFlags.LAr.DT.storeET_ID:
       from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
       acc.merge(LArRawSCDataReadingCfg(inputFlags))
       acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg'))
       from LArCellRec.LArRAWtoSuperCellConfig import LArRAWtoSuperCellCfg
       acc.merge(LArRAWtoSuperCellCfg(inputFlags,mask=inputFlags.LAr.DT.doSCMasking, SCellContainerOut=inputFlags.LAr.DT.ET_IDKey) )
       
       acc.merge(addToESD(inputFlags, ["CaloCellContainer#"+inputFlags.LAr.DT.ET_IDKey]))
       acc.merge(addToAOD(inputFlags, ["CaloCellContainer#"+inputFlags.LAr.DT.ET_IDKey]))

    # should we create additional containers ?
    if inputFlags.LAr.DT.storeET_additional:
       from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
       acc.merge(LArRawSCDataReadingCfg(inputFlags))
       acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg'))
       from LArCellRec.LArRAWtoSuperCellConfig import LArRAWtoSuperCellCfg
       acc.merge(LArRAWtoSuperCellCfg(inputFlags,name='LArRAWtoSuperCellPlus', mask=inputFlags.LAr.DT.doSCMasking, SCellContainerOut=inputFlags.LAr.DT.ET_PlusKey, bcidShift=1) )
       outContainers = ["CaloCellContainer#"+inputFlags.LAr.DT.ET_PlusKey]
       acc.merge(LArRAWtoSuperCellCfg(inputFlags,name='LArRAWtoSuperCellMinus', mask=inputFlags.LAr.DT.doSCMasking, SCellContainerOut=inputFlags.LAr.DT.ET_MinusKey, bcidShift=-1) )
       outContainers += ["CaloCellContainer#"+inputFlags.LAr.DT.ET_MinusKey]

       acc.merge(addToESD(inputFlags,outContainers)) 
       acc.merge(addToAOD(inputFlags,outContainers)) 

    return acc



if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags=initConfigFlags()
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)


   #from AthenaConfiguration.TestDefaults import defaultTestFiles
   #flags.Input.Files = defaultTestFiles.RAW_RUN2
   flags.Input.Files = ["/eos/home-p/pavol/data/data23_13p6TeV.00452669.express_express.merge.RAW._lb0987._SFO-ALL._0001.1"]

   flags.LAr.DT.storeET_ID = True
   flags.LAr.DT.storeET_additional = True

   flags.lock()

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
   cfg = MainServicesCfg(flags)
   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg.merge(CaloRecoCfg(flags))

   acc = LArSuperCellCfg(flags)
   cfg.merge(acc)

   from AthenaCommon.SystemOfUnits import GeV
   from AthenaConfiguration.ComponentFactory import CompFactory
   cfg.addEventAlgo(CompFactory.CaloCellDumper(InputContainer=flags.LAr.DT.ET_IDKey,EnergyCut=1*GeV),sequenceName="AthAlgSeq")

   cfg.getService("MessageSvc").OutputLevel=DEBUG
   cfg.getService("StoreGateSvc").Dump=True

   cfg.printConfig()

   flags.dump()
   f=open("LArSuperCell.pkl","wb")
   cfg.store(f)
   f.close()

   cfg.run(10)
