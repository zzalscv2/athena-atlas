# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
###############################################################
#
# Minimal athena options to execute the CutFlowSvc with algorithms
#
## @file TestCutFlowSvcOutput.py
##
## @brief Test basic CutFlowSvc functionality with a few algorithms
##
## @author Tadej Novak <tadej.novak@cern.ch>
#
#==============================================================

# debug logging
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Logging import logging
logging.getLogger('CreateCutFlowSvc').setLevel(DEBUG)

# basic job configuration
import AthenaCommon.AtlasUnixStandardJob  # noqa: F401

# setup the input
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
if 'inputFiles' in dir():
    athenaCommonFlags.FilesInput = inputFiles.split(',')  # noqa: F821
    del inputFiles
elif 'alternativeInput' in dir() and alternativeInput is True:  # noqa: F821
    athenaCommonFlags.FilesInput = [
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/mc16_13TeV.410501.PowhegPythia8EvtGen_A14_ttbar_hdamp258p75_nonallhad.merge.AOD.e5458_s3126_r9364_r9315/AOD.11182705._000001.pool.root.1'
    ]
else:
    athenaCommonFlags.FilesInput = [
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MetadataTests/dummy/File1.pool.root',
        '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MetadataTests/dummy/File2.pool.root',
    ]

# get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# get a handle to the ServiceManager and the Tool service
from AthenaCommon.AppMgr import ServiceMgr, ToolSvc

# get a handle to the ApplicationManager
from AthenaCommon.AppMgr import theApp

# load POOL support
import AthenaPoolCnvSvc.ReadAthenaPool  # noqa: F401

# setup the CutFlowSvc and corresponding tools
from EventBookkeeperTools.CutFlowHelpers import CreateCutFlowSvc
CreateCutFlowSvc(seq=topSequence)

# add a test algorithm
from EventBookkeeperTools.EventBookkeeperToolsConf import TestFilterReentrantAlg
topSequence += TestFilterReentrantAlg("TestReentrant1", FilterKey="TestReentrant1", Modulo=2)
topSequence += TestFilterReentrantAlg("TestReentrant2", FilterKey="TestReentrant2", Modulo=4)

# for tests
from AthenaCommon.ConcurrencyFlags import jobproperties as Concurrency
suffix = f"_legacy_{Concurrency.ConcurrencyFlags.NumThreads()}_{Concurrency.ConcurrencyFlags.NumProcs()}"
if 'alternativeInput' in dir() and alternativeInput is True:  # noqa: F821
    suffix += "_alt"

if Concurrency.ConcurrencyFlags.NumProcs() > 0:
    from AthenaMP.AthenaMPFlags import jobproperties as AthenaMPJobProps
    # shared writer
    if 'sharedWriter' in dir() and sharedWriter is True:  # noqa: F821
        AthenaMPJobProps.AthenaMPFlags.UseSharedWriter = True
        suffix += "_sharedWriter"

    AthenaMPJobProps.AthenaMPFlags.WorkerTopDir = f"athenaMP_workers{suffix}"
    AthenaMPJobProps.AthenaMPFlags.EventOrdersFile = f"athenamp_eventorders{suffix}.txt"

# output options
from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
Stream1 = AthenaPoolOutputStream('Stream1', f'OutputAOD{suffix}.root', asAlg=True)
Stream1.ItemList = ['xAOD::EventInfo#*', 'xAOD::EventAuxInfo#*']
Stream1.MetadataItemList += ['xAOD::CutBookkeeperContainer#CutBookkeepers*', 'xAOD::CutBookkeeperAuxContainer#CutBookkeepers*Aux.']
Stream1.MetadataItemList += ['xAOD::CutBookkeeperContainer#IncompleteCutBookkeepers*', 'xAOD::CutBookkeeperAuxContainer#IncompleteCutBookkeepers*Aux.']

# set debug logging
ServiceMgr.MessageSvc.defaultLimit = 9999999
ServiceMgr.MetaDataSvc.OutputLevel = DEBUG
ServiceMgr.CutFlowSvc.OutputLevel = DEBUG
ToolSvc.BookkeeperTool.OutputLevel = DEBUG

# run on 100 events
theApp.EvtMax = 100
