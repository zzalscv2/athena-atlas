# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def OutputStreamSequencerSvcCfg(flags, incidentName=''):
    result = ComponentAccumulator()
    service = CompFactory.OutputStreamSequencerSvc("OutputStreamSequencerSvc", SequenceIncidentName=incidentName)
    result.addService(service)
    return result
    

