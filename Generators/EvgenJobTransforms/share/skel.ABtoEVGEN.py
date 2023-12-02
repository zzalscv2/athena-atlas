#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
"""Functionality core of the Gen_tf transform"""

##==============================================================
## Basic configuration
##==============================================================

## Create sequences for generators, clean-up algs, filters and analyses
## and import standard framework objects with standard local scope names
from __future__ import print_function
from __future__ import division

from future import standard_library
standard_library.install_aliases()

import os, re, string
import AthenaCommon.AlgSequence as acas
import AthenaCommon.AppMgr as acam
from AthenaCommon.AthenaCommonFlags import jobproperties

from xAODEventInfoCnv.xAODEventInfoCnvConf import xAODMaker__EventInfoCnvAlg
acam.athMasterSeq += xAODMaker__EventInfoCnvAlg(xAODKey="TMPEvtInfo")

theApp = acam.theApp
acam.athMasterSeq += acas.AlgSequence("EvgenGenSeq")
genSeq = acam.athMasterSeq.EvgenGenSeq
acam.athMasterSeq += acas.AlgSequence("EvgenPreFilterSeq")
prefiltSeq = acam.athMasterSeq.EvgenPreFilterSeq
acam.athMasterSeq += acas.AlgSequence("EvgenTestSeq")
testSeq = acam.athMasterSeq.EvgenTestSeq
## NOTE: LogicalExpressionFilter is an algorithm, not a sequence
from EvgenProdTools.LogicalExpressionFilter import LogicalExpressionFilter
acam.athMasterSeq += LogicalExpressionFilter("EvgenFilterSeq")
filtSeq = acam.athMasterSeq.EvgenFilterSeq
topSeq = acas.AlgSequence()
anaSeq = topSeq
topSeq += acas.AlgSequence("EvgenPostSeq")
postSeq = topSeq.EvgenPostSeq
#topAlg = topSeq #< alias commented out for now, so that accidental use throws an error


##==============================================================
## Configure standard Athena services
##==============================================================

## Special setup for event generation
include("AthenaCommon/Atlas.UnixStandardJob.py")
include("PartPropSvc/PartPropSvc.py")

## Run performance monitoring (memory logging)
from PerfMonComps.PerfMonFlags import jobproperties as perfmonjp
perfmonjp.PerfMonFlags.doMonitoring = True
perfmonjp.PerfMonFlags.doSemiDetailedMonitoring = True

## Random number services
from RngComps.RngCompsConf import AthRNGSvc
svcMgr += AthRNGSvc()

## Jobs should stop if an include fails.
jobproperties.AthenaCommonFlags.AllowIgnoreConfigError = False

## Compatibility with jets
#from RecExConfig.RecConfFlags import jobproperties
#jobproperties.RecConfFlags.AllowBackNavigation = True

## Set up a standard logger
from AthenaCommon.Logging import logging
evgenLog = logging.getLogger('Generate_ab')


##==============================================================
## Run arg handling
##==============================================================

## Announce arg checking
evgenLog.debug("****************** CHECKING EVENT GENERATION ARGS *****************")
evgenLog.debug(str(runArgs))

if hasattr(runArgs, "runNumber"):
   evgenLog.warning("##########################################################################" )         
   evgenLog.warning("runNumber - no longer a valid argument, do not use it ! " )         
   evgenLog.warning("##########################################################################")

if hasattr(runArgs, "inputGenConfFile"):
   raise RuntimeError("inputGenConfFile is invalid !! Gridpacks and config. files/links to be put into DSID directory ")

## Ensure that an output name has been given
# TODO: Allow generation without writing an output file (if outputEVNTFile is None)?
if not hasattr(runArgs, "outputEVNTFile") and not hasattr(runArgs, "outputEVNT_PreFile"):
    raise RuntimeError("No output evgen EVNT or EVNT_Pre file provided.")

## Ensure that mandatory args have been supplied (complain before processing the includes)
if not hasattr(runArgs, "ecmEnergy"):
    raise RuntimeError("No center of mass energy provided.")
