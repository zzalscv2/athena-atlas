# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def DiTauOutputCfg(flags):

   from OutputStreamAthenaPool.OutputStreamConfig import addToESD,addToAOD
   result=ComponentAccumulator()

   DiTauOutputList  = [ "xAOD::DiTauJetContainer#DiTauJets" ]
   DiTauOutputList += [ "xAOD::DiTauJetAuxContainer#DiTauJetsAux." ]

   result.merge(addToESD(flags,DiTauOutputList))
   result.merge(addToAOD(flags,DiTauOutputList))
   return result

def DiTauReconstructionCfg(flags):

    result = ComponentAccumulator()

    from DiTauRec.DiTauBuilderConfig import DiTauBuilderCfg
    result.merge(DiTauBuilderCfg(flags))

    if (flags.Output.doWriteESD or flags.Output.doWriteAOD):
        result.merge(DiTauOutputCfg(flags))

    return result


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()

    ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]


    # Use latest MC21 tag to pick up latest muon folders apparently needed
    ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    ConfigFlags.Output.ESDFileName = "ESD.pool.root"
    ConfigFlags.Output.AODFileName = "AOD.pool.root"

    nThreads=1
    ConfigFlags.Concurrency.NumThreads = nThreads
    if nThreads>0:
        ConfigFlags.Scheduler.ShowDataDeps = True
        ConfigFlags.Scheduler.ShowDataFlow = True
        ConfigFlags.Scheduler.ShowControlFlow = True
        ConfigFlags.Concurrency.NumConcurrentEvents = nThreads

    from AthenaCommon.AlgScheduler import AlgScheduler
    AlgScheduler.ShowControlFlow( True )
    AlgScheduler.ShowDataDependencies( True )
    AlgScheduler.setDataLoaderAlg('SGInputLoader')

    ConfigFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    StoreGateSvc=CompFactory.StoreGateSvc
    cfg.addService(StoreGateSvc("DetectorStore"))

    # this delcares to the scheduler that EventInfo object comes from the input
    loadFromSG = [('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                  ( 'AthenaAttributeList' , 'StoreGateSvc+Input' ),
                  ( 'CaloCellContainer' , 'StoreGateSvc+AllCalo' )]
    cfg.addEventAlgo(CompFactory.SGInputLoader(Load=loadFromSG), sequenceName="AthAlgSeq")

    cfg.merge(DiTauReconstructionCfg(ConfigFlags))

    cfg.run(1000)
