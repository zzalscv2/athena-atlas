# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

"""
Tools configurations for ISF_FastCaloSimServices
KG Tan, 04/12/2012
"""

from AthenaCommon.CfgGetter import getService
from AthenaCommon import CfgMgr
from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags
from ISF_Algorithms.collection_merger_helpers import generate_mergeable_collection_name


def getAdditionalParticleParametrizationFileNames():
    return [
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_10000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_10000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_10000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_10000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_10000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_100000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_100000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_100000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_100000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_100000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_1000000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_2000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_2000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_2000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_2000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_2000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_20000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_20000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_20000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_20000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_20000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_200000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_5000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_5000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_5000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_5000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_5000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_50000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_50000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_50000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_50000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_50000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_500000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_500000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_500000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_500000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_500000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_400/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_400/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_400/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_400/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_400/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_600/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_600/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_600/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_600/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_600/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_800/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_800/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_800/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_800/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_211/EN_800/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_10000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_10000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_10000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_10000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_10000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_100000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_1000000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_2000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_2000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_2000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_2000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_2000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_20000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_20000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_20000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_20000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_20000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_200000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_5000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_5000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_5000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_5000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_5000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_50000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_500000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_500000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_500000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_500000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_500000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_400/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_400/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_400/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_400/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_400/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_600/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_600/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_600/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_600/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_600/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_800/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_800/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_800/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_800/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_22/EN_800/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_10000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_10000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_10000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_10000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_10000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_100000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_1000000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_2000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_2000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_2000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_2000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_2000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_20000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_20000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_20000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_20000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_20000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_200000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_5000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_5000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_5000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_5000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_5000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_50000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_500000/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_500000/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_500000/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_500000/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_500000/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_400/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_400/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_400/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_400/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_400/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_600/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_600/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_600/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_600/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_600/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_800/eta_central",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_800/eta_crack",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_800/eta_endcap1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_800/eta_endcap2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:EnergyResults/pdgid_11/EN_800/eta_fcal",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_5000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_2000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_10000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_1000000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_500000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_20000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_100000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_200000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_22/EN_50000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_5000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_2000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_10000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_1000000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_500000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_20000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_100000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_200000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_11/EN_50000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_5000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_2000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_10000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_1000000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_500000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_20000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_100000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_4",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_200000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_14",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_12",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_8",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_13",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_20",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_9",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_11",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_10",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_5",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_6",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_1",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_7",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_2",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_19",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_0",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_3",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_18",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_15",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_16",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_17",
      "DB=/GLOBAL/AtlfastII/FastCaloSimParam:2:ShapeResults/pdgid_211/EN_50000/calosample_4",
      ]
# definitions taken from file:
#   FastSimulationJobTransforms/jobConfig.FastCaloSim_DB.py
def setAdditionalParticleParametrizationFileNames( FastShowerCellBuilderTool ):
    FastShowerCellBuilderTool.AdditionalParticleParametrizationFileNames = getAdditionalParticleParametrizationFileNames()

