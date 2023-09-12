# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from AthenaCommon.CFElements import seqAND, parOR
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def bmumuxRecoSequenceCfg(flags, rois, muons):

    acc = ComponentAccumulator()

    recoSequence = seqAND('bmumuxViewNode')
    acc.addSequence(recoSequence)

    dataObjects = [('TrigRoiDescriptorCollection', 'StoreGateSvc+%s' % rois),
                   ('xAOD::MuonContainer', 'StoreGateSvc+%s' % muons)]
    acc.addEventAlgo(CompFactory.AthViews.ViewDataVerifier('VDV_bmumux', DataObjects=dataObjects), sequenceName=recoSequence.name)

    from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg, trigInDetPrecisionTrackingCfg
    acc.merge(trigInDetFastTrackingCfg(flags, rois, signatureName='bmumux'), sequenceName=recoSequence.name)

    precisionTrackingSequence = parOR('precisionTrackingInBmumux')
    acc.addSequence(precisionTrackingSequence, parentName=recoSequence.name)
    acc.merge(trigInDetPrecisionTrackingCfg(flags, rois, signatureName='bmumux'), sequenceName=precisionTrackingSequence.name)

    return acc
