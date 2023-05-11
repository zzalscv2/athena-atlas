#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
import logging

def SetupArguments():
    from argparse import ArgumentParser
    parser = ArgumentParser()
    parser.add_argument("--threads", "-t", type=int, help="number of threads", default=1)
    parser.add_argument("--outputFile", default="IsoCloseByTestFile.root", help="File name of the output file", metavar="FILE")
    parser.add_argument("--inputFile", "-i", default=[], help="Input file to run on ", nargs="+")
    parser.add_argument("--dir", "-d", default=[], help="List of directories containing the root files", nargs="+")
    parser.add_argument("--maxEvents", default=-1, type=int, help="How many events shall be run maximally")
    parser.add_argument("--skipEvents", default=0, type=int, help="How many events shall be skipped at the beginning of the job")
    return parser

if __name__ == "__main__":   
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArguments().parse_args()
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = [iii for ii in [i.split(',') for i in args.inputFile] for iii in ii]
    from os import listdir
    for direc in args.dir:
        flags.Input.Files += ["%s/%s" % (direc, x) for x in listdir(direc) if x[x.rfind(".") + 1:] in ["root", "1"]]
    if len(flags.Input.Files) == 0:
        logging.warning("No input files were parsed")
    flags.Exec.MaxEvents = args.maxEvents
    flags.Exec.SkipEvents = args.skipEvents
    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    msgService = cfg.getService('MessageSvc')
    msgService.Format = "S:%s E:%e % F%128W%S%7W%R%T  %0W%M"

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    ### Setup the isolation builder
    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg
    from IsolationSelection.IsolationSelectionConfig import (IsoCloseByCorrSkimmingAlgCfg, TestIsoCloseByCorrectionCfg, 
                                                             IsolationSelectionToolCfg, IsoCloseByCorrectionToolCfg, IsoCloseByCaloDecorCfg)
    # A selection of WP is probably needed, as only a few variables are in CP content !
    #   maybe MUON derivations can add some other ones for studies
    listofTTVAWP = [ 'Nonprompt_All_MaxWeight' ]
    ### Setup the full Derivation package from the ID. Actually I only want the TTVA decorator.
    from DerivationFrameworkInDet.InDetCommonConfig import InDetCommonCfg
    cfg.merge(InDetCommonCfg(flags,
                             DoVertexFinding = False,
                             AddPseudoTracks=False,
                             DecoLRTTTVA = False,
                             MergeLRT = False))
    for WP in listofTTVAWP:
        cfg.merge(DerivationTrackIsoCfg(flags, WP = WP, object_type = ('Electrons', 'Muons', 'Photons')))
        cfg.merge(IsoCloseByCorrSkimmingAlgCfg(flags, ttva_wp = WP,
                                               OutContainerKey="AssocCloseByTracks"))
    ### Pflow isolation
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from IsolationAlgs.IsolationSteeringDerivConfig import IsolationSteeringDerivCfg
    cfg.merge(IsolationSteeringDerivCfg(flags))
    #### Calibration tools
    from MuonMomentumCorrections.MCastCfg import setupCalibratedMuonProviderCfg
    cfg.merge(setupCalibratedMuonProviderCfg(flags))
    from ElectronPhotonFourMomentumCorrection.EgammaCalibCfg import setupEgammaCalibProviderCfg
    cfg.merge(setupEgammaCalibProviderCfg(flags, "ElectronProvider", Input="Electrons", Output="CalibElectrons"))
    #cfg.merge(setupEgammaCalibProviderCfg(flags, "PhotonProvider", Input="Photons", Output="CalibPhotons"))
    iso_tool = cfg.popToolsAndMerge(IsolationSelectionToolCfg(flags, 
                                                              ElectronWP="PflowTight_FixedRad",
                                                              MuonWP="PflowTight_FixedRad",
                                                              InMuonContainer = "Muons",
                                                              InElecContainer = "Electrons"))
    iso_corr_tool = cfg.popToolsAndMerge(IsoCloseByCorrectionToolCfg(flags,
                                                                     BackupPrefix="vanilla",                                        
                                                                     IsolationSelectionTool = iso_tool,
                                                                     SelectionDecorator = "considerInCorrection",
                                                                     IsolationSelectionDecorator =  "correctedIsol"))
    ### Associate the close-by pflow objects and the calorimeter clusters
    cfg.merge(IsoCloseByCaloDecorCfg(flags,
                                     containers = ["Electrons", "Muons", "Photons"] ))

    cfg.merge(TestIsoCloseByCorrectionCfg(flags,
                                          BackupPrefix="vanilla",
                                          MuonContainer = "CalibratedMuons",
                                          EleContainer = "CalibElectrons",
                                          PhotContainer = "",
                                          TrackKey = "AssocCloseByTracks",
                                          IsolationSelectionTool = iso_tool,
                                          SelectionDecorator = "considerInCorrection",
                                          IsolationDecorator = "defaultIso",
                                          UpdatedIsoDecorator="correctedIsol",
                                          IsoCloseByCorrTool = iso_corr_tool))
    
    histSvc = CompFactory.THistSvc(Output=["ISOCORRECTION DATAFILE='%s', OPT='RECREATE'"%(args.outputFile)])
    cfg.addService(histSvc)

    sc = cfg.run(flags.Exec.MaxEvents)
    if not sc.isSuccess():
        exit(1)
