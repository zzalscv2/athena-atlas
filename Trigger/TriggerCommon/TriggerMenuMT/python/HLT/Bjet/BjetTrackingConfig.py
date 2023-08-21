# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def secondStageBjetTrackingCfg(flags, inputRoI: str, inputVertex: str, inputJets: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg
    acc.merge(trigInDetFastTrackingCfg(flags, inputRoI, signatureName="bjet"))

    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    acc.merge(trigInDetPrecisionTrackingCfg(flags, inputRoI, signatureName="bjet"))

    verifier = CompFactory.AthViews.ViewDataVerifier(name = 'VDVsecondStageBjetTracking',
                                                     DataObjects = [('xAOD::VertexContainer', f'StoreGateSvc+{inputVertex}'),
                                                                    ('xAOD::JetContainer', f'StoreGateSvc+{inputJets}')] )
    acc.addEventAlgo(verifier)

    return acc
