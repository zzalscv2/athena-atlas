/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloUtils/CaloTopoTowerBuilderToolBase.h"

CaloTopoTowerBuilderToolBase::CaloTopoTowerBuilderToolBase(const std::string& name
							   , const std::string& type
							   , const IInterface* parent)
  : AthAlgTool(name,type,parent)
{
  declareInterface<ICaloTopoTowerBuilderToolBase>(this);
}

CaloTopoTowerBuilderToolBase::~CaloTopoTowerBuilderToolBase()
= default;

StatusCode CaloTopoTowerBuilderToolBase::initialize()
{
  return this->initializeTool();
}

void CaloTopoTowerBuilderToolBase::setTowerSeg(const CaloTowerSeg& theTowerSeg)
{
  ATH_MSG_DEBUG(" in CaloTopoTowerBuilderToolBase::setTowerSeg ");
  m_theTowerSeg = theTowerSeg;
  ATH_MSG_DEBUG("   neta,nphi,etamin,etamax " << theTowerSeg.neta() << " " << theTowerSeg.nphi() 
		<< " " << theTowerSeg.etamin() << " " << theTowerSeg.etamax());
}
