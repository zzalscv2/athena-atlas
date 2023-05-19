#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.Enums import LHCPeriod

def getHIClusterGeoWeightFile(flags):
    """Returns the correct geo.cluster.XXX.root file based on the Run2/Run3 and data/MC"""

    # TODO: There should be geo.cluster.XXX.root files for each geometry version (flags.GeoModel.AtlasVersion)
    #       that would contain all the relevant runs. Now, we hope that if we have input file from Run2, it's
    #       from 2018. If the input file is from other years, this will not work and the name has to be set manually.
    #       See also ATLHI-489

    if flags.HeavyIon.Jet.HIClusterGeoWeightFile != "auto":
        return flags.HeavyIon.Jet.HIClusterGeoWeightFile

    if flags.Input.isMC:
        return 'cluster.geo.HIJING_2018.root'
    else:
        if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2]:
            return 'cluster.geo.DATA_PbPb_2018v2.root'
        else:
            return 'cluster.geo.DATA_PbPb_2022.root'
