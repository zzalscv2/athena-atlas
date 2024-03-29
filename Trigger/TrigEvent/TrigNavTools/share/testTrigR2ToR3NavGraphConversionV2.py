#!/usr/bin/env python
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Output file can be checked (and navigation graphs converted using):
#  
# athena TrigNavTools/navGraphDump.py
# see there for more info


if __name__=='__main__':
    import sys

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    flags.Input.Files=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357772.physics_Main.recon.AOD.r13286/AOD.27654050._000557.pool.root.1"]

    flags.Output.AODFileName = "outAOD.pool.root"
    flags.Detector.GeometryLAr=True
    flags.Detector.GeometryTile=True
    flags.Exec.MaxEvents = 1000
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.ComponentFactory import CompFactory

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))


    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    cfg.merge(MetaDataSvcCfg(flags))

    confSvc = CompFactory.TrigConf.xAODConfigSvc("xAODConfigSvc")
    cfg.addService(confSvc)
    from AthenaCommon.Constants import DEBUG
    alg = CompFactory.Run2ToRun3TrigNavConverterV2("TrigNavCnv", OutputLevel=DEBUG, TrigConfigSvc=confSvc)
    alg.doSelfValidation = False
    alg.doCompression = True
    alg.addTauTracks = False

    alg.Collections = ["xAOD::TrigEMCluster", "xAOD::TrigEMClusterContainer", "xAOD::TrigRingerRings", "xAOD::TrigRingerRingsContainer", "xAOD::TrigRNNOutput", "xAOD::TrigRNNOutputContainer", "xAOD::CaloClusterContainer", "xAOD::L2StandAloneMuonContainer", "xAOD::L2StandAloneMuonAuxContainer", "xAOD::L2CombinedMuonContainer", "xAOD::L2CombinedMuonAuxContainer","xAOD::L2IsoMuonContainer", "xAOD::MuonContainer", "xAOD::MuonAuxContainer","xAOD::TauJetContainer", "xAOD::ElectronContainer", "xAOD::PhotonContainer", "xAOD::JetContainer", "xAOD::BTaggingContainer", "xAOD::BTagVertexContainer", "xAOD::JetElementContainer", "xAOD::TrigMissingET", "xAOD::TrigBphysContainer"]
    if (alg.addTauTracks):
        alg.Collections.append("xAOD::TauTrackContainer")
    
    # simple mu test cases
    alg.Chains = ["HLT_mu4","HLT_mu6","HLT_mu10","HLT_mu6_2mu4","HLT_mu22"]

    alg.Rois = ["initialRoI","forID","forID1","forID2","forMS","forSA","forTB","forMT","forCB"]


    cfg.addEventAlgo(alg, sequenceName="AthAlgSeq")
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    outputType="AOD"
    toRecord = ["xAOD::TrigCompositeContainer#HLTNav_All", "xAOD::TrigCompositeAuxContainer#HLTNav_AllAux.",
                "xAOD::TrigCompositeContainer#HLTNav_Summary", "xAOD::TrigCompositeAuxContainer#HLTNav_SummaryAux."]
    outputCfg = OutputStreamCfg(flags, outputType, ItemList=toRecord, disableEventTag=True, takeItemsFromInput = True)
    streamAlg = outputCfg.getEventAlgo("OutputStream"+outputType)
    # need to expand possible options for the OutputStreamCfg to be able to pass also the metadata containers
    streamAlg.MetadataItemList += ["xAOD::TriggerMenuContainer#TriggerMenu", "xAOD::TriggerMenuAuxContainer#TriggerMenuAux."]
    cfg.addPublicTool(CompFactory.xAODMaker.TriggerMenuMetaDataTool("TriggerMenuMetaDataTool"))
    cfg.addService( CompFactory.MetaDataSvc("MetaDataSvc", MetaDataTools = [cfg.getPublicTool("TriggerMenuMetaDataTool")]))

    cfg.merge(outputCfg)

    # input EDM needs calo det descrition for conversion (uff)
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(LArGMCfg(flags))
    cfg.merge(TileGMCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=False) # set True for exhaustive info
    sc = cfg.run()
    sys.exit(0 if sc.isSuccess() else 1)
