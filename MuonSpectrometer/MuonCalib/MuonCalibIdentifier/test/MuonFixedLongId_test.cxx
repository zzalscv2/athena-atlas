#undef NDEBUG
#include <cassert>

#include "MuonCalibIdentifier/MuonFixedLongId.h"

void test_mdt() {
  for (std::string_view stationName: { "BIL", "EIL", "BEE", "BMG" }) {
    for (int stationEta: { -8, -1, +3 }) {
      for (int stationPhi: { 1, 5, 8 }) {
        for (int multiLayer: { 1, 2 }) {
          for (int tubeLayer: { 1, 4 }) {
            for (int tube: { 1, 70 }) {
              MuonCalib::MuonFixedLongId fid;
              assert(fid.setTechnology(MuonCalib::MuonFixedLongId::technologyMDT));
              assert(fid.setStationName(fid.stationStringToFixedStationNumber(stationName)));
              assert(fid.setStationEta(stationEta));
              assert(fid.setStationPhi(stationPhi));
              assert(fid.setMdtMultilayer(multiLayer));
              assert(fid.setMdtTubeLayer(tubeLayer));
              assert(fid.setMdtTube(tube));

              assert(MuonCalib::MuonFixedLongId::technologyMDT == fid.technology());
              assert(stationName == fid.stationNameString());
              assert(stationEta == fid.eta());
              assert(stationPhi == fid.phi());
              assert(multiLayer == fid.mdtMultilayer());
              assert(tubeLayer == fid.mdtTubeLayer());
              assert(tube == fid.mdtTube());
            }
          }
        }
      }
    }
  }
}

