#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# Author: Christos Anastopoulos
# Date: Jan, 2023
# Brief: Test for CaloDetDescrManager
#

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaPython.PyAthenaComps import Alg, StatusCode
import ROOT


class CaloDetDescrManagerAlg (Alg):
    def execute(self):
        ctx = self.getContext()
        mgr = self.condStore['CaloDetDescrManager'].find(ctx.eventID())
        phi = 0.0
        for sampl in [ROOT.CaloSampling.PreSamplerB,
                      ROOT.CaloSampling.EMB1,
                      ROOT.CaloSampling.EMB2,
                      ROOT.CaloSampling.EMB3,
                      ROOT.CaloSampling.PreSamplerE,
                      ROOT.CaloSampling.EME1,
                      ROOT.CaloSampling.EME2,
                      ROOT.CaloSampling.EME3,
                      ROOT.CaloSampling.HEC0,
                      ROOT.CaloSampling.HEC1,
                      ROOT.CaloSampling.HEC2,
                      ROOT.CaloSampling.HEC3,
                      ROOT.CaloSampling.TileBar0,
                      ROOT.CaloSampling.TileBar1,
                      ROOT.CaloSampling.TileBar2,
                      ROOT.CaloSampling.TileGap1,
                      ROOT.CaloSampling.TileGap2,
                      ROOT.CaloSampling.TileGap3,
                      ROOT.CaloSampling.TileExt0,
                      ROOT.CaloSampling.TileExt1,
                      ROOT.CaloSampling.TileExt2,
                      ROOT.CaloSampling.FCAL0,
                      ROOT.CaloSampling.FCAL1,
                      ROOT.CaloSampling.FCAL2,
                      ]:
            print("Sampling:{}".format(sampl))
            for i in range(21):
                eta = 1e-2 + 0.2*i
                dd = mgr.get_element(sampl, eta, phi)
                if (dd):
                    print("----: eta:{:.2f}, phi:{:.2f}".
                          format(eta, phi))
                    print("CaloDD: r_raw:{:.2f}, r:{:.2f}, dr:{:.2f}".
                          format(dd.r_raw(), dd.r(), dd.dr()))
                    print("CaloDD: z_raw:{:.2f}, z:{:.2f}, dz:{:.2f}".
                          format(dd.z_raw(), dd.z(), dd.dz()))
            print()

        return StatusCode.Success


def testCfg(configFlags):
    result = ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(LArGMCfg(configFlags))
    result.merge(TileGMCfg(configFlags))
    result.addEventAlgo(CaloDetDescrManagerAlg('CaloDetDescrManagerAlg'))

    return result


ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
ConfigFlags.Input.TimeStamp = 1000
ConfigFlags.Detector.GeometryLAr = True
ConfigFlags.Detector.GeometryTile = True
ConfigFlags.needFlagsCategory('Tile')
ConfigFlags.needFlagsCategory('LAr')

ConfigFlags.lock()
acc = MainServicesCfg(ConfigFlags)

acc.merge(McEventSelectorCfg(ConfigFlags))

acc.merge(testCfg(ConfigFlags))
acc.run(1)
