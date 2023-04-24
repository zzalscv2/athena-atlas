# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
# TRUTH0.py - direct and complete translation of HepMC in EVNT to xAOD truth 
# No additional information is added

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TRUTH0Cfg(ConfigFlags):
    """Main config for TRUTH0"""
    acc = ComponentAccumulator()
    
    # Ensure EventInfoCnvAlg is scheduled
    if "EventInfo#McEventInfo" in ConfigFlags.Input.TypedCollections and "xAOD::EventInfo#EventInfo" not in ConfigFlags.Input.TypedCollections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        acc.merge(EventInfoCnvAlgCfg(ConfigFlags, inputKey="McEventInfo", outputKey="EventInfo", disableBeamSpot=True))
 
    # Decide what kind of input HepMC container we are dealing with
    # and schedule the xAOD converter appropriately
    from xAODTruthCnv.xAODTruthCnvConfig import GEN_EVNT2xAODCfg
    if "McEventCollection#GEN_EVENT" in ConfigFlags.Input.TypedCollections:
        acc.merge(GEN_EVNT2xAODCfg(ConfigFlags,name="GEN_EVNT2xAOD",AODContainerName="GEN_EVENT"))
    elif "McEventCollection#TruthEvent" in ConfigFlags.Input.TypedCollections:
        acc.merge(GEN_EVNT2xAODCfg(name="GEN_EVNT2xAOD",AODContainerName="TruthEvent"))
    else:
        raise RuntimeError("No recognised HepMC truth information found in the input")
 
    # Contents
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TRUTH0SlimmingHelper = SlimmingHelper("TRUTH0SlimmingHelper", NamesAndTypes = ConfigFlags.Input.TypedCollections, ConfigFlags = ConfigFlags)
    TRUTH0SlimmingHelper.AppendToDictionary = {'EventInfo':'xAOD::EventInfo','EventInfoAux':'xAOD::EventAuxInfo',
                                               'TruthEvents':'xAOD::TruthEventContainer','TruthEventsAux':'xAOD::TruthEventAuxContainer',
                                               'TruthVertices':'xAOD::TruthVertexContainer','TruthVerticesAux':'xAOD::TruthVertexAuxContainer',
                                               'TruthParticles':'xAOD::TruthParticleContainer','TruthParticlesAux':'xAOD::TruthParticleAuxContainer'} 

    TRUTH0SlimmingHelper.AllVariables = [ 'EventInfo',
                                          'TruthEvents', 
                                          'TruthVertices',
                                          'TruthParticles']

    # Create output stream 
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import InfileMetaDataCfg
    TRUTH0ItemList = TRUTH0SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(ConfigFlags, "DAOD_TRUTH0", ItemList=TRUTH0ItemList))
    acc.merge(InfileMetaDataCfg(ConfigFlags, "DAOD_TRUTH0"))
 
    return acc
