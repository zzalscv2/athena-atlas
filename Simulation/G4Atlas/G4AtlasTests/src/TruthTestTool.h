/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTESTS_TRUTHTESTTOOL_H
#define G4ATLASTESTS_TRUTHTESTTOOL_H

#include "SimTestToolBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"


class TruthTestTool : public SimTestToolBase {


public:

  TruthTestTool(const std::string& type, const std::string& name, const IInterface* parent);

  virtual StatusCode initialize() override;
  virtual StatusCode processEvent() override;

private:
  /// SG key for Event Info
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfo", "EventInfo", "EventInfo name"};

  // histograms
  TH1 *m_n_vert, *m_n_part;
  TH1 *m_n_vert_prim, *m_n_vert_sec;
  TH1 *m_n_part_prim, *m_n_part_sec;
  TH1 *m_x_vert, *m_y_vert, *m_z_vert;
  TH1 *m_n_generations;
  TH1 *m_vtx_r, *m_vtx_z;
  TH2 *m_vtx_zr_indet, *m_vtx_xy_indet;
  TH1 *m_meanx_vert, *m_meany_vert, *m_meanz_vert;
  TH1 *m_px_truth, *m_py_truth, *m_pz_truth, *m_pt_truth, *m_log_pt_truth;
  TH1 *m_theta_truth, *m_eta_truth, *m_phi_truth;
  TH1 *m_origin;
  TH1 *m_particle_status, *m_particle_type;
  TH1 *m_p_gen,*m_log_p_gen, *m_eta_gen,*m_phi_gen,*m_pion_mass;

  int m_mcEventDump;
};

#endif
