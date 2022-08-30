# Copyright (C) 2002-20222 CERN for the benefit of the ATLAS collaboration

# Access the algorithm sequence.
from AthenaCommon.AlgSequence import AlgSequence
algSeq = AlgSequence()

# Add asg::EventStoreTestAlg to the job.
from AsgExampleTools.AsgExampleToolsConf import asg__EventStoreTestAlg, asg__EventStoreTestTool
algSeq += asg__EventStoreTestAlg( "EventStoreTestAlg",
                                  Tool = asg__EventStoreTestTool( "EventStoreTestTool" ) )

# Make the job run for just 1 "event".
theApp.EvtMax = 1
