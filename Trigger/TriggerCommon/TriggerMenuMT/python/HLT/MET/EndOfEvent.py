# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""Code to setup MET reconstruction during the end-of-event sequence"""

from typing import List

from AthenaCommon.CFElements import parOR
from .HLTInputConfig import HLTInputConfigRegistry
from ..CommonSequences.FullScanDefs import caloFSRoI, trkFSRoI

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


def getMETRecoSequences(purposes: List[str]):
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
    registry = HLTInputConfigRegistry()
    steps, _ = registry.build_steps(ef_reco_algs)
    rois = [caloFSRoI]
    if len(steps) > 2:
        # If we have more than two steps then we also need the FS tracking RoI
        rois.append(trkFSRoI)
    algorithms = [alg for step in steps for alg in step]
    streams = ["Main", "VBFDelayed"]
    return parOR(f"{''.join(purposes)}EndOfEventRecoSequence", algorithms), rois, streams