else:
    evgenLog.info(' ecmEnergy = ' + str(runArgs.ecmEnergy) )
if not hasattr(runArgs, "randomSeed"):
    raise RuntimeError("No random seed provided.")
if not hasattr(runArgs, "firstEvent"):
    raise RuntimeError("No first number provided.")
if (runArgs.firstEvent <= 0):
    evgenLog.warning("Run argument firstEvent should be > 0")

if hasattr(runArgs, "inputEVNT_PreFile"):
   evgenLog.info("inputEVNT_PreFile = " + ','.join(runArgs.inputEVNT_PreFile))

##==============================================================
## Configure standard Athena and evgen services
##==============================================================

## Announce start of job configuration
evgenLog.debug("****************** CONFIGURING EVENT GENERATION *****************")

## Functions for operating on generator names
## NOTE: evgenConfig, topSeq, svcMgr, theApp, etc. should NOT be explicitly re-imported in JOs
from EvgenJobTransforms.EvgenConfig import evgenConfig
from EvgenJobTransforms.EvgenConfig import gens_known, gens_lhef, gen_sortkey, gens_testhepmc, gens_notune

## Sanity check the event record (not appropriate for all generators)
from EvgenProdTools.EvgenProdToolsConf import TestHepMC
testSeq += TestHepMC(CmEnergy=runArgs.ecmEnergy*Units.GeV)
if not hasattr(svcMgr, 'THistSvc'):
    from GaudiSvc.GaudiSvcConf import THistSvc
    svcMgr += THistSvc()
svcMgr.THistSvc.Output = ["TestHepMCname DATAFILE='TestHepMC.root' OPT='RECREATE'"]

## Configure the event counting (AFTER all filters)
# TODO: Rewrite in Python?
from EvgenProdTools.EvgenProdToolsConf import CountHepMC

import AthenaPoolCnvSvc.ReadAthenaPool
svcMgr.EventSelector.FirstEvent = runArgs.firstEvent
theApp.EvtMax = -1
if not hasattr(postSeq, "CountHepMC"):
    postSeq += CountHepMC(InputEventInfo="TMPEvtInfo",
                          OutputEventInfo="EventInfo",
                          mcEventWeightsKey="")

#postSeq.CountHepMC.RequestedOutput = evgenConfig.nEventsPerJob if runArgs.maxEvents == -1 else runArgs.maxEvents
postSeq.CountHepMC.FirstEvent = runArgs.firstEvent
postSeq.CountHepMC.CorrectHepMC = True
postSeq.CountHepMC.CorrectEventID = True
postSeq.CountHepMC.CorrectRunNumber = False

if hasattr(runArgs,"inputEVNT_PreFile"):
   from AthenaCommon.AppMgr import ServiceMgr
   #fix iov metadata
   if not hasattr(ServiceMgr.ToolSvc, 'IOVDbMetaDataTool'):
      ServiceMgr.ToolSvc += CfgMgr.IOVDbMetaDataTool()
   runNum = int((runArgs.jobConfig[0])[-6:])
   ServiceMgr.ToolSvc.IOVDbMetaDataTool.MinMaxRunNumbers = [runNum, runNum+1]

## Print out the contents of the first 5 events (after filtering)
# TODO: Allow configurability from command-line/exec/include args
if hasattr(runArgs, "printEvts") and runArgs.printEvts > 0:
    from TruthIO.TruthIOConf import PrintMC
    postSeq += PrintMC()
    postSeq.PrintMC.McEventKey = "GEN_EVENT"
    postSeq.PrintMC.VerboseOutput = True
    postSeq.PrintMC.PrintStyle = "Barcode"
    postSeq.PrintMC.FirstEvent = 1
    postSeq.PrintMC.LastEvent  = runArgs.printEvts

## Add Rivet_i to the job
# TODO: implement auto-setup of analyses triggered on evgenConfig.keywords (from T Balestri)
if hasattr(runArgs, "rivetAnas"):
    from Rivet_i.Rivet_iConf import Rivet_i
    anaSeq += Rivet_i()
    anaSeq.Rivet_i.Analyses = runArgs.rivetAnas
    anaSeq.Rivet_i.AnalysisPath = os.environ['PWD']
    if hasattr(runArgs, "outputYODAFile"):
      anaSeq.Rivet_i.HistoFile = runArgs.outputYODAFile

