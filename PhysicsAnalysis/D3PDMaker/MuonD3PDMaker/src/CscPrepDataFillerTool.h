/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id: CscPrepDataFillerTool.h 535262 2013-01-29 13:54:17Z salekd $
#ifndef D3PDMAKER_CSCPREPDATAFILLERTOOL_H
#define D3PDMAKER_CSCPREPDATAFILLERTOOL_H

#include "MuonPrepRawData/CscPrepData.h"
#include "MuonPrepRawData/CscPrepDataCollection.h"
#include "D3PDMakerUtils/BlockFillerTool.h"
#include "D3PDMakerUtils/SGCollectionGetterTool.h"
#include <string>

namespace D3PD {

/**
 * @brief Block filler tool for the CSC hits
 *
 * @author David Salek <David.Salek@cern.ch>
 *
 * $Revision: 535262 $
 * $Date: 2013-01-29 14:54:17 +0100 (Tue, 29 Jan 2013) $
 */
class CscPrepDataFillerTool
  : public BlockFillerTool<Muon::CscPrepData>
{
public:
  /// Regular Gaudi AlgTool constructor
  CscPrepDataFillerTool (const std::string& type,
                         const std::string& name,
                         const IInterface* parent);

  /// Function booking the ntuple variables
  virtual StatusCode book();

  /// Function filling the ntuple variables for a single object
  virtual StatusCode fill (const Muon::CscPrepData& obj);


private:
 float* m_x = nullptr;
 float* m_y = nullptr;
 float* m_z = nullptr;
 float* m_time = nullptr;
 float* m_locX = nullptr;
 int* m_status = nullptr;
 int* m_timeStatus = nullptr;
 int* m_charge = nullptr;
};

} // namespace D3PD

#endif
