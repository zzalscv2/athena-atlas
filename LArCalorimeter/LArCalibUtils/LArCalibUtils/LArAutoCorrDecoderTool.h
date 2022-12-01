//Dear emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCALIBUTILS_LARAUTOCORRDECODERTOOL_H
#define LARCALIBUTILS_LARAUTOCORRDECODERTOOL_H

#include "LArElecCalib/ILArAutoCorrDecoderTool.h"

#include "LArElecCalib/ILArAutoCorr.h"
#include <Eigen/Dense>

#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"

#include "AthenaBaseComps/AthAlgTool.h"

class LArAutoCorrDecoderTool: 
        public extends<AthAlgTool,ILArAutoCorrDecoderTool>

{
 public:
  
  // constructor
  using base_class::base_class;

  
  // destructor 
  virtual ~LArAutoCorrDecoderTool();
  
  // retrieve methods 
  const Eigen::MatrixXd AutoCorr( const HWIdentifier&  CellID, int gain, unsigned nSamples) const;
  // initialize and finalize methods
  virtual StatusCode initialize();
  virtual StatusCode finalize(){return StatusCode::SUCCESS;}

  static const InterfaceID& interfaceID() { 
    return ILArAutoCorrDecoderTool::interfaceID();
  } 

 private:

  //Properties:
  Gaudi::Property<unsigned> m_decodemode{"DecodeMode",0};
  Gaudi::Property<bool> m_alwaysHighGain{"UseAlwaysHighGain",false};
  Gaudi::Property<bool> m_isSC{"isSC",false};
  Gaudi::Property<std::string> m_keyAutoCorr{"KeyAutoCorr","LArAutoCorr"};

  const Eigen::MatrixXd ACDiagonal( const HWIdentifier&  CellID, int gain, unsigned nSamples) const;
  const Eigen::MatrixXd ACPhysics( const HWIdentifier&  CellID, int gain, unsigned nSamples) const;

  const LArOnlineID_Base*  m_onlineID=nullptr;
};

#endif