##==============================================================
## Pre- and main config parsing
##==============================================================

## Announce JO loading
evgenLog.debug("****************** LOADING PRE-INCLUDES AND JOB CONFIG *****************")

## Pre-include
if hasattr(runArgs, "preInclude"):
    for fragment in runArgs.preInclude:
        include(fragment)

## Pre-exec
if hasattr(runArgs, "preExec"):
    evgenLog.info("Transform pre-exec")
    for cmd in runArgs.preExec:
        evgenLog.info(cmd)
        exec(cmd)

# TODO: Explain!!!
def OutputTXTFile():
    outputTXTFile = None
    if hasattr(runArgs,"outputTXTFile"): outputTXTFile=runArgs.outputTXTFile
    return outputTXTFile

## Main job option include
## Only permit one jobConfig argument for evgen: does more than one _ever_ make sense?
if len(runArgs.jobConfig) != 1:
    evgenLog.error("You must supply one and only one jobConfig file argument")
    sys.exit(1)
evgenLog.info("Using JOBOPTSEARCHPATH!! = '%s'" % os.environ["JOBOPTSEARCHPATH"])
FIRST_DIR = (os.environ['JOBOPTSEARCHPATH']).split(":")[0]

jofiles = [f for f in os.listdir(FIRST_DIR) if (f.startswith('mc') and f.endswith('.py'))]

if len(jofiles) !=1:
    evgenLog.error("You must supply one and only one jobOption file in DSID directory")
    sys.exit(1)

jofile = jofiles[0]
joparts = (os.path.basename(jofile)).split(".")

if joparts[0].startswith("mc"): #and all(c in string.digits for c in joparts[0][2:]):
    officialJO = True
    ## Check that there are exactly 3 name parts separated by '.': mc, physicsShort, .py
    if len(joparts) != 3:
        evgenLog.error(jofile + " name format is wrong: must be of the form mc.<physicsShort>.py: please rename.")
        sys.exit(1)
    ## Check the length limit on the physicsShort portion of the filename
    jo_physshortpart = joparts[1]
    if len(jo_physshortpart) > 50:
        evgenLog.error(jofile + " contains a physicsShort field of more than 50 characters: please rename.")
        sys.exit(1)
    ## There must be at least 2 physicsShort sub-parts separated by '_': gens, (tune)+PDF, and process
    jo_physshortparts = jo_physshortpart.split("_")
    if len(jo_physshortparts) < 2:
        evgenLog.error(jofile + " has too few physicsShort fields separated by '_': should contain <generators>(_<tune+PDF_if_available>)_<process>. Please rename.")
        sys.exit(1)
    ## NOTE: a further check on physicsShort consistency is done below, after fragment loading
    check_jofiles="/cvmfs/atlas.cern.ch/repo/sw/Generators/MC16JobOptions/scripts/check_jo_consistency.py"
    if os.path.exists(check_jofiles):
        include(check_jofiles)
        check_naming(os.path.basename(jofile))
    else:
        evgenLog.error("check_jo_consistency.py not found")
        sys.exit(1)

## Include the JO fragment
include(jofile)

##==============================================================
## Config validation and propagation to services, generators, etc.
##==============================================================

## Announce start of JO checking
evgenLog.debug("****************** CHECKING EVGEN CONFIGURATION *****************")

## Print out options
for opt in str(evgenConfig).split(os.linesep):
    evgenLog.info(opt)

evgenLog.info(".transform =                  Gen_tf")
## Sort and check generator name / JO name consistency
##
## Check that the common fragments are not obsolete:
if evgenConfig.obsolete:
    evgenLog.error("JOs or icludes are obsolete, please check them")
    sys.exit(1)
## Check that the generators list is not empty:
if not evgenConfig.generators:
    evgenLog.error("No entries in evgenConfig.generators: invalid configuration, please check your JO")
    sys.exit(1)
