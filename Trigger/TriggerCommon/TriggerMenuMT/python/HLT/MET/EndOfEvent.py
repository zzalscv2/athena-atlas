# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""Code to setup MET reconstruction during the end-of-event sequence"""

from typing import List

from AthenaCommon.CFElements import parOR
from AthenaCommon.Configurable import ConfigurableCABehavior
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from .HLTInputConfig import HLTInputConfigRegistry
from ..CommonSequences.FullScanDefs import caloFSRoI, trkFSRoI
from ..Config.MenuComponents import extractAlgorithmsAndAppendCA
from ..Jet.JetMenuSequencesConfig import getCaloInputMaker, getTrackingInputMaker

_algs_by_purpose = {
    "metcalo": ["cell", "tcpufit", "tcpufit_sig30"],
    "mettrk": [
        "pfopufit",
        "pfsum",
        "pfsum_vssk",
        "pfsum_cssk",
        "trkmht",
        "mhtpufit_pf",
        "mhtpufit_em",
        "pfopufit_sig30",
        "nn",
    ],
}


def getMETRecoSequences(flags, purposes: List[str]):
    """Create the full reconstruction sequences to run at the end of the event

    Parameters
    ----------
    purposes : List[str]
        The purpose keys given in the associated acceptedevts chain

    Returns
    -------
    An OR containing the configured algorithms
    A list of the required RoIs
    A list of the required streams
    """
    ef_reco_algs = []
    for purpose in purposes:
        ef_reco_algs += [
            alg for alg in _algs_by_purpose.get(purpose, []) if alg not in ef_reco_algs
        ]
    seqname = f"{''.join(purposes)}EndOfEventRecoSequence"
    with ConfigurableCABehavior():
        registry = HLTInputConfigRegistry()
        step_cas, _ = registry.build_steps(flags, ef_reco_algs, return_ca=True)
        merged_ca = ComponentAccumulator()
        merged_ca.addSequence(parOR(seqname),primary=True)
        if 'metcalo' in purposes:
            merged_ca.addEventAlgo(getCaloInputMaker(), seqname)
        if 'mettrk' in purposes:
            merged_ca.addEventAlgo(getTrackingInputMaker('ftf'), seqname)
        for ca in step_cas:
            merged_ca.merge(ca,seqname)

    rois = [caloFSRoI]
    if len(step_cas) > 2:
        # If we have more than two steps then we also need the FS tracking RoI
        rois.append(trkFSRoI)
    algorithms = extractAlgorithmsAndAppendCA(merged_ca)
    
    streams = ["Main", "VBFDelayed"]
    return algorithms[0], rois, streams
