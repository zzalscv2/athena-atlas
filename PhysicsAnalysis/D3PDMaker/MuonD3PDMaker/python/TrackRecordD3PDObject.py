# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Import all needed modules:
from D3PDMakerCoreComps.D3PDObject   import make_SGDataVector_D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


# Create the configurable:
TrackRecordD3PDObject = \
           make_SGDataVector_D3PDObject ('TrackRecordCollection',
                                         'MuonEntryLayerFilter',
                                         'ms_entry_truth_',
                                         'TrackRecordD3PDObject',
                                         default_getterClass = D3PD.TrackRecordCollectionGetterTool)

# Add blocks to it:
TrackRecordD3PDObject.defineBlock( 0, "BasicInfo",
                                   D3PD.TrackRecordFillerTool )
TrackRecordD3PDObject.defineBlock( 0, "TruthHits",
                                   D3PD.MuonTruthHitsFillerTool )
