# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Import all needed modules:
from D3PDMakerCoreComps.D3PDObject  import make_SGDataVector_D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


# Create the configurable:
MDTSimHitD3PDObject = \
           make_SGDataVector_D3PDObject ('AtlasHitsVector<MDTSimHit>',
                                         'MDT_Hits',
                                         'mdt_hit_', 'MDTSimHitD3PDObject')

# Add blocks to it:
MDTSimHitD3PDObject.defineBlock( 0, "BasicInfo",
                                 D3PD.MDTSimHitFillerTool )
