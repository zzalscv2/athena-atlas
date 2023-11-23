# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
import json

def FoldDecoratorCfg(flags, jetCollection='AntiKt4EMPFlowJets', prefix=''):
    ca = ComponentAccumulator()
    evt_id = 'mcEventNumber' if flags.Input.isMC else 'eventNumber'
    common = dict(
        eventID=f'EventInfo.{evt_id}',
        salt=42,
        jetCollection=jetCollection,
        associations=['constituentLinks', 'GhostTrack'],
        jetVariableSaltSeeds={
            'constituentLinks': 0,
            'GhostTrack': 1,
        },
    )
    ca.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.FoldDecoratorAlg(
            f'{prefix}FoldHashWithHits{jetCollection}',
            **common,
            constituentChars=json.dumps({
                'GhostTrack': [
                    'numberOfPixelHits',
                    'numberOfSCTHits',
                ]
            }),
            constituentSaltSeeds={
                'numberOfPixelHits': 2,
                'numberOfSCTHits': 3,
            },
            jetFoldHash=f'{prefix}jetFoldHash',
        )
    )

    ca.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.FoldDecoratorAlg(
            f'{prefix}FoldHashWithoutHits{jetCollection}',
            **common,
            jetFoldHash=f'{prefix}jetFoldHash_noHits',
        )
    )

    return ca
