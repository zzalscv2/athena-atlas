#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


# should choose a better default ??
#DEFAULT_INPUTFILE = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.14795494._005958.pool.root.1"
DEFAULT_INPUTFILE = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/aod/AOD-22.0.48/AOD-22.0.48-full.pool.root"

from argparse import ArgumentParser
parser = ArgumentParser(prog="StandardTests: runs standard jet reconstruction from an ESD",
                        usage="Call with an input file, pass -n=0 to skip execution, -t 0 for serial or 1 for threaded execution.")
#
parser.add_argument("-H", "--Help", default=False, action="store_true", help="Evidently pyROOT interferes with help :(")
#
parser.add_argument("-f", "--filesIn", type=str, help="Comma-separated list of input files",
                    default=DEFAULT_INPUTFILE)
parser.add_argument("-M", "--msgLvl",   default="INFO", help="The message verbosity")
parser.add_argument("-n", "--nEvents",  default=10, type=int, help="The number of events to run. 0 skips execution")
#
parser.add_argument("-t", "--nThreads", default=1, type=int, help="The number of concurrent threads to run. 0 uses serial Athena.")
parser.add_argument("-D", "--dumpSG",   default=False, action="store_true", help="Toggle StoreGate dump on each event")
parser.add_argument("-j", "--jetType",   default="smallR", type=str, choices={"smallR","largeR", "cssk", "VR", "deriv"},
                    help="the type of jet definitions to test")

#
args = parser.parse_args()

if args.Help:
    parser.print_help()
    import sys
    sys.exit(0)



from pprint import pprint
from JetRecConfig.JetRecConfig import JetRecCfg

# Set message levels
from AthenaCommon import Logging, Constants
msgLvl = getattr(Constants,args.msgLvl)
Logging.log.setLevel(msgLvl)
tlog = Logging.logging.getLogger('test_StandardJets')

# Config flags steer the job at various levels
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = args.filesIn.split(",")
flags.Exec.OutputLevel = msgLvl



# Flags relating to multithreaded execution
flags.Concurrency.NumThreads = args.nThreads
if args.nThreads>0:
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Concurrency.NumConcurrentEvents = args.nThreads

# Prevent the flags from being modified
flags.lock()


# Get a ComponentAccumulator setting up the fundamental Athena job
from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
cfg=MainServicesCfg(flags) 


# ***********************************************
# Prepare the JetDefinition to be tested
if args.jetType=='smallR':
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow, AntiKt4LCTopo, AntiKt4Truth
    jetdefs0 = []
    from RecExConfig.AutoConfiguration import IsInInputFile
    if IsInInputFile("xAOD::CaloClusterContainer","CaloCalFwdTopoTowers"):
        jetdefs0 = [AntiKt4EMPFlow, AntiKt4LCTopo, AntiKt4Truth]
    else:
        ghostdefs_PFlow_noTower = [g for g in AntiKt4EMPFlow.ghostdefs if g != "Tower"]
        AntiKt4EMPFlow_noTower = AntiKt4EMPFlow.clone(ghostdefs=ghostdefs_PFlow_noTower)
        ghostdefs_LC_noTower = [g for g in AntiKt4LCTopo.ghostdefs if g != "Tower"]
        AntiKt4LCTopo_noTower = AntiKt4LCTopo.clone(ghostdefs=ghostdefs_LC_noTower)
        jetdefs0 = [AntiKt4EMPFlow_noTower, AntiKt4LCTopo_noTower, AntiKt4Truth]
    # we add a suffix to avoid conflicts with in-file existing jets
    jetdefs =[ d.clone(prefix="New") for d in jetdefs0 ]
    alljetdefs = jetdefs0+jetdefs

elif args.jetType=='largeR':
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopoTrimmed, AntiKt10TruthTrimmed, AntiKt10LCTopoSoftDrop
    jetdefs = [ AntiKt10LCTopoTrimmed, AntiKt10TruthTrimmed , AntiKt10LCTopoSoftDrop ]
    alljetdefs = jetdefs