def getPunchThroughTool(name="ISF_PunchThroughTool", **kwargs):
    kwargs.setdefault("FilenameLookupTable"     , ISF_FastCaloSimFlags.PunchThroughParamsInputFilename())
    kwargs.setdefault("FilenameInverseCdf"      , ISF_FastCaloSimFlags.PunchThroughParamsInverseCdfFilename())
    kwargs.setdefault("FilenameInversePca"      , ISF_FastCaloSimFlags.PunchThroughParamsInversePcaFilename())
    kwargs.setdefault("EnergyFactor"            , [ 0.98,  0.831, 0.896, 0.652, 0.717, 1., 0.877, 0.858, 0.919 ]    )
    kwargs.setdefault("DoAntiParticles"         , [ 0,   1,    0,     1,     1,     0,   0,    0,    0 ]    )
    kwargs.setdefault("PunchThroughInitiators"  , [ 211, 321, 311, 310, 130, 2212, 2112]        )
    kwargs.setdefault("InitiatorsMinEnergy"     , [ 65536, 65536, 65536, 65536, 65536, 65536, 65536]                                         )
    kwargs.setdefault("InitiatorsEtaRange"      , [ -3.2,   3.2 ]                               )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("PunchThroughParticles"   , [ 2212,   211,    22,     11,     13,     2112,   321,    310,    130 ]    )
    kwargs.setdefault("CorrelatedParticle"      , []    )
    kwargs.setdefault("FullCorrelationEnergy"   , [ 100000., 100000., 100000., 100000.,      0., 100000., 100000., 100000., 100000.]    )
    kwargs.setdefault("MinEnergy"               , [   938.3,   135.6,     50.,     50.,   105.7,   939.6, 493.7,   497.6,   497.6 ]    )
    kwargs.setdefault("MaxNumParticles"         , [      -1,      -1,      -1,      -1,      -1,    -1,     -1,     -1,     -1 ]    )
    kwargs.setdefault("EnvelopeDefSvc"          , getService('AtlasGeometry_EnvelopeDefSvc')         )
    kwargs.setdefault("BeamPipeRadius"          , 500.                                               )

    from ISF_PunchThroughTools.ISF_PunchThroughToolsConf import ISF__PunchThroughTool
    return ISF__PunchThroughTool(name, **kwargs )

def getPunchThroughClassifier(name="ISF_PunchThroughClassifier", **kwargs):
    kwargs.setdefault("ScalerConfigFileName"     , ISF_FastCaloSimFlags.PunchThroughClassifierScalerFilename() )
    kwargs.setdefault("NetworkConfigFileName"     , ISF_FastCaloSimFlags.PunchThroughClassifierNetworkFilename() )
    kwargs.setdefault("CalibratorConfigFileName"    , ISF_FastCaloSimFlags.PunchThroughClassifierCalibratorFilename())
    from ISF_PunchThroughTools.ISF_PunchThroughToolsConf import ISF__PunchThroughClassifier
    return ISF__PunchThroughClassifier(name, **kwargs )

def getEmptyCellBuilderTool(name="ISF_EmptyCellBuilderTool", **kwargs):
    from FastCaloSim.FastCaloSimConf import EmptyCellBuilderTool
    return EmptyCellBuilderTool(name, **kwargs )

def getNIMatEffUpdator(name="ISF_NIMatEffUpdator", **kwargs):
    from TrkExTools.TrkExToolsConf import Trk__NIMatEffUpdator as NIMatEffUpdator
    return NIMatEffUpdator(name, **kwargs )

def getNIPropagator(name="ISF_NIPropagator", **kwargs):
    kwargs.setdefault("MaterialEffects" , False )

    from TrkExSTEP_Propagator.TrkExSTEP_PropagatorConf import Trk__STEP_Propagator as STEP
    return STEP(name, **kwargs )


def getNINavigator(name="ISF_NINavigator", **kwargs):
    from TrkExTools.TimedExtrapolator import getNINavigator as navigatorConfig
    return navigatorConfig(name, **kwargs)


def getNITimedExtrapolator(name="ISF_NITimedExtrapolator", **kwargs):
    kwargs.setdefault("MaterialEffectsUpdators", ['ISF_NIMatEffUpdator'])
    kwargs.setdefault("ApplyMaterialEffects", False)
    kwargs.setdefault("STEP_Propagator", 'ISF_NIPropagator')
    kwargs.setdefault("Navigator", 'ISF_NINavigator')

    from TrkExTools.TrkExToolsConf import Trk__TimedExtrapolator as TimedExtrapolator
    return TimedExtrapolator(name, **kwargs)