## Check for duplicates:
if len(evgenConfig.generators) > len(set(evgenConfig.generators)):
    evgenLog.error("Duplicate entries in evgenConfig.generators: invalid configuration, please check your JO")
    sys.exit(1)
## Sort the list of generator names into standard form
gennames = sorted(evgenConfig.generators, key=gen_sortkey)
## Check that the actual generators, tune, and main PDF are consistent with the JO name
if joparts[0].startswith("MC"): #< if this is an "official" JO
    genpart = jo_physshortparts[0]
    expectedgenpart = ''.join(gennames)
    ## We want to record that HERWIG was used in metadata, but in the JO naming we just use a "Herwig" label
    expectedgenpart = expectedgenpart.replace("HerwigJimmy", "Herwig")
    def _norm(s):
        # TODO: add EvtGen to this normalization for MC14?
        return s.replace("Photospp", "").replace("Photos", "").replace("TauolaPP", "").replace("Tauolapp", "").replace("Tauola", "")
    def _norm2(s):
        return s.replace("Py", "Pythia").replace("MG","MadGraph").replace("Ph","Powheg").replace("Hpp","Herwigpp").replace("H7","Herwig7").replace("Sh","Sherpa").replace("Ag","Alpgen").replace("EG","EvtGen").replace("PG","ParticleGun")

    def _short2(s):
         return s.replace("Pythia","Py").replace("MadGraph","MG").replace("Powheg","Ph").replace("Herwigpp","Hpp").replace("Herwig7","H7").replace("Sherpa","Sh").replace("Alpgen","Ag").replace("EvtGen","EG").replace("PG","ParticleGun")
 
    if genpart != _norm(expectedgenpart)  and _norm2(genpart) != _norm(expectedgenpart):
        evgenLog.error("Expected first part of JO name to be '%s' or '%s', but found '%s'" % (_norm(expectedgenpart), _norm(_short2(expectedgenpart)), genpart))
        evgenLog.error("gennames '%s' " %(expectedgenpart))
        sys.exit(1)


    del _norm
    ## Check if the tune/PDF part is needed, and if so whether it's present
    if not gens_notune(gennames) and len(jo_physshortparts) < 3:
        evgenLog.error(jofile + " with generators " + expectedgenpart +
                       " has too few physicsShort fields separated by '_'." +
                       " It should contain <generators>_<tune+PDF_<process>. Please rename.")
        sys.exit(1)

## Check that the evgenConfig.nEventsPerJob setting is acceptable
## nEventsPerJob defines the production event sizes and must be sufficiently "round"
rounding = 0
if hasattr(runArgs,'inputGeneratorFile') and ',' in runArgs.inputGeneratorFile:
   multiInput = runArgs.inputGeneratorFile.count(',')+1
else:
   multiInput = 0
# check if default nEventsPErJob used
if not evgenConfig.nEventsPerJob:
    evgenLog.info('#############################################################')
    evgenLog.info(' !!!! no nEventsPerJob set !!!  The default 10000 used. !!! ')
    evgenLog.info('#############################################################')
else:
    evgenLog.info(' nEventsPerJob = ' + str(evgenConfig.nEventsPerJob)  )


if evgenConfig.minevents > 0 :
    raise RuntimeError("evgenConfig.minevents is obsolete and should be removed from the JOs")

if evgenConfig.nEventsPerJob < 1:
    raise RuntimeError("evgenConfig.nEventsPerJob must be at least 1")
elif evgenConfig.nEventsPerJob > 100000:
    raise RuntimeError("evgenConfig.nEventsPerJob can be max. 100000")