elif args.jetType=='cssk':
    from JetRecConfig.StandardSmallRJets import AntiKt4LCTopo,AntiKt4EMPFlow
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    # remove the calib tool from modifiers, because no standard calib defined for the custom def below
    nocalibL = [m for m in AntiKt4LCTopo.modifiers if m!="Calib:T0:mc" ]
    AntiKt4LCTopoCSSK = AntiKt4LCTopo.clone(inputdef = cst.LCTopoCSSK, modifiers=nocalibL )
    nocalibL = [m for m in AntiKt4EMPFlow.modifiers if m!="Calib:T0:mc" ]
    AntiKt4EMPFlowCSSK = AntiKt4EMPFlow.clone(inputdef = cst.EMPFlowCSSK, modifiers=nocalibL )
    jetdefs = [AntiKt4LCTopoCSSK,AntiKt4EMPFlowCSSK]
    alljetdefs = jetdefs
elif args.jetType=='VR':
    from JetRecConfig.StandardSmallRJets import AntiKtVR30Rmax4Rmin02PV0Track
    jetdefs = [AntiKtVR30Rmax4Rmin02PV0Track]
    alljetdefs = jetdefs
# Derivation test
elif args.jetType=='deriv':
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo, AntiKt4EMPFlow, AntiKt4Truth, AntiKtVR30Rmax4Rmin02PV0Track
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlowByVertex
    from JetRecConfig.StandardLargeRJets import AntiKt10LCTopoTrimmed, AntiKt10TruthTrimmed
    # Not in AOD
    jetdefs = [AntiKt4Truth,AntiKtVR30Rmax4Rmin02PV0Track,AntiKt10TruthTrimmed,AntiKt10LCTopoTrimmed, AntiKt4EMPFlowByVertex]
    # In AOD, need renaming
    DerivAntiKt4EMTopo = AntiKt4EMTopo.clone(prefix="Deriv",modifiers=list(AntiKt4EMTopo.modifiers)+["QGTagging"])
    DerivAntiKt4EMPFlow = AntiKt4EMPFlow.clone(prefix="Deriv",modifiers=list(AntiKt4EMPFlow.modifiers)+["QGTagging","fJVT","NNJVT"])

    jetdefs += [DerivAntiKt4EMTopo,DerivAntiKt4EMPFlow]
    alljetdefs = jetdefs


# ***********************************************
if args.nEvents == 0:
    # Don't run over events --> just run the jet config.
    # Add the components from our jet reconstruction job
    for jetdef in jetdefs:
        cfg.merge( JetRecCfg(flags,jetdef) )        
    import sys
    tlog.info("Performed jet config. Exiting now")
    sys.exit(0)


    
# ***********************************************
# else setup a full job



# Add the components for reading in pool files
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg.merge(PoolReadCfg(flags))

# Nowadays the jet calibration tool requires the EventInfo
# to be decorated with lumi info, which is not in Run 2 AODs
from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
cfg.merge(LuminosityCondAlgCfg(flags))

from LumiBlockComps.LumiBlockMuWriterConfig import LumiBlockMuWriterCfg
cfg.merge(LumiBlockMuWriterCfg(flags))



# =======================
# If running on ESD the CHSXYZParticleFlowObjects container pre-exist and can get in the way. Just rename them manually here :
if 'CHSChargedParticleFlowObjects' in flags.Input.Collections:
    from SGComps.AddressRemappingConfig import InputRenameCfg
    cfg.merge( InputRenameCfg("xAOD::PFOContainer", "CHSNeutralParticleFlowObjects" , "CHSNeutralParticleFlowObjects_original") )
    cfg.merge( InputRenameCfg("xAOD::PFOContainer", "CHSChargedParticleFlowObjects" , "CHSChargedParticleFlowObjects_original") )


    
# Add the components from our jet reconstruction job
for jetdef in jetdefs:
    cfg.merge( JetRecCfg(flags,jetdef) )        


# Now get the output stream components
outputlist = ["EventInfo#*"]    
# append all these jets to our output file
for j in alljetdefs:
    key = j.fullname()
    outputlist += [f"xAOD::JetContainer#{key}",
                   f"xAOD::JetAuxContainer#{key}Aux.-PseudoJet"]
    if "ByVertex" in key:
        outputlist += ["xAOD::VertexContainer#PrimaryVertices"]


from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
cfg.merge(OutputStreamCfg(flags,"xAOD",ItemList=outputlist))
pprint( cfg.getEventAlgo("OutputStreamxAOD").ItemList )

# Optionally, print the contents of the store every event
cfg.getService("StoreGateSvc").Dump = args.dumpSG

# Run the job
if args.nEvents>0:
    cfg.run(maxEvents=args.nEvents)
