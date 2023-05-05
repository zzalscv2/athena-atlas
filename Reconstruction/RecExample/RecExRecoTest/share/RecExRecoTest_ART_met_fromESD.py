from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]

from AthenaCommon.GlobalFlags import globalflags
globalflags.ConditionsTag.set_Value_and_Lock("OFLCOND-MC21-SDR-RUN3-10")

# Work around cling bugs, triggered by the use of TauAnalysisTools
# in METMakerConfig.
import ROOT
ROOT.gROOT.ProcessLine ('#include "xAODTracking/TrackParticleContainer.h"')
ROOT.gROOT.ProcessLine ('#include "xAODJet/JetContainer.h"')
ROOT.gROOT.ProcessLine ('#include "xAODCaloEvent/CaloClusterContainer.h"')

from RecExConfig.RecFlags import rec
rec.doEgamma.set_Value_and_Lock(False)
rec.doMuon.set_Value_and_Lock(True)

from CaloRec.CaloRecFlags import jobproperties
jobproperties.CaloRecFlags.Enabled.set_Value_and_Lock(False)
jobproperties.CaloRecFlags.doCaloCluster.set_Value_and_Lock(False)
jobproperties.CaloRecFlags.doCaloTopoCluster.set_Value_and_Lock(False)

#this turns off CaloCluster2xAOD
rec.doWritexAOD.set_Value_and_Lock(False)
#nothing to say on these
rec.doWriteTAG.set_Value_and_Lock(False)
rec.doTruth.set_Value_and_Lock(False)
rec.doAODCaloCells.set_Value_and_Lock(False)
rec.doTrigger.set_Value_and_Lock(False)
#Turns off xAODRingSetConfWriter
rec.doCaloRinger.set_Value_and_Lock(False)

#disables VertexCnvAlg
from InDetRecExample.InDetJobProperties import jobproperties
jobproperties.InDetJobProperties.doxAOD.set_Value_and_Lock(False)
#Disables AllExecutedEvents
rec.doFileMetaData.set_Value_and_Lock(False)

athenaCommonFlags.EvtMax=10

UserAlgs=["RecExRecoTest/RecExRecoTests_setupReadID.py"]
UserAlgs+=["RecExRecoTest/RecExRecoTest_setupJetEtMissPFlow.py"]
UserAlgs+=["METReconstruction/METReconstruction_jobOptions.py"]
include ("RecExCommon/RecExCommon_topOptions.py")

import AthenaCommon.PropertiesManip as manip

manip.appendItemsToList(StreamAOD,'ItemList','xAOD::PFOContainer_v1#CHSParticleFlowObjects')
manip.appendItemsToList(StreamAOD,'ItemList','xAOD::PFOAuxContainer_v1#CHSParticleFlowObjectsAux.')


