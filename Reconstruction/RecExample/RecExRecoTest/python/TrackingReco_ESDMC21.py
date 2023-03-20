# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    ConfigFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]
    # Use latest MC21 tag to pick up latest muon folders apparently needed
    ConfigFlags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    calo_seeds=True

    import os
    if os.environ.get('ATHENA_CORE_NUMBER',None) is not None :
        ConfigFlags.Concurrency.NumThreads = int(os.environ['ATHENA_CORE_NUMBER'])

    if not calo_seeds :
        ConfigFlags.Tracking.doCaloSeededBrem=False
        ConfigFlags.Tracking.doHadCaloSeededSSS=False
        ConfigFlags.Tracking.doCaloSeededAmbi=False
        ConfigFlags.Detector.EnableCalo=False

    ConfigFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(ConfigFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(ConfigFlags))

    if calo_seeds :
        from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
        acc.merge(CaloTopoClusterCfg(ConfigFlags))

    if ConfigFlags.Tracking.doTruth :
        from xAODTruthCnv.RedoTruthLinksConfig import RedoTruthLinksAlgCfg
        acc.merge( RedoTruthLinksAlgCfg(ConfigFlags) )

    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(ConfigFlags))

    with open("config.pkl", "wb") as file:
      acc.store(file)

    acc.run(100)