void test_rpc() {
  for (std::string_view stationName: { "BML", "BMF", "BOF", "BOL" }) {
    for (int stationEta: { -8, -1, +3 }) {
      for (int stationPhi: { 1, 5, 8 }) {
        for (int rpcDoubletR: { 1, 2 }) {
          for (int rpcDoubletZ: { 1, 4 }) {
            for (int rpcDoubletPhi: { 1, 2 }) {
              for (int rpcGasGap: { 1, 2 }) {
                for (int rpcMeasuresPhi: { 0, 1 }) {
                  for (int rpcStrip: { 1, 70, 128 }) {
                    MuonCalib::MuonFixedLongId fid;
                    assert(fid.setTechnology(MuonCalib::MuonFixedLongId::technologyRPC));
                    assert(fid.setStationName(fid.stationStringToFixedStationNumber(stationName)));
                    assert(fid.setStationEta(stationEta));
                    assert(fid.setStationPhi(stationPhi));
                    assert(fid.setRpcDoubletR(rpcDoubletR));
                    assert(fid.setRpcDoubletZ(rpcDoubletZ));
                    assert(fid.setRpcDoubletPhi(rpcDoubletPhi));
                    assert(fid.setRpcGasGap(rpcGasGap));
                    assert(fid.setRpcMeasuresPhi(rpcMeasuresPhi));
                    assert(fid.setRpcStrip(rpcStrip));

                    assert(MuonCalib::MuonFixedLongId::technologyRPC == fid.technology());
                    assert(stationName == fid.stationNameString());
                    assert(stationEta == fid.eta());
                    assert(stationPhi == fid.phi());
                    assert(rpcDoubletR == fid.rpcDoubletR());
                    assert(rpcDoubletZ == fid.rpcDoubletZ());
                    assert(rpcDoubletPhi == fid.rpcDoubletPhi());
                    assert(rpcGasGap == fid.rpcGasGap());
                    assert(rpcMeasuresPhi == fid.rpcMeasuresPhi());
                    assert(rpcStrip == fid.rpcStrip());
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void test_csc() {
  for (std::string_view stationName: { "CSL", "CSS" }) {
    for (int stationEta: { -1, +1 }) {
      for (int stationPhi: { 1, 5, 8 }) {
        for (int cscChamberLayer: { 1, 2 }) {
          for (int cscWireLayer: { 1, 4 }) {
            for (int cscMeasuresPhi: { 0, 1 }) {
              for (int cscStrip: { 1, 256 }) {
                MuonCalib::MuonFixedLongId fid;
                assert(fid.setTechnology(MuonCalib::MuonFixedLongId::technologyCSC));
                assert(fid.setStationName(fid.stationStringToFixedStationNumber(stationName)));
                assert(fid.setStationEta(stationEta));
                assert(fid.setStationPhi(stationPhi));
                assert(fid.setCscChamberLayer(cscChamberLayer));
                assert(fid.setCscWireLayer(cscWireLayer));
                assert(fid.setCscMeasuresPhi(cscMeasuresPhi));
                assert(fid.setCscStrip(cscStrip));

                assert(MuonCalib::MuonFixedLongId::technologyCSC == fid.technology());
                assert(stationName == fid.stationNameString());
                assert(stationEta == fid.eta());
                assert(stationPhi == fid.phi());
                assert(cscChamberLayer == fid.cscChamberLayer());
                assert(cscWireLayer == fid.cscWireLayer());
                assert(cscMeasuresPhi == fid.cscMeasuresPhi());
                assert(cscStrip == fid.cscStrip());
              }
            }
          }
        }
      }
    }
  }
}

void test_tgc() {
  for (std::string_view stationName: { "T1F", "T2E", "T4F" }) {
    for (int stationEta: { -6, +1 }) {
      for (int stationPhi: { 1, 5, 64 }) {
        for (int tgcGasGap: { 1, 4 }) {
          for (int tgcIsStrip: { 0, 1 }) {
            for (int tgcChannel: {1, 256 }) {
              MuonCalib::MuonFixedLongId fid;
              assert(fid.setTechnology(MuonCalib::MuonFixedLongId::technologyTGC));
              assert(fid.setStationName(fid.stationStringToFixedStationNumber(stationName)));
              assert(fid.setStationEta(stationEta));
              assert(fid.setStationPhi(stationPhi));
              assert(fid.setTgcGasGap(tgcGasGap));
              assert(fid.setTgcIsStrip(tgcIsStrip));
              assert(fid.setTgcChannel(tgcChannel));

              assert(MuonCalib::MuonFixedLongId::technologyTGC == fid.technology());
              assert(stationName == fid.stationNameString());
              assert(stationEta == fid.eta());
              assert(stationPhi == fid.phi());
              assert(tgcGasGap == fid.tgcGasGap());
              assert(tgcIsStrip == fid.tgcIsStrip());
              assert(tgcChannel == fid.tgcChannel());
            }
          }
        }
      }
    }
  }
}

void test_mmg() {
  for (std::string_view stationName: { "MMS", "MML" }) {
    for (int stationEta: { -2, +1 }) {
      for (int stationPhi: { 1, 5, 8 }) {
        for (int mmgMultilayer: { 1, 2 }) {
          for (int mmgGasGap: { 1, 4 }) {
            for (int mmgStrip: { 0, 3071, 5119 }) {
              MuonCalib::MuonFixedLongId fid;
              assert(fid.setTechnology(MuonCalib::MuonFixedLongId::technologyMMG));
              assert(fid.setStationName(fid.stationStringToFixedStationNumber(stationName)));
              assert(fid.setStationEta(stationEta));
              assert(fid.setStationPhi(stationPhi));
              assert(fid.setMmgMultilayer(mmgMultilayer));
              assert(fid.setMmgGasGap(mmgGasGap));
              assert(fid.setMmgStrip(mmgStrip));

              assert(MuonCalib::MuonFixedLongId::technologyMMG == fid.technology());
              assert(stationName == fid.stationNameString());
              assert(stationEta == fid.eta());
              assert(stationPhi == fid.phi());
              assert(mmgMultilayer == fid.mmgMultilayer());
              assert(mmgGasGap == fid.mmgGasGap());
              assert(mmgStrip == fid.mmgStrip());
            }
          }
        }
      }
    }
  }
}

void test_stg() {
  for (std::string_view stationName: { "STS", "STL" }) {
    for (int stationEta: { -3, +2 }) {
      for (int stationPhi: { 1, 5, 8 }) {
        for (int stgMultilayer: { 1, 2 }) {
          for (int stgGasGap: { 1, 4 }) {
            for (MuonCalib::MuonFixedLongId::StgChannelType stgChannelType: {
                MuonCalib::MuonFixedLongId::StgChannelType::stgChannelPad,
                MuonCalib::MuonFixedLongId::StgChannelType::stgChannelStrip,
                MuonCalib::MuonFixedLongId::StgChannelType::stgChannelWire
                }) {
              for (int stgChannel: { 1, 2048 }) {
                MuonCalib::MuonFixedLongId fid;
                assert(fid.setTechnology(MuonCalib::MuonFixedLongId::technologySTG));
                assert(fid.setStationName(fid.stationStringToFixedStationNumber(stationName)));
                assert(fid.setStationEta(stationEta));
                assert(fid.setStationPhi(stationPhi));
                assert(fid.setStgMultilayer(stgMultilayer));
                assert(fid.setStgGasGap(stgGasGap));
                assert(fid.setStgChannelType(stgChannelType));
                assert(fid.setStgChannel(stgChannel));

                assert(MuonCalib::MuonFixedLongId::technologySTG == fid.technology());
                assert(stationName == fid.stationNameString());
                assert(stationEta == fid.eta());
                assert(stationPhi == fid.phi());
                assert(stgMultilayer == fid.stgMultilayer());
                assert(stgGasGap == fid.stgGasGap());
                assert(stgChannelType == fid.stgChannelType());
                assert(stgChannel == fid.stgChannel());
              }
            }
          }
        }
      }
    }
  }
}

int main() {
  test_mdt();
  test_rpc();
  test_csc();
  test_tgc();
  test_mmg();
  test_stg();
}

