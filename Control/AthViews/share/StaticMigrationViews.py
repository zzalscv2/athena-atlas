###############################################################
#
# Job options file
#
# Based on AthExStoreGateExamples
#
#==============================================================

#--------------------------------------------------------------
# ATLAS default Application Configuration options
#--------------------------------------------------------------

from GaudiHive.GaudiHiveConf import ForwardSchedulerSvc
svcMgr += ForwardSchedulerSvc()
svcMgr.ForwardSchedulerSvc.CheckDependencies = True

# Full job is a list of algorithms
from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

manualViewName1 = "view1"
manualViewName2 = "view2"

#Make views
job += CfgMgr.AthViews__ViewMakeAlg("make_alg")
job.make_alg.ViewNames = [ manualViewName1, manualViewName2 ]
job.make_alg.ExtraOutputs = [ ( 'int', manualViewName1 + '_view_start' ), ( 'int', manualViewName2 + '_view_start' ) ]

#Make one view
job += CfgMgr.AthViews__DFlowAlg1("dflow_alg1")
job.dflow_alg1.ViewName = manualViewName1
#
job += CfgMgr.AthViews__DFlowAlg2("dflow_alg2")
job.dflow_alg2.ViewName = manualViewName1
#
job += CfgMgr.AthViews__DFlowAlg3("dflow_alg3")
job.dflow_alg3.ViewName = manualViewName1

#Make another view
job += CfgMgr.AthViews__DFlowAlg1("dflow_alg4")
job.dflow_alg4.ViewName = manualViewName2
#
job += CfgMgr.AthViews__DFlowAlg2("dflow_alg5")
job.dflow_alg5.ViewName = manualViewName2
#
job += CfgMgr.AthViews__DFlowAlg3("dflow_alg6")
job.dflow_alg6.ViewName = manualViewName2

#Merge views
job += CfgMgr.AthViews__ViewMergeAlg("merge_alg")
job.merge_alg.ExtraInputs = [ ( 'int', manualViewName1 + '_dflow_dummy' ), ( 'int', manualViewName2 + '_dflow_dummy' ) ]

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
theApp.EvtMax = 10

#==============================================================
#
# End of job options file
#
###############################################################

