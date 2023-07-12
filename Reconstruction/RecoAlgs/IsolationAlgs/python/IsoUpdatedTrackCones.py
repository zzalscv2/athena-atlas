# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging

log = logging.getLogger(__name__)


def GetUpdatedIsoTrackCones(postfix="", object_types=("Electrons", "Photons", "Muons"),WP="Nonprompt_All_MaxWeight",TTVATool=None):
    """ Return a list of IsolationBuilder algorithms to calculate TTVA moments

    ---------
    Arguments
    ---------
    postfix: Optionally provide a postfix to add to the names of the isolation
             builder algorithm instances
    object_types: The object types to which to add the moments
    """

    import ROOT
    # This is a doubly nested list
    ptcone_list = [
        [ROOT.xAOD.Iso.IsolationType.ptcone40, ROOT.xAOD.Iso.IsolationType.ptcone30, ROOT.xAOD.Iso.IsolationType.ptcone20]
    ]
    trkcor_list = [[ROOT.xAOD.Iso.IsolationTrackCorrection.coreTrackPtr]]

    do_egamma = any(x in object_types for x in ("Electrons", "Photons"))

    algs = []
    for track_pt in (500, 1000):
        for loose_cone in (True, False):
            if loose_cone and not do_egamma:
                # Loose cone isolation variables only for electrons and photons
                continue
            cone_str = "LooseCone" if loose_cone else ""
            name = f"{WP}TTVA{cone_str}_pt{track_pt}"
            # Build up extra IsolationBuilder kwargs
            kwargs = {}
            if "Electrons" in object_types:
                kwargs["ElIsoTypes"] = ptcone_list
                kwargs["ElCorTypes"] = trkcor_list
                kwargs["ElCorTypesExtra"] = [[]]
                kwargs["CustomConfigurationNameEl"] = name
            if "Electrons" in object_types and "LRT" in postfix:
                kwargs["ElectronCollectionContainerName"] = "LRTElectrons"
            if "Photons" in object_types:
                kwargs["PhIsoTypes"] = ptcone_list
                kwargs["PhCorTypes"] = trkcor_list
                kwargs["PhCorTypesExtra"] = [[]]
                kwargs["CustomConfigurationNamePh"] = name
            if "Muons" in object_types and not loose_cone:
                kwargs["MuIsoTypes"] = ptcone_list
                kwargs["MuCorTypes"] = trkcor_list
                kwargs["MuCorTypesExtra"] = [[]]
                kwargs["CustomConfigurationNameMu"] = name
            if "Muons" in object_types and not loose_cone and "LRT" in postfix:
                kwargs["MuonCollectionContainerName"] = "MuonsLRT"
            toolkwargs = {}
            from AthenaCommon.BeamFlags import jobproperties
            if jobproperties.Beam.beamType == 'cosmics':
                toolkwargs['VertexLocation'] = ''
            from AthenaCommon import CfgMgr
            if TTVATool is None:
                if WP != "MVATight":
                    TTVATool = CfgMgr.CP__TrackVertexAssociationTool(WP,
                                WorkingPoint=WP)
                else:
                    TTVATool = CfgMgr.CP__MVATrackVertexAssociationTool(WP,
                                WorkingPoint="Tight")
              
            algs.append(CfgMgr.IsolationBuilder(
                            f"IsolationBuilder{WP}{cone_str}{track_pt}{postfix}",
                            TrackIsolationTool=CfgMgr.xAOD__TrackIsolationTool(
                                f"TrackIsolationTool{WP}{track_pt}",
                                TrackSelectionTool=CfgMgr.InDet__InDetTrackSelectionTool(
                                    minPt=track_pt, CutLevel="Loose"
                                ),
                                TTVATool=TTVATool,
                                CoreTrackEtaRange=0.01 if loose_cone else 0.0,
                                **toolkwargs,
                            ),
                            **kwargs,
                            )
                        )
    return algs

def iso_vars():
    # Get the list of isolation variables calculated by these functions
    iso_vars = []
    for track_pt in 500, 1000:
        for cone_str in "", "LooseCone":
            name = f"Nonprompt_All_MaxWeightTTVA{cone_str}_pt{track_pt}"
            iso_vars += ["ptconeCorrBitset_"+name, "ptconecoreTrackPtrCorrection_"+name]
            for cone_size in 20, 30, 40:
                for var_str in "", "var":
                    iso_vars.append(f"pt{var_str}cone{cone_size}_{name}")
    return iso_vars
