#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__ == '__main__':
    ##################################################
    # Add an argument parser
    ##################################################
    from AthenaCommon.Logging import logging
    local_log = logging.getLogger('run_gep')
    info = local_log.info
    
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('-i', '--input',
                   metavar='KEY',
                   default='ttbar',
                   help='Key of the input from TrigValInputs to be used, default=%(default)s')
    p.add_argument('-e', '--execute',
                   action='store_true',
                   help='After building the configuration, also process a few events')
    p.add_argument('-n', '--nevents',
                   metavar='N',
                   type=int,
                   default=25,
                   help='Number of events to process if --execute is used, default=%(default)s')
    p.add_argument('-g', '--gepInput',
                   metavar='N',
                   type=bool,
                   default=False,
                   help='use input files previously used for GEP jobs')
    p.add_argument('-f', '--gepInputRun3',
                   action='store_true',
                   help='use Run3 (with jFexJetRoI) input file')

    p.add_argument('-c', '--clusterAlgs',
                        default='WFS',
                        help='commma separated list of stategies for GepClusterAlg: [WFS, Calo420]')
    
    p.add_argument('-j', '--jetAlgs',
                        default='Cone',
                        help='commma separated list of stategies for GepJetAlg:[Cone, ModAntikT]')

    args = p.parse_args()

    clusterAlgNames = args.clusterAlgs.split(',')
    jetAlgNames = args.jetAlgs.split(',')
    info('clusterAlgs: ' + str(clusterAlgNames))
    info('jetAlgs: ' + str(jetAlgNames))

    ##################################################
    # Configure all the flags
    ##################################################
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from TrigValTools.TrigValSteering import Input

    assert not (args.gepInput and args.gepInputRun3)
    
    if args.gepInput:
         ifile = (
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc20e_13TeV/"
        "valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.ESD.e4993_s3227_r12689/myESD.pool.root")
         flags.Input.Files = [ifile]
    elif args.gepInputRun3:
        # ifile = (
            # '/afs/cern.ch/work/m/martyniu/public/forPeter/'
            # 'mc21.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8357_e7400_s3775_r13614_r13614/'
            #'AOD.29004489._000039.pool.root.1')

        ifile = '/afs/cern.ch/work/m/martyniu/public/forPeter/RDO/tmp.RDO'
        flags.Input.Files = [ifile]
    else:
        flags.Input.Files = Input.get_input(args.input).paths

    info('Command line args: ' + str(args))

    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Common.isOnline = not flags.Input.isMC
    flags.Exec.MaxEvents = args.nevents
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True
    if flags.Common.isOnline:
        flags.IOVDb.GlobalTag = flags.Trigger.OnlineCondTag

    # TODO 1: Reverse this into a special setting for Run-2 data input when the default geo tag is changed to Run-3
    # TODO 2: Any better way of figuring this out than run number?
    if not flags.Input.isMC and flags.Input.RunNumber[0] > 400000:
        flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-02-00-00'

    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    flags.lock()
    flags.dump()

    ##################################################
    # Set up central services: Main + Input reading + L1Menu + Output writing
    ##################################################
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
    else:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu, createL1PrescalesFileFromMenu
    acc.merge(L1ConfigSvcCfg(flags))
    generateL1Menu(flags)
    createL1PrescalesFileFromMenu(flags)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    FexEDMList = [
        'xAOD::eFexEMRoIContainer#L1_eEMRoI','xAOD::eFexEMRoIAuxContainer#L1_eEMRoIAux.',
        'xAOD::eFexTauRoIContainer#L1_eTauRoI','xAOD::eFexTauRoIAuxContainer#L1_eTauRoIAux.',
        'xAOD::jFexTauRoIContainer#L1_jFexTauRoI','xAOD::jFexTauRoIAuxContainer#L1_jFexTauRoIAux.',
        'xAOD::jFexSRJetRoIContainer#L1_jFexSRJetRoI','xAOD::jFexSRJetRoIAuxContainer#L1_jFexSRJetRoIAux.',
        'xAOD::jFexLRJetRoIContainer#L1_jFexLRJetRoI','xAOD::jFexLRJetRoIAuxContainer#L1_jFexLRJetRoIAux.',
        'xAOD::jFexMETRoIContainer#L1_jFexMETRoI','xAOD::jFexMETRoIAuxContainer#L1_jFexMETRoIAux.',
        'xAOD::jFexSumETRoIContainer#L1_jFexSumETRoI','xAOD::jFexSumETRoIAuxContainer#L1_jFexSumETRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexSRJetRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexSRJetRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexLRJetRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexLRJetRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexRhoRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexRhoRoIAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarEJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarEJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMHTComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMHTComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMSTComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMSTComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsNoiseCut','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsNoiseCutAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsRms','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsRmsAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarENoiseCut','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarENoiseCutAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarERms','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarERmsAux.',

    ]
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=FexEDMList))

    ##################################################
    # The configuration fragment to be tested
    ##################################################

    from L1CaloFEXSim.L1CaloFEXSimCfg import L1CaloFEXSimCfg
    acc.merge(L1CaloFEXSimCfg(flags))

    # Add in CellMaking, outputs a CaloCellContainer named AllCalo
    from CaloRec.CaloCellMakerConfig import CaloCellMakerCfg
    acc.merge(CaloCellMakerCfg(flags))


    # PS I think CaloTopoClusterToolCfg does 420 out of the box,
    # and not 422. Need to investigate how to control this
    # Further: expected that CaloClustering would be needed
    # only if we are not running Gep clusteeing algs. However,
    # when adding the Clustering config as an _alternative_ to
    # Gep clustring, I get Calo errors such as:
    # ERROR Unresolved conditions dependency:
    #        ( 'CaloNoise' , 'ConditionStore+totalNoise' ) 
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    
    calo_acc = CaloTopoClusterCfg(flags)

    from AthenaCommon.Constants import DEBUG,INFO
    gepAlgs_output_level = DEBUG

    acc.merge(CaloTopoClusterCfg(flags))
    from TrigGepPerf.GepClusterTimingAlgConfig import GepClusterTimingAlgCfg
    acc.merge(GepClusterTimingAlgCfg(flags,
                                     OutputLevel=gepAlgs_output_level))
    
    ##################################################
    # GEP configuration
    ##################################################
    # Run clustering and jet finding algorithms
    # These may be produced by statndard ATLAS Alorithms, or by
    # GEP Algorithms.

    from TrigGepPerf.GepPi0AlgConfig import GepPi0AlgCfg
    # currently caloCellsProducer can be caloCellsFromCaloCells or
    # caloCellsFromCaloClusters
    acc.merge(GepPi0AlgCfg(flags,
                           name='GepPi0Alg',
                           caloCellsProducer="caloCellsFromCaloCells",
                           OutputLevel=gepAlgs_output_level))
    
    # PS not yet tested: topoclAlgs = ['Calo422'] 
    known_cluster_algs = ['WFS', 'Calo420']
    for a in clusterAlgNames:
        assert a in known_cluster_algs

    known_jet_algs = ['Cone', 'ModAntikT']
    for a in jetAlgNames:
        assert a in known_jet_algs

    # Create a number of Algorithms equal to the size of the product
    # len(ClusterAlgs) x len(JetAlgs). Will eventually add in
    # > 1 MET alg, and pileup suppression Algs.
    for cluster_alg in clusterAlgNames:
        if cluster_alg == 'Calo420':
            caloClustersKey='CaloTopoClusters' # from config dump
        else:
            from TrigGepPerf.GepClusteringAlgConfig import GepClusteringAlgCfg
            
            caloClustersKey='GEP'+cluster_alg+'Clusters'
            gepclustering_cfg = GepClusteringAlgCfg(
                flags,
                TopoClAlg=cluster_alg,
                outputCaloClustersKey=caloClustersKey,
                OutputLevel=gepAlgs_output_level)


            info('gepclustering_cfg dump:')
            gepclustering_cfg.printConfig(withDetails=True,
                                          summariseProps=True)

            acc.merge(gepclustering_cfg)

  
        

        puSuppressionAlgs = ['']

        for puSuppressionAlg in puSuppressionAlgs:

            tcLabel = cluster_alg + puSuppressionAlg

            for jetAlg in jetAlgNames:

                from TrigGepPerf.GepJetAlgConfig import GepJetAlgCfg 
                alg_name='Gep'+cluster_alg + jetAlg + 'JetAlg'
                acc.merge(GepJetAlgCfg(
                    flags,
                    name=alg_name,
                    jetAlgName=jetAlg,
                    caloClustersKey=caloClustersKey,
                    outputJetsKey='GEP' + cluster_alg + jetAlg +'Jets',
                    OutputLevel=gepAlgs_output_level))
                
                info('\nGepJetAlg properties dump\n')
                info(str(acc.getEventAlgo(alg_name)._properties))
        
            from TrigGepPerf.GepMETAlgConfig import GepMETAlgCfg 
            alg_name='GepMET'+ cluster_alg +'Alg'
            acc.merge(GepMETAlgCfg(
                flags,
                name=alg_name,
                caloClustersKey=caloClustersKey,
                outputMETKey='GEP'+ cluster_alg +'MET',
                OutputLevel=gepAlgs_output_level))

                    
            from TrigGepPerf.GepMETPufitAlgConfig import GepMETPufitAlgCfg 
            alg_name='GepMET' + cluster_alg + 'PufitAlg'
            acc.merge(GepMETPufitAlgCfg(
                flags,
                name=alg_name,
                caloClustersKey=caloClustersKey,
                outputMETPufitKey='GEP'+ cluster_alg + 'METPufit',
                OutputLevel=gepAlgs_output_level))
    ##################################################
    # Save and optionally run the configuration
    ##################################################
    with open("L1Sim.pkl", "wb") as f:
        acc.store(f)
        f.close()

    if args.execute:
        sc = acc.run()
        if sc.isFailure():
            exit(1)

    
