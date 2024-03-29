#
# For post-include MT configuration, we need to add some explicit data
# dependencies for the AthenaMT scheduler.
#

# Setup the algorithm and output sequences
from AthenaCommon.AlgSequence import AlgSequence
topSeq = AlgSequence()
from AthenaCommon.AppMgr import theApp
StreamHITS = theApp.getOutputStream( "StreamHITS" )

topSeq.G4AtlasAlg.ExtraInputs =  [('McEventCollection','StoreGateSvc+BeamTruthEvent')]
topSeq.G4AtlasAlg.ExtraOutputs = [('SiHitCollection','StoreGateSvc+SCT_Hits')]
StreamHITS.ExtraInputs += topSeq.G4AtlasAlg.ExtraOutputs

# Disable alg filtering - doesn't work in multi-threading
StreamHITS.AcceptAlgs = []

# Override algorithm cloning settings
nThreads = jp.ConcurrencyFlags.NumThreads()
topSeq.BeamEffectsAlg.Cardinality = nThreads
topSeq.G4AtlasAlg.Cardinality = nThreads