def getTimedExtrapolator(name="TimedExtrapolator", **kwargs):
    kwargs.setdefault("MaterialEffectsUpdators", ['ISF_NIMatEffUpdator'])
    kwargs.setdefault("ApplyMaterialEffects", False)
    kwargs.setdefault("STEP_Propagator", 'ISF_NIPropagator')
    kwargs.setdefault("Navigator", 'ISF_NINavigator')

    from TrkExTools.TrkExToolsConf import Trk__TimedExtrapolator as TimedExtrapolator
    return TimedExtrapolator(name, **kwargs)
## FastShowerCellBuilderTool

def getDefaultFastShowerCellBuilderTool(name, **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags
    kwargs.setdefault("RandomService", simFlags.RandomSvcMT() )
    kwargs.setdefault("RandomStreamName", ISF_FastCaloSimFlags.RandomStreamName() )
    kwargs.setdefault("DoSimulWithInnerDetectorTruthOnly", True )
    kwargs.setdefault("ID_cut_off_r",                      [1150] )
    kwargs.setdefault("ID_cut_off_z",                      [ 3490 ] )
    kwargs.setdefault("DoSimulWithInnerDetectorV14TruthCuts", True )
    kwargs.setdefault("DoNewEnergyEtaSelection",              True )
    kwargs.setdefault("DoEnergyInterpolation",                True )
    kwargs.setdefault("use_Ekin_for_depositions",             True )
    kwargs.setdefault("McLocation",                           ISF_FastCaloSimFlags.FastShowerInputCollection() )
    kwargs.setdefault("ParticleParametrizationFileName",      '')
    kwargs.setdefault("Extrapolator",                         'ISF_NITimedExtrapolator' )
    from FastCaloSim.FastCaloSimFactory import FastCaloSimFactory
    return FastCaloSimFactory(name, **kwargs)

def getFastShowerCellBuilderTool(name="ISF_FastShowerCellBuilderTool", **kwargs):
    localFileNameList = getAdditionalParticleParametrizationFileNames()
    localFileNameList.insert(0,"L1_L2_egamma_corr.config20.root")
    kwargs.setdefault('AdditionalParticleParametrizationFileNames', localFileNameList)
    from IOVDbSvc.CondDB import conddb
    conddb.addFolder('GLOBAL_OFL','/GLOBAL/AtlfastII/FastCaloSimParam <tag>FastCaloSim_v2</tag>')
    return getDefaultFastShowerCellBuilderTool(name, **kwargs)

def getLegacyFastShowerCellBuilderTool(name="ISF_LegacyFastShowerCellBuilderTool", **kwargs):
    FastShowerCellBuilderTool = getFastShowerCellBuilderTool(name,**kwargs)
    FastShowerCellBuilderTool.Invisibles += [ 13 ]
    return FastShowerCellBuilderTool

def getPileupFastShowerCellBuilderTool(name="ISF_PileupFastShowerCellBuilderTool", **kwargs):
    #
    # weights from:
    # http://acode-browser.usatlas.bnl.gov/lxr/source/atlas/Simulation/FastShower/FastCaloSim/FastCaloSim/FastCaloSim_CaloCell_ID.h
    weightsfcs = [
      ### LAr presampler
      #FirstSample=CaloCell_ID::PreSamplerB,
      2.0,
      ### LAr barrel
      #PreSamplerB=CaloCell_ID::PreSamplerB,
      #EMB1=CaloCell_ID::EMB1,
      #EMB2=CaloCell_ID::EMB2,
      #EMB3=CaloCell_ID::EMB3,
      2.0, 2.0, 2.0, 2.0,
      ### LAr EM endcap
      #PreSamplerE=CaloCell_ID::PreSamplerE,
      #EME1=CaloCell_ID::EME1,
      #EME2=CaloCell_ID::EME2,
      #EME3=CaloCell_ID::EME3,
      2.0, 2.0, 2.0, 2.0,
      ### Hadronic end cap cal.
      #HEC0=CaloCell_ID::HEC0,
      #HEC1=CaloCell_ID::HEC1,
      #HEC2=CaloCell_ID::HEC2,
      #HEC3=CaloCell_ID::HEC3,
      2.0, 2.0, 2.0, 2.0,
      ### Tile barrel
      #TileBar0=CaloCell_ID::TileBar0,
      #TileBar1=CaloCell_ID::TileBar1,
      #TileBar2=CaloCell_ID::TileBar2,
      1.0, 1.0, 1.0,
      ### Tile gap (ITC & scint)
      #TileGap1=CaloCell_ID::TileGap1,
      #TileGap2=CaloCell_ID::TileGap2,
      #TileGap3=CaloCell_ID::TileGap3,
      1.0, 1.0, 1.0,
      ### Tile extended barrel
      #TileExt0=CaloCell_ID::TileExt0,
      #TileExt1=CaloCell_ID::TileExt1,
      #TileExt2=CaloCell_ID::TileExt2,
      1.0, 1.0, 1.0,
      ### Forward EM endcap
      #FCAL0=CaloCell_ID::FCAL0,
      #FCAL1=CaloCell_ID::FCAL1,
      #FCAL2=CaloCell_ID::FCAL2,
      1.0, 1.0, 1.0,
      ### Beware of MiniFCAL! We don't have it, so different numbers after FCAL2
      #LastSample = CaloCell_ID::FCAL2,
      #MaxSample = LastSample+1
      1.0, 1.0,
    ]

    kwargs.setdefault("sampling_energy_reweighting", weightsfcs )
    return getFastShowerCellBuilderTool(name, **kwargs)

def getFastHitConvertTool(name="ISF_FastHitConvertTool", **kwargs):
    # Suffix which the FastCaloSim HIT collection will receive
    collectionSuffix = '_FastCaloSim'
    region = 'CALO'

    caloRegionList = ['EMB', 'EMEC', 'FCAL', 'HEC']
    for caloRegion in caloRegionList:
        bareCollectionName = f'LArHit{caloRegion}'
        inputProperty = f'LAr{caloRegion}Hits'
        # Generates a mergeable collection name for different CALO regions
        hitContainerName = generate_mergeable_collection_name(bareCollectionName, collectionSuffix, inputProperty, region)
        kwargs.setdefault(f'{caloRegion.lower()}HitContainername', hitContainerName)

    # For tile
    hitContainerName = generate_mergeable_collection_name('TileHitVec', collectionSuffix, 'TileHits', region)
    kwargs.setdefault('tileHitContainername', hitContainerName)

    from FastCaloSimHit.FastCaloSimHitConf import FastHitConvertTool
    return FastHitConvertTool(name,**kwargs)


def getAddNoiseCellBuilderTool(name="ISF_AddNoiseCellBuilderTool", **kwargs):
    from FastCaloSim.AddNoiseCellBuilderToolDefault import AddNoiseCellBuilderToolDefault
    return AddNoiseCellBuilderToolDefault(name, **kwargs )

def getCaloCellContainerFinalizerTool(name="ISF_CaloCellContainerFinalizerTool", **kwargs):
    from CaloRec.CaloRecConf import CaloCellContainerFinalizerTool
    return CaloCellContainerFinalizerTool(name, **kwargs )

def getCaloCellContainerFCSFinalizerTool(name="ISF_CaloCellContainerFCSFinalizerTool", **kwargs):
    from FastCaloSim.FastCaloSimConf import CaloCellContainerFCSFinalizerTool
    return CaloCellContainerFCSFinalizerTool(name, **kwargs )

def getFastHitConvAlg(name="ISF_FastHitConvAlg", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags
    kwargs.setdefault("CaloCellsInputName"  , ISF_FastCaloSimFlags.CaloCellsName() )
    # TODO: do we need this?
    #from AthenaCommon.DetFlags import DetFlags
    #if DetFlags.pileup.LAr_on() or DetFlags.pileup.Tile_on():
    #  kwargs.setdefault("doPileup", True)
    #else:
    #  kwargs.setdefault("doPileup", False)
    from FastCaloSimHit.FastCaloSimHitConf import FastHitConv
    return FastHitConv(name, **kwargs )

def getFastCaloToolBase(name="ISF_FastCaloToolBase", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags
    kwargs.setdefault("BatchProcessMcTruth"              , False                                             )
    kwargs.setdefault("SimulateUndefinedBarcodeParticles", False                                             )
    kwargs.setdefault("CaloCellsOutputName"              , ISF_FastCaloSimFlags.CaloCellsName()              )

    kwargs.setdefault("CaloCellMakerTools_setup"         , [ 'ISF_EmptyCellBuilderTool' ] )
    kwargs.setdefault("CaloCellMakerTools_simulate"      , [ 'ISF_FastShowerCellBuilderTool' ])
    kwargs.setdefault("CaloCellMakerTools_release"       , [ #'ISF_AddNoiseCellBuilderTool',
                                                             'ISF_CaloCellContainerFinalizerTool',
                                                             'ISF_FastHitConvertTool' ])
    kwargs.setdefault("Extrapolator"                     , 'ISF_NITimedExtrapolator')
    # register the FastCaloSim random number streams
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed(ISF_FastCaloSimFlags.RandomStreamName()):
        simFlags.RandomSeedList.addSeed( ISF_FastCaloSimFlags.RandomStreamName(), 98346412, 12461240 )
    kwargs.setdefault("ParticleTruthSvc"                 , simFlags.TruthStrategy.TruthServiceName() )
    return CfgMgr.ISF__FastCaloTool(name, **kwargs)

def getFastCaloTool(name="ISF_FastCaloTool", **kwargs):
    return getFastCaloToolBase(name, **kwargs)

def getFastCaloPileupTool(name="ISF_FastCaloPileupTool", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags
    kwargs.setdefault("CaloCellsOutputName"              , ISF_FastCaloSimFlags.CaloCellsName()+'PileUp'     )
    kwargs.setdefault("CaloCellMakerTools_simulate"      , [ 'ISF_PileupFastShowerCellBuilderTool' ])
    return getFastCaloToolBase(name, **kwargs)

def getLegacyAFIIFastCaloTool(name="ISF_LegacyAFIIFastCaloTool", **kwargs):
    kwargs.setdefault("BatchProcessMcTruth" , True )
    kwargs.setdefault("CaloCellMakerTools_simulate"      , [ 'ISF_LegacyFastShowerCellBuilderTool' ] )
    return getFastCaloToolBase(name, **kwargs)

def getFastCaloSimV2Tool(name="ISF_FastCaloSimV2Tool", **kwargs):
    from ISF_FastCaloSimServices.ISF_FastCaloSimJobProperties import ISF_FastCaloSimFlags

    kwargs.setdefault("CaloCellsOutputName"              , ISF_FastCaloSimFlags.CaloCellsName()   )

    kwargs.setdefault("CaloCellMakerTools_setup"         , [ 'ISF_EmptyCellBuilderTool' ] )
    kwargs.setdefault("CaloCellMakerTools_release"       , [ 'ISF_CaloCellContainerFCSFinalizerTool',
                                                           'ISF_FastHitConvertTool' ])
    kwargs.setdefault("FastCaloSimCaloExtrapolation"     , 'FastCaloSimCaloExtrapolation')

    kwargs.setdefault("ParamSvc", "ISF_FastCaloSimV2ParamSvc")
    # register the FastCaloSim random number streams
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomStream"                     , ISF_FastCaloSimFlags.RandomStreamName())
    kwargs.setdefault("RandomSvc"                        , simFlags.RandomSvcMT())
    kwargs.setdefault("PunchThroughTool"                 , 'ISF_PunchThroughTool')

    kwargs.setdefault("ParticleTruthSvc"                 , simFlags.TruthStrategy.TruthServiceName() )
    return CfgMgr.ISF__FastCaloSimV2Tool(name, **kwargs )