else:
    allowed_nEventsPerJob_lt1000 = [1, 2, 5, 10, 20, 25, 50, 100, 200, 500, 1000]
    msg = "evgenConfig.nEventsPerJob = %d: " % evgenConfig.nEventsPerJob

    if evgenConfig.nEventsPerJob >= 1000 and evgenConfig.nEventsPerJob <=10000 and (evgenConfig.nEventsPerJob % 1000 != 0 or 10000 % evgenConfig.nEventsPerJob != 0) :
           msg += "nEventsPerJob in range [1K, 10K] must be a multiple of 1K and a divisor of 10K"
           raise RuntimeError(msg)
    elif evgenConfig.nEventsPerJob > 10000  and evgenConfig.nEventsPerJob % 10000 != 0:
           msg += "nEventsPerJob >10K must be a multiple of 10K"
           raise RuntimeError(msg)
    elif evgenConfig.nEventsPerJob < 1000 and evgenConfig.nEventsPerJob not in allowed_nEventsPerJob_lt1000:
           msg += "nEventsPerJob in range <= 1000 must be one of %s" % allowed_nEventsPerJob_lt1000
           raise RuntimeError(msg)
    postSeq.CountHepMC.RequestedOutput = evgenConfig.nEventsPerJob if runArgs.maxEvents == -1  else runArgs.maxEvents
    evgenLog.info('Requested output events = '+str(postSeq.CountHepMC.RequestedOutput))

## Check that the keywords are in the list of allowed words (and exit if processing an official JO)
if evgenConfig.keywords:
    ## Get the allowed keywords file from the JO package if possibe
    # TODO: Make the package name configurable
    kwfile = "EvgenJobTransforms/evgenkeywords.txt"
    kwpath = None
    for p in os.environ["JOBOPTSEARCHPATH"].split(":"):
        kwpath = os.path.join(p, kwfile)
        if os.path.exists(kwpath):
            break
        kwpath = None
    ## Load the allowed keywords from the file
    allowed_keywords = []
    if kwpath:
        evgenLog.info("evgenkeywords = " + kwpath)
        kwf = open(kwpath, "r")
        for l in kwf:
            allowed_keywords += l.strip().lower().split()
        ## Check the JO keywords against the allowed ones
        evil_keywords = []
        for k in evgenConfig.keywords:
            if k not in allowed_keywords:
                evil_keywords.append(k)
        if evil_keywords:
            msg = "evgenConfig.keywords contains non-standard keywords: %s. " % ", ".join(evil_keywords)
            msg += "Please check the allowed keywords list and fix."
            evgenLog.error(msg)
            if officialJO:
                sys.exit(1)
    else:
        evgenLog.warning("evgenkeywords = not found ")

## Configure and schedule jet finding algorithms
## NOTE: This generates algorithms for jet containers defined in the user's JO fragment
if evgenConfig.findJets:
    include("EvgenJobTransforms/Generate_TruthJets.py")

## Configure POOL streaming to the output EVNT format file
from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
from AthenaPoolCnvSvc.AthenaPoolCnvSvcConf import AthenaPoolCnvSvc
# remove because it was removed from Database/AthenaPOOL/AthenaPoolCnvSvc
#svcMgr.AthenaPoolCnvSvc.CommitInterval = 10 #< tweak for MC needs
if hasattr(runArgs, "outputEVNTFile"):
  poolFile = runArgs.outputEVNTFile
elif hasattr(runArgs, "outputEVNT_PreFile"):
  poolFile = runArgs.outputEVNT_PreFile
else:
  raise RuntimeError("Output pool file, either EVNT or EVNT_Pre, is not known.")


StreamEVGEN = AthenaPoolOutputStream("StreamEVGEN", poolFile, asAlg=True, noTag=True , eventInfoKey="EventInfo")
if hasattr(runArgs, "inputEVNT_PreFile") :
  svcMgr.EventSelector.InputCollections = runArgs.inputEVNT_PreFile
  StreamEVGEN.TakeItemsFromInput = True
  postSeq.CountHepMC.CorrectRunNumber = True

StreamEVGEN.ForceRead = True
StreamEVGEN.ItemList += ["EventInfo#*", "xAOD::EventInfo#EventInfo*", "xAOD::EventAuxInfo#EventInfoAux.*", "McEventCollection#*"]
StreamEVGEN.RequireAlgs += ["EvgenFilterSeq"]
## Used for pile-up (remove dynamic variables except flavour labels)
if evgenConfig.saveJets:
    StreamEVGEN.ItemList += ["xAOD::JetContainer_v1#*"]
    StreamEVGEN.ItemList += ["xAOD::JetAuxContainer_v1#*.TruthLabelID.PartonTruthLabelID"]
