# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def EventInfoBeamSpotDecoratorAlgCfg(flags, name="EventInfoBeamSpotDecoratorAlg", eventInfoKey="EventInfo", **kwargs):
    result = ComponentAccumulator()

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result.merge(BeamSpotCondAlgCfg(flags))

    kwargs.setdefault("beamPosXKey", f"{eventInfoKey}.beamPosX")
    kwargs.setdefault("beamPosYKey", f"{eventInfoKey}.beamPosY")
    kwargs.setdefault("beamPosZKey", f"{eventInfoKey}.beamPosZ")
    kwargs.setdefault("beamPosSigmaXKey", f"{eventInfoKey}.beamPosSigmaX")
    kwargs.setdefault("beamPosSigmaYKey", f"{eventInfoKey}.beamPosSigmaY")
    kwargs.setdefault("beamPosSigmaZKey", f"{eventInfoKey}.beamPosSigmaZ")
    kwargs.setdefault("beamPosSigmaXYKey", f"{eventInfoKey}.beamPosSigmaXY")
    kwargs.setdefault("beamTiltXZKey", f"{eventInfoKey}.beamTiltXZ")
    kwargs.setdefault("beamTiltYZKey", f"{eventInfoKey}.beamTiltYZ")
    kwargs.setdefault("beamStatusKey", f"{eventInfoKey}.beamStatus")

    result.addEventAlgo(CompFactory.xAODMaker.EventInfoBeamSpotDecoratorAlg(name, **kwargs))

    return result
