// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CaloSysD3PDMaker/src/CellFillerTool.h
 * @author Francesco Lanni 
 * @date  2012
 * @brief Block filler tool for making hits in SuperCell, and fill ntuple.
 */

#ifndef CALOSYSD3PDMAKER_LARSCHITFILLERTOOL_H
#define CALOSYSD3PDMAKER_LARSCHITFILLERTOOL_H

// Gaudi/Athena include(s):
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "D3PDMakerUtils/BlockFillerTool.h"
#include "D3PDMakerUtils/SGCollectionGetterTool.h"
#include "LArSimEvent/LArHitContainer.h"
#include "LArElecCalib/ILArfSampl.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include <vector>
#include <list>
#include <map>

// Forward declaration(s):
class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;
class TileID;
class LArOnlineID;
class Identifier;

class CaloCell_SuperCell_ID; 

#include "LArSimEvent/LArHit.h"

namespace D3PD {


/**
 * @brief Block filler tool for EM samplings from a CaloCluster.
 */
class LArSCHitFillerTool
  : public BlockFillerTool<LArHitContainer>
{
public:
  /**
   * @brief Standard Gaudi tool constructor.
   * @param type The name of the tool type.
   * @param name The tool name.
   * @param parent The tool's Gaudi parent.
   */
  LArSCHitFillerTool (const std::string& type,
		const std::string& name,
		const IInterface* parent);


  /// Book variables for this block.
  virtual StatusCode initialize();
  virtual StatusCode book();
  //  virtual StatusCode  handle(const Incident&); 
  virtual StatusCode fill (const LArHitContainer& p);

private:
 
  const LArEM_ID   *m_emid;
  const LArFCAL_ID *m_fcalid;
  const LArHEC_ID  *m_hecid;
  const TileID     *m_tileid;    
  const LArOnlineID* m_onlineHelper;
  const ILArfSampl*   m_dd_fSampl;

  /// parameters
  int *m_nSC;
  std::vector<double> *m_E;
  std::vector<double> *m_eta;
  std::vector<double> *m_phi;
  std::vector<double> *m_Eoff;
  std::vector<double> *m_Et;
  std::vector<double> *m_fsampl;

  std::vector<int> *m_calo;
  std::vector<int> *m_region;
  std::vector<int> *m_sampling;
  std::vector<int> *m_ieta;
  std::vector<int> *m_jphi;
  std::vector<unsigned int> *m_offlId;

  std::vector<float> m_etaCut;
  std::vector<float> m_phiCut;
  std::vector< unsigned int > m_caloNums; 
  std::vector< unsigned int >  m_caloLayers;

  bool m_caloEtaSelection;
  bool m_caloPhiSelection;
  bool m_caloLayerSelection;
  bool m_caloSelection;


  /// Property: Offline / supercell mapping tool.
  ToolHandle<ICaloSuperCellIDTool>     m_scidtool;
  
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{
    this,"CaloSuperCellDetDescrManager","CaloSuperCellDetDescrManager","SG key of the resulting CaloSuperCellDetDescrManager" };

  /// idHlper 
  const CaloCell_SuperCell_ID* m_sc_idHelper ; 

};


} // namespace D3PD


#endif // not CALOSYSD3PDMAKER_LARSCHITFILLERTOOL_H