if evgenConfig.savePileupTruthParticles:
   StreamEVGEN.ItemList += ["xAOD::TruthParticleContainer#TruthPileupParticles*"]
   StreamEVGEN.ItemList += ["xAOD::TruthParticleAuxContainer#TruthPileupParticlesAux.*"]


## Set the run numbers
dsid = os.path.basename(runArgs.jobConfig[0])
if not dsid.isdigit():
    dsid = "999999"
svcMgr.EventSelector.RunNumber = int(dsid)

if postSeq.CountHepMC.CorrectRunNumber == True:
    postSeq.CountHepMC.NewRunNumber = int(dsid)
    evgenLog.info("Set new run number in skel NewRunNumber = " + str(postSeq.CountHepMC.NewRunNumber))
else:
    evgenLog.info("No new run number set in skel RunNumber = " + dsid)

## Handle beam info
import EventInfoMgt.EventInfoMgtInit
svcMgr.TagInfoMgr.ExtraTagValuePairs.update({"beam_energy": str(int(runArgs.ecmEnergy*Units.GeV/2.0))})
svcMgr.TagInfoMgr.ExtraTagValuePairs.update({"beam_type": 'collisions'})
if len(evgenConfig.keywords)>0:
    # Assume that this is the correct list of keywords we should keep
    svcMgr.TagInfoMgr.ExtraTagValuePairs.update({"keywords": ", ".join(evgenConfig.keywords).lower()})

# Set AMITag in in-file metadata
from PyUtils import AMITagHelper
AMITagHelper.SetAMITag(runArgs=runArgs)

## Propagate energy argument to the generators
# TODO: Standardise energy setting in the GenModule interface
include("EvgenJobTransforms/Generate_ecmenergies.py")

## Propagate DSID and seed to the generators
include("EvgenJobTransforms/Generate_dsid_ranseed.py")

## Propagate debug output level requirement to generators
if (hasattr( runArgs, "VERBOSE") and runArgs.VERBOSE ) or (hasattr( runArgs, "loglevel") and runArgs.loglevel == "DEBUG") or (hasattr( runArgs, "loglevel")and runArgs.loglevel == "VERBOSE"):
   include("EvgenJobTransforms/Generate_debug_level.py")

## Add special config option (extended model info for BSM scenarios)
svcMgr.TagInfoMgr.ExtraTagValuePairs.update({"specialConfiguration": evgenConfig.specialConfig})

## Remove TestHepMC if it's inappropriate for this generator combination
# TODO: replace with direct del statements in the generator common JO fragments?
if hasattr(testSeq, "TestHepMC") and not gens_testhepmc(evgenConfig.generators):
    evgenLog.info("Removing TestHepMC sanity checker")
    del testSeq.TestHepMC


##==============================================================
## Handling of a post-include/exec args at the end of standard configuration
##==============================================================

if hasattr(runArgs, "postInclude"):
    for fragment in runArgs.postInclude:
        include(fragment)

if hasattr(runArgs, "postExec"):
    evgenLog.info("Transform post-exec")
    for cmd in runArgs.postExec:
        evgenLog.info(cmd)
        exec(cmd)


##==============================================================
## Show the algorithm sequences and algs now that config is complete
##==============================================================
acas.dumpMasterSequence()


##==============================================================
## Input file arg handling
##==============================================================

## Announce start of input file handling
evgenLog.debug("****************** HANDLING EVGEN INPUT FILES *****************")

## Dat files
datFile = None
if "McAtNlo" in evgenConfig.generators and "Herwig" in evgenConfig.generators:
    datFile = "inparmMcAtNlo.dat"
elif "Alpgen" in evgenConfig.generators:
    datFile = "inparmAlpGen.dat"
elif "Protos" in evgenConfig.generators:
    datFile = "protos.dat"
elif "ProtosLHEF" in evgenConfig.generators:
    datFile = "protoslhef.dat"
