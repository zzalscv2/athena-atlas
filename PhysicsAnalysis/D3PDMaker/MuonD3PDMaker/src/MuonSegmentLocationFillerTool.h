/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file MuonD3PDMaker/src/MuonSegmentLocationFillerTool.h
 * @author srivas prasad <srivas.prasad@cern.ch>
 * @date Feb 2010
 * @brief Position/direction/chamber-location for muon segments, detail level 1
 */

#ifndef D3PDMAKER_MUONSEGMENTLOCATIONFILLERTOOL_H
#define D3PDMAKER_MUONSEGMENTLOCATIONFILLERTOOL_H 1

#include "D3PDMakerUtils/BlockFillerTool.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"

namespace Trk { 
  class Segment;
}

namespace D3PD {

class MuonSegmentLocationFillerTool
  : public BlockFillerTool<Trk::Segment>
{
public:
  MuonSegmentLocationFillerTool (const std::string& type,
                        const std::string& name,
                        const IInterface* parent);

  /// Standard Gaudi initialize method.
  StatusCode initialize();

  virtual StatusCode book();

  virtual StatusCode fill (const Trk::Segment& p);

private:
  // global position
  float *m_x = nullptr;
  float *m_y = nullptr;
  float *m_z = nullptr;

  // global direction
  float *m_phi = nullptr;
  float *m_theta = nullptr;

  // local position
  float *m_locX = nullptr;
  float *m_locY = nullptr;

  // local direction - conventions for Moore and Muonboy are different!
  float *m_thetaXZ = nullptr; // local x-z angle
  float *m_thetaYZ = nullptr; // local y-z angle
  float *m_thetaXZ_IP = nullptr; // local x-z angle pointing to the IP
  float *m_thetaYZ_IP = nullptr; // local y-z angle pointing to the IP

  // chamber summary
  int *m_sector = nullptr;  // phi sector - 1 to 16
  int *m_stationEta = nullptr; // station eta
  bool *m_isEndcap = nullptr; // 1 for endcap, 0 for barrel
  int *m_stationName = nullptr; // Station name in MuonFixedId scheme

  // hit counts
  int* m_nphiHits = nullptr;
  int* m_netaHits = nullptr;
  int* m_netaTrigHits = nullptr;
  int* m_npadHits = nullptr;
  int* m_npseudoHits = nullptr;

  std::vector<int>*   m_id = nullptr; // fixed id
  std::vector<int>*   m_type = nullptr;  // 1000*channelType (0:pad,1:eta,2:phi) + 0:MDT,1:RPC,2:TGC,3:CSC,4:STGC,5::MM,6::Pseudo)
  std::vector<float>* m_error = nullptr; // error
  std::vector<float>* m_residual = nullptr; // residual
  std::vector<float>* m_biasedPull = nullptr; // biased pull
  std::vector<float>* m_unbiasedPull = nullptr; // unbiased pull

  // tools
  ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc {this, "edmHelper", 
    "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc", 
    "Handle to the service providing the IMuonEDMHelperSvc interface" };
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  ToolHandle<MuonCalib::IIdToFixedIdTool> m_idToFixedIdTool;
  ToolHandle<Trk::IPropagator>  m_slPropagator;
  ToolHandle<Trk::IResidualPullCalculator>  m_pullCalculator;
  Trk::MagneticFieldProperties* m_magFieldProperties = nullptr;
};

} // namespace D3PD

#endif
