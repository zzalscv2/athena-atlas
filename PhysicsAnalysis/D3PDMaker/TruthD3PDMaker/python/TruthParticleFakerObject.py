# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## @file TruthD3PDMaker/python/TruthParticleFakerObject.py
## @brief Truth D3PD object for single particles
## @author Zach Marshall <zach.marshall@cern.ch>
## @date Nov, 2010
##

import TruthD3PDMaker

import D3PDMakerCoreComps
from D3PDMakerCoreComps.D3PDObject import D3PDObject
from D3PDMakerConfig.D3PDMakerFlags  import D3PDMakerFlags

from TruthD3PDAnalysis.AllTruthFilterTool import AllTruthFilterTool

def make_TruthParticleFaker_D3PDObject( default_prefix, default_sgkey,
                                 default_object_name = "",
                                 default_filter = AllTruthFilterTool(),
                                 default_label = None, **other_defaults ):

    def make_obj( name, prefix, object_name,
                  getter = None, sgkey = None, filter = default_filter,
                  label = default_label, **kw ):

        if sgkey is None: sgkey = default_sgkey
        if label is None: label = prefix
        if getter is None:
            getter = TruthD3PDMaker.GenParticleGetterTool (name + "_Getter",
                                                           Label = label,
                                                           SGKey = sgkey,
                                                           Selector = filter )

        defs = other_defaults.copy()
        defs.update( kw )

        from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
        return D3PDMakerCoreComps.VectorFillerTool( name,
                                                    Prefix = prefix,
                                                    Getter = getter,
                                                    ObjectName = object_name,
                                                    SaveMetadata = \
                                                    D3PDMakerFlags.SaveObjectMetadata(),
                                                    **defs )

    return D3PDObject( make_obj, default_prefix, default_object_name )

TruthParticleFakerD3PDObject = make_TruthParticleFaker_D3PDObject( 'tpf_' ,
                                                     D3PDMakerFlags.TruthParticlesSGKey(),
                                                     "TruthParticleFakerD3PDObject",
                                                     AllTruthFilterTool(),
                                                     'TruthParticleFaker_' )

TruthParticleFakerElD3PDObject = make_TruthParticleFaker_D3PDObject( 'tpfel_' ,
                                                     D3PDMakerFlags.TruthParticlesSGKey(),
                                                     "TruthParticleFakerElD3PDObject",
                                                     AllTruthFilterTool(),
                                                     'tpfel_' )

TruthParticleFakerElD3PDObject.defineBlock( 0, 'TruthParticleFaker',
                                         TruthD3PDMaker.TruthParticleFakerTool , PDG_ID=11 , 
                                         WriteCharge=True , WritePn=True , WriteE=True , WriteEt=True )

TruthParticleFakerMuD3PDObject = make_TruthParticleFaker_D3PDObject( 'tpfmu_' ,
                                                     D3PDMakerFlags.TruthParticlesSGKey(),
                                                     "TruthParticleFakerMuD3PDObject",
                                                     AllTruthFilterTool(),
                                                     'tpfmu_' )

TruthParticleFakerMuD3PDObject.defineBlock( 0, 'TruthParticleFaker',
                                         TruthD3PDMaker.TruthParticleFakerTool , PDG_ID=13 , 
                                         WriteCharge=True , WritePn=True , WriteE=True , WriteEt=False )

TruthParticleFakerPhD3PDObject = make_TruthParticleFaker_D3PDObject( 'tpfph_' ,
                                                     D3PDMakerFlags.TruthParticlesSGKey(),
                                                     "TruthParticleFakerPhD3PDObject",
                                                     AllTruthFilterTool(),
                                                     'tpfph_' )

TruthParticleFakerPhD3PDObject.defineBlock( 0, 'TruthParticleFaker',
                                         TruthD3PDMaker.TruthParticleFakerTool , PDG_ID=22 , 
                                         WriteCharge=False  , WritePn=True , WriteE=True , WriteEt=True )

TruthParticleFakerTauD3PDObject = make_TruthParticleFaker_D3PDObject( 'tpftau_' ,
                                                     D3PDMakerFlags.TruthParticlesSGKey(),
                                                     "TruthParticleFakerTauD3PDObject",
                                                     AllTruthFilterTool(),
                                                     'tpftau_' )

TruthParticleFakerTauD3PDObject.defineBlock( 0, 'TruthParticleFaker',
                                         TruthD3PDMaker.TruthParticleFakerTool , PDG_ID=15 , 
                                         WriteCharge=True  , WritePn=False , WriteE=False , WriteEt=True )