elif "AcerMC" in evgenConfig.generators:
    datFile = "inparmAcerMC.dat"
elif "CompHep" in evgenConfig.generators:
    datFile = "inparmCompHep.dat"

## Events files
eventsFile = None

## Helper functions for input file handling
def find_unique_file(pattern):
    "Return a matching file, provided it is unique"
    import glob
    files = glob.glob(pattern)
    ## Check that there is exactly 1 match
    if not files:
        raise RuntimeError("No '%s' file found" % pattern)
    elif len(files) > 1:
        raise RuntimeError("More than one '%s' file found" % pattern)
    return files[0]

def mk_symlink(srcfile, dstfile):
    "Make a symlink safely"
    if dstfile:
        if os.path.exists(dstfile) and not os.path.samefile(dstfile, srcfile):
            os.remove(dstfile)
        if not os.path.exists(dstfile):
            evgenLog.info("Symlinking %s to %s" % (srcfile, dstfile))
            os.symlink(srcfile, dstfile)
        else:
            evgenLog.debug("Symlinking: %s is already the same as %s" % (dstfile, srcfile))


## Do the aux-file copying
if evgenConfig.auxfiles:
    from PyJobTransformsCore.trfutil import get_files
    get_files(evgenConfig.auxfiles, keepDir=False, errorIfNotFound=True)


##==============================================================
## Write out metadata for reporting to AMI
##==============================================================

def _checkattr(attr, required=False):
    if not hasattr(evgenConfig, attr) or not getattr(evgenConfig, attr):
        msg = "evgenConfig attribute '%s' not found." % attr
        if required:
            raise RuntimeError("Required " + msg)
        return False
    return True

if _checkattr("description", required=True):
    msg = evgenConfig.description
    if _checkattr("notes"):
        msg += " " + evgenConfig.notes
    printfunc ("MetaData: %s = %s" % ("physicsComment", msg))
if _checkattr("generators", required=True):
    printfunc ("MetaData: %s = %s" % ("generatorName", "+".join(gennames)))
if _checkattr("process"):
    printfunc ("MetaData: %s = %s" % ("physicsProcess", evgenConfig.process))
if _checkattr("tune"):
    printfunc ("MetaData: %s = %s" % ("generatorTune", evgenConfig.tune))
if _checkattr("hardPDF"):
    printfunc ("MetaData: %s = %s" % ("hardPDF", evgenConfig.hardPDF))
if _checkattr("softPDF"):
    printfunc ("MetaData: %s = %s" % ("softPDF", evgenConfig.softPDF))
if _checkattr("nEventsPerJob"):
    printfunc ("MetaData: %s = %s" % ("nEventsPerJob", evgenConfig.nEventsPerJob))
if _checkattr("keywords"):
    printfunc ("MetaData: %s = %s" % ("keywords", ", ".join(evgenConfig.keywords).lower()))
if _checkattr("specialConfig"):
    printfunc ("MetaData: %s = %s" % ("specialConfig", evgenConfig.specialConfig))
# TODO: Require that a contact / JO author is always set
if _checkattr("contact"):
    printfunc ("MetaData: %s = %s" % ("contactPhysicist", ", ".join(evgenConfig.contact)))

# Output list of generator filters used
filterNames = [alg.getType() for alg in acas.iter_algseq(filtSeq)]
excludedNames = ['AthSequencer', 'PyAthena::Alg', 'TestHepMC']
filterNames = list(set(filterNames) - set(excludedNames))
printfunc ("MetaData: %s = %s" % ("genFilterNames", ", ".join(filterNames)))


##==============================================================
## Dump evgenConfig so it can be recycled in post-run actions
##==============================================================

from PyJobTransformsCore.runargs import RunArguments
runPars = RunArguments()
runPars.nEventsPerJob = evgenConfig.nEventsPerJob
runPars.maxeventsstrategy = evgenConfig.maxeventsstrategy
with open("config.pickle", "wb") as f:
    import pickle
    pickle.dump(runPars, f)


##==============================================================
## Get ready to run...
##==============================================================
evgenLog.info("****************** STARTING EVENT GENERATION *****************")
