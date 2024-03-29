/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DIGITIZATIONTESTS_MCEVENTCOLLECTIONTESTTOOL
#define DIGITIZATIONTESTS_MCEVENTCOLLECTIONTESTTOOL

#include "DigiTestToolBase.h"

class PixelID;

class McEventCollectionTestTool : public DigiTestToolBase {

public:

  McEventCollectionTestTool( const std::string& type,
                             const std::string& name,
                             const IInterface* parent );

  StatusCode initialize();

  StatusCode processEvent();

  StatusCode finalize();

 private:
  StringProperty m_collection;
  // histograms
  TH1 *m_nGenEvents;
  TH1 *m_nEmptyGenEvents;
  TH1 *m_sig_n_vert, *m_sig_n_part;
  TH1 *m_bkg_n_vert, *m_bkg_n_part;
 };

#endif
