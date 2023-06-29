/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------
// File and Version Information:
//
// Description: see CaloTopoSplitterHashCluster.h
// 
// Environment:
//      Software developed for the ATLAS Detector at CERN LHC
//
// Author List:
//      Sven Menke
//
//-----------------------------------------------------------------------

//-----------------------
// This Class's Header --
//-----------------------
#include "CaloTopoSplitterHashCluster.h"
#include "CaloTopoSplitterClusterCell.h"

    
//-----------
// Methods --
//-----------

void CaloTopoSplitterHashCluster::add (HashCell& hashCell)
{
  if ( !m_parentCluster ) {
    const CaloTopoSplitterClusterCell* pCell = hashCell.getCaloTopoTmpClusterCell();
    m_parentCluster = pCell->getParentCluster();
    m_parentClusterIndex = pCell->getParentClusterIndex();
  }
  
  Base::add(hashCell);
  m_hasValidEnergy = false;
  m_centroid.reset();
}

void CaloTopoSplitterHashCluster::remove (const HashCell& hashCell)
{
  const CaloTopoSplitterClusterCell* cell = hashCell.getCaloTopoTmpClusterCell();
  {
    pointer_list::iterator iter =
      std::find (m_members.begin(), m_members.end(), cell);
    if (iter != m_members.end())
      m_members.erase (iter);
  }

  float ratio = cell->getSignedRatio(); 
  if ( ratio >= m_maxRatio ) {
    for(iterator iter = begin(); iter!= end(); ++iter)
    {
      float myRatio = iter->getSignedRatio();
      if ( iter == m_members.begin() || myRatio > m_maxRatio ) 
        m_maxRatio = myRatio;
    }
  }
  m_hasValidEnergy = false;
  m_centroid.reset();
}

void CaloTopoSplitterHashCluster::add
  (CaloTopoSplitterHashCluster& rClus)
{
  if ( !m_parentCluster) {
    m_parentCluster = rClus.m_parentCluster;
    m_parentClusterIndex = rClus.m_parentClusterIndex;
  }

  Base::add(rClus);
  m_hasValidEnergy = false;
  m_centroid.reset();
}

float CaloTopoSplitterHashCluster::getEnergy()
{
  if ( !m_hasValidEnergy) 
    this->calcEnergy();
  return m_energy;
}

const HepGeom::Vector3D<double> & CaloTopoSplitterHashCluster::getCentroid()
{
  if ( !m_centroid) 
    this->calcCentroid();
  return m_centroid.value();
}

void CaloTopoSplitterHashCluster::calcEnergy()
{
  if ( ! m_members.empty() ) {
    m_energy = 0;
    for( iterator iter=begin(); iter!= end(); ++iter)
    {
      CaloTopoSplitterClusterCell *pClusCell = *iter;
      xAOD::CaloCluster::cell_iterator itrCell = pClusCell->getCellIterator();
      float myWeight = itrCell.weight();//pClusCell->getParentCluster()->getCellWeight(itrCell);
      if ( pClusCell->getShared() ) {
	if ( pClusCell->getCaloTopoTmpHashCluster() == this ) 
	  myWeight *= pClusCell->getSharedWeight();
	else  
	  myWeight *= (1.-pClusCell->getSharedWeight());
      }	
      m_energy += myWeight*itrCell->e();
    }
  }
  m_hasValidEnergy = true;
}

void CaloTopoSplitterHashCluster::calcCentroid()
{
  m_centroid.emplace(0,0,0);  

  if ( !m_members.empty() ) {
    double thisAbsEng,absEng = 0;
    
    for( iterator iter = begin(); iter != end(); ++iter)
    {
      CaloTopoSplitterClusterCell *pClusCell = *iter;
      xAOD::CaloCluster::cell_iterator itrCell = pClusCell->getCellIterator();
      float myWeight = itrCell.weight();//pClusCell->getParentCluster()->getCellWeight(itrCell);
      if ( pClusCell->getShared() ) {
	if ( pClusCell->getCaloTopoTmpHashCluster() == this ) 
	  myWeight *= pClusCell->getSharedWeight();
	else  
	  myWeight *= (1.-pClusCell->getSharedWeight());
      }	

      thisAbsEng = fabs(myWeight*itrCell->e());
      absEng += thisAbsEng;
      HepGeom::Vector3D<double> thisPos(itrCell->x(), itrCell->y(), itrCell->z());
      m_centroid.value() += thisAbsEng*thisPos;
    }
    if ( absEng > 0 ) 
      m_centroid.value() *= (1./absEng);
  }
}

